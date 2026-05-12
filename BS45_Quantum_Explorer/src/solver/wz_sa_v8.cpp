/*
 * Wang-Zhu BS Solver v8 — Phased CD then AB SA
 * CP493 - Directed Research - Daniel Gordon
 *
 * Restored from the original wz_sa_bs43.cpp that successfully reproduced
 * BS(43,42). v4-v7 used a JOINT (A,B,C,D) SA which never broke cost=24/32.
 *
 * The phased decomposition is the key insight:
 *   1. Solve CD with a RELAXED cost: only penalize NPAF that AB cannot
 *      compensate (i.e. |corr_CD[s]| > 2*(n1-s)). Most of the NPAF is
 *      "free" — AB will fix it later.
 *   2. With CD fixed, solve AB so that NPAF[A] + NPAF[B] = -NPAF[CD].
 *
 * v8 changes vs original:
 *   - seed_offset CLI argument (matches our SLURM array jobs)
 *   - Stronger RNG seeding (random_device + seed_seq, not just time())
 *   - Cleaned up logging
 *
 * Usage:  ./wz_sa_v8 <n> [seed_offset]
 * Cluster: g++ -O3 -march=native -std=c++17 -fopenmp -o wz_sa_v8 src/solver/wz_sa_v8.cpp
 */

#include <algorithm>
#include <atomic>
#include <chrono>
#include <climits>
#include <cmath>
#include <cstring>
#include <fstream>
#include <iostream>
#include <random>
#include <tuple>
#include <vector>

#ifdef _OPENMP
#include <omp.h>
#endif

using namespace std;
using Clock = chrono::steady_clock;

static atomic<bool> g_found{false};
static int g_sol[4][128];
static int G_N;
static Clock::time_point G_T0;

// Diagnostic counters: best costs ever seen across all threads, plus
// CD success rate (so we can tell if CD phase is even closing).
static atomic<int> g_best_cd_cost{INT_MAX};
static atomic<int> g_best_ab_cost{INT_MAX};
static atomic<long long> g_cd_attempts{0};
static atomic<long long> g_cd_successes{0};
static atomic<long long> g_ab_attempts{0};

static inline void update_min_atomic(atomic<int> &a, int v) {
  int cur = a.load(memory_order_relaxed);
  while (v < cur && !a.compare_exchange_weak(cur, v, memory_order_relaxed)) {
  }
}

// All 16 combinations
int comb16[16][4];
// The 8 combinations where product = 1 (sum = 0 mod 4)
int comb8_pos[8][4];
// The 8 combinations where product = -1 (sum = 2 mod 4)
int comb8_neg[8][4];
// Middle combinations (for length 45, the 22nd index)
int comb4[4][2] = {{1, 1}, {1, -1}, {-1, 1}, {-1, -1}};

void init_combs() {
  int p = 0, n_idx = 0;
  for (int i = 0; i < 16; i++) {
    comb16[i][0] = (i & 8) ? 1 : -1;
    comb16[i][1] = (i & 4) ? 1 : -1;
    comb16[i][2] = (i & 2) ? 1 : -1;
    comb16[i][3] = (i & 1) ? 1 : -1;

    int prod = comb16[i][0] * comb16[i][1] * comb16[i][2] * comb16[i][3];
    if (prod == 1) {
      for (int j = 0; j < 4; j++)
        comb8_pos[p][j] = comb16[i][j];
      p++;
    } else {
      for (int j = 0; j < 4; j++)
        comb8_neg[n_idx][j] = comb16[i][j];
      n_idx++;
    }
  }
}

// Hall polynomial filter
bool hall_ok(const int *X, int xlen, const int *Y, int ylen) {
  double limit = 4.0 * G_N + 2.0;
  for (int j = 1; j <= 200; j++) {
    double th = j * M_PI / 100.0;
    double rx = 0, ix = 0, ry = 0, iy = 0;
    for (int i = 0; i < xlen; i++) {
      rx += X[i] * cos(i * th);
      ix += X[i] * sin(i * th);
    }
    for (int i = 0; i < ylen; i++) {
      ry += Y[i] * cos(i * th);
      iy += Y[i] * sin(i * th);
    }
    if (rx * rx + ix * ix + ry * ry + iy * iy > limit + 0.5)
      return false;
  }
  return true;
}

