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
  int cost(int tc, int td, int n1, int n, const int *ab_full = nullptr) const {
    int diff_c = abs(sum_c - tc);
    int diff_d = abs(sum_d - td);
    int pen = 0;
    int ms = max(n1, n);
    if (ab_full) {
      // Coupled objective (alternating refinement): CD must cancel a fixed AB
      // exactly. cost 0 here == full NPAF=0 solution for (this CD, that AB).
      for (int s = 1; s < ms; s++)
        pen += abs(corr[s] + ab_full[s]);
    } else {
      // Warm-start objective: sum-matching only. NOTE: the magnitude term
      // below is always 0 for any valid CD, since |corr_CD[s]| <= 2(n-s) <
      // 2(n1-s). It exists only to seed the refinement with a sum-correct CD.
      for (int s = 1; s < ms; s++) {
        int max_ab = (s < n1) ? 2 * (n1 - s) : 0;
        if (abs(corr[s]) > max_ab) {
          pen += abs(corr[s]) - max_ab;
        }
      }
    }
    return diff_c * 5 + diff_d * 5 + pen;
  }
};

bool solve_CD_SA(int n, int n1, int tc, int td, CDState &best_state,
                 mt19937 &rng, int sig_idx, const int *ab_full = nullptr);

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

// AB-phase plateau diagnostics. Populated whenever solve_AB_SA terminates
// with 0 < best_cost < 64. Tells us WHICH shifts dominate the residual and
// WHERE (which cost buckets) AB attempts get stuck.
static atomic<long long> g_ab_shift_residual[128];
static atomic<long long> g_ab_term_hist[256];
static atomic<long long> g_ab_terminations{0};
static atomic<long long> g_ab_sum_residual{0};
static atomic<long long> g_ab_npaf_residual{0};

// Commit B: AB champion sharing + adaptive multi-AB-per-CD counters.
static atomic<long long> g_ab_champion_hits{0};      // # of warm-starts from champion
static atomic<long long> g_ab_attempts_skipped{0};   // # of AB tries skipped by early-exit

// Commit C: alternating CD<->AB refinement (block coordinate descent on the
// true coupled objective). g_refine_rounds = total CD-against-AB passes run.
static atomic<long long> g_refine_rounds{0};

// Commit D: stall detector + CD perturbation kick to escape BCD local minima.
// g_refine_kicks = total perturbation kicks fired between refinement rounds.
static atomic<long long> g_refine_kicks{0};

// Commit E: escalating kick magnitude + AB-side perturbation + per-sig tracking.
// kesc counts kicks fired with kick_level > 0 (escalated past the default size).
// g_sig_best_ab[i] = lowest coupled-cost seen for signature index i (INT_MAX = none).
static atomic<long long> g_refine_kick_escalations{0};
static constexpr int kMaxSigs = 1024;
static atomic<int> g_sig_best_ab[kMaxSigs];

// Commit F: endgame exhaustive polish — when SA-BCD gets the coupled cost
// below kPolishThreshold, deterministically enumerate 1-pair and 2-pair flips
// from the current (best_cd, best_ab) to see if a real solution lives in the
// nearby neighborhood. Addresses the proven SA bottleneck (Commit E sig-lock
// diagnostic showed even the known-correct sig plateaus at coupled cost ~28
// on BS(43); BS(28) plateaus at 4 — both within reach of exhaustive 2-flips).
static constexpr int kPolishThreshold = 16;
static atomic<long long> g_polish_attempts{0};
static atomic<long long> g_polish_improvements{0};
static atomic<long long> g_polish_solutions{0};

