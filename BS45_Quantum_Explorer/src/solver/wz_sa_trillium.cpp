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
  double initial_temp = 30.0;      // Lower start since swaps don't touch sums
  double cooling_rate = 0.9999985; // Slower cooling for deeper exploration
  int iterations = 8000000;        // More iterations per restart
  int restarts = 15;
  int reheat_threshold = 1500000;
  double reheat_ratio = 0.5;
};

// ===================================
// Joint 4-Sequence Solver
// ===================================
// Optimizes A,B (length n1) and C,D (length n) simultaneously.
// Cost = sum_s |NPAF(s)| + 5*(sum penalties)
// Mutation: flip one random element from any of the 4 sequences.
// This avoids the two-phase failure mode where CD produces values
// that AB cannot cancel.

struct JointState {
  int A[128], B[128], C[128], D[128];
  int sum_a, sum_b, sum_c, sum_d;
  int npaf[128]; // npaf[s] = PAF_A(s) + PAF_B(s) + PAF_C(s) + PAF_D(s)

  int cost(int ta, int tb, int tc, int td, int ms) const {
    int pen = 0;
    for (int s = 1; s < ms; s++)
      pen += abs(npaf[s]);
    return pen + 5 * (abs(sum_a - ta) + abs(sum_b - tb) +
                      abs(sum_c - tc) + abs(sum_d - td));
  }
};

// Greedy hill-climb: try flipping each element of all 4 sequences
static bool greedy_hill_climb_joint(int n1, int n, int ta, int tb, int tc,
                                     int td, int ms, JointState &state,
                                     int &cost_out) {
  bool improved = true;
  while (improved) {
    improved = false;
    // Try all elements of A, B, C, D
    for (int seq = 0; seq < 4; seq++) {
      int *arr;
      int *sum_ptr;
      int len;
      if (seq == 0) { arr = state.A; sum_ptr = &state.sum_a; len = n1; }
      else if (seq == 1) { arr = state.B; sum_ptr = &state.sum_b; len = n1; }
      else if (seq == 2) { arr = state.C; sum_ptr = &state.sum_c; len = n; }
      else { arr = state.D; sum_ptr = &state.sum_d; len = n; }

      // Determine the paired array for NPAF computation
      int *pair_arr;
      int pair_len;
      if (seq == 0) { pair_arr = state.A; pair_len = n1; }
      else if (seq == 1) { pair_arr = state.B; pair_len = n1; }
      else if (seq == 2) { pair_arr = state.C; pair_len = n; }
      else { pair_arr = state.D; pair_len = n; }

      // Phase 1: Try single-element flips
      for (int idx = 0; idx < len; idx++) {
        int old_val = arr[idx];
        int delta_npaf[128] = {0};
        for (int s = 1; s < ms; s++) {
          if (idx + s < len)
            delta_npaf[s] += (-2 * old_val) * arr[idx + s];
          if (idx - s >= 0)
            delta_npaf[s] += arr[idx - s] * (-2 * old_val);
        }
        arr[idx] = -old_val;
        *sum_ptr -= 2 * old_val;
        for (int s = 1; s < ms; s++)
          state.npaf[s] += delta_npaf[s];

        int trial_cost = state.cost(ta, tb, tc, td, ms);
        if (trial_cost < cost_out) {
          cost_out = trial_cost;
          improved = true;
          if (cost_out == 0) return true;
        } else {
          arr[idx] = old_val;
          *sum_ptr += 2 * old_val;
          for (int s = 1; s < ms; s++)
            state.npaf[s] -= delta_npaf[s];
        }
      }

      // Phase 2: Try sum-preserving swaps within this sequence
      for (int i = 0; i < len; i++) {
        for (int j = i + 1; j < len; j++) {
          if (arr[i] == arr[j]) continue; // Only swap opposite values
          int vi = arr[i]; // vi and vj have opposite signs
          // Compute delta NPAF for swapping arr[i] and arr[j]
          int delta_npaf2[128] = {0};
          for (int s = 1; s < ms; s++) {
            // Effect from position i (changing vi to -vi)
            if (i + s < len && i + s != j)
              delta_npaf2[s] -= 2 * vi * arr[i + s];
            if (i - s >= 0 && i - s != j)
              delta_npaf2[s] -= 2 * vi * arr[i - s];
            // Effect from position j (changing -vi to vi)
            if (j + s < len && j + s != i)
              delta_npaf2[s] += 2 * vi * arr[j + s];
            if (j - s >= 0 && j - s != i)
              delta_npaf2[s] += 2 * vi * arr[j - s];
          }
          // Apply swap
          arr[i] = -vi; arr[j] = vi;
          for (int s = 1; s < ms; s++)
            state.npaf[s] += delta_npaf2[s];

          int trial_cost = state.cost(ta, tb, tc, td, ms);
          if (trial_cost < cost_out) {
            cost_out = trial_cost;
            improved = true;
            if (cost_out == 0) return true;
          } else {
            arr[i] = vi; arr[j] = -vi;
            for (int s = 1; s < ms; s++)
              state.npaf[s] -= delta_npaf2[s];
          }
        }
      }
    }
  }
  return cost_out == 0;
}

