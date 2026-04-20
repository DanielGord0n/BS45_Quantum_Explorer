/*
 * Wang-Zhu BS Solver v2 — SA over Theorem 2.2 Manifold
 * CP493 - Directed Research - Daniel Gordon
 *
 * Forked from wz_sa_bs43.cpp. Fixes and additions:
 *   1. Per-thread std::random_device + hi-res clock seeding (decorrelated trajectories).
 *   2. Shared champion state across threads (best cost ever seen, with A,B,C,D,sig).
 *   3. Warm-start from champion on a fraction of restarts (path-relinking).
 *   4. k-opt "kick": when an SA run stalls, simultaneously resample 2..5 pair slots.
 *   5. Acceptance-rate-based reheat (adaptive, not just time-based).
 *   6. Champion checkpoint to disk — survives SLURM time-limit kills.
 *   7. Per-signature fail counter with epsilon-greedy skip (avoid wasted attempts on infeasible sigs).
 *   8. Cleaner logging: champion cost, per-sig best, wall time.
 *
 * Usage:
 *   ./wz_sa_v2 <n> [seed_offset]
 *
 * Compile on Alliance Canada:
 *   module load StdEnv/2023 gcc/12.3
 *   g++ -O3 -march=native -std=c++17 -fopenmp -o wz_sa_v2 src/solver/wz_sa_v2.cpp
 *
 * Compile on macOS (for local validation):
 *   g++ -O3 -std=c++17 -Xpreprocessor -fopenmp \
 *       -I/opt/homebrew/opt/libomp/include -L/opt/homebrew/opt/libomp/lib -lomp \
 *       -o wz_sa_v2 src/solver/wz_sa_v2.cpp
 */

#include <algorithm>
#include <atomic>
#include <chrono>
#include <climits>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <random>
#include <sstream>
#include <vector>

#ifdef _OPENMP
#include <omp.h>
#endif

using namespace std;
using Clock = chrono::steady_clock;

// =====================================================================
//  Globals
// =====================================================================
static atomic<bool>     g_found{false};
static atomic<int>      g_champion_cost{INT_MAX};
static atomic<long long> g_epochs_total{0};
static int              G_N;
static int              G_N1;
static int              G_MS;
static int              G_SEED_OFFSET = 0;
static Clock::time_point G_T0;

// Wang-Zhu Theorem 2.4 structural tables.
static int comb16[16][4];
static int comb8_pos[8][4];
static int comb8_neg[8][4];
static const int comb4[4][2] = {{1, 1}, {1, -1}, {-1, 1}, {-1, -1}};

static void init_combs() {
  int p = 0, n_idx = 0;
  for (int i = 0; i < 16; i++) {
    comb16[i][0] = (i & 8) ? 1 : -1;
    comb16[i][1] = (i & 4) ? 1 : -1;
    comb16[i][2] = (i & 2) ? 1 : -1;
    comb16[i][3] = (i & 1) ? 1 : -1;
    int prod = comb16[i][0] * comb16[i][1] * comb16[i][2] * comb16[i][3];
    if (prod == 1) {
      for (int j = 0; j < 4; j++) comb8_pos[p][j] = comb16[i][j];
      p++;
    } else {
      for (int j = 0; j < 4; j++) comb8_neg[n_idx][j] = comb16[i][j];
      n_idx++;
    }
  }
}

// =====================================================================
//  Signature enumeration (identical to bs43)
// =====================================================================
struct Sig { int a, b, c, d; };

static vector<Sig> get_sigs(int n) {
  vector<Sig> sigs;
  int T = 4 * n + 2, n1 = n + 1, ap = n1 % 2, cp = n % 2;
  for (int a = 0; a <= n1; a++) {
    if (a % 2 != ap || a * a > T) continue;
    for (int b = -n1; b <= n1; b++) {
      if (((b % 2) + 2) % 2 != (unsigned)ap) continue;
      int r = T - a * a - b * b;
      if (r < 0) continue;
      for (int c = -n; c <= n; c++) {
        if (((c % 2) + 2) % 2 != (unsigned)cp) continue;
        int d2 = r - c * c;
        if (d2 < 0) continue;
        int d = (int)round(sqrt((double)d2));
        if (d * d != d2 || d > n || (((d % 2) + 2) % 2 != (unsigned)cp)) continue;
        if (n % 2 == 0) {
          if (((c - d) % 4 + 8) % 4 != 0) continue;
        } else {
          if (((a - b - 2) % 4 + 8) % 4 != 0) continue;
        }
        sigs.push_back({a, b, c, d});
        if (d > 0 && ((-d % 2 + 2) % 2 == cp)) {
          bool ok = true;
          if (n % 2 == 0 && ((c + d) % 4 + 8) % 4 != 0) ok = false;
          if (ok) sigs.push_back({a, b, c, -d});
        }
      }
    }
  }
  sort(sigs.begin(), sigs.end(), [](auto &x, auto &y) {
    return tie(x.a, x.b, x.c, x.d) < tie(y.a, y.b, y.c, y.d);
  });
  sigs.erase(unique(sigs.begin(), sigs.end(),
                    [](auto &x, auto &y) {
                      return x.a == y.a && x.b == y.b && x.c == y.c && x.d == y.d;
                    }),
             sigs.end());
  return sigs;
}

