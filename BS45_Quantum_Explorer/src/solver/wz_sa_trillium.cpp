/*
 * Wang-Zhu BS Solver — SA over Theorem 2.2 Manifold
 * CP493 - Directed Research - Daniel Gordon
 *
 * TRILLIUM SUPERCOMPUTER VERSION
 * Optimized for Trillium (192-core AMD EPYC nodes).
 * Features: adaptive restart SA, reheating, faster cooling.
 * Uses all available cores.
 *
 * Compile on Trillium:
 *   module load StdEnv/2023 gcc/12.3
 *   g++ -O3 -march=native -std=c++17 -fopenmp -o wz_sa
 * src/solver/wz_sa_trillium.cpp
 */

#include <algorithm>
#include <atomic>
#include <chrono>
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
static atomic<int> g_best_cost{999999};
static atomic<int> g_ab_best_cost{999999};
static int g_sol[4][128];
static int G_N;
static Clock::time_point G_T0;

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
  double initial_temp = 100.0;
  double cooling_rate = 0.999995; // Deep exploration cooling
  int iterations = 2000000;       // Long cycles for thorough search
  int restarts = 10;              // Fewer restarts, each goes deep
  int reheat_threshold = 500000;  // Patient stuck detection
  double reheat_ratio = 0.5;      // Reheat to 50%
};