int npaf_at(const int *A, const int *B, int n1, const int *C, const int *D,
            int n2, int s) {
  int c = 0;
  if (s < n1)
    for (int i = 0; i < n1 - s; i++)
      c += A[i] * A[i + s] + B[i] * B[i + s];
  if (s < n2)
    for (int i = 0; i < n2 - s; i++)
      c += C[i] * C[i + s] + D[i] * D[i + s];
  return c;
}

struct Sig {
  int a, b, c, d;
};
vector<Sig> get_sigs(int n) {
  vector<Sig> sigs;
  int T = 4 * n + 2, n1 = n + 1, ap = n1 % 2, cp = n % 2;
  for (int a = 0; a <= n1; a++) {
    if (a % 2 != ap || a * a > T)
      continue;
    for (int b = -n1; b <= n1; b++) {
      if (((b % 2) + 2) % 2 != (unsigned)ap)
        continue;
      int r = T - a * a - b * b;
      if (r < 0)
        continue;
      for (int c = -n; c <= n; c++) {
        if (((c % 2) + 2) % 2 != (unsigned)cp)
          continue;
        int d2 = r - c * c;
        if (d2 < 0)
          continue;
        int d = (int)round(sqrt((double)d2));
        if (d * d != d2 || d > n || (((d % 2) + 2) % 2 != (unsigned)cp))
          continue;
        if (n % 2 == 0) {
          if (((c - d) % 4 + 8) % 4 != 0)
            continue;
        } else {
          if (((a - b - 2) % 4 + 8) % 4 != 0)
            continue;
        }
        sigs.push_back({a, b, c, d});
        if (d > 0 && ((-d % 2 + 2) % 2 == cp)) {
          bool ok = true;
          if (n % 2 == 0 && ((c + d) % 4 + 8) % 4 != 0)
            ok = false;
          if (ok)
            sigs.push_back({a, b, c, -d});
        }
      }
    }
  }
  sort(sigs.begin(), sigs.end(), [](auto &x, auto &y) {
    return tie(x.a, x.b, x.c, x.d) < tie(y.a, y.b, y.c, y.d);
  });
  sigs.erase(unique(sigs.begin(), sigs.end(),
                    [](auto &x, auto &y) {
                      return x.a == y.a && x.b == y.b && x.c == y.c &&
                             x.d == y.d;
                    }),
             sigs.end());
  return sigs;
}

struct SAParams {
  double initial_temp = 50.0;
  double cooling_rate = 0.9999; // Faster cooling for shorter cycles
  int iterations = 500000;      // Shorter cycles = more restarts
  int restarts = 20;            // Number of restart cycles per epoch
  int reheat_threshold = 50000; // Reheat if no improvement for this many iters
  double reheat_ratio = 0.25;   // Reheat to 25% of initial temp
};

// ===================================
// CD SA Solver
// ===================================
// For n=44, CD lengths are 44. depth pairs = 44/2 = 22 pairs (d=0..21).
// d=0 uses comb16. d=1..21 uses comb8_pos.
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
      if (abs(corr[s]) > max_ab) {
        pen += abs(corr[s]) - max_ab;
      }
    }
    return diff_c * 5 + diff_d * 5 + pen;
  }
};

bool solve_CD_SA(int n, int n1, int tc, int td, CDState &best_state,
                 mt19937 &rng, int sig_idx);

static void cd_init_random(CDState &curr, int n, mt19937 &rng) {
  memset(curr.corr, 0, sizeof(curr.corr));
  curr.sum_c = 0;
  curr.sum_d = 0;
  for (int d = 0; d < n / 2; d++) {
    int left = d;
    int right = n - 1 - d;
    const int *c_ptr;
    if (d == 0)
      c_ptr = comb16[uniform_int_distribution<>(0, 15)(rng)];
    else
      c_ptr = comb8_pos[uniform_int_distribution<>(0, 7)(rng)];
    curr.C[left] = c_ptr[0];
    curr.D[left] = c_ptr[1];
    curr.C[right] = c_ptr[2];
    curr.D[right] = c_ptr[3];
    curr.sum_c += c_ptr[0] + c_ptr[2];
    curr.sum_d += c_ptr[1] + c_ptr[3];
  }
  // For odd n, the middle position is unpaired; use comb4 (free ±1 ±1).
  if (n % 2 == 1) {
    int mid = n / 2;
    const int *m_ptr = comb4[uniform_int_distribution<>(0, 3)(rng)];
    curr.C[mid] = m_ptr[0];
    curr.D[mid] = m_ptr[1];
    curr.sum_c += m_ptr[0];
    curr.sum_d += m_ptr[1];
  }
}