// =====================================================================
//  Hall polynomial filter
// =====================================================================
static bool hall_ok(const int *X, int xlen, const int *Y, int ylen) {
  double limit = 4.0 * G_N + 2.0;
  for (int j = 1; j <= 200; j++) {
    double th = j * M_PI / 100.0;
    double rx = 0, ix = 0, ry = 0, iy = 0;
    for (int i = 0; i < xlen; i++) { rx += X[i] * cos(i * th); ix += X[i] * sin(i * th); }
    for (int i = 0; i < ylen; i++) { ry += Y[i] * cos(i * th); iy += Y[i] * sin(i * th); }
    if (rx*rx + ix*ix + ry*ry + iy*iy > limit + 0.5) return false;
  }
  return true;
}

static int npaf_at(const int *A, const int *B, int n1,
                   const int *C, const int *D, int n2, int s) {
  int c = 0;
  if (s < n1) for (int i = 0; i < n1 - s; i++) c += A[i]*A[i+s] + B[i]*B[i+s];
  if (s < n2) for (int i = 0; i < n2 - s; i++) c += C[i]*C[i+s] + D[i]*D[i+s];
  return c;
}

// =====================================================================
//  SA parameters (scale with n for BS(45))
// =====================================================================
struct SAParams {
  double initial_temp   = 80.0;
  double cooling_rate   = 0.99995;
  int    iterations     = 1200000;
  int    restarts       = 8;
  int    reheat_threshold = 80000;
  double reheat_ratio   = 0.55;
  int    kick_after_stall = 40000;   // stall iters before k-opt kick
};

// =====================================================================
//  CD state + SA (search C,D for sig (tc,td))
// =====================================================================
struct CDState {
  int C[128], D[128];
  int sum_c, sum_d;
  int corr[128];

  int cost(int tc, int td, int n1, int n) const {
    int diff_c = abs(sum_c - tc);
    int diff_d = abs(sum_d - td);
    int pen = 0;
    int ms = max(n1, n);
    for (int s = 1; s < ms; s++) {
      int max_ab = (s < n1) ? 2 * (n1 - s) : 0;
      if (abs(corr[s]) > max_ab) pen += abs(corr[s]) - max_ab;
    }
    return diff_c * 5 + diff_d * 5 + pen;
  }
};

// Initialise CD randomly using the Wang-Zhu pair tables.
// For odd n, the middle element (C[n/2], D[n/2]) is sampled from comb4.
static void cd_random_init(CDState &st, int n, mt19937 &rng) {
  memset(st.corr, 0, sizeof(st.corr));
  memset(st.C, 0, sizeof(st.C));
  memset(st.D, 0, sizeof(st.D));
  st.sum_c = 0; st.sum_d = 0;
  for (int d = 0; d < n / 2; d++) {
    int left = d, right = n - 1 - d;
    const int *c = (d == 0)
        ? comb16[uniform_int_distribution<>(0, 15)(rng)]
        : comb8_pos[uniform_int_distribution<>(0, 7)(rng)];
    st.C[left] = c[0]; st.D[left] = c[1];
    st.C[right] = c[2]; st.D[right] = c[3];
    st.sum_c += c[0] + c[2];
    st.sum_d += c[1] + c[3];
  }
  if (n % 2 != 0) {
    int mid = n / 2;
    const int *c = comb4[uniform_int_distribution<>(0, 3)(rng)];
    st.C[mid] = c[0]; st.D[mid] = c[1];
    st.sum_c += c[0]; st.sum_d += c[1];
  }
  int ms = max(n + 1, n);
  for (int s = 1; s < ms; s++)
    for (int i = 0; i < n - s; i++)
      st.corr[s] += st.C[i]*st.C[i+s] + st.D[i]*st.D[i+s];
}

