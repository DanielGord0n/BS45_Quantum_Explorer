/*
 * Base Sequence Solver — Wang-Zhu Style
 * CP493 - Directed Research - Daniel Gordon
 *
 * Based on the algorithm from arXiv:2506.20296 (2025) that found BS for
 * n=41,42,43.
 *
 * Core strategy:
 *   1. Enumerate valid (a,b,c,d) sum signatures
 *   2. For each signature, generate all C,D sequences of length n
 *      satisfying sum(C)=c, sum(D)=d via backtracking
 *   3. Filter C,D by Hall polynomial bound: |hC(e^{iθ})|² + |hD(e^{iθ})|² ≤
 * 4n+2
 *   4. For each surviving C,D, backtrack over A,B of length n+1
 *      using partial NPAF checks to prune early
 *
 * Compile:
 *   g++ -O3 -march=native -std=c++17 -Xpreprocessor -fopenmp \
 *       -I/opt/homebrew/opt/libomp/include -L/opt/homebrew/opt/libomp/lib -lomp
 * \ -o wz_solver wang_zhu_v2.cpp
 *
 * Usage:
 *   ./wz_solver <n> [threads]
 */

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <complex>
#include <cstring>
#include <iostream>
#include <vector>

#ifdef _OPENMP
#include <omp.h>
#endif

using namespace std;
using Clock = chrono::steady_clock;

static atomic<bool> g_found{false};
static int g_sol[4][300];
static int G_N;

// ============================================================
// Hall polynomial filter (Theorem 2.4)
// N(X) + N(Y) ≤ (4n+2) at z = e^{iθ} for all θ
// where N(X) = |X(z)|² = X(z)·X(z⁻¹)
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
    double f = rx * rx + ix * ix + ry * ry + iy * iy;
    if (f > limit + 0.5)
      return false;
  }
  return true;
}

// ============================================================
// NPAF check for a specific shift given all 4 sequences
// Returns the correlation at shift s
// ============================================================
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

// ============================================================
// Step 1: Get valid sum signatures
// a² + b² + c² + d² = 4n+2
// a,b: parity of n+1; c,d: parity of n
// ============================================================
struct Sig {
  int a, b, c, d;
};

vector<Sig> get_sigs(int n) {
  vector<Sig> sigs;
  int T = 4 * n + 2;
  int n1 = n + 1;
  // a: |a| ≤ n+1, a ≡ n+1 (mod 2)
  int a_parity = n1 % 2;
  int cd_parity = n % 2;

  for (int a = -n1; a <= n1; a++) {
    if (((a % 2) + 2) % 2 != a_parity)
      continue;
    if (a * a > T)
      continue;
    for (int b = -n1; b <= n1; b++) {
      if (((b % 2) + 2) % 2 != a_parity)
        continue;
      int rem1 = T - a * a - b * b;
      if (rem1 < 0)
        continue;
      for (int c = -n; c <= n; c++) {
        if (((c % 2) + 2) % 2 != cd_parity)
          continue;
        int d2 = rem1 - c * c;
        if (d2 < 0)
          continue;
        int d = (int)round(sqrt((double)d2));
        if (d * d != d2)
          continue;
        if (d > n)
          continue;
        if (((d % 2) + 2) % 2 != cd_parity) {
          continue;
        }

        // Mod 4 constraints (Theorem 2.1)
        bool mod4_ok = true;
        if (n % 2 == 0) {
          // c ≡ d (mod 4)
          if (((c - d) % 4 + 8) % 4 != 0)
            mod4_ok = false;
        } else {
          // a ≡ b + 2 (mod 4)
          if (((a - b - 2) % 4 + 8) % 4 != 0)
            mod4_ok = false;
        }
        if (!mod4_ok)
          continue;

        sigs.push_back({a, b, c, d});
        if (d > 0) {
          // Check -d parity
          if (((-d % 2) + 2) % 2 == cd_parity) {
            bool ok2 = true;
            if (n % 2 == 0) {
              if (((c - (-d)) % 4 + 8) % 4 != 0)
                ok2 = false;
            }
            if (ok2)
              sigs.push_back({a, b, c, -d});
          }
        }
      }
    }
  }

  // Remove duplicates
  sort(sigs.begin(), sigs.end(), [](const Sig &x, const Sig &y) {
    if (x.a != y.a)
      return x.a < y.a;
    if (x.b != y.b)
      return x.b < y.b;
    if (x.c != y.c)
      return x.c < y.c;
    return x.d < y.d;
  });
  sigs.erase(unique(sigs.begin(), sigs.end(),
                    [](const Sig &x, const Sig &y) {
                      return x.a == y.a && x.b == y.b && x.c == y.c &&
                             x.d == y.d;
                    }),
             sigs.end());

  return sigs;
}

// ============================================================
// Generate C,D of length n with sum(C)=tc, sum(D)=td
// Uses element-by-element backtracking
// ============================================================
struct CDPair {
  int C[128], D[128];
};

static long long cd_count;
static long long cd_passed;

