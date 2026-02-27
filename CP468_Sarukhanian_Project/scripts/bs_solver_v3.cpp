/*
 * Final Base Sequence Solver v3
 * CP493 - Directed Research - Daniel Gordon
 *
 * Key insight: The bottleneck is getting from energy ~8 to 0.
 * Single-flip Tabu gets stuck because the solution requires
 * simultaneous 2-flip moves. This solver adds:
 *   - 2-flip compound moves when energy is low (< threshold)
 *   - Much longer per-restart budget with aggressive perturbation
 *   - BS(n+1, n) unequal length support
 *   - stdout flushing for real-time progress
 *
 * Compile:
 *   g++ -O3 -march=native -std=c++17 -Xpreprocessor -fopenmp \
 *       -I/opt/homebrew/opt/libomp/include -L/opt/homebrew/opt/libomp/lib -lomp
 * \ -o bs_solve bs_solver_v3.cpp
 *
 * Usage:
 *   ./bs_solve <n> [minutes] [threads]
 */

#include <algorithm>
#include <atomic>
#include <chrono>
#include <climits>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <random>

#ifdef _OPENMP
#include <omp.h>
#endif

using namespace std;
using Clock = chrono::steady_clock;

static const int MX = 300;
static atomic<bool> found{false};
static atomic<long long> g_best{LLONG_MAX};
static int g_sol[4][MX];
static int G_N1, G_N2; // A,B length = N1; C,D length = N2

struct State {
  int n1, n2; // seq lengths: seq[0],seq[1]=n1; seq[2],seq[3]=n2
  int seq[4][MX];
  int corr[MX]; // corr[s] for s=1..max(n1,n2)-1
  int tabu[4][MX];
  long long freq[4][MX];
  int best[4][MX];
  long long best_e, cur_e;
  int dc[MX]; // delta corr buffer
  mt19937 rng;
  int max_s;

  int slen(int j) { return j < 2 ? n1 : n2; }

  void init(int _n1, int _n2, unsigned seed) {
    n1 = _n1;
    n2 = _n2;
    max_s = max(n1, n2);
    rng.seed(seed);
    best_e = LLONG_MAX;
    memset(freq, 0, sizeof(freq));
  }

  void randomize() {
    uniform_int_distribution<int> d(0, 1);
    for (int j = 0; j < 4; j++)
      for (int i = 0; i < slen(j); i++)
        seq[j][i] = d(rng) ? 1 : -1;
    for (int j = 0; j < 4; j++)
      seq[j][0] = 1; // symmetry
  }

  long long calc_energy() {
    long long e = 0;
    for (int s = 1; s < max_s; s++) {
      int c = 0;
      for (int j = 0; j < 4; j++) {
        int L = slen(j);
        for (int i = 0; i < L - s; i++)
          c += seq[j][i] * seq[j][i + s];
      }
      corr[s] = c;
      e += (long long)c * c;
    }
    cur_e = e;
    return e;
  }

  long long delta(int j, int p) {
    long long de = 0;
    int L = slen(j);
    for (int s = 1; s < max_s; s++) {
      int d = 0;
      if (p + s < L)
        d += seq[j][p] * seq[j][p + s];
      if (p - s >= 0)
        d += seq[j][p - s] * seq[j][p];
      d *= -2;
      dc[s] = d;
      long long o = corr[s], n = o + d;
      de += n * n - o * o;
    }
    return de;
  }

  void flip(int j, int p, long long de) {
    for (int s = 1; s < max_s; s++)
      corr[s] += dc[s];
    seq[j][p] *= -1;
    cur_e += de;
    freq[j][p]++;
  }

  void save() {
    best_e = cur_e;
    memcpy(best, seq, sizeof(seq));
  }

  void restore() {
    memcpy(seq, best, sizeof(seq));
    calc_energy();
  }

  void perturb(int k) {
    uniform_int_distribution<int> dj(0, 3);
    for (int i = 0; i < k; i++) {
      int j = dj(rng);
      uniform_int_distribution<int> di(1, slen(j) - 1);
      seq[j][di(rng)] *= -1;
    }
    calc_energy();
  }