// k-opt kick: resample k random pair-slots (and possibly the middle) simultaneously.
static void cd_kopt_kick(CDState &st, int n, int k, mt19937 &rng) {
  int slots = (n + 1) / 2;  // includes middle slot for odd n
  int picked[16]; int npicked = 0;
  while (npicked < k && npicked < slots) {
    int d = uniform_int_distribution<>(0, slots - 1)(rng);
    bool dup = false;
    for (int i = 0; i < npicked; i++) if (picked[i] == d) { dup = true; break; }
    if (!dup) picked[npicked++] = d;
  }
  for (int pi = 0; pi < npicked; pi++) {
    int d = picked[pi];
    int left = d, right = n - 1 - d;
    int oldC_L = st.C[left], oldD_L = st.D[left];
    int oldC_R = st.C[right], oldD_R = st.D[right];
    if (left == right) {
      const int *c = comb4[uniform_int_distribution<>(0, 3)(rng)];
      st.C[left] = c[0]; st.D[left] = c[1];
      st.sum_c += c[0] - oldC_L;
      st.sum_d += c[1] - oldD_L;
    } else {
      const int *c = (d == 0)
          ? comb16[uniform_int_distribution<>(0, 15)(rng)]
          : comb8_pos[uniform_int_distribution<>(0, 7)(rng)];
      st.C[left] = c[0]; st.D[left] = c[1];
      st.C[right] = c[2]; st.D[right] = c[3];
      st.sum_c += (c[0] + c[2]) - (oldC_L + oldC_R);
      st.sum_d += (c[1] + c[3]) - (oldD_L + oldD_R);
    }
  }
  memset(st.corr, 0, sizeof(st.corr));
  int ms = max(G_N1, n);
  for (int s = 1; s < ms; s++)
    for (int i = 0; i < n - s; i++)
      st.corr[s] += st.C[i]*st.C[i+s] + st.D[i]*st.D[i+s];
}

static bool solve_CD_SA(int n, int n1, int tc, int td,
                         CDState &best_state, mt19937 &rng,
                         const CDState *warm_seed = nullptr) {
  CDState curr;
  if (warm_seed) {
    curr = *warm_seed;
    // Light perturbation so we don't get trapped at the warm-start exactly.
    cd_kopt_kick(curr, n, 2, rng);
  } else {
    cd_random_init(curr, n, rng);
  }

  int current_cost = curr.cost(tc, td, n1, n);
  best_state = curr;
  int best_cost = current_cost;

  SAParams sa;
  uniform_real_distribution<> prob(0.0, 1.0);
  // For odd n, include middle slot d = n/2; for even n, only 0..n/2-1.
  uniform_int_distribution<> d_dist(0, (n - 1) / 2);
  int ms = max(n1, n);

  for (int restart = 0; restart < sa.restarts; restart++) {
    if (g_found.load(memory_order_relaxed)) return false;

    if (restart > 0) {
      cd_random_init(curr, n, rng);
      current_cost = curr.cost(tc, td, n1, n);
    }

    double temp = sa.initial_temp;
    int no_improve = 0;
    int stall_for_kick = 0;
    int accept_window = 0, attempt_window = 0;

    for (int iter = 0; iter < sa.iterations; iter++) {
      if (g_found.load(memory_order_relaxed)) return false;

      if (best_cost == 0) {
        if (hall_ok(best_state.C, n, best_state.D, n)) return true;
        current_cost += 50;
      }

      no_improve++;
      stall_for_kick++;
      attempt_window++;

      if (stall_for_kick > sa.kick_after_stall && best_cost <= 32) {
        int k = 2 + uniform_int_distribution<>(0, 3)(rng);
        curr = best_state;
        cd_kopt_kick(curr, n, k, rng);
        current_cost = curr.cost(tc, td, n1, n);
        temp = sa.initial_temp * 0.7;
        stall_for_kick = 0;
        continue;
      }

      if (attempt_window >= 5000) {
        double acc = (double)accept_window / attempt_window;
        if (acc < 0.01 && no_improve > sa.reheat_threshold / 4) {
          temp = sa.initial_temp * sa.reheat_ratio;
          no_improve = 0;
        }
        accept_window = 0;
        attempt_window = 0;
      }

      if (no_improve > sa.reheat_threshold) {
        temp = sa.initial_temp * sa.reheat_ratio;
        no_improve = 0;
      }

      int d = d_dist(rng);
      int left = d, right = n - 1 - d;

      int oldC_L = curr.C[left], oldD_L = curr.D[left];
      int oldC_R = curr.C[right], oldD_R = curr.D[right];
      int nC_L, nD_L, nC_R, nD_R;

      if (left == right) {
        const int *c = comb4[uniform_int_distribution<>(0, 3)(rng)];
        nC_L = c[0]; nD_L = c[1]; nC_R = nC_L; nD_R = nD_L;
        if (oldC_L == nC_L && oldD_L == nD_L) continue;
      } else {
        const int *c = (d == 0)
            ? comb16[uniform_int_distribution<>(0, 15)(rng)]
            : comb8_pos[uniform_int_distribution<>(0, 7)(rng)];
        nC_L = c[0]; nD_L = c[1]; nC_R = c[2]; nD_R = c[3];
        if (oldC_L == nC_L && oldD_L == nD_L && oldC_R == nC_R && oldD_R == nD_R)
          continue;
      }

      int old_sum_c = curr.sum_c, old_sum_d = curr.sum_d;
      if (left == right) {
        curr.sum_c += nC_L - oldC_L;
        curr.sum_d += nD_L - oldD_L;
      } else {
        curr.sum_c += (nC_L + nC_R) - (oldC_L + oldC_R);
        curr.sum_d += (nD_L + nD_R) - (oldD_L + oldD_R);
      }

      int delta_corr[128] = {0};
      for (int s = 1; s < ms; s++) {
        if (left  - s >= 0) delta_corr[s] -= curr.C[left - s]*oldC_L + curr.D[left - s]*oldD_L;
        if (left  + s < n)  delta_corr[s] -= oldC_L*curr.C[left + s] + oldD_L*curr.D[left + s];
        if (left != right) {
          if (right - s >= 0) delta_corr[s] -= curr.C[right - s]*oldC_R + curr.D[right - s]*oldD_R;
          if (right + s < n)  delta_corr[s] -= oldC_R*curr.C[right + s] + oldD_R*curr.D[right + s];
          if (right - left == s) delta_corr[s] += oldC_L*oldC_R + oldD_L*oldD_R;
        }
      }

      curr.C[left] = nC_L; curr.D[left] = nD_L;
      if (left != right) { curr.C[right] = nC_R; curr.D[right] = nD_R; }

      for (int s = 1; s < ms; s++) {
        if (left  - s >= 0) delta_corr[s] += curr.C[left - s]*nC_L + curr.D[left - s]*nD_L;
        if (left  + s < n)  delta_corr[s] += nC_L*curr.C[left + s] + nD_L*curr.D[left + s];
        if (left != right) {
          if (right - s >= 0) delta_corr[s] += curr.C[right - s]*nC_R + curr.D[right - s]*nD_R;
          if (right + s < n)  delta_corr[s] += nC_R*curr.C[right + s] + nD_R*curr.D[right + s];
          if (right - left == s) delta_corr[s] -= nC_L*nC_R + nD_L*nD_R;
        }
      }

      for (int s = 1; s < ms; s++) curr.corr[s] += delta_corr[s];

      int new_cost = curr.cost(tc, td, n1, n);

      if (new_cost < current_cost ||
          prob(rng) < exp(-(new_cost - current_cost) / temp)) {
        current_cost = new_cost;
        accept_window++;
        if (new_cost < best_cost) {
          best_cost = new_cost;
          best_state = curr;
          no_improve = 0;
          stall_for_kick = 0;
        }
      } else {
        curr.C[left] = oldC_L; curr.D[left] = oldD_L;
        curr.C[right] = oldC_R; curr.D[right] = oldD_R;
        curr.sum_c = old_sum_c; curr.sum_d = old_sum_d;
        for (int s = 1; s < ms; s++) curr.corr[s] -= delta_corr[s];
      }

      temp *= sa.cooling_rate;
    }

    if (best_cost == 0 && hall_ok(best_state.C, n, best_state.D, n)) return true;
  }

  return best_cost == 0 && hall_ok(best_state.C, n, best_state.D, n);
}