bool solve_CD_SA(int n, int n1, int tc, int td, CDState &best_state,
                 mt19937 &rng, int sig_idx, const int *ab_full) {
  CDState curr;
  cd_init_random(curr, n, rng);
  // Compute correlations
  int ms = max(n1, n);
  for (int s = 1; s < ms; s++) {
    for (int i = 0; i < n - s; i++) {
      curr.corr[s] += curr.C[i] * curr.C[i + s] + curr.D[i] * curr.D[i + s];
    }
  }

  int current_cost = curr.cost(tc, td, n1, n, ab_full);
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
          // The champion's cached cost is the warm-start (sum-only) cost.
          // In refinement mode the objective differs, so recompute.
          current_cost = ab_full ? curr.cost(tc, td, n1, n, ab_full) : ch_cost;
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
        current_cost = curr.cost(tc, td, n1, n, ab_full);
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
        current_cost = curr.cost(tc, td, n1, n, ab_full);
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
        int new_cost_mid = curr.cost(tc, td, n1, n, ab_full);
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

      int new_cost = curr.cost(tc, td, n1, n, ab_full);

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

  // In refinement mode the "cost" is the coupled NPAF residual, the same
  // objective the AB phase minimizes — surface it via g_best_ab_cost so the
  // log's bestAB reflects progress from BOTH search directions.
  if (ab_full)
    update_min_atomic(g_best_ab_cost, best_cost);
  else
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

// Per-signature AB champion for warm-starts across threads (mirrors CDChampion).
// Stores the best-so-far A,B state for each signature. corr depends only on A,B
// so it remains valid across the champion's lifetime; the cost is recomputed at
// warm-start time because it depends on the current cd_full (different per CD).
struct ABChampion {
  int cost;
  ABState state;
};
static vector<ABChampion> g_ab_champ;

// ===================================
// Commit F: Endgame polish helpers
// ===================================
// Helper: full coupled cost = 5*all-sum-mismatch + Sum|corr_CD[s]+corr_AB[s]|.
// (Same objective both blocks optimize; just measured here from the full state.)
static int compute_coupled_cost(const CDState &cd, const ABState &ab,
                                int sig_a, int sig_b, int sig_c, int sig_d,
                                int n, int n1) {
  int ms = max(n1, n);
  int cost = 5 * abs(ab.sum_a - sig_a) + 5 * abs(ab.sum_b - sig_b) +
             5 * abs(cd.sum_c - sig_c) + 5 * abs(cd.sum_d - sig_d);
  for (int s = 1; s < ms; s++)
    cost += abs(cd.corr[s] + ab.corr[s]);
  return cost;
}

// Replace CD pair (positions L=d, R=n-1-d). For odd-n middle (L==R), cR/dR ignored.
// Updates sums and recomputes corr from scratch (cheap: O(n^2) ~ 2000 ops for n=43).
static void apply_cd_pair(CDState &cd, int n, int d, int cL, int dL, int cR, int dR) {
  int L = d, R = n - 1 - d;
  cd.sum_c += cL - cd.C[L];
  cd.sum_d += dL - cd.D[L];
  cd.C[L] = cL;
  cd.D[L] = dL;
  if (L != R) {
    cd.sum_c += cR - cd.C[R];
    cd.sum_d += dR - cd.D[R];
    cd.C[R] = cR;
    cd.D[R] = dR;
  }
  int ms = max(n + 1, n);
  memset(cd.corr, 0, sizeof(cd.corr));
  for (int s = 1; s < ms; s++)
    for (int i = 0; i < n - s; i++)
      cd.corr[s] += cd.C[i] * cd.C[i + s] + cd.D[i] * cd.D[i + s];
}

static void apply_ab_pair(ABState &ab, int n1, int d, int aL, int bL, int aR, int bR) {
  int L = d, R = n1 - 1 - d;
  ab.sum_a += aL - ab.A[L];
  ab.sum_b += bL - ab.B[L];
  ab.A[L] = aL;
  ab.B[L] = bL;
  if (L != R) {
    ab.sum_a += aR - ab.A[R];
    ab.sum_b += bR - ab.B[R];
    ab.A[R] = aR;
    ab.B[R] = bR;
  }
  int ms = max(n1, G_N);
  memset(ab.corr, 0, sizeof(ab.corr));
  for (int s = 1; s < ms; s++)
    for (int i = 0; i < n1 - s; i++)
      ab.corr[s] += ab.A[i] * ab.A[i + s] + ab.B[i] * ab.B[i + s];
}

// Enumerate valid pair options for CD position d.
// Returns vector of (cL, dL, cR, dR) 4-tuples (R irrelevant if L==R for middle).
struct CDOpt { int cL, dL, cR, dR; };
struct ABOpt { int aL, bL, aR, bR; };

static vector<CDOpt> get_cd_options(int n, int d) {
  vector<CDOpt> opts;
  if (n % 2 == 1 && d == n / 2) {
    for (int i = 0; i < 4; i++)
      opts.push_back({comb4[i][0], comb4[i][1], 0, 0});
  } else if (d == 0) {
    for (int i = 0; i < 16; i++)
      opts.push_back({comb16[i][0], comb16[i][1], comb16[i][2], comb16[i][3]});
  } else {
    for (int i = 0; i < 8; i++)
      opts.push_back({comb8_pos[i][0], comb8_pos[i][1], comb8_pos[i][2], comb8_pos[i][3]});
  }
  return opts;
}

static vector<ABOpt> get_ab_options(int n1, int d) {
  vector<ABOpt> opts;
  if (n1 % 2 == 1 && d == n1 / 2) {
    for (int i = 0; i < 4; i++)
      opts.push_back({comb4[i][0], comb4[i][1], 0, 0});
  } else if (d == 0) {
    for (int i = 0; i < 8; i++)
      opts.push_back({comb8_neg[i][0], comb8_neg[i][1], comb8_neg[i][2], comb8_neg[i][3]});
  } else {
    for (int i = 0; i < 8; i++)
      opts.push_back({comb8_pos[i][0], comb8_pos[i][1], comb8_pos[i][2], comb8_pos[i][3]});
  }
  return opts;
}

// Exhaustive 1-pair greedy descent + 2-pair check around (best_cd, best_ab).
// Returns true iff coupled cost 0 reached (full NPAF=0 solution). Mutates the
// states in place: any 1-pair improvement is kept; 2-pair only applied if it
// reaches 0 (we don't keep non-zero 2-pair improvements because the next BCD
// round would likely just undo them).
static bool endgame_polish(CDState &best_cd, ABState &best_ab,
                           int sig_a, int sig_b, int sig_c, int sig_d,
                           int n, int n1) {
  g_polish_attempts.fetch_add(1, memory_order_relaxed);

  int best_cost = compute_coupled_cost(best_cd, best_ab, sig_a, sig_b, sig_c, sig_d, n, n1);
  if (best_cost == 0) {
    g_polish_solutions.fetch_add(1, memory_order_relaxed);
    return true;
  }
  int initial_cost = best_cost;

  int cd_positions = (n % 2 == 1) ? (n / 2 + 1) : (n / 2);
  int ab_positions = (n1 % 2 == 1) ? (n1 / 2 + 1) : (n1 / 2);

  // ===== 1-pair greedy descent (alternating CD and AB) =====
  bool improved_this_pass = true;
  while (improved_this_pass && best_cost > 0) {
    improved_this_pass = false;
    // CD 1-pair
    for (int d = 0; d < cd_positions; d++) {
      auto opts = get_cd_options(n, d);
      CDState save = best_cd;
      for (const auto &opt : opts) {
        best_cd = save;
        apply_cd_pair(best_cd, n, d, opt.cL, opt.dL, opt.cR, opt.dR);
        int c = compute_coupled_cost(best_cd, best_ab, sig_a, sig_b, sig_c, sig_d, n, n1);
        if (c < best_cost) {
          best_cost = c;
          save = best_cd;  // promote
          improved_this_pass = true;
          if (c == 0) {
            g_polish_solutions.fetch_add(1, memory_order_relaxed);
            g_polish_improvements.fetch_add(1, memory_order_relaxed);
            return true;
          }
        }
      }
      best_cd = save;
    }
    // AB 1-pair
    for (int d = 0; d < ab_positions; d++) {
      auto opts = get_ab_options(n1, d);
      ABState save = best_ab;
      for (const auto &opt : opts) {
        best_ab = save;
        apply_ab_pair(best_ab, n1, d, opt.aL, opt.bL, opt.aR, opt.bR);
        int c = compute_coupled_cost(best_cd, best_ab, sig_a, sig_b, sig_c, sig_d, n, n1);
        if (c < best_cost) {
          best_cost = c;
          save = best_ab;
          improved_this_pass = true;
          if (c == 0) {
            g_polish_solutions.fetch_add(1, memory_order_relaxed);
            g_polish_improvements.fetch_add(1, memory_order_relaxed);
            return true;
          }
        }
      }
      best_ab = save;
    }
  }

  // ===== 2-pair check (CD+CD, AB+AB, CD+AB) — only accept if reaches 0 =====
  CDState save_cd = best_cd;
  ABState save_ab = best_ab;

  // CD+CD
  for (int d1 = 0; d1 < cd_positions && best_cost > 0; d1++) {
    auto opts1 = get_cd_options(n, d1);
    for (const auto &o1 : opts1) {
      best_cd = save_cd;
      apply_cd_pair(best_cd, n, d1, o1.cL, o1.dL, o1.cR, o1.dR);
      CDState after_d1 = best_cd;
      for (int d2 = d1 + 1; d2 < cd_positions; d2++) {
        auto opts2 = get_cd_options(n, d2);
        for (const auto &o2 : opts2) {
          best_cd = after_d1;
          apply_cd_pair(best_cd, n, d2, o2.cL, o2.dL, o2.cR, o2.dR);
          int c = compute_coupled_cost(best_cd, best_ab, sig_a, sig_b, sig_c, sig_d, n, n1);
          if (c == 0) {
            g_polish_solutions.fetch_add(1, memory_order_relaxed);
            g_polish_improvements.fetch_add(1, memory_order_relaxed);
            return true;
          }
        }
      }
    }
  }
  best_cd = save_cd;

  // AB+AB
  for (int d1 = 0; d1 < ab_positions && best_cost > 0; d1++) {
    auto opts1 = get_ab_options(n1, d1);
    for (const auto &o1 : opts1) {
      best_ab = save_ab;
      apply_ab_pair(best_ab, n1, d1, o1.aL, o1.bL, o1.aR, o1.bR);
      ABState after_d1 = best_ab;
      for (int d2 = d1 + 1; d2 < ab_positions; d2++) {
        auto opts2 = get_ab_options(n1, d2);
        for (const auto &o2 : opts2) {
          best_ab = after_d1;
          apply_ab_pair(best_ab, n1, d2, o2.aL, o2.bL, o2.aR, o2.bR);
          int c = compute_coupled_cost(best_cd, best_ab, sig_a, sig_b, sig_c, sig_d, n, n1);
          if (c == 0) {
            g_polish_solutions.fetch_add(1, memory_order_relaxed);
            g_polish_improvements.fetch_add(1, memory_order_relaxed);
            return true;
          }
        }
      }
    }
  }
  best_ab = save_ab;

  // CD+AB cross
  for (int dC = 0; dC < cd_positions && best_cost > 0; dC++) {
    auto optsC = get_cd_options(n, dC);
    for (const auto &oC : optsC) {
      best_cd = save_cd;
      apply_cd_pair(best_cd, n, dC, oC.cL, oC.dL, oC.cR, oC.dR);
      for (int dA = 0; dA < ab_positions; dA++) {
        auto optsA = get_ab_options(n1, dA);
        for (const auto &oA : optsA) {
          best_ab = save_ab;
          apply_ab_pair(best_ab, n1, dA, oA.aL, oA.bL, oA.aR, oA.bR);
          int c = compute_coupled_cost(best_cd, best_ab, sig_a, sig_b, sig_c, sig_d, n, n1);
          if (c == 0) {
            g_polish_solutions.fetch_add(1, memory_order_relaxed);
            g_polish_improvements.fetch_add(1, memory_order_relaxed);
            return true;
          }
        }
      }
    }
  }
  best_cd = save_cd;
  best_ab = save_ab;

  if (best_cost < initial_cost)
    g_polish_improvements.fetch_add(1, memory_order_relaxed);
  return false;
}

bool solve_AB_SA(int n1, int ta, int tb, const int *cd_full,
                 ABState &best_state, mt19937 &rng, int sig_idx) {
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

    // Re-randomize (or warm-start from champion) for restarts after the first.
    if (restart > 0) {
      bool used_champion = false;
      if (sig_idx >= 0 && (int)g_ab_champ.size() > sig_idx) {
        int ch_cost;
#pragma omp critical(ab_champ)
        ch_cost = g_ab_champ[sig_idx].cost;
        if (ch_cost < INT_MAX && prob(rng) < 0.3) {
#pragma omp critical(ab_champ)
          curr = g_ab_champ[sig_idx].state;
          // Champion's corr is the AB self-correlation (depends only on A,B).
          // Recompute cost against the *current* cd_full (different per CD success).
          current_cost = curr.cost(ta, tb, n1, cd_full);
          used_champion = true;
          g_ab_champion_hits.fetch_add(1, memory_order_relaxed);
        }
      }
      if (!used_champion) {
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

    // Push to per-signature AB champion (best so far for this signature
    // across all threads). Mirrors the CD champion update pattern.
    if (sig_idx >= 0 && (int)g_ab_champ.size() > sig_idx) {
#pragma omp critical(ab_champ)
      {
        if (best_cost < g_ab_champ[sig_idx].cost) {
          g_ab_champ[sig_idx].cost = best_cost;
          g_ab_champ[sig_idx].state = best_state;
        }
      }
    }

    if (best_cost == 0)
      return true;
  }

  update_min_atomic(g_best_ab_cost, best_cost);

  // Plateau diagnostic: record which shifts dominate the residual and
  // which cost bucket this attempt terminated in. Only sample attempts
  // that ended in a typical plateau range (skip wildly bad terminations).
  if (best_cost > 0 && best_cost < 64) {
    int sum_diff = abs(best_state.sum_a - ta) + abs(best_state.sum_b - tb);
    long long npaf_pen = 0;
    for (int s = 1; s < ms; s++) {
      int r = best_state.corr[s] + cd_full[s];
      if (r != 0) {
        int ar = r < 0 ? -r : r;
        npaf_pen += ar;
        g_ab_shift_residual[s].fetch_add(ar, memory_order_relaxed);
      }
    }
    g_ab_sum_residual.fetch_add(sum_diff, memory_order_relaxed);
    g_ab_npaf_residual.fetch_add(npaf_pen, memory_order_relaxed);
    g_ab_term_hist[best_cost].fetch_add(1, memory_order_relaxed);
    g_ab_terminations.fetch_add(1, memory_order_relaxed);
  }

  // Commit E: per-sig best AB tracking. Distinguishes "all sigs floor at 8" (search
  // limit) from "sig X gets to 0 while others plateau" (focus compute on X).
  if (sig_idx >= 0 && sig_idx < kMaxSigs)
    update_min_atomic(g_sig_best_ab[sig_idx], best_cost);

  return best_cost == 0;
}

int main(int argc, char **argv) {
  if (argc < 2) {
    cerr << "Usage: " << argv[0] << " <n> [seed_offset] [a,b,c,d]" << endl
         << "  a,b,c,d (optional): lock signature for diagnostic single-sig run"
         << endl;
    return 1;
  }
  int n = atoi(argv[1]);
  int seed_offset = (argc >= 3) ? atoi(argv[2]) : 0;
  // Optional argv[3] = "a,b,c,d" to lock signature selection to a single sig.
  // Diagnostic only: prove the SA can find a known solution (e.g. BS(43,42)
  // with sig 7,11,0,0) when given the right sig — separates SA bottleneck
  // from signature-selection bottleneck.
  int lock_a = 0, lock_b = 0, lock_c = 0, lock_d = 0;
  bool lock_sig = false;
  if (argc >= 4) {
    int parsed = sscanf(argv[3], "%d,%d,%d,%d",
                        &lock_a, &lock_b, &lock_c, &lock_d);
    if (parsed != 4) {
      cerr << "ERROR: --lock-sig requires 'a,b,c,d' format (got '"
           << argv[3] << "')" << endl;
      return 1;
    }
    lock_sig = true;
  }
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
  cout << "Loaded " << sigs.size() << " valid sum signatures." << endl;

  int locked_sig_idx = -1;
  if (lock_sig) {
    for (int i = 0; i < (int)sigs.size(); i++) {
      if (sigs[i].a == lock_a && sigs[i].b == lock_b &&
          sigs[i].c == lock_c && sigs[i].d == lock_d) {
        locked_sig_idx = i;
        break;
      }
    }
    if (locked_sig_idx < 0) {
      cerr << "ERROR: lock-sig (" << lock_a << "," << lock_b << ","
           << lock_c << "," << lock_d
           << ") not found in enumerated sigs for n=" << n << endl;
      return 1;
    }
    cout << "*** SIGNATURE LOCKED to index " << locked_sig_idx
         << " = (" << lock_a << "," << lock_b << "," << lock_c << ","
         << lock_d << ") — DIAGNOSTIC MODE ***" << endl;
  }
  cout << endl;

  g_cd_champ.assign(sigs.size(), CDChampion{INT_MAX, {}});
  g_ab_champ.assign(sigs.size(), ABChampion{INT_MAX, {}});

  // Commit E: per-signature bestAB. Crash loudly if a future n exceeds the
  // static bound rather than silently overflowing.
  if ((int)sigs.size() > kMaxSigs) {
    cerr << "ERROR: sigs.size()=" << sigs.size()
         << " exceeds kMaxSigs=" << kMaxSigs
         << "; bump kMaxSigs in wz_sa_v8.cpp" << endl;
    return 1;
  }
  for (int i = 0; i < (int)sigs.size(); i++)
    g_sig_best_ab[i].store(INT_MAX, memory_order_relaxed);

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
      int si = (locked_sig_idx >= 0)
                   ? locked_sig_idx
                   : uniform_int_distribution<>(0, sigs.size() - 1)(rng);
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

        ABState best_ab;
        bool have_ab = false;   // best_ab holds a valid state
        bool found_ab = false;  // full NPAF=0 solution reached
        const int kAbTriesInit = 4;

        // One AB pass against the current cd_full. Keeps the best AB across
        // tries (solve_AB_SA overwrites its out-param, so we compare costs).
        auto ab_pass = [&]() {
          for (int t = 0; t < kAbTriesInit && !found_ab &&
                          !g_found.load(memory_order_relaxed);
               t++) {
            ABState trial;
            g_ab_attempts.fetch_add(1, memory_order_relaxed);
            if (solve_AB_SA(n1, sig.a, sig.b, cd_full, trial, rng, si)) {
              best_ab = trial;
              have_ab = true;
              found_ab = true;
              return;
            }
            if (!have_ab ||
                trial.cost(sig.a, sig.b, n1, cd_full) <
                    best_ab.cost(sig.a, sig.b, n1, cd_full)) {
              best_ab = trial;
              have_ab = true;
            }
          }
        };

        // Initial AB pass against the sum-correct warm-start CD.
        ab_pass();

        // === Alternating refinement (the real coupled solve) ===
        // BCD on the TRUE coupled objective Sum|corr_CD[s]+corr_AB[s]|: freeze
        // AB, move CD; freeze CD, move AB; repeat. Each half-step is guarded
        // so it never regresses the coupled cost. When the alternation stalls
        // (consecutive rounds with no improvement) we perturb CD by flipping
        // 2-3 random pairs — same k-pair kick the inner SAs already use, but
        // lifted up to the BCD level to escape coupled local minima (cf. the
        // bestAB=8/12 plateau Commit C produced on BS(28)/BS(34)).
        const int kRefineRounds = 16;
        int prev_coupled = INT_MAX;
        int stall_count = 0;
        int kick_level = 0;  // Commit E: escalating kick magnitude
        uniform_int_distribution<> cd_d_dist(
            0, (n % 2 == 1) ? n / 2 : n / 2 - 1);
        uniform_int_distribution<> ab_d_dist(0, (n1 - 1) / 2);
        for (int round = 0;
             round < kRefineRounds && !found_ab && have_ab &&
             !g_found.load(memory_order_relaxed);
             round++) {
          g_refine_rounds.fetch_add(1, memory_order_relaxed);

          // Freeze AB: ab_full[s] = autocorrelation of the current best AB.
          int ab_full[128] = {0};
          for (int s = 1; s < ms; s++)
            for (int i = 0; i < n1 - s; i++)
              ab_full[s] += best_ab.A[i] * best_ab.A[i + s] +
                            best_ab.B[i] * best_ab.B[i + s];

          // CD step: move CD against the frozen AB on the coupled objective.
          // sig_idx = -1: the CD champion pool is keyed to the warm-start
          // objective, so it must not be mixed into refinement scoring.
          CDState refined_cd;
          bool cd_solved =
              solve_CD_SA(n, n1, sig.c, sig.d, refined_cd, rng, -1, ab_full);
          int refined_coupled =
              refined_cd.cost(sig.c, sig.d, n1, n, ab_full);
          int incumbent_coupled =
              best_cd.cost(sig.c, sig.d, n1, n, ab_full);
          if (cd_solved || refined_coupled < incumbent_coupled) {
            best_cd = refined_cd;
          }
          // Commit E: per-sig min (CD-side).
          if (si >= 0 && si < kMaxSigs)
            update_min_atomic(g_sig_best_ab[si],
                              min(refined_coupled, incumbent_coupled));
          if (cd_solved) {
            for (int s = 1; s < ms; s++) {
              cd_full[s] = 0;
              for (int k = 0; k < n - s; k++)
                cd_full[s] += best_cd.C[k] * best_cd.C[k + s] +
                              best_cd.D[k] * best_cd.D[k + s];
            }
            found_ab = true;
            break;
          }

          // Recompute cd_full from best_cd for the AB step.
          for (int s = 1; s < ms; s++) {
            cd_full[s] = 0;
            for (int k = 0; k < n - s; k++)
              cd_full[s] += best_cd.C[k] * best_cd.C[k + s] +
                            best_cd.D[k] * best_cd.D[k + s];
          }

          // AB step: solve AB against (possibly new) best_cd.
          ab_pass();
          if (found_ab) break;

          // End-of-round coupled cost: sum |best_cd.corr[s] + corr_AB[s]|
          // where corr_AB is recomputed from the round-end best_ab.
          int end_coupled = 0;
          for (int s = 1; s < ms; s++) {
            int v = 0;
            for (int i = 0; i < n1 - s; i++)
              v += best_ab.A[i] * best_ab.A[i + s] +
                   best_ab.B[i] * best_ab.B[i + s];
            end_coupled += abs(best_cd.corr[s] + v);
          }
          if (si >= 0 && si < kMaxSigs)
            update_min_atomic(g_sig_best_ab[si], end_coupled);

          // Commit F: when SA-BCD gets us close (coupled cost below threshold),
          // try an exhaustive 1-pair greedy descent + 2-pair check. Polish does
          // not need RNG and is deterministic — if no 2-flip neighbor reaches 0,
          // we know the current basin is at least 3 pair-flips deep.
          if (end_coupled > 0 && end_coupled <= kPolishThreshold) {
            // best_ab.corr is currently the AB self-correlation (set inside the
            // ab_pass via solve_AB_SA's last write). end_coupled was computed
            // from a fresh AB autocorr above, so we recompute best_ab.corr from
            // scratch to keep it in sync with what compute_coupled_cost uses.
            memset(best_ab.corr, 0, sizeof(best_ab.corr));
            for (int s = 1; s < ms; s++)
              for (int i = 0; i < n1 - s; i++)
                best_ab.corr[s] += best_ab.A[i] * best_ab.A[i + s] +
                                   best_ab.B[i] * best_ab.B[i + s];
            if (endgame_polish(best_cd, best_ab, sig.a, sig.b, sig.c, sig.d,
                               n, n1)) {
              // Recompute cd_full for the verification path below.
              for (int s = 1; s < ms; s++) {
                cd_full[s] = 0;
                for (int k = 0; k < n - s; k++)
                  cd_full[s] += best_cd.C[k] * best_cd.C[k + s] +
                                best_cd.D[k] * best_cd.D[k + s];
              }
              found_ab = true;
              break;
            }
          }

          // Stall detector: this round failed to improve the coupled cost.
          if (end_coupled >= prev_coupled) {
            stall_count++;
          } else {
            stall_count = 0;
            kick_level = 0;  // improvement -> reset escalation
          }
          prev_coupled = end_coupled;

          // Kick: when stalled, perturb either best_cd or best_ab at end of
          // round so the NEXT round's first block-step sees a different
          // target. Escalates the kick size if successive kicks don't help.
          if (stall_count >= 2) {
            int k_kick = 2 + kick_level +
                         uniform_int_distribution<>(0, 1)(rng);
            bool kick_ab = (uniform_int_distribution<>(0, 1)(rng) == 1);
            if (kick_ab) {
              // Perturb best_ab: same comb mutation as solve_AB_SA's in-SA kick.
              for (int kk = 0; kk < k_kick; kk++) {
                int dk = ab_d_dist(rng);
                int lk = dk, rk = n1 - 1 - dk;
                if (lk == rk) {
                  const int *m_ptr =
                      comb4[uniform_int_distribution<>(0, 3)(rng)];
                  best_ab.sum_a += m_ptr[0] - best_ab.A[lk];
                  best_ab.sum_b += m_ptr[1] - best_ab.B[lk];
                  best_ab.A[lk] = m_ptr[0];
                  best_ab.B[lk] = m_ptr[1];
                } else {
                  const int *c_ptr =
                      (dk == 0)
                          ? comb8_neg[uniform_int_distribution<>(0, 7)(rng)]
                          : comb8_pos[uniform_int_distribution<>(0, 7)(rng)];
                  best_ab.sum_a += (c_ptr[0] + c_ptr[2]) -
                                   (best_ab.A[lk] + best_ab.A[rk]);
                  best_ab.sum_b += (c_ptr[1] + c_ptr[3]) -
                                   (best_ab.B[lk] + best_ab.B[rk]);
                  best_ab.A[lk] = c_ptr[0];
                  best_ab.B[lk] = c_ptr[1];
                  best_ab.A[rk] = c_ptr[2];
                  best_ab.B[rk] = c_ptr[3];
                }
              }
              memset(best_ab.corr, 0, sizeof(best_ab.corr));
              for (int s = 1; s < ms; s++)
                for (int i = 0; i < n1 - s; i++)
                  best_ab.corr[s] += best_ab.A[i] * best_ab.A[i + s] +
                                     best_ab.B[i] * best_ab.B[i + s];
            } else {
              // Perturb best_cd: same as Commit D.
              for (int kk = 0; kk < k_kick; kk++) {
                int dk = cd_d_dist(rng);
                if (n % 2 == 1 && dk == n / 2) {
                  int mid = n / 2;
                  const int *m_ptr =
                      comb4[uniform_int_distribution<>(0, 3)(rng)];
                  best_cd.sum_c += m_ptr[0] - best_cd.C[mid];
                  best_cd.sum_d += m_ptr[1] - best_cd.D[mid];
                  best_cd.C[mid] = m_ptr[0];
                  best_cd.D[mid] = m_ptr[1];
                } else {
                  int lk = dk, rk = n - 1 - dk;
                  const int *c_ptr =
                      (dk == 0)
                          ? comb16[uniform_int_distribution<>(0, 15)(rng)]
                          : comb8_pos[uniform_int_distribution<>(0, 7)(rng)];
                  best_cd.sum_c +=
                      (c_ptr[0] + c_ptr[2]) -
                      (best_cd.C[lk] + best_cd.C[rk]);
                  best_cd.sum_d +=
                      (c_ptr[1] + c_ptr[3]) -
                      (best_cd.D[lk] + best_cd.D[rk]);
                  best_cd.C[lk] = c_ptr[0];
                  best_cd.D[lk] = c_ptr[1];
                  best_cd.C[rk] = c_ptr[2];
                  best_cd.D[rk] = c_ptr[3];
                }
              }
              memset(best_cd.corr, 0, sizeof(best_cd.corr));
              for (int s = 1; s < ms; s++)
                for (int k = 0; k < n - s; k++)
                  best_cd.corr[s] += best_cd.C[k] * best_cd.C[k + s] +
                                     best_cd.D[k] * best_cd.D[k + s];
            }
            g_refine_kicks.fetch_add(1, memory_order_relaxed);
            if (kick_level > 0)
              g_refine_kick_escalations.fetch_add(1, memory_order_relaxed);
            kick_level = min(kick_level + 1, 4);  // cap at +4 -> kick=6-7
            stall_count = 0;
            prev_coupled = INT_MAX;  // post-kick clean slate
          }
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
        long long abterm = g_ab_terminations.load(memory_order_relaxed);
        long long absum = g_ab_sum_residual.load(memory_order_relaxed);
        long long abnpaf = g_ab_npaf_residual.load(memory_order_relaxed);
        long long abch = g_ab_champion_hits.load(memory_order_relaxed);
        long long absk = g_ab_attempts_skipped.load(memory_order_relaxed);
        long long refr = g_refine_rounds.load(memory_order_relaxed);
        long long refk = g_refine_kicks.load(memory_order_relaxed);
        long long resc = g_refine_kick_escalations.load(memory_order_relaxed);
        long long pola = g_polish_attempts.load(memory_order_relaxed);
        long long poli = g_polish_improvements.load(memory_order_relaxed);
        long long pols = g_polish_solutions.load(memory_order_relaxed);

        // Commit E: top-5 signatures by lowest coupled cost ever seen.
        pair<int, int> top_sigs[5] = {{INT_MAX, -1}, {INT_MAX, -1},
                                       {INT_MAX, -1}, {INT_MAX, -1},
                                       {INT_MAX, -1}};
        for (int si2 = 0; si2 < (int)sigs.size() && si2 < kMaxSigs; si2++) {
          int v = g_sig_best_ab[si2].load(memory_order_relaxed);
          if (v < top_sigs[4].first) {
            top_sigs[4] = {v, si2};
            for (int k = 4; k > 0 && top_sigs[k].first < top_sigs[k - 1].first; k--)
              swap(top_sigs[k], top_sigs[k - 1]);
          }
        }

        // Top-5 shifts by accumulated residual.
        pair<long long, int> top_shifts[5] = {{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}};
        for (int s = 1; s < 128; s++) {
          long long v = g_ab_shift_residual[s].load(memory_order_relaxed);
          if (v > top_shifts[4].first) {
            top_shifts[4] = {v, s};
            for (int k = 4; k > 0 && top_shifts[k].first > top_shifts[k - 1].first; k--)
              swap(top_shifts[k], top_shifts[k - 1]);
          }
        }

        // Top-5 termination cost buckets by count.
        pair<long long, int> top_buckets[5] = {{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}};
        for (int c = 1; c < 256; c++) {
          long long v = g_ab_term_hist[c].load(memory_order_relaxed);
          if (v > top_buckets[4].first) {
            top_buckets[4] = {v, c};
            for (int k = 4; k > 0 && top_buckets[k].first > top_buckets[k - 1].first; k--)
              swap(top_buckets[k], top_buckets[k - 1]);
          }
        }

        cout << "[" << t << "s] epochs=" << current_total
             << " speed=" << speed
             << " bestCD=" << (bcd == INT_MAX ? -1 : bcd)
             << " bestAB=" << (bab == INT_MAX ? -1 : bab)
             << " CDok=" << cds << "/" << cda
             << " ABtry=" << aba
             << " ABterm=" << abterm
             << " ABchamp=" << abch
             << " ABskip=" << absk
             << " refine=" << refr
             << " kicks=" << refk
             << " kesc=" << resc
             << " polish=" << pola << "/" << poli << "/" << pols
             << " ab_resid=sum:" << absum << "/npaf:" << abnpaf
             << " shifts_top=";
        for (int k = 0; k < 5; k++) {
          if (top_shifts[k].first == 0) break;
          if (k) cout << ",";
          cout << "s" << top_shifts[k].second << ":" << top_shifts[k].first;
        }
        cout << " term_hist=";
        for (int k = 0; k < 5; k++) {
          if (top_buckets[k].first == 0) break;
          if (k) cout << ",";
          cout << top_buckets[k].second << ":" << top_buckets[k].first;
        }
        cout << " sig_best=";
        for (int k = 0; k < 5; k++) {
          if (top_sigs[k].second < 0) break;
          if (k) cout << ",";
          cout << top_sigs[k].second << ":" << top_sigs[k].first;
        }
        cout << "\n" << flush;
      }
    }
  }

  return g_found.load() ? 0 : 1;
}
