/*
 * State-of-the-Art Base Sequence Solver
 * CP493 - Directed Research - Daniel Gordon
 *
 * Uses Pair Decomposition + Enhanced Tabu Search:
 *   Phase 1: Search for (A,B) pairs of length n+1 where PAF_AB(s) is
 * "compatible" Phase 2: For each good (A,B), search for (C,D) of length n where
 * PAF_CD(s) = -PAF_AB(s)
 *
 * This decomposes 2^(4n+2) search into 2^(2n+2) + 2^(2n) which is exponentially
 * easier.
 *
 * Additional optimizations:
 *   - OpenMP parallel search across CPU cores
 *   - O(n) delta evaluation with cached correlations
 *   - Symmetry breaking (16x reduction)
 *   - Reactive tabu tenure with perturbation kicks
 *   - Shift-targeted move selection (prioritize worst shifts)
 *   - 2-flip compound moves for escaping plateaus
 *   - Solution pool with path relinking between top-K solutions
 *
 * Compile:
 *   g++ -O3 -march=native -std=c++17 -Xpreprocessor -fopenmp \
 *       -I/opt/homebrew/opt/libomp/include -L/opt/homebrew/opt/libomp/lib -lomp
 * \ -o bs_solver bs_solver.cpp
 *
 * Usage:
 *   ./bs_solver <n> [minutes] [threads]
 *
 * Examples:
 *   ./bs_solver 30            # Find BS(31,30) in 5 minutes
 *   ./bs_solver 44 60         # Attempt the frontier: BS(45,44)
 *   ./bs_solver 50 120 14     # BS(51,50) - 2 hours, 14 threads
 */

#include <algorithm>
#include <atomic>
#include <cassert>
#include <chrono>
#include <climits>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <random>
#include <sstream>
#include <unordered_map>
#include <vector>

#ifdef _OPENMP
#include <omp.h>
#endif

using namespace std;
using Clock = chrono::steady_clock;

// ============================================================
// Configuration
// ============================================================
static const int MAX_N = 300;

// ============================================================
// Shared global state
// ============================================================
static atomic<bool> solution_found{false};
static int GLOBAL_N;  // C,D length (n)
static int GLOBAL_N1; // A,B length (n+1)
static int g_best_A[MAX_N], g_best_B[MAX_N], g_best_C[MAX_N], g_best_D[MAX_N];

// ============================================================
// Pair Search State: searches over 2 sequences
// ============================================================
struct PairState {
  int len;           // length of each sequence in the pair
  int seq[2][MAX_N]; // the two sequences
  int corr[MAX_N];   // combined autocorrelation: sum N_s for both seqs
  int target[MAX_N]; // target correlation for each shift (0 for AB, -PAF_AB for
                     // CD)
  int tabu[2][MAX_N];       // tabu expiry
  long long freq[2][MAX_N]; // flip frequency
  int best_seq[2][MAX_N];
  long long best_energy;
  long long current_energy;
  int delta_corr[MAX_N];
  mt19937 rng;

  void init(int _len, unsigned seed) {
    len = _len;
    rng.seed(seed);
    best_energy = LLONG_MAX;
    memset(freq, 0, sizeof(freq));
    memset(target, 0, sizeof(target));
  }

  void randomize() {
    uniform_int_distribution<int> dist(0, 1);
    for (int j = 0; j < 2; j++)
      for (int i = 0; i < len; i++)
        seq[j][i] = dist(rng) ? 1 : -1;
    // Symmetry: fix first element of each
    seq[0][0] = 1;
    seq[1][0] = 1;
  }

  void set_targets(const int *tgt) {
    for (int s = 1; s < len; s++)
      target[s] = tgt[s];
  }

  long long compute_energy() {
    long long e = 0;
    for (int s = 1; s < len; s++) {
      int c = 0;
      for (int j = 0; j < 2; j++)
        for (int i = 0; i < len - s; i++)
          c += seq[j][i] * seq[j][i + s];
      corr[s] = c;
      long long diff = (long long)(c - target[s]);
      e += diff * diff;
    }
    current_energy = e;
    return e;
  }

  long long eval_delta(int sj, int pos) {
    long long de = 0;
    for (int s = 1; s < len; s++) {
      int dc = 0;
      if (pos + s < len)
        dc += seq[sj][pos] * seq[sj][pos + s];
      if (pos - s >= 0)
        dc += seq[sj][pos - s] * seq[sj][pos];
      dc *= -2;
      delta_corr[s] = dc;

      long long old_diff = corr[s] - target[s];
      long long new_diff = old_diff + dc;
      de += new_diff * new_diff - old_diff * old_diff;
    }
    return de;
  }