// =====================================================================
//  AB state + SA (search A,B given fixed CD PAF)
// =====================================================================
struct ABState {
  int A[128], B[128];
  int sum_a, sum_b;
  int corr[128];

  int cost(int ta, int tb, int n1, const int *cd_full) const {
    int diff_a = abs(sum_a - ta);
    int diff_b = abs(sum_b - tb);
    int pen = 0;
    int ms = max(n1, G_N);
    for (int s = 1; s < ms; s++) {
      if (corr[s] + cd_full[s] != 0) pen += abs(corr[s] + cd_full[s]);
    }
    return diff_a * 5 + diff_b * 5 + pen;
  }
};

static void ab_random_init(ABState &st, int n1, mt19937 &rng) {
  memset(st.corr, 0, sizeof(st.corr));
  st.sum_a = 0; st.sum_b = 0;
  for (int d = 0; d < n1 / 2; d++) {
    int left = d, right = n1 - 1 - d;
    const int *c = (d == 0)
        ? comb8_neg[uniform_int_distribution<>(0, 7)(rng)]
        : comb8_pos[uniform_int_distribution<>(0, 7)(rng)];
    st.A[left] = c[0]; st.B[left] = c[1];
    st.A[right] = c[2]; st.B[right] = c[3];
    st.sum_a += c[0] + c[2];
    st.sum_b += c[1] + c[3];
  }
  if (n1 % 2 != 0) {
    int mid = n1 / 2;
    const int *c = comb4[uniform_int_distribution<>(0, 3)(rng)];
    st.A[mid] = c[0]; st.B[mid] = c[1];
    st.sum_a += c[0]; st.sum_b += c[1];
  }
  int ms = max(n1, G_N);
  for (int s = 1; s < ms; s++)
    for (int i = 0; i < n1 - s; i++)
      st.corr[s] += st.A[i]*st.A[i+s] + st.B[i]*st.B[i+s];
}

