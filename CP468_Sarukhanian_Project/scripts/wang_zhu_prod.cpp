/*
 * Wang-Zhu BS Solver — Final Production
 * CP493 - Directed Research - Daniel Gordon
 *
 * Based on arXiv:2506.20296 (2025) — the algorithm that found BS for
 * n=41,42,43.
 *
 * Strategy:
 *   1. Enumerate valid (a,b,c,d) sum signatures (Theorem 2.1)
 *   2. Generate C,D via backtracking with NPAF feasibility pruning
 *      at each step: for each shift s, check that the partial CD
 *      autocorrelation can still be cancelled by some AB pair
 *   3. Filter C,D by Hall polynomial (Theorem 2.4)
 *   4. Backtrack A,B with NPAF pruning: after placing each element,
 *      check that the combined NPAF can still reach 0
 *
 * Compile:
 *   g++ -O3 -march=native -std=c++17 -Xpreprocessor -fopenmp \
 *       -I/opt/homebrew/opt/libomp/include -L/opt/homebrew/opt/libomp/lib -lomp
 * \ -o wz_prod wang_zhu_prod.cpp
 *
 * Usage:
 *   ./wz_prod <n> [threads]
 */

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstring>
#include <iostream>
#include <vector>

#ifdef _OPENMP
#include <omp.h>
#endif

using namespace std;
using Clock = chrono::steady_clock;

static const int MX = 128;
static atomic<bool> g_found{false};
static int g_sol[4][MX];
static int G_N;
static Clock::time_point G_T0;

// ============================================================
// Hall polynomial filter
// ============================================================
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

// ============================================================
// Sum signatures
// ============================================================
struct Sig {
  int a, b, c, d;
};