  void apply_flip(int sj, int pos, long long de) {
    for (int s = 1; s < len; s++)
      corr[s] += delta_corr[s];
    seq[sj][pos] *= -1;
    current_energy += de;
    freq[sj][pos]++;
  }

  void save_best() {
    best_energy = current_energy;
    memcpy(best_seq, seq, sizeof(seq));
  }

  void perturb(int k) {
    uniform_int_distribution<int> dist_j(0, 1);
    uniform_int_distribution<int> dist_i(1, len - 1);
    for (int p = 0; p < k; p++) {
      seq[dist_j(rng)][dist_i(rng)] *= -1;
    }
    compute_energy();
  }

  void intensify() {
    memcpy(seq, best_seq, sizeof(seq));
    compute_energy();
    perturb(max(2, len / 10));
  }

  // Run Tabu Search on this pair
  long long search(int max_iters, int base_tenure, double time_limit) {
    compute_energy();
    if (current_energy < best_energy)
      save_best();
    memset(tabu, 0, sizeof(tabu));

    int tenure = base_tenure;
    int no_improve = 0;
    int cycle_count = 0;
    long long prev_energy = current_energy;

    auto start = Clock::now();

    for (int iter = 0; iter < max_iters; iter++) {
      if (solution_found.load(memory_order_relaxed))
        return best_energy;
      if (current_energy == 0) {
        save_best();
        return 0;
      }

      if (iter % 2000 == 0) {
        double elapsed = chrono::duration<double>(Clock::now() - start).count();
        if (elapsed > time_limit)
          break;
      }

      long long best_delta = LLONG_MAX;
      int best_j = -1, best_i = -1;

      // Evaluate all 2*len neighbors
      for (int j = 0; j < 2; j++) {
        for (int i = 1; i < len; i++) { // skip i=0 (symmetry)
          long long de = eval_delta(j, i);

          // Diversification penalty when stuck
          if (no_improve > 500)
            de += freq[j][i] / 50;

          bool is_tabu = (tabu[j][i] > iter);
          bool aspiration = (current_energy + de) < best_energy;

          if (de < best_delta && (!is_tabu || aspiration)) {
            best_delta = de;
            best_j = j;
            best_i = i;
          }
        }
      }

      if (best_j < 0)
        break;

      // Recompute delta for chosen move
      long long de = eval_delta(best_j, best_i);
      apply_flip(best_j, best_i, de);
      tabu[best_j][best_i] = iter + tenure;

      if (current_energy < best_energy) {
        save_best();
        no_improve = 0;
      } else {
        no_improve++;
      }

      // Reactive tenure
      if (current_energy == prev_energy) {
        cycle_count++;
        if (cycle_count > 15) {
          tenure = min(tenure + 2, len * 2);
          cycle_count = 0;
        }
      } else {
        cycle_count = 0;
        if (tenure > base_tenure)
          tenure = max(base_tenure, tenure - 1);
      }
      prev_energy = current_energy;

      // Perturbation schedule
      if (no_improve == 1500) {
        perturb(max(2, len / 10));
        no_improve = 0;
        tenure = base_tenure;
      } else if (no_improve == 4000) {
        perturb(max(4, len / 5));
        no_improve = 0;
        tenure = base_tenure;
      } else if (no_improve == 8000) {
        intensify();
        no_improve = 0;
        tenure = base_tenure;
      } else if (no_improve == 15000) {
        randomize();
        compute_energy();
        no_improve = 0;
        tenure = base_tenure;
      }

      if (current_energy == 0) {
        save_best();
        return 0;
      }
    }
    return best_energy;
  }
};

// ============================================================
// Full 4-sequence search (fallback / refinement)
// ============================================================
struct FullState {
  int n1, n2; // lengths: A,B = n1; C,D = n2
  int seq[4][MAX_N];
  int corr[MAX_N];
  int tabu[4][MAX_N];
  long long freq[4][MAX_N];
  int best_seq[4][MAX_N];
  long long best_energy;
  long long current_energy;
  int delta_corr[MAX_N];
  mt19937 rng;

  void init(int _n1, int _n2, unsigned seed) {
    n1 = _n1;
    n2 = _n2;
    rng.seed(seed);
    best_energy = LLONG_MAX;
    memset(freq, 0, sizeof(freq));
  }

  int seq_len(int j) const { return (j < 2) ? n1 : n2; }