// Per-signature CD champion for warm-starts across threads.
struct CDChampion {
  int cost;
  CDState state;
};
static vector<CDChampion> g_cd_champ;

bool solve_CD_SA(int n, int n1, int tc, int td, CDState &best_state,
                 mt19937 &rng, int sig_idx) {
  CDState curr;
  cd_init_random(curr, n, rng);
  // Compute correlations
  int ms = max(n1, n);
  for (int s = 1; s < ms; s++) {
    for (int i = 0; i < n - s; i++) {
      curr.corr[s] += curr.C[i] * curr.C[i + s] + curr.D[i] * curr.D[i + s];
    }
  }

  int current_cost = curr.cost(tc, td, n1, n);
  best_state = curr;
  int best_cost = current_cost;

  SAParams sa;
  uniform_real_distribution<> prob(0.0, 1.0);
  // For odd n, include d == n/2 to allow mutation of the middle position.
  uniform_int_distribution<> d_dist(0, (n % 2 == 1) ? n / 2 : n / 2 - 1);

  // Adaptive restart loop: run multiple short SA cycles
  for (int restart = 0; restart < sa.restarts; restart++) {
    if (g_found.load(memory_order_relaxed))
      return false;

    // Re-randomize state for each restart (except first which uses initial).
    // With probability 0.3, warm-start from the per-sig champion if we have
    // one that's better than a random init would likely be.
    if (restart > 0) {
      bool used_champion = false;
      if (sig_idx >= 0 && (int)g_cd_champ.size() > sig_idx) {
        int ch_cost;
#pragma omp critical(cd_champ)
        ch_cost = g_cd_champ[sig_idx].cost;
        if (ch_cost < INT_MAX && prob(rng) < 0.3) {
#pragma omp critical(cd_champ)
          curr = g_cd_champ[sig_idx].state;
          current_cost = ch_cost;
          used_champion = true;
        }
      }
      if (!used_champion) {
        cd_init_random(curr, n, rng);
        for (int s = 1; s < ms; s++)
          curr.corr[s] = 0;
        for (int s = 1; s < ms; s++)
          for (int i = 0; i < n - s; i++)
            curr.corr[s] +=
                curr.C[i] * curr.C[i + s] + curr.D[i] * curr.D[i + s];
        current_cost = curr.cost(tc, td, n1, n);
      }
    }

    double temp = sa.initial_temp;
    int no_improve = 0;

    for (int iter = 0; iter < sa.iterations; iter++) {
      if (g_found.load(memory_order_relaxed))
        return false;

      if (best_cost == 0) {
        if (hall_ok(best_state.C, n, best_state.D, n))
          return true;
        current_cost += 50;
      }

      // Reheat if stuck
      no_improve++;
      if (no_improve > sa.reheat_threshold) {
        temp = sa.initial_temp * sa.reheat_ratio;
        no_improve = 0;
      }

      // k-pair kick: when stuck above cost 0, resample 2-3 pairs simultaneously
      // and accept unconditionally to escape coordinated basins. Standard ILS.
      if (no_improve > 30000 && best_cost > 0) {
        int k_kick = 2 + uniform_int_distribution<>(0, 1)(rng);
        for (int kk = 0; kk < k_kick; kk++) {
          int dk = d_dist(rng);
          if (n % 2 == 1 && dk == n / 2) {
            int mid = n / 2;
            const int *m_ptr = comb4[uniform_int_distribution<>(0, 3)(rng)];
            curr.sum_c += m_ptr[0] - curr.C[mid];
            curr.sum_d += m_ptr[1] - curr.D[mid];
            curr.C[mid] = m_ptr[0];
            curr.D[mid] = m_ptr[1];
          } else {
            int lk = dk, rk = n - 1 - dk;
            const int *c_ptr =
                (dk == 0)
                    ? comb16[uniform_int_distribution<>(0, 15)(rng)]
                    : comb8_pos[uniform_int_distribution<>(0, 7)(rng)];
            curr.sum_c += (c_ptr[0] + c_ptr[2]) - (curr.C[lk] + curr.C[rk]);
            curr.sum_d += (c_ptr[1] + c_ptr[3]) - (curr.D[lk] + curr.D[rk]);
            curr.C[lk] = c_ptr[0];
            curr.D[lk] = c_ptr[1];
            curr.C[rk] = c_ptr[2];
            curr.D[rk] = c_ptr[3];
          }
        }
        memset(curr.corr, 0, sizeof(curr.corr));
        for (int s = 1; s < ms; s++)
          for (int i = 0; i < n - s; i++)
            curr.corr[s] +=
                curr.C[i] * curr.C[i + s] + curr.D[i] * curr.D[i + s];
        current_cost = curr.cost(tc, td, n1, n);
        no_improve = 0;
        temp = sa.initial_temp * 0.5;
        continue;
      }

      int d = d_dist(rng);

      // Odd-n middle position: single-index mutation with comb4.
      if (n % 2 == 1 && d == n / 2) {
        int mid = n / 2;
        int oldC = curr.C[mid], oldD = curr.D[mid];
        const int *m_ptr = comb4[uniform_int_distribution<>(0, 3)(rng)];
        int nC = m_ptr[0], nD = m_ptr[1];
        if (oldC == nC && oldD == nD)
          continue;
        int old_sum_c = curr.sum_c, old_sum_d = curr.sum_d;
        curr.sum_c += nC - oldC;
        curr.sum_d += nD - oldD;
        int delta_corr_mid[128] = {0};
        for (int s = 1; s < ms; s++) {
          if (mid - s >= 0)
            delta_corr_mid[s] -=
                curr.C[mid - s] * oldC + curr.D[mid - s] * oldD;
          if (mid + s < n)
            delta_corr_mid[s] -=
                oldC * curr.C[mid + s] + oldD * curr.D[mid + s];
        }
        curr.C[mid] = nC;
        curr.D[mid] = nD;
        for (int s = 1; s < ms; s++) {
          if (mid - s >= 0)
            delta_corr_mid[s] += curr.C[mid - s] * nC + curr.D[mid - s] * nD;
          if (mid + s < n)
            delta_corr_mid[s] += nC * curr.C[mid + s] + nD * curr.D[mid + s];
        }
        for (int s = 1; s < ms; s++)
          curr.corr[s] += delta_corr_mid[s];
        int new_cost_mid = curr.cost(tc, td, n1, n);
        if (new_cost_mid < current_cost ||
            prob(rng) < exp(-(new_cost_mid - current_cost) / temp)) {
          current_cost = new_cost_mid;
          if (new_cost_mid < best_cost) {
            best_cost = new_cost_mid;
            best_state = curr;
            no_improve = 0;
          }
        } else {
          curr.C[mid] = oldC;
          curr.D[mid] = oldD;
          curr.sum_c = old_sum_c;
          curr.sum_d = old_sum_d;
          for (int s = 1; s < ms; s++)
            curr.corr[s] -= delta_corr_mid[s];
        }
        temp *= sa.cooling_rate;
        continue;
      }

      int left = d;
      int right = n - 1 - d;

      int oldC_L = curr.C[left], oldD_L = curr.D[left];
      int oldC_R = curr.C[right], oldD_R = curr.D[right];

      const int *c_ptr;
      if (d == 0)
        c_ptr = comb16[uniform_int_distribution<>(0, 15)(rng)];
      else
        c_ptr = comb8_pos[uniform_int_distribution<>(0, 7)(rng)];

      int nC_L = c_ptr[0], nD_L = c_ptr[1];
      int nC_R = c_ptr[2], nD_R = c_ptr[3];

      if (oldC_L == nC_L && oldD_L == nD_L && oldC_R == nC_R && oldD_R == nD_R)
        continue;

      int old_sum_c = curr.sum_c;
      int old_sum_d = curr.sum_d;
      curr.sum_c += (nC_L + nC_R) - (oldC_L + oldC_R);
      curr.sum_d += (nD_L + nD_R) - (oldD_L + oldD_R);

      int delta_corr[128] = {0};

      for (int s = 1; s < ms; s++) {
        if (left - s >= 0)
          delta_corr[s] -=
              curr.C[left - s] * oldC_L + curr.D[left - s] * oldD_L;
        if (left + s < n)
          delta_corr[s] -=
              oldC_L * curr.C[left + s] + oldD_L * curr.D[left + s];
        if (right - s >= 0)
          delta_corr[s] -=
              curr.C[right - s] * oldC_R + curr.D[right - s] * oldD_R;
        if (right + s < n)
          delta_corr[s] -=
              oldC_R * curr.C[right + s] + oldD_R * curr.D[right + s];
        if (right - left == s)
          delta_corr[s] += oldC_L * oldC_R + oldD_L * oldD_R;
      }

      curr.C[left] = nC_L;
      curr.D[left] = nD_L;
      curr.C[right] = nC_R;
      curr.D[right] = nD_R;

      for (int s = 1; s < ms; s++) {
        if (left - s >= 0)
          delta_corr[s] += curr.C[left - s] * nC_L + curr.D[left - s] * nD_L;
        if (left + s < n)
          delta_corr[s] += nC_L * curr.C[left + s] + nD_L * curr.D[left + s];
        if (right - s >= 0)
          delta_corr[s] += curr.C[right - s] * nC_R + curr.D[right - s] * nD_R;
        if (right + s < n)
          delta_corr[s] += nC_R * curr.C[right + s] + nD_R * curr.D[right + s];
        if (right - left == s)
          delta_corr[s] -= nC_L * nC_R + nD_L * nD_R;
      }

      for (int s = 1; s < ms; s++)
        curr.corr[s] += delta_corr[s];

      int new_cost = curr.cost(tc, td, n1, n);

      if (new_cost < current_cost ||
          prob(rng) < exp(-(new_cost - current_cost) / temp)) {
        current_cost = new_cost;
        if (new_cost < best_cost) {
          best_cost = new_cost;
          best_state = curr;
          no_improve = 0;
        }
      } else {
        curr.C[left] = oldC_L;
        curr.D[left] = oldD_L;
        curr.C[right] = oldC_R;
        curr.D[right] = oldD_R;
        curr.sum_c = old_sum_c;
        curr.sum_d = old_sum_d;
        for (int s = 1; s < ms; s++)
          curr.corr[s] -= delta_corr[s];
      }

      temp *= sa.cooling_rate;
    }

    // Push best of this restart to the per-sig champion if it's an improvement.
    if (sig_idx >= 0 && (int)g_cd_champ.size() > sig_idx) {
#pragma omp critical(cd_champ)
      {
        if (best_cost < g_cd_champ[sig_idx].cost) {
          g_cd_champ[sig_idx].cost = best_cost;
          g_cd_champ[sig_idx].state = best_state;
        }
      }
    }

    // Early out if this restart found cost 0
    if (best_cost == 0 && hall_ok(best_state.C, n, best_state.D, n))
      return true;
  }

  update_min_atomic(g_best_cd_cost, best_cost);
  return best_cost == 0 && hall_ok(best_state.C, n, best_state.D, n);
}