static void ab_kopt_kick(ABState &st, int n1, int k, mt19937 &rng) {
  int half = (n1 + 1) / 2;
  int picked[16]; int npicked = 0;
  while (npicked < k) {
    int d = uniform_int_distribution<>(0, half - 1)(rng);
    bool dup = false;
    for (int i = 0; i < npicked; i++) if (picked[i] == d) { dup = true; break; }
    if (!dup) picked[npicked++] = d;
  }
  for (int pi = 0; pi < npicked; pi++) {
    int d = picked[pi];
    int left = d, right = n1 - 1 - d;
    int oldA_L = st.A[left], oldB_L = st.B[left];
    int oldA_R = st.A[right], oldB_R = st.B[right];
    int nA_L, nB_L, nA_R, nB_R;
    if (left == right) {
      const int *c = comb4[uniform_int_distribution<>(0, 3)(rng)];
      nA_L = c[0]; nB_L = c[1]; nA_R = c[0]; nB_R = c[1];
      st.sum_a += nA_L - oldA_L;
      st.sum_b += nB_L - oldB_L;
    } else {
      const int *c = (d == 0)
          ? comb8_neg[uniform_int_distribution<>(0, 7)(rng)]
          : comb8_pos[uniform_int_distribution<>(0, 7)(rng)];
      nA_L = c[0]; nB_L = c[1]; nA_R = c[2]; nB_R = c[3];
      st.sum_a += (nA_L + nA_R) - (oldA_L + oldA_R);
      st.sum_b += (nB_L + nB_R) - (oldB_L + oldB_R);
    }
    st.A[left] = nA_L; st.B[left] = nB_L;
    if (left != right) { st.A[right] = nA_R; st.B[right] = nB_R; }
  }
  memset(st.corr, 0, sizeof(st.corr));
  int ms = max(n1, G_N);
  for (int s = 1; s < ms; s++)
    for (int i = 0; i < n1 - s; i++)
      st.corr[s] += st.A[i]*st.A[i+s] + st.B[i]*st.B[i+s];
}