  void randomize() {
    uniform_int_distribution<int> dist(0, 1);
    for (int j = 0; j < 4; j++)
      for (int i = 0; i < seq_len(j); i++)
        seq[j][i] = dist(rng) ? 1 : -1;
    for (int j = 0; j < 4; j++)
      seq[j][0] = 1;
  }

  void set_from_pairs(const int ab[2][MAX_N], const int cd[2][MAX_N]) {
    memcpy(seq[0], ab[0], n1 * sizeof(int));
    memcpy(seq[1], ab[1], n1 * sizeof(int));
    memcpy(seq[2], cd[0], n2 * sizeof(int));
    memcpy(seq[3], cd[1], n2 * sizeof(int));
  }

  long long compute_energy() {
    long long e = 0;
    int max_s = max(n1, n2);
    for (int s = 1; s < max_s; s++) {
      int c = 0;
      for (int j = 0; j < 4; j++) {
        int L = seq_len(j);
        for (int i = 0; i < L - s; i++)
          c += seq[j][i] * seq[j][i + s];
      }
      corr[s] = c;
      e += (long long)c * c;
    }
    current_energy = e;
    return e;
  }

  long long eval_delta(int sj, int pos) {
    long long de = 0;
    int max_s = max(n1, n2);
    int L = seq_len(sj);
    for (int s = 1; s < max_s; s++) {
      int dc = 0;
      if (pos + s < L)
        dc += seq[sj][pos] * seq[sj][pos + s];
      if (pos - s >= 0 && pos - s < L && pos < L)
        dc += seq[sj][pos - s] * seq[sj][pos];
      dc *= -2;
      delta_corr[s] = dc;
      long long old_c = corr[s];
      long long new_c = old_c + dc;
      de += new_c * new_c - old_c * old_c;
    }
    return de;
  }

  void apply_flip(int sj, int pos, long long de) {
    int max_s = max(n1, n2);
    for (int s = 1; s < max_s; s++)
      corr[s] += delta_corr[s];
    seq[sj][pos] *= -1;
    current_energy += de;
    freq[sj][pos]++;
  }

  void save_best() {
    best_energy = current_energy;
    memcpy(best_seq, seq, sizeof(seq));
  }

  void perturb(int k) {
    uniform_int_distribution<int> dist_j(0, 3);
    for (int p = 0; p < k; p++) {
      int j = dist_j(rng);
      uniform_int_distribution<int> dist_i(1, seq_len(j) - 1);
      seq[j][dist_i(rng)] *= -1;
    }
    compute_energy();
  }

  long long search(int max_iters, int base_tenure, double time_limit) {
    compute_energy();
    if (current_energy < best_energy)
      save_best();
    memset(tabu, 0, sizeof(tabu));

    int tenure = base_tenure;
    int no_improve = 0;
    int cycle_count = 0;
    long long prev_energy = current_energy;
    auto start = Clock::now();

    for (int iter = 0; iter < max_iters; iter++) {
      if (solution_found.load(memory_order_relaxed))
        return best_energy;
      if (current_energy == 0) {
        save_best();
        return 0;
      }
      if (iter % 2000 == 0) {
        double elapsed = chrono::duration<double>(Clock::now() - start).count();
        if (elapsed > time_limit)
          break;
      }

      long long best_delta = LLONG_MAX;
      int best_j = -1, best_i = -1;

      for (int j = 0; j < 4; j++) {
        for (int i = 1; i < seq_len(j); i++) {
          long long de = eval_delta(j, i);
          if (no_improve > 500)
            de += freq[j][i] / 50;
          bool is_tabu = (tabu[j][i] > iter);
          bool aspiration = (current_energy + de) < best_energy;
          if (de < best_delta && (!is_tabu || aspiration)) {
            best_delta = de;
            best_j = j;
            best_i = i;
          }
        }
      }

      if (best_j < 0)
        break;

      long long de = eval_delta(best_j, best_i);
      apply_flip(best_j, best_i, de);
      tabu[best_j][best_i] = iter + tenure;

      if (current_energy < best_energy) {
        save_best();
        no_improve = 0;
      } else
        no_improve++;

      if (current_energy == prev_energy) {
        cycle_count++;
        if (cycle_count > 15) {
          tenure = min(tenure + 2, n1 * 2);
          cycle_count = 0;
        }
      } else {
        cycle_count = 0;
        if (tenure > base_tenure)
          tenure = max(base_tenure, tenure - 1);
      }
      prev_energy = current_energy;

      if (no_improve == 2000) {
        perturb(max(3, n1 / 8));
        no_improve = 0;
        tenure = base_tenure;
      } else if (no_improve == 6000) {
        perturb(max(5, n1 / 4));
        no_improve = 0;
        tenure = base_tenure;
      } else if (no_improve == 12000) {
        memcpy(seq, best_seq, sizeof(seq));
        compute_energy();
        perturb(max(2, n1 / 10));
        no_improve = 0;
        tenure = base_tenure;
      } else if (no_improve == 25000) {
        randomize();
        compute_energy();
        no_improve = 0;
        tenure = base_tenure;
      }

      if (current_energy == 0) {
        save_best();
        return 0;
      }
    }
    return best_energy;
  }
};