// Swap-based perturbation: preserves sums exactly
static void perturb_joint_swaps(JointState &state, int n1, int n, int ms,
                                 int K, mt19937 &rng) {
  for (int p = 0; p < K; p++) {
    int seq_id = uniform_int_distribution<>(0, 3)(rng);
    int *arr;
    int len;
    if (seq_id == 0) { arr = state.A; len = n1; }
    else if (seq_id == 1) { arr = state.B; len = n1; }
    else if (seq_id == 2) { arr = state.C; len = n; }
    else { arr = state.D; len = n; }

    int pos_plus[128], pos_minus[128];
    int np = 0, nm = 0;
    for (int k = 0; k < len; k++) {
      if (arr[k] == 1) pos_plus[np++] = k;
      else pos_minus[nm++] = k;
    }
    if (np == 0 || nm == 0) continue;

    int pi = pos_plus[uniform_int_distribution<>(0, np - 1)(rng)];
    int pj = pos_minus[uniform_int_distribution<>(0, nm - 1)(rng)];
    for (int s = 1; s < ms; s++) {
      int d = 0;
      if (pi + s < len && pi + s != pj) d -= 2 * arr[pi + s];
      if (pi - s >= 0 && pi - s != pj) d -= 2 * arr[pi - s];
      if (pj + s < len && pj + s != pi) d += 2 * arr[pj + s];
      if (pj - s >= 0 && pj - s != pi) d += 2 * arr[pj - s];
      state.npaf[s] += d;
    }
    arr[pi] = -1; arr[pj] = 1;
  }
}

// Flip-based perturbation (for broad exploration)
static void perturb_joint_flips(JointState &state, int n1, int n, int ms,
                                 int K, mt19937 &rng) {
  int total = 2 * n1 + 2 * n;
  uniform_int_distribution<> elem_dist(0, total - 1);
  for (int p = 0; p < K; p++) {
    int elem = elem_dist(rng);
    int *arr;
    int *sum_ptr;
    int idx, len;
    if (elem < n1) {
      arr = state.A; sum_ptr = &state.sum_a; idx = elem; len = n1;
    } else if (elem < 2 * n1) {
      arr = state.B; sum_ptr = &state.sum_b; idx = elem - n1; len = n1;
    } else if (elem < 2 * n1 + n) {
      arr = state.C; sum_ptr = &state.sum_c; idx = elem - 2 * n1; len = n;
    } else {
      arr = state.D; sum_ptr = &state.sum_d; idx = elem - 2 * n1 - n; len = n;
    }
    int old_val = arr[idx];
    for (int s = 1; s < ms; s++) {
      if (idx + s < len)
        state.npaf[s] += (-2 * old_val) * arr[idx + s];
      if (idx - s >= 0)
        state.npaf[s] += arr[idx - s] * (-2 * old_val);
    }
    arr[idx] = -old_val;
    *sum_ptr -= 2 * old_val;
  }
}