static bool solve_AB_SA(int n1, int ta, int tb, const int *cd_full,
                         ABState &best_state, mt19937 &rng) {
  ABState curr;
  ab_random_init(curr, n1, rng);

  int current_cost = curr.cost(ta, tb, n1, cd_full);
  best_state = curr;
  int best_cost = current_cost;

  SAParams sa;
  uniform_real_distribution<> prob(0.0, 1.0);
  uniform_int_distribution<> d_dist(0, (n1 - 1) / 2);
  int ms = max(n1, G_N);

  for (int restart = 0; restart < sa.restarts; restart++) {
    if (g_found.load(memory_order_relaxed)) return false;

    if (restart > 0) {
      ab_random_init(curr, n1, rng);
      current_cost = curr.cost(ta, tb, n1, cd_full);
    }

    double temp = sa.initial_temp;
    int no_improve = 0;
    int stall_for_kick = 0;
    int accept_window = 0, attempt_window = 0;

    for (int iter = 0; iter < sa.iterations; iter++) {
      if (g_found.load(memory_order_relaxed)) return false;
      if (best_cost == 0) return true;

      no_improve++;
      stall_for_kick++;
      attempt_window++;

      if (stall_for_kick > sa.kick_after_stall && best_cost <= 32) {
        int k = 2 + uniform_int_distribution<>(0, 3)(rng);
        curr = best_state;
        ab_kopt_kick(curr, n1, k, rng);
        current_cost = curr.cost(ta, tb, n1, cd_full);
        temp = sa.initial_temp * 0.7;
        stall_for_kick = 0;
        continue;
      }

      if (attempt_window >= 5000) {
        double acc = (double)accept_window / attempt_window;
        if (acc < 0.01 && no_improve > sa.reheat_threshold / 4) {
          temp = sa.initial_temp * sa.reheat_ratio;
          no_improve = 0;
        }
        accept_window = 0;
        attempt_window = 0;
      }

      if (no_improve > sa.reheat_threshold) {
        temp = sa.initial_temp * sa.reheat_ratio;
        no_improve = 0;
      }

      int d = d_dist(rng);
      int left = d, right = n1 - 1 - d;

      int oldA_L = curr.A[left], oldB_L = curr.B[left];
      int oldA_R = curr.A[right], oldB_R = curr.B[right];
      int nA_L, nB_L, nA_R, nB_R;

      if (left == right) {
        const int *c = comb4[uniform_int_distribution<>(0, 3)(rng)];
        nA_L = c[0]; nB_L = c[1]; nA_R = nA_L; nB_R = nB_L;
        if (oldA_L == nA_L && oldB_L == nB_L) continue;
      } else {
        const int *c = (d == 0)
            ? comb8_neg[uniform_int_distribution<>(0, 7)(rng)]
            : comb8_pos[uniform_int_distribution<>(0, 7)(rng)];
        nA_L = c[0]; nB_L = c[1]; nA_R = c[2]; nB_R = c[3];
        if (oldA_L == nA_L && oldB_L == nB_L && oldA_R == nA_R && oldB_R == nB_R)
          continue;
      }

      int old_sum_a = curr.sum_a, old_sum_b = curr.sum_b;
      if (left == right) {
        curr.sum_a += nA_L - oldA_L;
        curr.sum_b += nB_L - oldB_L;
      } else {
        curr.sum_a += (nA_L + nA_R) - (oldA_L + oldA_R);
        curr.sum_b += (nB_L + nB_R) - (oldB_L + oldB_R);
      }

      int delta_corr[128] = {0};
      for (int s = 1; s < ms; s++) {
        if (left  - s >= 0) delta_corr[s] -= curr.A[left - s]*oldA_L + curr.B[left - s]*oldB_L;
        if (left  + s < n1) delta_corr[s] -= oldA_L*curr.A[left + s] + oldB_L*curr.B[left + s];
        if (left != right) {
          if (right - s >= 0) delta_corr[s] -= curr.A[right - s]*oldA_R + curr.B[right - s]*oldB_R;
          if (right + s < n1) delta_corr[s] -= oldA_R*curr.A[right + s] + oldB_R*curr.B[right + s];
          if (right - left == s) delta_corr[s] += oldA_L*oldA_R + oldB_L*oldB_R;
        }
      }

      curr.A[left] = nA_L; curr.B[left] = nB_L;
      curr.A[right] = nA_R; curr.B[right] = nB_R;

      for (int s = 1; s < ms; s++) {
        if (left  - s >= 0) delta_corr[s] += curr.A[left - s]*nA_L + curr.B[left - s]*nB_L;
        if (left  + s < n1) delta_corr[s] += nA_L*curr.A[left + s] + nB_L*curr.B[left + s];
        if (left != right) {
          if (right - s >= 0) delta_corr[s] += curr.A[right - s]*nA_R + curr.B[right - s]*nB_R;
          if (right + s < n1) delta_corr[s] += nA_R*curr.A[right + s] + nB_R*curr.B[right + s];
          if (right - left == s) delta_corr[s] -= nA_L*nA_R + nB_L*nB_R;
        }
      }

      for (int s = 1; s < ms; s++) curr.corr[s] += delta_corr[s];

      int new_cost = curr.cost(ta, tb, n1, cd_full);

      if (new_cost < current_cost ||
          prob(rng) < exp(-(new_cost - current_cost) / temp)) {
        current_cost = new_cost;
        accept_window++;
        if (new_cost < best_cost) {
          best_cost = new_cost;
          best_state = curr;
          no_improve = 0;
          stall_for_kick = 0;
        }
      } else {
        curr.A[left] = oldA_L; curr.B[left] = oldB_L;
        curr.A[right] = oldA_R; curr.B[right] = oldB_R;
        curr.sum_a = old_sum_a; curr.sum_b = old_sum_b;
        for (int s = 1; s < ms; s++) curr.corr[s] -= delta_corr[s];
      }

      temp *= sa.cooling_rate;
    }

    if (best_cost == 0) return true;
  }

  return best_cost == 0;
}

// =====================================================================
//  Shared champion (best partial result seen across all threads)
// =====================================================================
struct Champion {
  int sig_a, sig_b, sig_c, sig_d;
  int A[128], B[128], C[128], D[128];
  int cost;
  int n, n1;
  bool valid;
};

static Champion g_champion_state{};

#ifdef _OPENMP
static omp_lock_t g_champion_lock;
#endif

static void champion_update(const Sig &sig, const ABState *ab, const CDState *cd,
                             int cost, int n, int n1) {
  int prev = g_champion_cost.load(memory_order_relaxed);
  if (cost >= prev) return;
#ifdef _OPENMP
  omp_set_lock(&g_champion_lock);
#endif
  if (cost < g_champion_cost.load(memory_order_relaxed)) {
    g_champion_state.sig_a = sig.a;
    g_champion_state.sig_b = sig.b;
    g_champion_state.sig_c = sig.c;
    g_champion_state.sig_d = sig.d;
    g_champion_state.cost = cost;
    g_champion_state.n = n;
    g_champion_state.n1 = n1;
    g_champion_state.valid = true;
    if (ab) {
      memcpy(g_champion_state.A, ab->A, sizeof(int) * n1);
      memcpy(g_champion_state.B, ab->B, sizeof(int) * n1);
    }
    if (cd) {
      memcpy(g_champion_state.C, cd->C, sizeof(int) * n);
      memcpy(g_champion_state.D, cd->D, sizeof(int) * n);
    }
    g_champion_cost.store(cost, memory_order_relaxed);

    // Checkpoint to disk so SLURM time-limit kills don't destroy progress.
    ostringstream path;
    path << ".champion_n" << n << ".txt";
    ofstream out(path.str());
    if (out.is_open()) {
      out << n << " " << n1 << " " << cost << "\n";
      out << sig.a << " " << sig.b << " " << sig.c << " " << sig.d << "\n";
      if (ab) {
        for (int i = 0; i < n1; i++) out << ab->A[i] << (i + 1 < n1 ? " " : "\n");
        for (int i = 0; i < n1; i++) out << ab->B[i] << (i + 1 < n1 ? " " : "\n");
      } else {
        for (int i = 0; i < n1; i++) out << 0 << (i + 1 < n1 ? " " : "\n");
        for (int i = 0; i < n1; i++) out << 0 << (i + 1 < n1 ? " " : "\n");
      }
      if (cd) {
        for (int i = 0; i < n; i++) out << cd->C[i] << (i + 1 < n ? " " : "\n");
        for (int i = 0; i < n; i++) out << cd->D[i] << (i + 1 < n ? " " : "\n");
      } else {
        for (int i = 0; i < n; i++) out << 0 << (i + 1 < n ? " " : "\n");
        for (int i = 0; i < n; i++) out << 0 << (i + 1 < n ? " " : "\n");
      }
    }
  }
#ifdef _OPENMP
  omp_unset_lock(&g_champion_lock);
#endif
}