// ===================================
// AB SA Solver
// ===================================
// For n1=45, d=0..22.
// d=0 uses comb8_neg (-1)
// d=1..21 uses comb8_pos (+1)
// d=22 uses comb4 (middle element)
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
      if (corr[s] + cd_full[s] != 0) {
        pen += abs(corr[s] + cd_full[s]);
      }
    }
    return diff_a * 5 + diff_b * 5 + pen;
  }
};

bool solve_AB_SA(int n1, int ta, int tb, const int *cd_full,
                 ABState &best_state, mt19937 &rng) {
  ABState curr;
  memset(curr.corr, 0, sizeof(curr.corr));
  curr.sum_a = 0;
  curr.sum_b = 0;
  int ms = max(n1, G_N);

  for (int d = 0; d < n1 / 2; d++) {
    int left = d;
    int right = n1 - 1 - d;
    const int *c_ptr;
    if (d == 0)
      c_ptr = comb8_neg[uniform_int_distribution<>(0, 7)(rng)];
    else
      c_ptr = comb8_pos[uniform_int_distribution<>(0, 7)(rng)];
    curr.A[left] = c_ptr[0];
    curr.B[left] = c_ptr[1];
    curr.A[right] = c_ptr[2];
    curr.B[right] = c_ptr[3];
    curr.sum_a += c_ptr[0] + c_ptr[2];
    curr.sum_b += c_ptr[1] + c_ptr[3];
  }
  // Middle element if odd
  if (n1 % 2 != 0) {
    int mid = n1 / 2;
    const int *c_ptr = comb4[uniform_int_distribution<>(0, 3)(rng)];
    curr.A[mid] = c_ptr[0];
    curr.B[mid] = c_ptr[1];
    curr.sum_a += c_ptr[0];
    curr.sum_b += c_ptr[1];
  }

  // Initial corr
  for (int s = 1; s < ms; s++) {
    for (int i = 0; i < n1 - s; i++) {
      curr.corr[s] += curr.A[i] * curr.A[i + s] + curr.B[i] * curr.B[i + s];
    }
  }

  int current_cost = curr.cost(ta, tb, n1, cd_full);
  best_state = curr;
  int best_cost = current_cost;

  SAParams sa;
  uniform_real_distribution<> prob(0.0, 1.0);
  uniform_int_distribution<> d_dist(0, (n1 - 1) / 2);

  // Adaptive restart loop
  for (int restart = 0; restart < sa.restarts; restart++) {
    if (g_found.load(memory_order_relaxed))
      return false;

    // Re-randomize for restarts after the first
    if (restart > 0) {
      memset(curr.corr, 0, sizeof(curr.corr));
      curr.sum_a = 0;
      curr.sum_b = 0;
      for (int d = 0; d < n1 / 2; d++) {
        int left = d, right = n1 - 1 - d;
        const int *c_ptr =
            (d == 0) ? comb8_neg[uniform_int_distribution<>(0, 7)(rng)]
                     : comb8_pos[uniform_int_distribution<>(0, 7)(rng)];
        curr.A[left] = c_ptr[0];
        curr.B[left] = c_ptr[1];
        curr.A[right] = c_ptr[2];
        curr.B[right] = c_ptr[3];
        curr.sum_a += c_ptr[0] + c_ptr[2];
        curr.sum_b += c_ptr[1] + c_ptr[3];
      }
      if (n1 % 2 != 0) {
        int mid = n1 / 2;
        const int *c_ptr = comb4[uniform_int_distribution<>(0, 3)(rng)];
        curr.A[mid] = c_ptr[0];
        curr.B[mid] = c_ptr[1];
        curr.sum_a += c_ptr[0];
        curr.sum_b += c_ptr[1];
      }
      for (int s = 1; s < ms; s++)
        for (int i = 0; i < n1 - s; i++)
          curr.corr[s] += curr.A[i] * curr.A[i + s] + curr.B[i] * curr.B[i + s];
      current_cost = curr.cost(ta, tb, n1, cd_full);
    }

    double temp = sa.initial_temp;
    int no_improve = 0;

    for (int iter = 0; iter < sa.iterations; iter++) {
      if (g_found.load(memory_order_relaxed))
        return false;
      if (best_cost == 0)
        return true;

      // Reheat if stuck
      no_improve++;
      if (no_improve > sa.reheat_threshold) {
        temp = sa.initial_temp * sa.reheat_ratio;
        no_improve = 0;
      }

      // k-pair kick (AB): resample 2-3 pairs simultaneously to escape basins.
      if (no_improve > 30000 && best_cost > 0) {
        int k_kick = 2 + uniform_int_distribution<>(0, 1)(rng);
        for (int kk = 0; kk < k_kick; kk++) {
          int dk = d_dist(rng);
          int lk = dk, rk = n1 - 1 - dk;
          if (lk == rk) {
            const int *m_ptr = comb4[uniform_int_distribution<>(0, 3)(rng)];
            curr.sum_a += m_ptr[0] - curr.A[lk];
            curr.sum_b += m_ptr[1] - curr.B[lk];
            curr.A[lk] = m_ptr[0];
            curr.B[lk] = m_ptr[1];
          } else {
            const int *c_ptr =
                (dk == 0)
                    ? comb8_neg[uniform_int_distribution<>(0, 7)(rng)]
                    : comb8_pos[uniform_int_distribution<>(0, 7)(rng)];
            curr.sum_a += (c_ptr[0] + c_ptr[2]) - (curr.A[lk] + curr.A[rk]);
            curr.sum_b += (c_ptr[1] + c_ptr[3]) - (curr.B[lk] + curr.B[rk]);
            curr.A[lk] = c_ptr[0];
            curr.B[lk] = c_ptr[1];
            curr.A[rk] = c_ptr[2];
            curr.B[rk] = c_ptr[3];
          }
        }
        memset(curr.corr, 0, sizeof(curr.corr));
        for (int s = 1; s < ms; s++)
          for (int i = 0; i < n1 - s; i++)
            curr.corr[s] +=
                curr.A[i] * curr.A[i + s] + curr.B[i] * curr.B[i + s];
        current_cost = curr.cost(ta, tb, n1, cd_full);
        no_improve = 0;
        temp = sa.initial_temp * 0.5;
        continue;
      }

      int d = d_dist(rng);
      int left = d;
      int right = n1 - 1 - d;

      int oldA_L = curr.A[left], oldB_L = curr.B[left];
      int oldA_R = curr.A[right], oldB_R = curr.B[right];
      int nA_L, nB_L, nA_R, nB_R;

      if (left == right) {
        const int *c_ptr = comb4[uniform_int_distribution<>(0, 3)(rng)];
        nA_L = c_ptr[0];
        nB_L = c_ptr[1];
        nA_R = nA_L;
        nB_R = nB_L;
        if (oldA_L == nA_L && oldB_L == nB_L)
          continue;
      } else {
        const int *c_ptr =
            (d == 0) ? comb8_neg[uniform_int_distribution<>(0, 7)(rng)]
                     : comb8_pos[uniform_int_distribution<>(0, 7)(rng)];
        nA_L = c_ptr[0];
        nB_L = c_ptr[1];
        nA_R = c_ptr[2];
        nB_R = c_ptr[3];
        if (oldA_L == nA_L && oldB_L == nB_L && oldA_R == nA_R &&
            oldB_R == nB_R)
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
        if (left - s >= 0)
          delta_corr[s] -=
              curr.A[left - s] * oldA_L + curr.B[left - s] * oldB_L;
        if (left + s < n1)
          delta_corr[s] -=
              oldA_L * curr.A[left + s] + oldB_L * curr.B[left + s];
        if (left != right) {
          if (right - s >= 0)
            delta_corr[s] -=
                curr.A[right - s] * oldA_R + curr.B[right - s] * oldB_R;
          if (right + s < n1)
            delta_corr[s] -=
                oldA_R * curr.A[right + s] + oldB_R * curr.B[right + s];
          if (right - left == s)
            delta_corr[s] += oldA_L * oldA_R + oldB_L * oldB_R;
        }
      }

      curr.A[left] = nA_L;
      curr.B[left] = nB_L;
      curr.A[right] = nA_R;
      curr.B[right] = nB_R;

      for (int s = 1; s < ms; s++) {
        if (left - s >= 0)
          delta_corr[s] += curr.A[left - s] * nA_L + curr.B[left - s] * nB_L;
        if (left + s < n1)
          delta_corr[s] += nA_L * curr.A[left + s] + nB_L * curr.B[left + s];
        if (left != right) {
          if (right - s >= 0)
            delta_corr[s] +=
                curr.A[right - s] * nA_R + curr.B[right - s] * nB_R;
          if (right + s < n1)
            delta_corr[s] +=
                nA_R * curr.A[right + s] + nB_R * curr.B[right + s];
          if (right - left == s)
            delta_corr[s] -= nA_L * nA_R + nB_L * nB_R;
        }
      }

      for (int s = 1; s < ms; s++)
        curr.corr[s] += delta_corr[s];

      int new_cost = curr.cost(ta, tb, n1, cd_full);

      if (new_cost < current_cost ||
          prob(rng) < exp(-(new_cost - current_cost) / temp)) {
        current_cost = new_cost;
        if (new_cost < best_cost) {
          best_cost = new_cost;
          best_state = curr;
          no_improve = 0;
        }
      } else {
        curr.A[left] = oldA_L;
        curr.B[left] = oldB_L;
        curr.A[right] = oldA_R;
        curr.B[right] = oldB_R;
        curr.sum_a = old_sum_a;
        curr.sum_b = old_sum_b;
        for (int s = 1; s < ms; s++)
          curr.corr[s] -= delta_corr[s];
      }

      temp *= sa.cooling_rate;
    }

    if (best_cost == 0)
      return true;
  }

  update_min_atomic(g_best_ab_cost, best_cost);
  return best_cost == 0;
}