vector<Sig> get_sigs(int n) {
  vector<Sig> sigs;
  int T = 4 * n + 2, n1 = n + 1;
  int ap = n1 % 2, cp = n % 2;
  for (int a = 0; a <= n1; a++) {
    if (a % 2 != ap)
      continue;
    if (a * a > T)
      continue;
    for (int b = -n1; b <= n1; b++) {
      if (((b % 2) + 2) % 2 != ap)
        continue;
      int r = T - a * a - b * b;
      if (r < 0)
        continue;
      for (int c = -n; c <= n; c++) {
        if (((c % 2) + 2) % 2 != cp)
          continue;
        int d2 = r - c * c;
        if (d2 < 0)
          continue;
        int d = (int)round(sqrt((double)d2));
        if (d * d != d2 || d > n)
          continue;
        if (((d % 2) + 2) % 2 != cp)
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
          bool ok2 = true;
          if (n % 2 == 0 && ((c + d) % 4 + 8) % 4 != 0)
            ok2 = false;
          if (ok2)
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

// ============================================================
// C,D generation with NPAF feasibility pruning
// ============================================================
struct CDPair {
  int C[MX], D[MX];
};

static atomic<long long> g_cd_t{0}, g_cd_ok{0};

void gen_cd(int n, int n1, int tc, int td, int pos, int *C, int *D, int sc,
            int sd, vector<CDPair> &out) {
  if (g_found.load(memory_order_relaxed))
    return;

  if (pos == n) {
    if (sc != tc || sd != td)
      return;
    g_cd_t.fetch_add(1, memory_order_relaxed);

    // Final feasibility: CD corr at each shift must be cancelable by AB
    int ms = max(n1, n);
    for (int s = 1; s < ms; s++) {
      int cd_c = 0;
      if (s < n)
        for (int i = 0; i < n - s; i++)
          cd_c += C[i] * C[i + s] + D[i] * D[i + s];
      int max_ab = (s < n1) ? 2 * (n1 - s) : 0;
      if (abs(cd_c) > max_ab)
        return;
    }

    if (!hall_ok(C, n, D, n))
      return;

    g_cd_ok.fetch_add(1, memory_order_relaxed);
    CDPair p;
    memcpy(p.C, C, n * sizeof(int));
    memcpy(p.D, D, n * sizeof(int));
    out.push_back(p);
    return;
  }

  int remaining = n - pos - 1;

  for (int cv : {-1, 1}) {
    int nsc = sc + cv;
    if (abs(tc - nsc) > remaining)
      continue;

    for (int dv : {-1, 1}) {
      int nsd = sd + dv;
      if (abs(td - nsd) > remaining)
        continue;

      C[pos] = cv;
      D[pos] = dv;

      // Pruning: recompute partial CD correlations from scratch
      // Elements 0..pos are placed. For shift s:
      //   partial_cd(s) = sum_{i=0}^{min(pos, n-s-1)} C[i]*C[i+s]+D[i]*D[i+s]
      //   BUT only count terms where BOTH i and i+s are placed (i.e. both <=
      //   pos)
      // Remaining: pairs (i, i+s) where at least one of i, i+s is in [pos+1,
      // n-1]
      bool ok = true;
      int ms = max(n1, n);
      for (int s = 1; s < ms && ok; s++) {
        int partial = 0;
        int max_i = min(pos, n - s - 1);
        // Count terms where both i and i+s are placed (<= pos)
        int fully_known = 0;
        for (int i = 0; i <= max_i; i++) {
          if (i + s <= pos) {
            partial += C[i] * C[i + s] + D[i] * D[i + s];
            fully_known++;
          }
        }
        // Total pairs for shift s in CD = n-s
        // Known pairs = fully_known
        // Remaining pairs: each contributes at most ±2
        int total_pairs = (s < n) ? n - s : 0;
        int rem_cd = (total_pairs - fully_known) * 2;
        int max_ab = (s < n1) ? 2 * (n1 - s) : 0;
        if (abs(partial) > rem_cd + max_ab)
          ok = false;
      }

      if (ok) {
        gen_cd(n, n1, tc, td, pos + 1, C, D, nsc, nsd, out);
      }
    }
  }
}

// ============================================================
// A,B backtracking with NPAF pruning
// cd_full[s] = full CD autocorrelation at shift s (precomputed)
// ============================================================
bool bt_ab(int n1, int n2, const int *cd_full, int *A, int *B, int pos, int ta,
           int tb, int sa, int sb) {
  if (g_found.load(memory_order_relaxed))
    return false;

  if (pos == n1) {
    if (sa != ta || sb != tb)
      return false;
    // Full NPAF check
    int ms = max(n1, n2);
    for (int s = 1; s < ms; s++) {
      int ab_c = 0;
      if (s < n1)
        for (int i = 0; i < n1 - s; i++)
          ab_c += A[i] * A[i + s] + B[i] * B[i + s];
      if (ab_c + cd_full[s] != 0)
        return false;
    }
    return true;
  }

  int remaining = n1 - pos - 1;

  for (int av : {-1, 1}) {
    if (abs(ta - (sa + av)) > remaining)
      continue;
    A[pos] = av;

    for (int bv : {-1, 1}) {
      if (abs(tb - (sb + bv)) > remaining)
        continue;
      B[pos] = bv;

      // Pruning: recompute partial AB correlations from scratch
      bool ok = true;
      int ms = max(n1, n2);
      for (int s = 1; s < ms && ok; s++) {
        int partial = 0;
        int fully_known = 0;
        int max_i = min(pos, n1 - s - 1);
        for (int i = 0; i <= max_i; i++) {
          if (i + s <= pos) {
            partial += A[i] * A[i + s] + B[i] * B[i + s];
            fully_known++;
          }
        }
        int total_ab_pairs = (s < n1) ? n1 - s : 0;
        int rem = (total_ab_pairs - fully_known) * 2;
        int total = partial + cd_full[s];
        if (abs(total) > rem)
          ok = false;
      }

      if (ok) {
        if (bt_ab(n1, n2, cd_full, A, B, pos + 1, ta, tb, sa + av, sb + bv))
          return true;
      }
    }
  }
  return false;
}

// ============================================================
// Main
// ============================================================
int main(int argc, char **argv) {
  if (argc < 2) {
    cerr << "Usage: " << argv[0] << " <n> [threads]" << endl;
    return 1;
  }
  int n = atoi(argv[1]);
  G_N = n;
  int n1 = n + 1;
  int ms = max(n1, n);

  int thr = 1;
#ifdef _OPENMP
  thr = omp_get_max_threads();
  if (argc > 2)
    thr = atoi(argv[2]);
  omp_set_num_threads(thr);
#endif

  cout << "========================================================" << endl;
  cout << "  BS(" << n1 << "," << n << ") — Wang-Zhu Algorithm" << endl;
  cout << "  (arXiv:2506.20296, 2025)" << endl;
  cout << "========================================================" << endl;
  cout << "Threads: " << thr << endl;
  cout << "NPAF pruning: CD gen + AB backtrack" << endl;
  cout << "========================================================\n" << endl;

  G_T0 = Clock::now();
  auto sigs = get_sigs(n);
  cout << "Sum signatures: " << sigs.size() << endl;

  long long total_bt = 0;

  for (int si = 0; si < (int)sigs.size() && !g_found.load(); si++) {
    auto &sig = sigs[si];
    g_cd_t.store(0);
    g_cd_ok.store(0);

    vector<CDPair> cds;
    int tmpC[MX] = {}, tmpD[MX] = {};
    gen_cd(n, n1, sig.c, sig.d, 0, tmpC, tmpD, 0, 0, cds);

    if (cds.empty())
      continue;

    double el = chrono::duration<double>(Clock::now() - G_T0).count();
    cout << "Sig " << si + 1 << "/" << sigs.size() << " (" << sig.a << ","
         << sig.b << "," << sig.c << "," << sig.d << ")"
         << ": " << g_cd_t.load() << " tested, " << cds.size() << " survived"
         << " [" << el << "s]" << endl
         << flush;

// Backtrack A,B for each C,D candidate (parallelized)
#pragma omp parallel for schedule(dynamic, 1) reduction(+ : total_bt)
    for (int ci = 0; ci < (int)cds.size(); ci++) {
      if (g_found.load(memory_order_relaxed))
        continue;

      // Precompute full CD autocorrelation
      int cd_full[MX] = {};
      for (int s = 1; s < ms; s++) {
        int c = 0;
        if (s < n)
          for (int k = 0; k < n - s; k++)
            c += cds[ci].C[k] * cds[ci].C[k + s] +
                 cds[ci].D[k] * cds[ci].D[k + s];
        cd_full[s] = c;
      }

      int A[MX] = {}, B[MX] = {};
      bool ok = bt_ab(n1, n, cd_full, A, B, 0, sig.a, sig.b, 0, 0);
      total_bt++;

      if (ok) {
        // Final verification
        bool valid = true;
        for (int s = 1; s < ms && valid; s++) {
          int c = 0;
          if (s < n1)
            for (int k = 0; k < n1 - s; k++)
              c += A[k] * A[k + s] + B[k] * B[k + s];
          c += cd_full[s];
          if (c != 0)
            valid = false;
        }
        if (valid) {
          g_found.store(true);
#pragma omp critical
          {
            memcpy(g_sol[0], A, n1 * sizeof(int));
            memcpy(g_sol[1], B, n1 * sizeof(int));
            memcpy(g_sol[2], cds[ci].C, n * sizeof(int));
            memcpy(g_sol[3], cds[ci].D, n * sizeof(int));
            double t = chrono::duration<double>(Clock::now() - G_T0).count();
            cout << "\n*** FOUND BS(" << n1 << "," << n << ") in " << t
                 << "s ***" << endl;
          }
        }
      }
    }
  }

  double t = chrono::duration<double>(Clock::now() - G_T0).count();
  cout << "\n========================================================" << endl;
  cout << "Time: " << t << "s (" << t / 60 << " min)" << endl;
  cout << "AB backtrack attempts: " << total_bt << endl;

  if (g_found.load()) {
    cout << "\nSUCCESS! BS(" << n1 << "," << n << ") VERIFIED!\n" << endl;
    const char *nm[] = {"A", "B", "C", "D"};
    int lens[] = {n1, n1, n, n};
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
    cout << "\nDelta-code length: " << (2 * n1 - 1) << endl;
  } else {
    cout << "\nNo solution found." << endl;
  }
  return g_found.load() ? 0 : 1;
}