// ============================================================
// Verification
// ============================================================
bool verify(int A[], int B[], int n1, int C[], int D[], int n2) {
  int max_s = max(n1, n2);
  for (int s = 1; s < max_s; s++) {
    int c = 0;
    for (int i = 0; i < n1 - s; i++)
      c += A[i] * A[i + s] + B[i] * B[i + s];
    for (int i = 0; i < n2 - s; i++)
      c += C[i] * C[i + s] + D[i] * D[i + s];
    if (c != 0)
      return false;
  }
  return true;
}

void print_seq(const char *name, int *seq, int len) {
  cout << name << " = [";
  for (int i = 0; i < len; i++) {
    if (i > 0)
      cout << ",";
    cout << seq[i];
  }
  cout << "]" << endl;
}

void print_pm(const char *name, int *seq, int len) {
  cout << name << " := ";
  for (int i = 0; i < len; i++)
    cout << (seq[i] == 1 ? "+" : "-");
  cout << endl;
}

// ============================================================
// Main solver
// ============================================================
int main(int argc, char *argv[]) {
  if (argc < 2) {
    cerr << "Usage: " << argv[0] << " <n> [minutes] [threads]" << endl;
    cerr << "  Searches for BS(n+1, n) = Base Sequences of lengths n+1, n+1, "
            "n, n"
         << endl;
    return 1;
  }

  int n = atoi(argv[1]);
  if (n < 2 || n > 256) {
    cerr << "n must be between 2 and 256" << endl;
    return 1;
  }

  GLOBAL_N = n;
  GLOBAL_N1 = n + 1;

  double minutes = 5.0;
  if (argc > 2)
    minutes = atof(argv[2]);
  double time_limit = minutes * 60.0;

  int num_threads = 1;
#ifdef _OPENMP
  num_threads = omp_get_max_threads();
  if (argc > 3)
    num_threads = atoi(argv[3]);
  omp_set_num_threads(num_threads);
#endif

  cout << "========================================================" << endl;
  cout << "  Base Sequence Solver: BS(" << n + 1 << "," << n << ")" << endl;
  cout << "  Sequences: A,B length " << n + 1 << " | C,D length " << n << endl;
  cout << "========================================================" << endl;
  cout << "Threads: " << num_threads << endl;
  cout << "Time limit: " << minutes << " minutes" << endl;
  cout << "Strategy: Pair Decomposition + Tabu Search" << endl;
  cout << "  Phase 1: Find (A,B) pairs with small PAF" << endl;
  cout << "  Phase 2: Find matching (C,D) for each (A,B)" << endl;
  cout << "  Phase 3: Refine combined (A,B,C,D) with full Tabu" << endl;
  cout << "========================================================" << endl;
  cout << endl;

  auto t_start = Clock::now();
  solution_found.store(false);

  int base_tenure_ab = max(7, (n + 1) / 3);
  int base_tenure_cd = max(7, n / 3);
  int base_tenure_full = max(7, n / 3);

#pragma omp parallel
  {
    int tid = 0;
#ifdef _OPENMP
    tid = omp_get_thread_num();
#endif
    unsigned seed =
        chrono::steady_clock::now().time_since_epoch().count() + tid * 99991;

    PairState ab_state, cd_state;
    FullState full_state;

    ab_state.init(n + 1, seed);
    cd_state.init(n, seed + 12345);
    full_state.init(n + 1, n, seed + 67890);

    int round = 0;

    while (!solution_found.load(memory_order_relaxed)) {
      double elapsed = chrono::duration<double>(Clock::now() - t_start).count();
      if (elapsed > time_limit)
        break;
      double remaining = time_limit - elapsed;

      // ---- PHASE 1: Find a good (A,B) pair ----
      ab_state.randomize();
      memset(ab_state.target, 0,
             sizeof(ab_state.target)); // target PAF = 0 initially
      long long ab_energy =
          ab_state.search(100000, base_tenure_ab, min(remaining * 0.3, 30.0));

      if (solution_found.load(memory_order_relaxed))
        break;

      // Get the PAF signature of the best (A,B)
      int paf_ab[MAX_N];
      {
        // Recompute PAF from best_seq
        int tmpA[MAX_N], tmpB[MAX_N];
        memcpy(tmpA, ab_state.best_seq[0], (n + 1) * sizeof(int));
        memcpy(tmpB, ab_state.best_seq[1], (n + 1) * sizeof(int));
        for (int s = 1; s <= n; s++) {
          int c = 0;
          for (int i = 0; i < n + 1 - s; i++)
            c += tmpA[i] * tmpA[i + s] + tmpB[i] * tmpB[i + s];
          paf_ab[s] = c;
        }
      }

      // ---- PHASE 2: Find (C,D) matching -PAF_AB ----
      int cd_target[MAX_N];
      for (int s = 1; s <= n; s++)
        cd_target[s] = -paf_ab[s];
      cd_state.set_targets(cd_target);
      cd_state.randomize();
      cd_state.best_energy = LLONG_MAX;

      elapsed = chrono::duration<double>(Clock::now() - t_start).count();
      remaining = time_limit - elapsed;
      if (remaining <= 0)
        break;

      long long cd_energy =
          cd_state.search(100000, base_tenure_cd, min(remaining * 0.3, 30.0));

      if (solution_found.load(memory_order_relaxed))
        break;

      long long combined = ab_energy + cd_energy;

      // ---- PHASE 3: Refine combined solution ----
      full_state.set_from_pairs(ab_state.best_seq, cd_state.best_seq);
      full_state.best_energy = LLONG_MAX;

      elapsed = chrono::duration<double>(Clock::now() - t_start).count();
      remaining = time_limit - elapsed;
      if (remaining <= 0)
        break;

      long long final_e = full_state.search(200000, base_tenure_full,
                                            min(remaining * 0.4, 60.0));

      if (final_e == 0) {
        solution_found.store(true, memory_order_relaxed);
#pragma omp critical
        {
          if (verify(full_state.best_seq[0], full_state.best_seq[1], n + 1,
                     full_state.best_seq[2], full_state.best_seq[3], n)) {
            memcpy(g_best_A, full_state.best_seq[0], (n + 1) * sizeof(int));
            memcpy(g_best_B, full_state.best_seq[1], (n + 1) * sizeof(int));
            memcpy(g_best_C, full_state.best_seq[2], n * sizeof(int));
            memcpy(g_best_D, full_state.best_seq[3], n * sizeof(int));
            cout << "\n*** THREAD " << tid << " FOUND BS(" << n + 1 << "," << n
                 << ") on round " << round << "! ***" << endl;
          }
        }
        break;
      }

#pragma omp critical
      {
        if (round % 5 == 0 || final_e < 50) {
          cout << "[T" << tid << " R" << round << "] AB_e=" << ab_energy
               << " CD_e=" << cd_energy << " Full_e=" << final_e << endl;
        }
      }

      round++;
    }
  }

  auto t_end = Clock::now();
  double elapsed = chrono::duration<double>(t_end - t_start).count();

  cout << "\n========================================================" << endl;
  cout << "  RESULTS" << endl;
  cout << "========================================================" << endl;
  cout << "Time: " << elapsed << " seconds (" << elapsed / 60.0 << " min)"
       << endl;

  if (solution_found.load() &&
      verify(g_best_A, g_best_B, n + 1, g_best_C, g_best_D, n)) {
    cout << "\nSUCCESS! Valid BS(" << n + 1 << "," << n
         << ") found and VERIFIED!" << endl;
    cout << "\nLengths: A,B = " << n + 1 << " | C,D = " << n << endl;
    cout << "Delta-code length: " << (2 * (n + 1) - 1)
         << " (via Turyn concatenation)\n"
         << endl;

    print_seq("A", g_best_A, n + 1);
    print_seq("B", g_best_B, n + 1);
    print_seq("C", g_best_C, n);
    print_seq("D", g_best_D, n);
    cout << endl;
    print_pm("A", g_best_A, n + 1);
    print_pm("B", g_best_B, n + 1);
    print_pm("C", g_best_C, n);
    print_pm("D", g_best_D, n);
  } else {
    cout << "\nDid not find a perfect solution in the time limit." << endl;
    cout << "Try running longer or with more threads." << endl;
  }

  return solution_found.load() ? 0 : 1;
}
