/*
 * Wang-Zhu BS Solver — Hybrid Joint + Decomposed SA
 * CP493 - Directed Research - Daniel Gordon
 *
 * TRILLIUM SUPERCOMPUTER VERSION
 * Strategy: Joint SA on all 4 sequences for exploration,
 * then freeze C,D and intensive A,B-only SA for the endgame.
 * This halves the variables from 110 to 56 at the critical moment.
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
static int G_N;
static Clock::time_point G_T0;

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

// ===================================
// Joint state: all 4 sequences
// ===================================
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

// ===================================
// A,B-only SA (endgame: C,D are frozen)
// ===================================
// Given fixed C,D with known PAF_CD, find A,B such that
// PAF_A(s) + PAF_B(s) = -PAF_CD(s) for all s
static int solve_AB_only(int n1, int ta, int tb, int ms,
                          const int *target_ab,
                          JointState &state, mt19937 &rng) {
  // target_ab[s] = what PAF_A(s)+PAF_B(s) should equal
  // Current paf_ab
  int paf_ab[128];
  for (int s = 1; s < ms; s++) {
    paf_ab[s] = 0;
    for (int i = 0; i < n1 - s; i++)
      paf_ab[s] += state.A[i] * state.A[i + s] + state.B[i] * state.B[i + s];
  }

  int current_cost = 0;
  for (int s = 1; s < ms; s++) current_cost += abs(paf_ab[s] - target_ab[s]);
  current_cost += 5 * (abs(state.sum_a - ta) + abs(state.sum_b - tb));

  int best_cost = current_cost;

  uniform_real_distribution<> prob(0.0, 1.0);
  int total = 2 * n1;
  uniform_int_distribution<> elem_dist(0, total - 1);

  // Multiple restarts of short SA runs
  for (int restart = 0; restart < 20; restart++) {
    if (g_found.load(memory_order_relaxed)) return best_cost;
    if (best_cost == 0) return 0;

    // On restarts > 0, perturb A,B from best
    if (restart > 0) {
      // Recompute paf_ab from state
      for (int s = 1; s < ms; s++) {
        paf_ab[s] = 0;
        for (int i = 0; i < n1 - s; i++)
          paf_ab[s] += state.A[i] * state.A[i + s] + state.B[i] * state.B[i + s];
      }
      current_cost = 0;
      for (int s = 1; s < ms; s++) current_cost += abs(paf_ab[s] - target_ab[s]);
      current_cost += 5 * (abs(state.sum_a - ta) + abs(state.sum_b - tb));

      // Random swaps to perturb
      for (int p = 0; p < 2 + (restart % 5); p++) {
        int seq = uniform_int_distribution<>(0, 1)(rng);
        int *arr = (seq == 0) ? state.A : state.B;
        int pos[128], neg[128]; int np = 0, nm = 0;
        for (int k = 0; k < n1; k++) {
          if (arr[k] == 1) pos[np++] = k; else neg[nm++] = k;
        }
        if (np > 0 && nm > 0) {
          int pi = pos[uniform_int_distribution<>(0, np - 1)(rng)];
          int pj = neg[uniform_int_distribution<>(0, nm - 1)(rng)];
          for (int s = 1; s < ms; s++) {
            int d = 0;
            if (pi + s < n1 && pi + s != pj) d -= 2 * arr[pi + s];
            if (pi - s >= 0 && pi - s != pj) d -= 2 * arr[pi - s];
            if (pj + s < n1 && pj + s != pi) d += 2 * arr[pj + s];
            if (pj - s >= 0 && pj - s != pi) d += 2 * arr[pj - s];
            paf_ab[s] += d;
          }
          arr[pi] = -1; arr[pj] = 1;
        }
      }
      current_cost = 0;
      for (int s = 1; s < ms; s++) current_cost += abs(paf_ab[s] - target_ab[s]);
      current_cost += 5 * (abs(state.sum_a - ta) + abs(state.sum_b - tb));
    }

    double temp = 15.0;
    for (int iter = 0; iter < 100000; iter++) {
      if (g_found.load(memory_order_relaxed)) return best_cost;

      int elem = elem_dist(rng);
      int seq = (elem < n1) ? 0 : 1;
      int *arr = (seq == 0) ? state.A : state.B;
      int *sum_ptr = (seq == 0) ? &state.sum_a : &state.sum_b;
      int idx = (seq == 0) ? elem : elem - n1;

      // 70% swap, 30% flip
      if (prob(rng) < 0.7) {
        int pos_p[128], pos_m[128]; int npp = 0, npm = 0;
        for (int k = 0; k < n1; k++) {
          if (arr[k] == 1) pos_p[npp++] = k; else pos_m[npm++] = k;
        }
        if (npp == 0 || npm == 0) { temp *= 0.99997; continue; }
        int pi = pos_p[uniform_int_distribution<>(0, npp - 1)(rng)];
        int pj = pos_m[uniform_int_distribution<>(0, npm - 1)(rng)];

        int delta[128];
        for (int s = 1; s < ms; s++) {
          delta[s] = 0;
          if (pi + s < n1 && pi + s != pj) delta[s] -= 2 * arr[pi + s];
          if (pi - s >= 0 && pi - s != pj) delta[s] -= 2 * arr[pi - s];
          if (pj + s < n1 && pj + s != pi) delta[s] += 2 * arr[pj + s];
          if (pj - s >= 0 && pj - s != pi) delta[s] += 2 * arr[pj - s];
        }

        int new_cost = 0;
        for (int s = 1; s < ms; s++)
          new_cost += abs(paf_ab[s] + delta[s] - target_ab[s]);
        new_cost += 5 * (abs(state.sum_a - ta) + abs(state.sum_b - tb));

        int diff = new_cost - current_cost;
        if (diff <= 0 || prob(rng) < exp(-diff / temp)) {
          arr[pi] = -1; arr[pj] = 1;
          for (int s = 1; s < ms; s++) paf_ab[s] += delta[s];
          current_cost = new_cost;
          if (current_cost < best_cost) {
            best_cost = current_cost;
          }
        }
      } else {
        int old_val = arr[idx];
        int delta[128];
        for (int s = 1; s < ms; s++) {
          delta[s] = 0;
          if (idx + s < n1) delta[s] += (-2 * old_val) * arr[idx + s];
          if (idx - s >= 0) delta[s] += arr[idx - s] * (-2 * old_val);
        }

        int new_sum = *sum_ptr - 2 * old_val;
        int new_cost = 0;
        for (int s = 1; s < ms; s++)
          new_cost += abs(paf_ab[s] + delta[s] - target_ab[s]);
        int sa = (seq == 0) ? new_sum : state.sum_a;
        int sb = (seq == 0) ? state.sum_b : new_sum;
        new_cost += 5 * (abs(sa - ta) + abs(sb - tb));

        int diff = new_cost - current_cost;
        if (diff <= 0 || prob(rng) < exp(-diff / temp)) {
          arr[idx] = -old_val;
          *sum_ptr = new_sum;
          for (int s = 1; s < ms; s++) paf_ab[s] += delta[s];
          current_cost = new_cost;
          if (current_cost < best_cost) {
            best_cost = current_cost;
          }
        }
      }
      temp *= 0.99997;
    }
  }
  return best_cost;
}

// ===================================
// Joint SA solver (exploration phase)
// ===================================
static bool solve_joint_SA(int n1, int n, int ta, int tb, int tc, int td,
                            JointState &best_state, mt19937 &rng,
                            bool warm_start = false) {
  int ms = max(n1, n);
  int total_elems = 2 * n1 + 2 * n;
  uniform_real_distribution<> prob(0.0, 1.0);
  uniform_int_distribution<> elem_dist(0, total_elems - 1);

  const double initial_temp = 30.0;
  const double cooling_rate = 0.99997;
  const int sa_iters = 500000;
  const int restarts = 40;

  JointState curr;
  int best_cost = 999999;

  for (int restart = 0; restart < restarts; restart++) {
    if (g_found.load(memory_order_relaxed)) return false;
    if (best_cost == 0) return true;

    if (restart == 0 && warm_start) {
      curr = best_state;
    } else if (restart > 0 && restart % 5 < 2 && best_cost < 999999) {
      // Perturb from best
      curr = best_state;
      int pert = 3 + (restart % 8);
      // Swap-based perturbation
      for (int p = 0; p < pert; p++) {
        int seq_id = uniform_int_distribution<>(0, 3)(rng);
        int *arr;
        int len;
        if (seq_id == 0) { arr = curr.A; len = n1; }
        else if (seq_id == 1) { arr = curr.B; len = n1; }
        else if (seq_id == 2) { arr = curr.C; len = n; }
        else { arr = curr.D; len = n; }
        int pos[128], neg[128]; int np = 0, nm = 0;
        for (int k = 0; k < len; k++) {
          if (arr[k] == 1) pos[np++] = k; else neg[nm++] = k;
        }
        if (np > 0 && nm > 0) {
          int pi = pos[uniform_int_distribution<>(0, np - 1)(rng)];
          int pj = neg[uniform_int_distribution<>(0, nm - 1)(rng)];
          for (int s = 1; s < ms; s++) {
            int d = 0;
            if (pi + s < len && pi + s != pj) d -= 2 * arr[pi + s];
            if (pi - s >= 0 && pi - s != pj) d -= 2 * arr[pi - s];
            if (pj + s < len && pj + s != pi) d += 2 * arr[pj + s];
            if (pj - s >= 0 && pj - s != pi) d += 2 * arr[pj - s];
            curr.npaf[s] += d;
          }
          arr[pi] = -1; arr[pj] = 1;
        }
      }
    } else {
      // Random init
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
    }

    int current_cost = curr.cost(ta, tb, tc, td, ms);
    if (current_cost < best_cost) {
      best_cost = current_cost;
      best_state = curr;
    }

    double temp = initial_temp;
    int no_improve = 0;

    for (int iter = 0; iter < sa_iters; iter++) {
      if (g_found.load(memory_order_relaxed)) return false;
      if (best_cost == 0) return true;

      int elem = elem_dist(rng);
      int *arr, *sum_ptr;
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
      int delta_npaf[128];
      for (int s = 1; s < ms; s++) {
        delta_npaf[s] = 0;
        if (idx + s < len) delta_npaf[s] += (-2 * old_val) * arr[idx + s];
        if (idx - s >= 0) delta_npaf[s] += arr[idx - s] * (-2 * old_val);
      }

      arr[idx] = -old_val;
      *sum_ptr -= 2 * old_val;
      for (int s = 1; s < ms; s++)
        curr.npaf[s] += delta_npaf[s];

      int new_cost = curr.cost(ta, tb, tc, td, ms);
      int diff = new_cost - current_cost;

      if (diff <= 0 || prob(rng) < exp(-diff / temp)) {
        current_cost = new_cost;
        no_improve = (diff < 0) ? 0 : no_improve + 1;
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
        no_improve++;
      }

      // Reheat
      if (no_improve > 150000) {
        temp = initial_temp * 0.5;
        no_improve = 0;
      }
      temp *= cooling_rate;
    }

    if (best_cost == 0) return true;

    // *** KEY INNOVATION: When close, freeze C,D and do A,B-only SA ***
    if (best_cost > 0 && best_cost <= 8) {
      // Compute PAF_CD target
      int paf_cd[128];
      for (int s = 1; s < ms; s++) {
        paf_cd[s] = 0;
        if (s < n)
          for (int i = 0; i < n - s; i++)
            paf_cd[s] += best_state.C[i] * best_state.C[i + s] +
                          best_state.D[i] * best_state.D[i + s];
      }
      int target_ab[128];
      for (int s = 1; s < ms; s++) target_ab[s] = -paf_cd[s];

      JointState ab_state = best_state;
      int ab_cost = solve_AB_only(n1, ta, tb, ms, target_ab, ab_state, rng);
      if (ab_cost < best_cost) {
        best_cost = ab_cost;
        best_state = ab_state;
        // Recompute npaf
        memset(best_state.npaf, 0, sizeof(best_state.npaf));
        for (int s = 1; s < ms; s++) {
          for (int i = 0; i < n1 - s; i++)
            best_state.npaf[s] += best_state.A[i] * best_state.A[i + s] +
                                   best_state.B[i] * best_state.B[i + s];
          if (s < n)
            for (int i = 0; i < n - s; i++)
              best_state.npaf[s] += best_state.C[i] * best_state.C[i + s] +
                                     best_state.D[i] * best_state.D[i + s];
        }
      }
      if (best_cost == 0) return true;
    }
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

  int thr = 1;
#ifdef _OPENMP
  thr = omp_get_max_threads();
  omp_set_num_threads(thr);
#endif

  cout << "========================================================" << endl;
  cout << "  BS(" << n1 << "," << n << ") — Hybrid SA Solver" << endl;
  cout << "  Joint SA + A,B-only Endgame" << endl;
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