// Compound swap search: try random pairs of swaps across two sequences
// This finds moves that no single swap can achieve
static bool compound_swap_search(int n1, int n, int ta, int tb, int tc, int td,
                                  int ms, JointState &best, int &best_cost,
                                  mt19937 &rng) {
  // Get arrays and lengths for all 4 sequences
  int *arrs[4] = {best.A, best.B, best.C, best.D};
  int lens[4] = {n1, n1, n, n};

  // Try 100000 random compound swaps
  for (int attempt = 0; attempt < 100000; attempt++) {
    if (g_found.load(memory_order_relaxed)) return false;
    if (best_cost == 0) return true;

    // Pick two different sequences
    int s1 = uniform_int_distribution<>(0, 3)(rng);
    int s2 = uniform_int_distribution<>(0, 2)(rng);
    if (s2 >= s1) s2++;

    JointState trial = best;
    int *arrs_t[4] = {trial.A, trial.B, trial.C, trial.D};

    // Do one swap in seq s1
    bool did_swap1 = false;
    {
      int *arr = arrs_t[s1];
      int len = lens[s1];
      int pp[128], pm[128]; int np = 0, nm = 0;
      for (int k = 0; k < len; k++) {
        if (arr[k] == 1) pp[np++] = k;
        else pm[nm++] = k;
      }
      if (np > 0 && nm > 0) {
        int pi = pp[uniform_int_distribution<>(0, np-1)(rng)];
        int pj = pm[uniform_int_distribution<>(0, nm-1)(rng)];
        for (int s = 1; s < ms; s++) {
          int d = 0;
          if (pi+s < len && pi+s != pj) d -= 2 * arr[pi+s];
          if (pi-s >= 0 && pi-s != pj) d -= 2 * arr[pi-s];
          if (pj+s < len && pj+s != pi) d += 2 * arr[pj+s];
          if (pj-s >= 0 && pj-s != pi) d += 2 * arr[pj-s];
          trial.npaf[s] += d;
        }
        arr[pi] = -1; arr[pj] = 1;
        did_swap1 = true;
      }
    }
    if (!did_swap1) continue;

    // Do one swap in seq s2
    {
      int *arr = arrs_t[s2];
      int len = lens[s2];
      int pp[128], pm[128]; int np = 0, nm = 0;
      for (int k = 0; k < len; k++) {
        if (arr[k] == 1) pp[np++] = k;
        else pm[nm++] = k;
      }
      if (np > 0 && nm > 0) {
        int pi = pp[uniform_int_distribution<>(0, np-1)(rng)];
        int pj = pm[uniform_int_distribution<>(0, nm-1)(rng)];
        for (int s = 1; s < ms; s++) {
          int d = 0;
          if (pi+s < len && pi+s != pj) d -= 2 * arr[pi+s];
          if (pi-s >= 0 && pi-s != pj) d -= 2 * arr[pi-s];
          if (pj+s < len && pj+s != pi) d += 2 * arr[pj+s];
          if (pj-s >= 0 && pj-s != pi) d += 2 * arr[pj-s];
          trial.npaf[s] += d;
        }
        arr[pi] = -1; arr[pj] = 1;
      }
    }

    int trial_cost = trial.cost(ta, tb, tc, td, ms);
    if (trial_cost < best_cost) {
      // Try greedy from here
      greedy_hill_climb_joint(n1, n, ta, tb, tc, td, ms, trial, trial_cost);
      if (trial_cost < best_cost) {
        best_cost = trial_cost;
        best = trial;
        if (best_cost == 0) return true;
        attempt = 0; // Reset on improvement
      }
    }
  }
  return best_cost == 0;
}

// ILS: swap-based perturb + greedy-climb
static bool iterated_local_search_joint(int n1, int n, int ta, int tb, int tc,
                                         int td, int ms, JointState &best,
                                         int &best_cost, mt19937 &rng) {
  for (int attempt = 0; attempt < 300000; attempt++) {
    if (g_found.load(memory_order_relaxed)) return false;
    if (best_cost == 0) return true;

    JointState trial = best;
    int perturb_size = 2 + (attempt % 10);

    // Use swap-based perturbation (preserves sums)
    perturb_joint_swaps(trial, n1, n, ms, perturb_size, rng);

    int trial_cost = trial.cost(ta, tb, tc, td, ms);
    greedy_hill_climb_joint(n1, n, ta, tb, tc, td, ms, trial, trial_cost);

    if (trial_cost < best_cost) {
      best_cost = trial_cost;
      best = trial;
      if (best_cost == 0) return true;
      attempt = 0;
    }
  }
  return best_cost == 0;
}