// Separate SA parameters tuned for the AB problem, which is much smaller
// than CD (n1=21 vs n=41). Lower temp prevents accepting bad moves;
// shorter cycles with more restarts gives better coverage.
struct ABSAParams {
  double initial_temp = 10.0;     // AB is smaller, needs lower temp
  double cooling_rate = 0.99999;  // Cool faster
  int iterations = 500000;        // Shorter cycles
  int restarts = 40;              // More restarts for diversity
  int reheat_threshold = 100000;  // Reheat if stuck
  double reheat_ratio = 0.3;      // Mild reheat
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
                 mt19937 &rng) {
  CDState curr;
  memset(curr.C, 0, sizeof(curr.C));
  memset(curr.D, 0, sizeof(curr.D));
  memset(curr.corr, 0, sizeof(curr.corr));
  curr.sum_c = 0;
  curr.sum_d = 0;

  // Initialize random adhering to rules
  for (int d = 0; d < n / 2; d++) {
    int left = d;
    int right = n - 1 - d;
    const int *c_ptr;
    c_ptr = comb16[uniform_int_distribution<>(0, 15)(rng)];

    curr.C[left] = c_ptr[0];
    curr.D[left] = c_ptr[1];
    curr.C[right] = c_ptr[2];
    curr.D[right] = c_ptr[3];
    curr.sum_c += c_ptr[0] + c_ptr[2];
    curr.sum_d += c_ptr[1] + c_ptr[3];
  }
  // BUG FIX: Handle middle element for odd-length sequences
  if (n % 2 != 0) {
    int mid = n / 2;
    const int *c_ptr = comb4[uniform_int_distribution<>(0, 3)(rng)];
    curr.C[mid] = c_ptr[0];
    curr.D[mid] = c_ptr[1];
    curr.sum_c += c_ptr[0];
    curr.sum_d += c_ptr[1];
  }
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
  // Include middle element in mutation range for odd n
  uniform_int_distribution<> d_dist(0, (n % 2 != 0) ? n / 2 : n / 2 - 1);

  // Adaptive restart loop: run multiple short SA cycles
  for (int restart = 0; restart < sa.restarts; restart++) {
    if (g_found.load(memory_order_relaxed))
      return false;

    // Re-randomize state for each restart (except first which uses initial)
    if (restart > 0) {
      memset(curr.C, 0, sizeof(curr.C));
      memset(curr.D, 0, sizeof(curr.D));
      memset(curr.corr, 0, sizeof(curr.corr));
      curr.sum_c = 0;
      curr.sum_d = 0;
      for (int d = 0; d < n / 2; d++) {
        int left = d, right = n - 1 - d;
        const int *c_ptr = comb16[uniform_int_distribution<>(0, 15)(rng)];
        curr.C[left] = c_ptr[0];
        curr.D[left] = c_ptr[1];
        curr.C[right] = c_ptr[2];
        curr.D[right] = c_ptr[3];
        curr.sum_c += c_ptr[0] + c_ptr[2];
        curr.sum_d += c_ptr[1] + c_ptr[3];
      }
      // BUG FIX: Handle middle element for odd-length sequences on restart
      if (n % 2 != 0) {
        int mid = n / 2;
        const int *c_ptr = comb4[uniform_int_distribution<>(0, 3)(rng)];
        curr.C[mid] = c_ptr[0];
        curr.D[mid] = c_ptr[1];
        curr.sum_c += c_ptr[0];
        curr.sum_d += c_ptr[1];
      }
      for (int s = 1; s < ms; s++)
        for (int i = 0; i < n - s; i++)
          curr.corr[s] += curr.C[i] * curr.C[i + s] + curr.D[i] * curr.D[i + s];
      current_cost = curr.cost(tc, td, n1, n);
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

      int d = d_dist(rng);
      int left = d;
      int right = n - 1 - d;

      int oldC_L = curr.C[left], oldD_L = curr.D[left];
      int oldC_R = curr.C[right], oldD_R = curr.D[right];
      int nC_L, nD_L, nC_R, nD_R;

      // Handle middle element (left == right) for odd n
      if (left == right) {
        const int *c_ptr = comb4[uniform_int_distribution<>(0, 3)(rng)];
        nC_L = c_ptr[0];
        nD_L = c_ptr[1];
        nC_R = nC_L;
        nD_R = nD_L;
        if (oldC_L == nC_L && oldD_L == nD_L)
          continue;
      } else {
        const int *c_ptr = comb16[uniform_int_distribution<>(0, 15)(rng)];
        nC_L = c_ptr[0];
        nD_L = c_ptr[1];
        nC_R = c_ptr[2];
        nD_R = c_ptr[3];
        if (oldC_L == nC_L && oldD_L == nD_L && oldC_R == nC_R && oldD_R == nD_R)
          continue;
      }

      int old_sum_c = curr.sum_c;
      int old_sum_d = curr.sum_d;
      if (left == right) {
        curr.sum_c += nC_L - oldC_L;
        curr.sum_d += nD_L - oldD_L;
      } else {
        curr.sum_c += (nC_L + nC_R) - (oldC_L + oldC_R);
        curr.sum_d += (nD_L + nD_R) - (oldD_L + oldD_R);
      }

      int delta_corr[128] = {0};

      for (int s = 1; s < ms; s++) {
        if (left - s >= 0)
          delta_corr[s] -=
              curr.C[left - s] * oldC_L + curr.D[left - s] * oldD_L;
        if (left + s < n)
          delta_corr[s] -=
              oldC_L * curr.C[left + s] + oldD_L * curr.D[left + s];
        if (left != right) {
          if (right - s >= 0)
            delta_corr[s] -=
                curr.C[right - s] * oldC_R + curr.D[right - s] * oldD_R;
          if (right + s < n)
            delta_corr[s] -=
                oldC_R * curr.C[right + s] + oldD_R * curr.D[right + s];
          if (right - left == s)
            delta_corr[s] += oldC_L * oldC_R + oldD_L * oldD_R;
        }
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
        if (left != right) {
          if (right - s >= 0)
            delta_corr[s] +=
                curr.C[right - s] * nC_R + curr.D[right - s] * nD_R;
          if (right + s < n)
            delta_corr[s] +=
                nC_R * curr.C[right + s] + nD_R * curr.D[right + s];
          if (right - left == s)
            delta_corr[s] -= nC_L * nC_R + nD_L * nD_R;
        }
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

    if (best_cost == 0 && hall_ok(best_state.C, n, best_state.D, n))
      return true;
  }

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
// Greedy hill-climbing: deterministically try every possible swap at every
// position. Repeats until no single-swap improvement is found.
static bool greedy_hill_climb_AB(int n1, int ta, int tb, const int *cd_full,
                                  ABState &state, int &cost_out) {
  int ms = max(n1, G_N);
  bool improved = true;
  while (improved) {
    improved = false;
    for (int d = 0; d <= (n1 - 1) / 2; d++) {
      int left = d, right = n1 - 1 - d;
      int oldA_L = state.A[left], oldB_L = state.B[left];
      int oldA_R = state.A[right], oldB_R = state.B[right];

      bool is_middle = (left == right);
      int num_combos = is_middle ? 4 : 16;

      int best_local_cost = cost_out;
      int best_combo = -1;
      for (int ci = 0; ci < num_combos; ci++) {
        int nA_L, nB_L, nA_R, nB_R;
        if (is_middle) {
          nA_L = comb4[ci][0]; nB_L = comb4[ci][1];
          nA_R = nA_L; nB_R = nB_L;
        } else {
          nA_L = comb16[ci][0]; nB_L = comb16[ci][1];
          nA_R = comb16[ci][2]; nB_R = comb16[ci][3];
        }
        if (nA_L == oldA_L && nB_L == oldB_L && nA_R == oldA_R && nB_R == oldB_R)
          continue;

        // Temporarily apply
        state.A[left] = nA_L; state.B[left] = nB_L;
        state.A[right] = nA_R; state.B[right] = nB_R;
        int old_sa = state.sum_a, old_sb = state.sum_b;
        if (is_middle) {
          state.sum_a += nA_L - oldA_L;
          state.sum_b += nB_L - oldB_L;
        } else {
          state.sum_a += (nA_L + nA_R) - (oldA_L + oldA_R);
          state.sum_b += (nB_L + nB_R) - (oldB_L + oldB_R);
        }
        // Full recompute of corr for correctness in greedy phase
        int saved_corr[128];
        memcpy(saved_corr, state.corr, sizeof(saved_corr));
        memset(state.corr, 0, sizeof(state.corr));
        for (int s = 1; s < ms; s++)
          for (int i = 0; i < n1 - s; i++)
            state.corr[s] += state.A[i] * state.A[i + s] + state.B[i] * state.B[i + s];
        int trial_cost = state.cost(ta, tb, n1, cd_full);

        if (trial_cost < best_local_cost) {
          best_local_cost = trial_cost;
          best_combo = ci;
        }

        // Revert
        state.A[left] = oldA_L; state.B[left] = oldB_L;
        state.A[right] = oldA_R; state.B[right] = oldB_R;
        state.sum_a = old_sa; state.sum_b = old_sb;
        memcpy(state.corr, saved_corr, sizeof(state.corr));
      }

      if (best_combo >= 0) {
        // Apply best swap permanently
        int nA_L, nB_L, nA_R, nB_R;
        if (is_middle) {
          nA_L = comb4[best_combo][0]; nB_L = comb4[best_combo][1];
          nA_R = nA_L; nB_R = nB_L;
        } else {
          nA_L = comb16[best_combo][0]; nB_L = comb16[best_combo][1];
          nA_R = comb16[best_combo][2]; nB_R = comb16[best_combo][3];
        }
        state.A[left] = nA_L; state.B[left] = nB_L;
        state.A[right] = nA_R; state.B[right] = nB_R;
        if (is_middle) {
          state.sum_a += nA_L - oldA_L;
          state.sum_b += nB_L - oldB_L;
        } else {
          state.sum_a += (nA_L + nA_R) - (oldA_L + oldA_R);
          state.sum_b += (nB_L + nB_R) - (oldB_L + oldB_R);
        }
        memset(state.corr, 0, sizeof(state.corr));
        for (int s = 1; s < ms; s++)
          for (int i = 0; i < n1 - s; i++)
            state.corr[s] += state.A[i] * state.A[i + s] + state.B[i] * state.B[i + s];
        cost_out = best_local_cost;
        improved = true;
        if (cost_out == 0) return true;
      }
    }
  }
  return cost_out == 0;
}

// Helper: apply a random perturbation to K positions of an ABState
static void perturb_AB(ABState &state, int n1, int K, mt19937 &rng) {
  uniform_int_distribution<> d_dist(0, (n1 - 1) / 2);
  int ms = max(n1, G_N);
  for (int p = 0; p < K; p++) {
    int d = d_dist(rng);
    int left = d, right = n1 - 1 - d;
    state.sum_a -= state.A[left] + (left != right ? state.A[right] : 0);
    state.sum_b -= state.B[left] + (left != right ? state.B[right] : 0);
    const int *c_ptr;
    if (left == right) {
      c_ptr = comb4[uniform_int_distribution<>(0, 3)(rng)];
      state.A[left] = c_ptr[0]; state.B[left] = c_ptr[1];
      state.sum_a += c_ptr[0]; state.sum_b += c_ptr[1];
    } else {
      c_ptr = comb16[uniform_int_distribution<>(0, 15)(rng)];
      state.A[left] = c_ptr[0]; state.B[left] = c_ptr[1];
      state.A[right] = c_ptr[2]; state.B[right] = c_ptr[3];
      state.sum_a += c_ptr[0] + c_ptr[2]; state.sum_b += c_ptr[1] + c_ptr[3];
    }
  }
  // Recompute correlations from scratch
  memset(state.corr, 0, sizeof(state.corr));
  for (int s = 1; s < ms; s++)
    for (int i = 0; i < n1 - s; i++)
      state.corr[s] += state.A[i] * state.A[i + s] + state.B[i] * state.B[i + s];
}

// Iterated Local Search: the key insight is that cost=40 is a 1-swap local
// minimum. No single position change can improve it. We need to change 2-4
// positions at once ("basin hop"), then re-climb from there.
// This is the standard approach for escaping these traps.
static bool iterated_local_search_AB(int n1, int ta, int tb, const int *cd_full,
                                      ABState &best_state, int &best_cost,
                                      mt19937 &rng) {
  // Try 50000 perturbation+climb cycles with varying perturbation sizes
  for (int attempt = 0; attempt < 50000; attempt++) {
    if (g_found.load(memory_order_relaxed)) return false;
    if (best_cost == 0) return true;

    ABState trial = best_state;
    // Cycle through perturbation sizes: 2,3,4,5,6,7 positions
    int perturb_size = 2 + (attempt % 6);
    perturb_AB(trial, n1, perturb_size, rng);

    int trial_cost = trial.cost(ta, tb, n1, cd_full);
    // Greedy-climb from perturbed state
    greedy_hill_climb_AB(n1, ta, tb, cd_full, trial, trial_cost);

    if (trial_cost < best_cost) {
      best_cost = trial_cost;
      best_state = trial;
      if (best_cost == 0) return true;
      // Reset attempt counter on improvement to keep searching from new basin
      attempt = 0;
    }
  }
  return best_cost == 0;
}

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
    c_ptr = comb16[uniform_int_distribution<>(0, 15)(rng)];
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

  ABSAParams sa;
  uniform_real_distribution<> prob(0.0, 1.0);
  uniform_int_distribution<> d_dist(0, (n1 - 1) / 2);

  // Adaptive restart loop
  for (int restart = 0; restart < sa.restarts; restart++) {
    if (g_found.load(memory_order_relaxed))
      return false;

    if (restart > 0) {
      if (restart % 2 == 0 && best_cost < 999999) {
        // WARM START: begin from best state found + random perturbations.
        // This is iterated local search — we preserve progress instead of
        // throwing it away. Much more effective when near a solution.
        curr = best_state;
        perturb_AB(curr, n1, max(2, n1 / 4), rng);
        current_cost = curr.cost(ta, tb, n1, cd_full);
      } else {
        // ODD RESTART: full random restart for diversity
        memset(curr.corr, 0, sizeof(curr.corr));
        curr.sum_a = 0;
        curr.sum_b = 0;
        for (int d = 0; d < n1 / 2; d++) {
          int left = d, right = n1 - 1 - d;
          const int *c_ptr = comb16[uniform_int_distribution<>(0, 15)(rng)];
          curr.A[left] = c_ptr[0]; curr.B[left] = c_ptr[1];
          curr.A[right] = c_ptr[2]; curr.B[right] = c_ptr[3];
          curr.sum_a += c_ptr[0] + c_ptr[2];
          curr.sum_b += c_ptr[1] + c_ptr[3];
        }
        if (n1 % 2 != 0) {
          int mid = n1 / 2;
          const int *c_ptr = comb4[uniform_int_distribution<>(0, 3)(rng)];
          curr.A[mid] = c_ptr[0]; curr.B[mid] = c_ptr[1];
          curr.sum_a += c_ptr[0]; curr.sum_b += c_ptr[1];
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
        const int *c_ptr = comb16[uniform_int_distribution<>(0, 15)(rng)];
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

    // Greedy hill-climbing phase after each SA restart
    if (best_cost > 0 && best_cost < 200) {
      ABState hc_state = best_state;
      int hc_cost = best_cost;
      greedy_hill_climb_AB(n1, ta, tb, cd_full, hc_state, hc_cost);
      if (hc_cost < best_cost) {
        best_cost = hc_cost;
        best_state = hc_state;
      }
    }
  }

  // ILS phase: SA found a local minimum. Now do basin-hopping:
  // perturb 2-4 positions, greedy-climb, repeat.
  // This escapes the cost=40 trap that single-swap can't break.
  if (best_cost > 0 && best_cost <= 100) {
    if (iterated_local_search_AB(n1, ta, tb, cd_full, best_state, best_cost, rng))
      return true;
  }

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
  thr = omp_get_max_threads(); // Trillium: use ALL 192 cores
  omp_set_num_threads(thr);
#endif

  cout << "========================================================" << endl;
  cout << "  BS(" << n1 << "," << n << ") — Thermodynamically Guided Solver"
       << endl;
  cout << "  Targeting NPAF=0 inside Theorem 2.2 Manifold" << endl;
  cout << "  [ Threads: " << thr << " | Seed offset: " << seed_offset << " ]" << endl;
  cout << "========================================================" << endl;

  G_T0 = Clock::now();
  auto sigs = get_sigs(n);
  cout << "Loaded " << sigs.size() << " valid sum signatures." << endl << endl;

  long long initial_epochs = 0;
  {
    ifstream state_in(".solver_state");
    if (state_in.is_open()) {
      state_in >> initial_epochs;
    }
  }

  // Search indefinitely, we are just hunting the minimum.
  long long global_tries = 0;

#pragma omp parallel reduction(+ : global_tries)
  {
    int tid = 0;
#ifdef _OPENMP
    tid = omp_get_thread_num();
#endif
    mt19937 rng(42 + tid * 1000 + time(NULL) + seed_offset * 100000);

    while (!g_found.load(memory_order_relaxed)) {
      // Pick a random signature to target so workers distribute load
      int si = uniform_int_distribution<>(0, sigs.size() - 1)(rng);
      auto &sig = sigs[si];

      CDState best_cd;
      bool found_cd = solve_CD_SA(n, n1, sig.c, sig.d, best_cd, rng);

      // Track global best cost for telemetry
      {
        int cd_cost = best_cd.cost(sig.c, sig.d, n1, n);
        int old_best = g_best_cost.load(memory_order_relaxed);
        while (cd_cost < old_best &&
               !g_best_cost.compare_exchange_weak(old_best, cd_cost,
                                                   memory_order_relaxed))
          ;
      }

      if (found_cd) {
        int cd_full[128] = {0};
        for (int s = 1; s < ms; s++) {
          for (int k = 0; k < n - s; k++)
            cd_full[s] += best_cd.C[k] * best_cd.C[k + s] +
                          best_cd.D[k] * best_cd.D[k + s];
        }

        // KEY FIX: Retry AB up to 50 times per valid CD pair.
        // Previously AB got only ONE attempt, wasting every successful CD find.
        for (int ab_attempt = 0;
             ab_attempt < 100 && !g_found.load(memory_order_relaxed);
             ab_attempt++) {

          ABState best_ab;
          bool found_ab = solve_AB_SA(n1, sig.a, sig.b, cd_full, best_ab, rng);

          // Track AB best cost for telemetry
          {
            int ab_cost = best_ab.cost(sig.a, sig.b, n1, cd_full);
            int old_ab = g_ab_best_cost.load(memory_order_relaxed);
            while (ab_cost < old_ab &&
                   !g_ab_best_cost.compare_exchange_weak(old_ab, ab_cost,
                                                         memory_order_relaxed))
              ;
          }

          if (found_ab) {
            bool valid = true;
            for (int s = 1; s < ms && valid; s++) {
              if (npaf_at(best_ab.A, best_ab.B, n1, best_cd.C, best_cd.D, n,
                          s) != 0)
                valid = false;
            }
            if (valid) {
              g_found.store(true);
#pragma omp critical
              {
                if (n >= 44)
                  cout << "\n*** WORLD RECORD DISCOVERY: FOUND BS(" << n1 << ","
                       << n << ") ***\n" << endl;
                else
                  cout << "\n*** REPRODUCTION CONFIRMED: FOUND BS(" << n1 << ","
                       << n << ") ***\n" << endl;

                cout << "A = {";
                for (int i = 0; i < n1; i++)
                  cout << best_ab.A[i] << (i < n1 - 1 ? "," : "");
                cout << "};" << endl;
                cout << "B = {";
                for (int i = 0; i < n1; i++)
                  cout << best_ab.B[i] << (i < n1 - 1 ? "," : "");
                cout << "};" << endl;
                cout << "C = {";
                for (int i = 0; i < n; i++)
                  cout << best_cd.C[i] << (i < n - 1 ? "," : "");
                cout << "};" << endl;
                cout << "D = {";
                for (int i = 0; i < n; i++)
                  cout << best_cd.D[i] << (i < n - 1 ? "," : "");
                cout << "};" << endl;

                double t =
                    chrono::duration<double>(Clock::now() - G_T0).count();
                cout << "\nTime: " << t << "s" << endl;
              }
              break; // exit AB retry loop
            }
          }
        } // end AB retry loop
      }

      global_tries++;
      if (tid == 0) {
        long long current_total = initial_epochs + (global_tries * thr);
        double t = chrono::duration<double>(Clock::now() - G_T0).count();
        double speed = (t > 0) ? ((global_tries * thr) / t) : 0.0;
        int gbest = g_best_cost.load(memory_order_relaxed);
        int ab_best = g_ab_best_cost.load(memory_order_relaxed);
        cout << "[" << t << "s] Epochs: " << current_total
             << " Speed: " << speed
             << " [CD cost: " << gbest
             << " | AB cost: " << (ab_best == 999999 ? -1 : ab_best) << "]\n"
             << flush;

        ofstream state_out(".solver_state");
        if (state_out.is_open()) {
          state_out << current_total << endl;
        }
      }
    }
  }

  return g_found.load() ? 0 : 1;
}