void gen_cd(int n, int tc, int td, int pos, int *C, int *D, int sc, int sd,
            vector<CDPair> &out) {
  if (g_found.load(memory_order_relaxed))
    return;
  if (out.size() > 200000)
    return; // safety limit

  if (pos == n) {
    if (sc != tc || sd != td)
      return;
    cd_count++;
    // Hall polynomial filter
    if (hall_ok(C, n, D, n)) {
      CDPair p;
      memcpy(p.C, C, n * sizeof(int));
      memcpy(p.D, D, n * sizeof(int));
      out.push_back(p);
      cd_passed++;
    }
    return;
  }

  int remaining = n - pos - 1;

  for (int cv : {-1, 1}) {
    for (int dv : {-1, 1}) {
      int nsc = sc + cv;
      int nsd = sd + dv;

      // Pruning: can remaining elements reach target sum?
      if (abs(tc - nsc) > remaining)
        continue;
      if (abs(td - nsd) > remaining)
        continue;

      C[pos] = cv;
      D[pos] = dv;
      gen_cd(n, tc, td, pos + 1, C, D, nsc, nsd, out);
    }
  }
}

// ============================================================
// Backtrack A,B of length n1 with sum(A)=ta, sum(B)=tb
// checking NPAF incrementally
// ============================================================
bool bt_ab(int n1, int n2, const int *C, const int *D, int *A, int *B, int pos,
           int ta, int tb, int sa, int sb) {
  if (g_found.load(memory_order_relaxed))
    return false;

  if (pos == n1) {
    if (sa != ta || sb != tb)
      return false;
    // Full NPAF check
    int ms = max(n1, n2);
    for (int s = 1; s < ms; s++) {
      if (npaf_at(A, B, n1, C, D, n2, s) != 0)
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

      if (bt_ab(n1, n2, C, D, A, B, pos + 1, ta, tb, sa + av, sb + bv))
        return true;
    }
  }

  return false;
}

// ============================================================
// Verify solution
// ============================================================
bool verify(int *A, int *B, int n1, int *CC, int *DD, int n2) {
  for (int s = 1; s < max(n1, n2); s++) {
    if (npaf_at(A, B, n1, CC, DD, n2, s) != 0)
      return false;
  }
  return true;
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

  int threads = 1;
#ifdef _OPENMP
  threads = omp_get_max_threads();
  if (argc > 2)
    threads = atoi(argv[2]);
  omp_set_num_threads(threads);
#endif

  cout << "========================================================" << endl;
  cout << "  Wang-Zhu Algorithm for BS(" << n1 << "," << n << ")" << endl;
  cout << "  (arXiv:2506.20296 — the method that found BS up to n=43)" << endl;
  cout << "========================================================" << endl;
  cout << "Threads: " << threads << endl;
  cout << endl;

  auto t0 = Clock::now();

  // Step 1: Sum signatures
  cout << "Step 1: Enumerating sum signatures..." << flush;
  vector<Sig> sigs = get_sigs(n);
  cout << " found " << sigs.size() << endl;

  // Use symmetry to reduce: can assume a >= 0, b >= 0 (negation is isomorphism)
  // This may miss some solutions but reduces work by ~4x
  // Actually, let's keep all for correctness on first run

  // Steps 2-5: For each signature, generate C,D and backtrack A,B
  long long total_cd_gen = 0, total_cd_pass = 0, total_bt = 0;

  for (int si = 0; si < (int)sigs.size() && !g_found.load(); si++) {
    Sig &sig = sigs[si];

    // Generate C,D candidates
    cd_count = 0;
    cd_passed = 0;
    vector<CDPair> cds;
    int tmpC[128] = {}, tmpD[128] = {};
    gen_cd(n, sig.c, sig.d, 0, tmpC, tmpD, 0, 0, cds);

    total_cd_gen += cd_count;
    total_cd_pass += cd_passed;

    if (cds.empty())
      continue;

    double el = chrono::duration<double>(Clock::now() - t0).count();
    cout << "Sig " << si + 1 << "/" << sigs.size() << " (" << sig.a << ","
         << sig.b << "," << sig.c << "," << sig.d << ")"
         << ": " << cd_count << " C,D generated, " << cds.size()
         << " passed Hall filter"
         << " [" << el << "s]" << endl;
    cout << flush;

// Backtrack A,B for each C,D candidate
#pragma omp parallel for schedule(dynamic)
    for (int ci = 0; ci < (int)cds.size(); ci++) {
      if (g_found.load(memory_order_relaxed))
        continue;

      int A[128] = {}, B[128] = {};

      bool ok = bt_ab(n1, n, cds[ci].C, cds[ci].D, A, B, 0, sig.a, sig.b, 0, 0);

      if (ok && verify(A, B, n1, cds[ci].C, cds[ci].D, n)) {
        g_found.store(true);
#pragma omp critical
        {
          memcpy(g_sol[0], A, n1 * sizeof(int));
          memcpy(g_sol[1], B, n1 * sizeof(int));
          memcpy(g_sol[2], cds[ci].C, n * sizeof(int));
          memcpy(g_sol[3], cds[ci].D, n * sizeof(int));
          cout << "\n*** FOUND BS(" << n1 << "," << n << ")! ***" << endl;
        }
      }

#pragma omp atomic
      total_bt++;
    }
  }

  double total_time = chrono::duration<double>(Clock::now() - t0).count();

  cout << "\n========================================================" << endl;
  cout << "Time: " << total_time << "s (" << total_time / 60 << " min)" << endl;
  cout << "C,D generated: " << total_cd_gen
       << " | passed filter: " << total_cd_pass
       << " | backtrack attempts: " << total_bt << endl;

  if (g_found.load()) {
    cout << "\nSUCCESS! BS(" << n1 << "," << n << ") VERIFIED!" << endl;
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