int main(int argc, char **argv) {
  if (argc < 2) {
    cerr << "Usage: " << argv[0] << " <n> [seed_offset]" << endl;
    return 1;
  }
  int n = atoi(argv[1]);
  int seed_offset = (argc >= 3) ? atoi(argv[2]) : 0;
  G_N = n;
  int n1 = n + 1;
  int ms = max(n1, n);

  init_combs();

  int thr = 1;
#ifdef _OPENMP
  thr = omp_get_max_threads();
  omp_set_num_threads(thr);
#endif

  cout << "========================================================" << endl;
  cout << "  BS(" << n1 << "," << n << ") v8 — Phased CD then AB SA" << endl;
  cout << "  [ Threads: " << thr << " | Seed offset: " << seed_offset << " ]"
       << endl;
  cout << "========================================================" << endl;

  G_T0 = Clock::now();
  auto sigs = get_sigs(n);
  cout << "Loaded " << sigs.size() << " valid sum signatures." << endl << endl;

  g_cd_champ.assign(sigs.size(), CDChampion{INT_MAX, {}});

  long long global_tries = 0;

#pragma omp parallel reduction(+ : global_tries)
  {
    int tid = 0;
#ifdef _OPENMP
    tid = omp_get_thread_num();
#endif
    std::random_device rd;
    uint64_t ns = chrono::high_resolution_clock::now().time_since_epoch().count();
    std::seed_seq seq{
      (uint32_t)rd(), (uint32_t)rd(), (uint32_t)rd(), (uint32_t)rd(),
      (uint32_t)tid, (uint32_t)(tid >> 16),
      (uint32_t)seed_offset, (uint32_t)(seed_offset >> 16),
      (uint32_t)ns, (uint32_t)(ns >> 32)
    };
    mt19937 rng(seq);

    while (!g_found.load(memory_order_relaxed)) {
      // Pick a random signature to target so workers distribute load
      int si = uniform_int_distribution<>(0, sigs.size() - 1)(rng);
      auto &sig = sigs[si];

      CDState best_cd;
      g_cd_attempts.fetch_add(1, memory_order_relaxed);
      bool found_cd = solve_CD_SA(n, n1, sig.c, sig.d, best_cd, rng, si);

      if (found_cd) {
        g_cd_successes.fetch_add(1, memory_order_relaxed);
        int cd_full[128] = {0};
        for (int s = 1; s < ms; s++) {
          for (int k = 0; k < n - s; k++)
            cd_full[s] += best_cd.C[k] * best_cd.C[k + s] +
                          best_cd.D[k] * best_cd.D[k + s];
        }

        // Multi-AB-per-CD: amortize expensive CD success over several AB tries.
        ABState best_ab;
        bool found_ab = false;
        for (int ab_try = 0;
             ab_try < 5 && !found_ab && !g_found.load(memory_order_relaxed);
             ab_try++) {
          g_ab_attempts.fetch_add(1, memory_order_relaxed);
          if (solve_AB_SA(n1, sig.a, sig.b, cd_full, best_ab, rng))
            found_ab = true;
        }

        if (found_ab) {
          bool valid = true;
          for (int s = 1; s < ms && valid; s++) {
            if (npaf_at(best_ab.A, best_ab.B, n1, best_cd.C, best_cd.D, n, s) !=
                0)
              valid = false;
          }
          if (valid) {
            g_found.store(true);
#pragma omp critical
            {
              if (n >= 44)
                cout << "\n*** WORLD RECORD DISCOVERY: BS(" << n1 << ","
                     << n << ") FOUND ***\n" << endl;
              else
                cout << "\n*** REPRODUCTION CONFIRMED: BS(" << n1 << ","
                     << n << ") FOUND ***\n" << endl;
              cout << "sig = (" << sig.a << "," << sig.b << "," << sig.c
                   << "," << sig.d << ")" << endl;
              cout << "A = {";
              for (int i = 0; i < n1; i++)
                cout << best_ab.A[i] << (i < n1 - 1 ? "," : "");
              cout << "};\n";
              cout << "B = {";
              for (int i = 0; i < n1; i++)
                cout << best_ab.B[i] << (i < n1 - 1 ? "," : "");
              cout << "};\n";
              cout << "C = {";
              for (int i = 0; i < n; i++)
                cout << best_cd.C[i] << (i < n - 1 ? "," : "");
              cout << "};\n";
              cout << "D = {";
              for (int i = 0; i < n; i++)
                cout << best_cd.D[i] << (i < n - 1 ? "," : "");
              cout << "};\n";

              double t = chrono::duration<double>(Clock::now() - G_T0).count();
              cout << "\nTime: " << t << "s\nSeed offset: " << seed_offset
                   << endl;
            }
          }
        }
      }

      global_tries++;
      if (tid == 0) {
        long long current_total = global_tries * thr;
        double t = chrono::duration<double>(Clock::now() - G_T0).count();
        double speed = (t > 0) ? ((global_tries * thr) / t) : 0.0;
        int bcd = g_best_cd_cost.load(memory_order_relaxed);
        int bab = g_best_ab_cost.load(memory_order_relaxed);
        long long cda = g_cd_attempts.load(memory_order_relaxed);
        long long cds = g_cd_successes.load(memory_order_relaxed);
        long long aba = g_ab_attempts.load(memory_order_relaxed);
        cout << "[" << t << "s] epochs=" << current_total
             << " speed=" << speed
             << " bestCD=" << (bcd == INT_MAX ? -1 : bcd)
             << " bestAB=" << (bab == INT_MAX ? -1 : bab)
             << " CDok=" << cds << "/" << cda
             << " ABtry=" << aba << "\n"
             << flush;
      }
    }
  }

  return g_found.load() ? 0 : 1;
}