  // Standard 1-flip Tabu Search iteration
  bool step_1flip(int iter, int tenure) {
    long long bd = LLONG_MAX;
    int bj = -1, bi = -1;

    for (int j = 0; j < 4; j++) {
      for (int i = 1; i < slen(j); i++) {
        long long de = delta(j, i);
        if (cur_e < 100 && freq[j][i] > 0)
          de += freq[j][i] / 20;
        bool tb = tabu[j][i] > iter;
        bool asp = (cur_e + de) < best_e;
        if (de < bd && (!tb || asp)) {
          bd = de;
          bj = j;
          bi = i;
        }
      }
    }
    if (bj < 0)
      return false;

    long long de = delta(bj, bi);
    flip(bj, bi, de);
    tabu[bj][bi] = iter + tenure;

    if (cur_e < best_e) {
      save();
      return true;
    }
    return false;
  }

  // 2-flip compound move: try all pairs of flips
  // Only used when energy is low (expensive but critical for final gap)
  bool step_2flip(int iter, int tenure) {
    long long bd = LLONG_MAX;
    int bj1 = -1, bi1 = -1, bj2 = -1, bi2 = -1;

    // First flip
    for (int j1 = 0; j1 < 4; j1++) {
      for (int i1 = 1; i1 < slen(j1); i1++) {
        long long de1 = delta(j1, i1);
        int saved_dc1[MX];
        memcpy(saved_dc1, dc, max_s * sizeof(int));

        // Apply first flip temporarily
        for (int s = 1; s < max_s; s++)
          corr[s] += saved_dc1[s];
        seq[j1][i1] *= -1;
        long long e_after1 = cur_e + de1;

        // Second flip
        for (int j2 = j1; j2 < 4; j2++) {
          int start_i2 = (j2 == j1) ? i1 + 1 : 1;
          for (int i2 = start_i2; i2 < slen(j2); i2++) {
            long long de2 = delta(j2, i2);
            long long total_de = de1 + de2;
            bool tb = tabu[j1][i1] > iter || tabu[j2][i2] > iter;
            bool asp = (cur_e + total_de) < best_e;
            if (total_de < bd && (!tb || asp)) {
              bd = total_de;
              bj1 = j1;
              bi1 = i1;
              bj2 = j2;
              bi2 = i2;
            }
          }
        }

        // Undo first flip
        seq[j1][i1] *= -1;
        for (int s = 1; s < max_s; s++)
          corr[s] -= saved_dc1[s];
      }
    }

    if (bj1 < 0)
      return false;

    // Apply both flips
    long long de1 = delta(bj1, bi1);
    flip(bj1, bi1, de1);
    tabu[bj1][bi1] = iter + tenure;

    long long de2 = delta(bj2, bi2);
    flip(bj2, bi2, de2);
    tabu[bj2][bi2] = iter + tenure;

    if (cur_e < best_e) {
      save();
      return true;
    }
    return false;
  }

  long long search(int max_iter, int base_ten, double tlimit) {
    calc_energy();
    if (cur_e < best_e)
      save();
    memset(tabu, 0, sizeof(tabu));

    int ten = base_ten;
    int no_imp = 0;
    int cycles = 0;
    long long prev = cur_e;
    auto t0 = Clock::now();

    // Threshold for switching to 2-flip mode
    long long two_flip_thresh = max(32LL, (long long)(max_s / 2));

    for (int it = 0; it < max_iter; it++) {
      if (found.load(memory_order_relaxed))
        return best_e;
      if (cur_e == 0) {
        save();
        return 0;
      }

      if (it % 5000 == 0) {
        double el = chrono::duration<double>(Clock::now() - t0).count();
        if (el > tlimit)
          break;
      }

      bool improved;
      if (cur_e <= two_flip_thresh && cur_e > 0) {
        // Use expensive 2-flip moves to crack the final gap
        improved = step_2flip(it, ten);
      } else {
        improved = step_1flip(it, ten);
      }

      if (improved)
        no_imp = 0;
      else
        no_imp++;

      // Reactive tenure
      if (cur_e == prev) {
        cycles++;
        if (cycles > 15) {
          ten = min(ten + 2, max_s * 2);
          cycles = 0;
        }
      } else {
        cycles = 0;
        if (ten > base_ten)
          ten = max(base_ten, ten - 1);
      }
      prev = cur_e;

      // Perturbation schedule
      if (no_imp == 3000) {
        perturb(max(3, max_s / 8));
        no_imp = 0;
        ten = base_ten;
      } else if (no_imp == 8000) {
        perturb(max(5, max_s / 4));
        no_imp = 0;
        ten = base_ten;
      } else if (no_imp == 15000) {
        restore();
        perturb(max(2, max_s / 10));
        no_imp = 0;
        ten = base_ten;
      } else if (no_imp == 30000) {
        randomize();
        calc_energy();
        no_imp = 0;
        ten = base_ten;
      }

      if (cur_e == 0) {
        save();
        return 0;
      }
    }
    return best_e;
  }
};