bool solve_joint_SA(int n1, int n, int ta, int tb, int tc, int td,
                    JointState &best_state, mt19937 &rng) {
  int ms = max(n1, n);
  int total_elems = 2 * n1 + 2 * n;

  JointState curr;
  memset(curr.npaf, 0, sizeof(curr.npaf));
  curr.sum_a = curr.sum_b = curr.sum_c = curr.sum_d = 0;

  // Initialize all sequences with random ±1
  for (int i = 0; i < n1; i++) {
    curr.A[i] = (uniform_int_distribution<>(0, 1)(rng) == 0) ? 1 : -1;
    curr.B[i] = (uniform_int_distribution<>(0, 1)(rng) == 0) ? 1 : -1;
    curr.sum_a += curr.A[i];
    curr.sum_b += curr.B[i];
  }
  for (int i = 0; i < n; i++) {
    curr.C[i] = (uniform_int_distribution<>(0, 1)(rng) == 0) ? 1 : -1;
    curr.D[i] = (uniform_int_distribution<>(0, 1)(rng) == 0) ? 1 : -1;
    curr.sum_c += curr.C[i];
    curr.sum_d += curr.D[i];
  }

  // Compute initial NPAF
  for (int s = 1; s < ms; s++) {
    for (int i = 0; i < n1 - s; i++)
      curr.npaf[s] += curr.A[i] * curr.A[i + s] + curr.B[i] * curr.B[i + s];
    for (int i = 0; i < n - s; i++)
      curr.npaf[s] += curr.C[i] * curr.C[i + s] + curr.D[i] * curr.D[i + s];
  }

  int current_cost = curr.cost(ta, tb, tc, td, ms);
  best_state = curr;
  int best_cost = current_cost;

  SAParams sa;
  uniform_real_distribution<> prob(0.0, 1.0);
  uniform_int_distribution<> elem_dist(0, total_elems - 1);

  for (int restart = 0; restart < sa.restarts; restart++) {
    if (g_found.load(memory_order_relaxed)) return false;

    if (restart > 0) {
      if (restart % 2 == 0 && best_cost < 999999) {
        curr = best_state;
        perturb_joint_swaps(curr, n1, n, ms, max(4, total_elems / 8), rng);
        current_cost = curr.cost(ta, tb, tc, td, ms);
      } else {
        memset(curr.npaf, 0, sizeof(curr.npaf));
        curr.sum_a = curr.sum_b = curr.sum_c = curr.sum_d = 0;
        for (int i = 0; i < n1; i++) {
          curr.A[i] = (uniform_int_distribution<>(0, 1)(rng) == 0) ? 1 : -1;
          curr.B[i] = (uniform_int_distribution<>(0, 1)(rng) == 0) ? 1 : -1;
          curr.sum_a += curr.A[i]; curr.sum_b += curr.B[i];
        }
        for (int i = 0; i < n; i++) {
          curr.C[i] = (uniform_int_distribution<>(0, 1)(rng) == 0) ? 1 : -1;
          curr.D[i] = (uniform_int_distribution<>(0, 1)(rng) == 0) ? 1 : -1;
          curr.sum_c += curr.C[i]; curr.sum_d += curr.D[i];
        }
        for (int s = 1; s < ms; s++) {
          for (int i = 0; i < n1 - s; i++)
            curr.npaf[s] += curr.A[i] * curr.A[i + s] + curr.B[i] * curr.B[i + s];
          for (int i = 0; i < n - s; i++)
            curr.npaf[s] += curr.C[i] * curr.C[i + s] + curr.D[i] * curr.D[i + s];
        }
        current_cost = curr.cost(ta, tb, tc, td, ms);
      }
    }

    double temp = sa.initial_temp;
    int no_improve = 0;

    for (int iter = 0; iter < sa.iterations; iter++) {
      if (g_found.load(memory_order_relaxed)) return false;
      if (best_cost == 0) return true;

      no_improve++;
      if (no_improve > sa.reheat_threshold) {
        temp = sa.initial_temp * sa.reheat_ratio;
        no_improve = 0;
      }

      // 60% swap mutations (sum-preserving), 40% flips
      bool do_swap = (prob(rng) < 0.6);

      if (do_swap) {
        // === SWAP MUTATION: pick a random sequence, swap two opposite elements ===
        int seq_id = uniform_int_distribution<>(0, 3)(rng);
        int *arr;
        int len;
        if (seq_id == 0) { arr = curr.A; len = n1; }
        else if (seq_id == 1) { arr = curr.B; len = n1; }
        else if (seq_id == 2) { arr = curr.C; len = n; }
        else { arr = curr.D; len = n; }

        // Find positions with +1 and -1
        int pos_plus[128], pos_minus[128];
        int np = 0, nm = 0;
        for (int k = 0; k < len; k++) {
          if (arr[k] == 1) pos_plus[np++] = k;
          else pos_minus[nm++] = k;
        }
        if (np == 0 || nm == 0) { temp *= sa.cooling_rate; continue; }

        int pi = pos_plus[uniform_int_distribution<>(0, np - 1)(rng)];
        int pj = pos_minus[uniform_int_distribution<>(0, nm - 1)(rng)];
        int vi = 1; // arr[pi] = 1, arr[pj] = -1

        // Compute swap delta: O(ms)
        int delta_npaf[128] = {0};
        for (int s = 1; s < ms; s++) {
          if (pi + s < len && pi + s != pj)
            delta_npaf[s] -= 2 * vi * arr[pi + s];
          if (pi - s >= 0 && pi - s != pj)
            delta_npaf[s] -= 2 * vi * arr[pi - s];
          if (pj + s < len && pj + s != pi)
            delta_npaf[s] += 2 * vi * arr[pj + s];
          if (pj - s >= 0 && pj - s != pi)
            delta_npaf[s] += 2 * vi * arr[pj - s];
        }

        // Apply swap
        arr[pi] = -1; arr[pj] = 1;
        for (int s = 1; s < ms; s++)
          curr.npaf[s] += delta_npaf[s];

        int new_cost = curr.cost(ta, tb, tc, td, ms);
        if (new_cost < current_cost ||
            prob(rng) < exp(-(new_cost - current_cost) / temp)) {
          current_cost = new_cost;
          if (new_cost < best_cost) {
            best_cost = new_cost;
            best_state = curr;
            no_improve = 0;
          }
        } else {
          arr[pi] = 1; arr[pj] = -1;
          for (int s = 1; s < ms; s++)
            curr.npaf[s] -= delta_npaf[s];
        }
      } else {
        // === FLIP MUTATION: flip one random element ===
        int elem = elem_dist(rng);
        int *arr;
        int *sum_ptr;
        int idx, len;
        if (elem < n1) {
          arr = curr.A; sum_ptr = &curr.sum_a; idx = elem; len = n1;
        } else if (elem < 2 * n1) {
          arr = curr.B; sum_ptr = &curr.sum_b; idx = elem - n1; len = n1;
        } else if (elem < 2 * n1 + n) {
          arr = curr.C; sum_ptr = &curr.sum_c; idx = elem - 2 * n1; len = n;
        } else {
          arr = curr.D; sum_ptr = &curr.sum_d; idx = elem - 2 * n1 - n; len = n;
        }

        int old_val = arr[idx];
        int old_sum = *sum_ptr;

        int delta_npaf[128] = {0};
        for (int s = 1; s < ms; s++) {
          if (idx + s < len)
            delta_npaf[s] += (-2 * old_val) * arr[idx + s];
          if (idx - s >= 0)
            delta_npaf[s] += arr[idx - s] * (-2 * old_val);
        }

        arr[idx] = -old_val;
        *sum_ptr -= 2 * old_val;
        for (int s = 1; s < ms; s++)
          curr.npaf[s] += delta_npaf[s];

        int new_cost = curr.cost(ta, tb, tc, td, ms);
        if (new_cost < current_cost ||
            prob(rng) < exp(-(new_cost - current_cost) / temp)) {
          current_cost = new_cost;
          if (new_cost < best_cost) {
            best_cost = new_cost;
            best_state = curr;
            no_improve = 0;
          }
        } else {
          arr[idx] = old_val;
          *sum_ptr = old_sum;
          for (int s = 1; s < ms; s++)
            curr.npaf[s] -= delta_npaf[s];
        }
      }

      temp *= sa.cooling_rate;
    }

    if (best_cost == 0) return true;

    // Greedy hill-climb after SA
    if (best_cost > 0 && best_cost < 200) {
      JointState hc = best_state;
      int hc_cost = best_cost;
      greedy_hill_climb_joint(n1, n, ta, tb, tc, td, ms, hc, hc_cost);
      if (hc_cost < best_cost) {
        best_cost = hc_cost;
        best_state = hc;
      }
      if (best_cost == 0) return true;
    }
  }

  // ILS phase (always run after SA if not solved)
  if (best_cost > 0) {
    if (iterated_local_search_joint(n1, n, ta, tb, tc, td, ms, best_state,
                                     best_cost, rng))
      return true;
  }

  // Compound swap search for endgame (cost < 20)
  if (best_cost > 0 && best_cost <= 20) {
    if (compound_swap_search(n1, n, ta, tb, tc, td, ms, best_state,
                              best_cost, rng))
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
  thr = omp_get_max_threads();
  omp_set_num_threads(thr);
#endif

  cout << "========================================================" << endl;
  cout << "  BS(" << n1 << "," << n << ") — Joint 4-Sequence SA Solver" << endl;
  cout << "  Targeting NPAF=0 across all sequences simultaneously" << endl;
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

  long long global_tries = 0;

#pragma omp parallel reduction(+ : global_tries)
  {
    int tid = 0;
#ifdef _OPENMP
    tid = omp_get_thread_num();
#endif
    mt19937 rng(42 + tid * 1000 + time(NULL) + seed_offset * 100000);

    while (!g_found.load(memory_order_relaxed)) {
      int si = uniform_int_distribution<>(0, sigs.size() - 1)(rng);
      auto &sig = sigs[si];

      JointState best;
      bool found = solve_joint_SA(n1, n, sig.a, sig.b, sig.c, sig.d, best, rng);

      // Track best cost
      {
        int c = best.cost(sig.a, sig.b, sig.c, sig.d, ms);
        int old_best = g_best_cost.load(memory_order_relaxed);
        while (c < old_best &&
               !g_best_cost.compare_exchange_weak(old_best, c,
                                                   memory_order_relaxed))
          ;
      }

      if (found) {
        // Final NPAF validation
        bool valid = true;
        for (int s = 1; s < ms && valid; s++) {
          if (npaf_at(best.A, best.B, n1, best.C, best.D, n, s) != 0)
            valid = false;
        }
        if (valid && hall_ok(best.C, n, best.D, n) &&
            hall_ok(best.A, n1, best.B, n1)) {
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
              cout << best.A[i] << (i < n1 - 1 ? "," : "");
            cout << "};" << endl;
            cout << "B = {";
            for (int i = 0; i < n1; i++)
              cout << best.B[i] << (i < n1 - 1 ? "," : "");
            cout << "};" << endl;
            cout << "C = {";
            for (int i = 0; i < n; i++)
              cout << best.C[i] << (i < n - 1 ? "," : "");
            cout << "};" << endl;
            cout << "D = {";
            for (int i = 0; i < n; i++)
              cout << best.D[i] << (i < n - 1 ? "," : "");
            cout << "};" << endl;

            double t = chrono::duration<double>(Clock::now() - G_T0).count();
            cout << "\nTime: " << t << "s" << endl;
          }
        }
      }

      global_tries++;
      if (tid == 0) {
        long long current_total = initial_epochs + (global_tries * thr);
        double t = chrono::duration<double>(Clock::now() - G_T0).count();
        double speed = (t > 0) ? ((global_tries * thr) / t) : 0.0;
        int gbest = g_best_cost.load(memory_order_relaxed);
        cout << "[" << t << "s] Epochs: " << current_total
             << " Speed: " << speed
             << " [Best cost: " << gbest << "]\n"
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
