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
  double initial_temp = 50.0;
  double cooling_rate = 0.999;  // Very fast cooling
  int iterations = 100000;      // Very short cycles
  int restarts = 50;            // Aggressive restarts for wide coverage
  int reheat_threshold = 20000; // Fast stuck detection
  double reheat_ratio = 0.25;   // Reheat to 25%
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
  memset(curr.corr, 0, sizeof(curr.corr));
  curr.sum_c = 0;
  curr.sum_d = 0;

  // Initialize random adhering to rules
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
  uniform_int_distribution<> d_dist(0, n / 2 - 1);

  // Adaptive restart loop: run multiple short SA cycles
  for (int restart = 0; restart < sa.restarts; restart++) {
    if (g_found.load(memory_order_relaxed))
      return false;

    // Re-randomize state for each restart (except first which uses initial)
    if (restart > 0) {
      memset(curr.corr, 0, sizeof(curr.corr));
      curr.sum_c = 0;
      curr.sum_d = 0;
      for (int d = 0; d < n / 2; d++) {
        int left = d, right = n - 1 - d;
        const int *c_ptr =
            (d == 0) ? comb16[uniform_int_distribution<>(0, 15)(rng)]
                     : comb8_pos[uniform_int_distribution<>(0, 7)(rng)];
        curr.C[left] = c_ptr[0];
        curr.D[left] = c_ptr[1];
        curr.C[right] = c_ptr[2];
        curr.D[right] = c_ptr[3];
        curr.sum_c += c_ptr[0] + c_ptr[2];
        curr.sum_d += c_ptr[1] + c_ptr[3];
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

      if (found_cd) {
        int cd_full[128] = {0};
        for (int s = 1; s < ms; s++) {
          for (int k = 0; k < n - s; k++)
            cd_full[s] += best_cd.C[k] * best_cd.C[k + s] +
                          best_cd.D[k] * best_cd.D[k + s];
        }

        ABState best_ab;
        bool found_ab = solve_AB_SA(n1, sig.a, sig.b, cd_full, best_ab, rng);

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

              double t = chrono::duration<double>(Clock::now() - G_T0).count();
              cout << "\nTime: " << t << "s" << endl;
            }
          }
        }
      }

      global_tries++;
      if (tid == 0) {
        long long current_total = initial_epochs + (global_tries * thr);
        double t = chrono::duration<double>(Clock::now() - G_T0).count();
        double speed = (t > 0) ? ((global_tries * thr) / t) : 0.0;
        cout << "[" << t << "s] SA epochs explored locally: " << current_total
             << " Speed: " << speed << "\n"
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