bool verify(int s[4][MX], int n1, int n2) {
  int ms = max(n1, n2);
  for (int s_ = 1; s_ < ms; s_++) {
    int c = 0;
    for (int i = 0; i < n1 - s_; i++)
      c += s[0][i] * s[0][i + s_] + s[1][i] * s[1][i + s_];
    for (int i = 0; i < n2 - s_; i++)
      c += s[2][i] * s[2][i + s_] + s[3][i] * s[3][i + s_];
    if (c != 0)
      return false;
  }
  return true;
}

int main(int argc, char **argv) {
  if (argc < 2) {
    cerr << "Usage: " << argv[0] << " <n> [minutes] [threads]" << endl;
    cerr << "  Searches for BS(n+1, n)" << endl;
    return 1;
  }

  int n = atoi(argv[1]);
  G_N1 = n + 1;
  G_N2 = n;

  double mins = 5.0;
  if (argc > 2)
    mins = atof(argv[2]);

  int threads = 1;
#ifdef _OPENMP
  threads = omp_get_max_threads();
  if (argc > 3)
    threads = atoi(argv[3]);
  omp_set_num_threads(threads);
#endif

  int base_ten = max(7, n / 3);

  cout << "==============================================" << endl;
  cout << "  BS Solver v3: BS(" << G_N1 << "," << G_N2 << ")" << endl;
  cout << "==============================================" << endl;
  cout << "Threads: " << threads << " | Time: " << mins << " min" << endl;
  cout << "Features: 1-flip + 2-flip compound moves" << endl;
  cout << "  2-flip activates when energy < " << max(32, max(G_N1, G_N2) / 2)
       << endl;
  cout << "==============================================" << endl;
  cout << flush;

  auto t0 = Clock::now();

#pragma omp parallel
  {
    int tid = 0;
#ifdef _OPENMP
    tid = omp_get_thread_num();
#endif
    State st;
    st.init(G_N1, G_N2, Clock::now().time_since_epoch().count() + tid * 77777);

    int rd = 0;
    while (!found.load(memory_order_relaxed)) {
      double el = chrono::duration<double>(Clock::now() - t0).count();
      if (el > mins * 60)
        break;

      st.randomize();
      st.best_e = LLONG_MAX;
      double rem = mins * 60 - el;

      long long e = st.search(500000, base_ten, min(rem, 120.0));

      if (e == 0) {
        found.store(true);
#pragma omp critical
        {
          if (verify(st.best, G_N1, G_N2)) {
            memcpy(g_sol, st.best, sizeof(g_sol));
            g_best.store(0);
            cout << "\n*** FOUND BS(" << G_N1 << "," << G_N2 << ") on thread "
                 << tid << " restart " << rd << "! ***" << endl;
            cout << flush;
          }
        }
      }

#pragma omp critical
      {
        long long ge = g_best.load();
        if (st.best_e < ge) {
          g_best.store(st.best_e);
          memcpy(g_sol, st.best, sizeof(g_sol));
        }
        if (rd % 3 == 0 || st.best_e < 50) {
          cout << "[T" << tid << " R" << rd << "] best=" << st.best_e
               << " global=" << g_best.load() << endl;
          cout << flush;
        }
      }
      rd++;
    }
  }

  double total = chrono::duration<double>(Clock::now() - t0).count();
  cout << "\n==============================================" << endl;
  cout << "Time: " << total << "s (" << total / 60 << " min)" << endl;

  long long fe = g_best.load();
  if (fe == 0 && verify(g_sol, G_N1, G_N2)) {
    cout << "SUCCESS! BS(" << G_N1 << "," << G_N2 << ") VERIFIED!\n" << endl;
    const char *nm[] = {"A", "B", "C", "D"};
    int lens[] = {G_N1, G_N1, G_N2, G_N2};
    for (int j = 0; j < 4; j++) {
      cout << nm[j] << " = [";
      for (int i = 0; i < lens[j]; i++) {
        if (i)
          cout << ",";
        cout << g_sol[j][i];
      }
      cout << "]" << endl;
    }
    cout << endl;
    for (int j = 0; j < 4; j++) {
      cout << nm[j] << " := ";
      for (int i = 0; i < lens[j]; i++)
        cout << (g_sol[j][i] == 1 ? "+" : "-");
      cout << endl;
    }
    cout << "\nDelta-code length: " << (2 * G_N1 - 1) << endl;
  } else {
    cout << "No solution found. Best energy: " << fe << endl;
  }

  return fe == 0 ? 0 : 1;
}