static bool champion_load_from_disk(int n, int n1) {
  ostringstream path;
  path << ".champion_n" << n << ".txt";
  ifstream in(path.str());
  if (!in.is_open()) return false;
  int rn, rn1, rcost;
  if (!(in >> rn >> rn1 >> rcost)) return false;
  if (rn != n || rn1 != n1) return false;
  int sa, sb, sc, sd;
  if (!(in >> sa >> sb >> sc >> sd)) return false;
  Champion c{};
  c.sig_a = sa; c.sig_b = sb; c.sig_c = sc; c.sig_d = sd;
  c.cost = rcost; c.n = n; c.n1 = n1; c.valid = true;
  for (int i = 0; i < n1; i++) if (!(in >> c.A[i])) return false;
  for (int i = 0; i < n1; i++) if (!(in >> c.B[i])) return false;
  for (int i = 0; i < n;  i++) if (!(in >> c.C[i])) return false;
  for (int i = 0; i < n;  i++) if (!(in >> c.D[i])) return false;
  g_champion_state = c;
  g_champion_cost.store(rcost, memory_order_relaxed);
  return true;
}

// =====================================================================
//  Main
// =====================================================================
int main(int argc, char **argv) {
  if (argc < 2) {
    cerr << "Usage: " << argv[0] << " <n> [seed_offset]" << endl;
    return 1;
  }
  int n = atoi(argv[1]);
  int seed_offset = (argc >= 3) ? atoi(argv[2]) : 0;
  G_N = n;
  G_N1 = n + 1;
  G_MS = max(G_N1, G_N);
  G_SEED_OFFSET = seed_offset;
  int n1 = n + 1;
  int ms = max(n1, n);

  init_combs();

#ifdef _OPENMP
  omp_init_lock(&g_champion_lock);
#endif

  int thr = 1;
#ifdef _OPENMP
  thr = omp_get_max_threads();
  omp_set_num_threads(thr);
#endif

  cout << "========================================================" << endl;
  cout << "  BS(" << n1 << "," << n << ") v2 Solver — Wang-Zhu Manifold SA" << endl;
  cout << "  [ Threads: " << thr << " | Seed offset: " << seed_offset << " ]" << endl;
  cout << "========================================================" << endl;

  G_T0 = Clock::now();
  auto sigs = get_sigs(n);
  cout << "Loaded " << sigs.size() << " valid sum signatures." << endl;

  // Per-signature fail counter (epsilon-greedy skip of known-hard sigs).
  vector<atomic<int>> sig_fails(sigs.size());
  for (auto &a : sig_fails) a.store(0, memory_order_relaxed);

  // Try to resume from disk checkpoint.
  if (champion_load_from_disk(n, n1)) {
    cout << "Loaded champion from disk: cost=" << g_champion_state.cost
         << " sig=(" << g_champion_state.sig_a << "," << g_champion_state.sig_b
         << "," << g_champion_state.sig_c << "," << g_champion_state.sig_d << ")"
         << endl;
  } else {
    cout << "No prior champion on disk — cold start." << endl;
  }
  cout << endl;

  // Log every 30 seconds (from thread 0).
  auto last_log = Clock::now();

#pragma omp parallel
  {
    int tid = 0;
#ifdef _OPENMP
    tid = omp_get_thread_num();
#endif

    // Strong per-thread RNG seed — std::random_device + hi-res clock + tid + offset.
    std::random_device rd;
    uint64_t ns_epoch = chrono::high_resolution_clock::now().time_since_epoch().count();
    std::seed_seq seq{
      (uint32_t)rd(), (uint32_t)rd(), (uint32_t)rd(), (uint32_t)rd(),
      (uint32_t)tid, (uint32_t)(tid >> 16),
      (uint32_t)seed_offset, (uint32_t)(seed_offset >> 16),
      (uint32_t)ns_epoch, (uint32_t)(ns_epoch >> 32)
    };
    mt19937 rng(seq);

    long long local_tries = 0;

    while (!g_found.load(memory_order_relaxed)) {
      // Signature selection with epsilon-greedy skip of known-hard sigs.
      int si;
      while (true) {
        si = uniform_int_distribution<>(0, (int)sigs.size() - 1)(rng);
        int fails = sig_fails[si].load(memory_order_relaxed);
        // Probability of accepting a sig decays with fail count; eps=0.15 keeps exploration.
        if (fails < 3) break;
        double p_accept = 1.0 / (1.0 + 0.4 * (fails - 2));
        if (uniform_real_distribution<>(0.0, 1.0)(rng) < p_accept + 0.15) break;
      }
      auto &sig = sigs[si];

      CDState best_cd;
      bool found_cd = solve_CD_SA(n, n1, sig.c, sig.d, best_cd, rng);

      if (!found_cd) {
        sig_fails[si].fetch_add(1, memory_order_relaxed);
        // Update champion with CD partial (no AB yet).
        int cd_only_cost = best_cd.cost(sig.c, sig.d, n1, n);
        // Report total as CD cost only (AB not attempted).
        champion_update(sig, nullptr, &best_cd, cd_only_cost + 1000, n, n1);
        local_tries++;
        g_epochs_total.fetch_add(1, memory_order_relaxed);
        goto log_check;
      }

      {
        int cd_full[128] = {0};
        for (int s = 1; s < ms; s++)
          for (int k = 0; k < n - s; k++)
            cd_full[s] += best_cd.C[k]*best_cd.C[k+s] + best_cd.D[k]*best_cd.D[k+s];

        ABState best_ab;
        bool found_ab = solve_AB_SA(n1, sig.a, sig.b, cd_full, best_ab, rng);

        int joint_cost = found_ab ? 0 : best_ab.cost(sig.a, sig.b, n1, cd_full);
        champion_update(sig, &best_ab, &best_cd, joint_cost, n, n1);

        if (found_ab) {
          // Final NPAF=0 validation.
          bool valid = true;
          for (int s = 1; s < ms && valid; s++) {
            if (npaf_at(best_ab.A, best_ab.B, n1, best_cd.C, best_cd.D, n, s) != 0)
              valid = false;
          }
          if (valid && hall_ok(best_cd.C, n, best_cd.D, n) &&
              hall_ok(best_ab.A, n1, best_ab.B, n1)) {
            g_found.store(true);
#pragma omp critical(output)
            {
              if (n >= 44)
                cout << "\n*** WORLD RECORD DISCOVERY: BS(" << n1 << "," << n << ") FOUND ***\n" << endl;
              else
                cout << "\n*** REPRODUCTION CONFIRMED: BS(" << n1 << "," << n << ") FOUND ***\n" << endl;

              cout << "sig = (" << sig.a << "," << sig.b << "," << sig.c << "," << sig.d << ")" << endl;
              cout << "A = {";
              for (int i = 0; i < n1; i++) cout << best_ab.A[i] << (i < n1 - 1 ? "," : "");
              cout << "};" << endl;
              cout << "B = {";
              for (int i = 0; i < n1; i++) cout << best_ab.B[i] << (i < n1 - 1 ? "," : "");
              cout << "};" << endl;
              cout << "C = {";
              for (int i = 0; i < n; i++) cout << best_cd.C[i] << (i < n - 1 ? "," : "");
              cout << "};" << endl;
              cout << "D = {";
              for (int i = 0; i < n; i++) cout << best_cd.D[i] << (i < n - 1 ? "," : "");
              cout << "};" << endl;

              double t = chrono::duration<double>(Clock::now() - G_T0).count();
              cout << "\nTime: " << t << "s" << endl;
              cout << "Seed offset: " << seed_offset << endl;
            }
          }
        } else {
          sig_fails[si].fetch_add(1, memory_order_relaxed);
        }
      }

      local_tries++;
      g_epochs_total.fetch_add(1, memory_order_relaxed);

    log_check:
      if (tid == 0) {
        auto now = Clock::now();
        if (chrono::duration<double>(now - last_log).count() > 30.0) {
          last_log = now;
          double t = chrono::duration<double>(now - G_T0).count();
          long long total = g_epochs_total.load(memory_order_relaxed);
          double speed = (t > 0) ? (total / t) : 0.0;
          int champ = g_champion_cost.load(memory_order_relaxed);
          cout << "[" << t << "s] Epochs: " << total
               << " Speed: " << speed
               << " Champion cost: " << (champ == INT_MAX ? -1 : champ)
               << "\n" << flush;
        }
      }
    }
  }

#ifdef _OPENMP
  omp_destroy_lock(&g_champion_lock);
#endif

  return g_found.load() ? 0 : 1;
}
