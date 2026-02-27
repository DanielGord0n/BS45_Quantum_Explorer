/*
 * Wang-Zhu BS Solver — Production v3
 * CP493 - Directed Research - Daniel Gordon
 *
 * Based on arXiv:2506.20296 (2025) — found BS for n=41,42,43.
 *
 * KEY OPTIMIZATION: 3-category pair classification for NPAF pruning.
 *
 * For shift s after placing positions 0..pos:
 *   Category 1 (known): both i and i+s ≤ pos → exact contribution
 *   Category 2 (mixed): i ≤ pos but i+s > pos → contribution ∈ {-1, +1}
 *   Category 3 (unknown): both i and i+s > pos → contribution ∈ {-2, +2}
 *
 * This gives MUCH tighter bounds than treating all unknown as ±2.
 *
 * Compile:
 *   g++ -O3 -march=native -std=c++17 -Xpreprocessor -fopenmp \
 *       -I/opt/homebrew/opt/libomp/include -L/opt/homebrew/opt/libomp/lib -lomp
 * \ -o wz_final wang_zhu_final.cpp
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

// NPAF at shift s
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

// Sum signatures
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

// ============================================================
// 3-category NPAF pruning bound at shift s after placing 0..pos
//
// For CD sequences of length n:
//   known pairs: i and i+s both in [0..pos]  → count = max(0, pos+1-s)
//   mixed pairs: i in [0..pos], i+s in [pos+1..n-1] → count = min(pos+1, n-s) -
//   known unknown pairs: both in [pos+1..n-1] → count = (n-s) - known - mixed
//
// known_corr = exact (tracked incrementally)
// mixed bound: each ∈ {-1, +1}, so total ∈ [-mixed_count, +mixed_count]
// unknown bound: each ∈ {-2, +2}, so total ∈ [-2*unknown_count,
// +2*unknown_count] max_slack = mixed_count + 2 * unknown_count
// ============================================================
inline int npaf_slack_cd(int s, int pos, int n, int n1) {
  int total_pairs = (s < n) ? n - s : 0;
  int known = max(0, pos + 1 - s);
  // mixed: i ≤ pos, i+s > pos, i+s < n  →  i ∈ [max(0,pos+1-s), min(pos,
  // n-s-1)] But known counts i ∈ [0, pos-s] (i.e. i+s ≤ pos) mixed counts i ∈
  // [pos-s+1, min(pos, n-s-1)]
  int mixed = min(pos + 1, total_pairs) - known;
  int unknown = total_pairs - known - mixed;
  int max_ab = (s < n1) ? 2 * (n1 - s) : 0;
  return mixed + 2 * unknown + max_ab;
}

inline int npaf_slack_ab(int s, int pos, int n1) {
  int total_pairs = (s < n1) ? n1 - s : 0;
  int known = max(0, pos + 1 - s);
  int mixed = min(pos + 1, total_pairs) - known;
  int unknown = total_pairs - known - mixed;
  return mixed + 2 * unknown;
}

// ============================================================
// C,D generation with tight 3-category pruning
// ============================================================
struct CDPair {
  int C[MX], D[MX];
};
static atomic<long long> g_cd_gen{0}, g_cd_hall{0};

void gen_cd(int n, int n1, int tc, int td, int pos, int *C, int *D, int sc,
            int sd, int *corr, vector<CDPair> &out, int max_cd) {
  if (g_found.load(memory_order_relaxed))
    return;
  if ((int)out.size() >= max_cd)
    return;

  int ms = max(n1, n);

  if (pos == n) {
    if (sc != tc || sd != td)
      return;
    g_cd_gen.fetch_add(1, memory_order_relaxed);
    // Final: all pairs known, check cancelability
    for (int s = 1; s < ms; s++) {
      int max_ab = (s < n1) ? 2 * (n1 - s) : 0;
      if (abs(corr[s]) > max_ab)
        return;
    }
    if (!hall_ok(C, n, D, n))
      return;
    g_cd_hall.fetch_add(1, memory_order_relaxed);
    CDPair p;
    memcpy(p.C, C, n * sizeof(int));
    memcpy(p.D, D, n * sizeof(int));
    out.push_back(p);
    return;
  }

  int rem = n - pos - 1;
  for (int cv : {-1, 1}) {
    int nsc = sc + cv;
    if (abs(tc - nsc) > rem)
      continue;
    for (int dv : {-1, 1}) {
      int nsd = sd + dv;
      if (abs(td - nsd) > rem)
        continue;

      C[pos] = cv;
      D[pos] = dv;

      // Save and update correlations
      int saved[MX];
      memcpy(saved, corr, ms * sizeof(int));
      for (int s = 1; s <= pos && s < n; s++)
        corr[s] += C[pos - s] * cv + D[pos - s] * dv;

      // 3-category pruning
      bool ok = true;
      for (int s = 1; s < ms && ok; s++) {
        int slack = npaf_slack_cd(s, pos, n, n1);
        if (abs(corr[s]) > slack)
          ok = false;
      }

      if (ok)
        gen_cd(n, n1, tc, td, pos + 1, C, D, nsc, nsd, corr, out, max_cd);
      memcpy(corr, saved, ms * sizeof(int));
    }
  }
}

// ============================================================
// A,B backtracking with tight 3-category pruning
// ============================================================
bool bt_ab(int n1, int n2, const int *cd_full, int *A, int *B, int pos, int ta,
           int tb, int sa, int sb, int *corr) {
  if (g_found.load(memory_order_relaxed))
    return false;
  int ms = max(n1, n2);

  if (pos == n1) {
    if (sa != ta || sb != tb)
      return false;
    for (int s = 1; s < ms; s++)
      if (corr[s] + cd_full[s] != 0)
        return false;
    return true;
  }

  int rem = n1 - pos - 1;
  for (int av : {-1, 1}) {
    if (abs(ta - (sa + av)) > rem)
      continue;
    A[pos] = av;
    for (int bv : {-1, 1}) {
      if (abs(tb - (sb + bv)) > rem)
        continue;
      B[pos] = bv;

      int saved[MX];
      memcpy(saved, corr, ms * sizeof(int));
      for (int s = 1; s <= pos && s < n1; s++)
        corr[s] += A[pos - s] * av + B[pos - s] * bv;

      bool ok = true;
      for (int s = 1; s < ms && ok; s++) {
        int slack = npaf_slack_ab(s, pos, n1);
        if (abs(corr[s] + cd_full[s]) > slack)
          ok = false;
      }

      if (ok &&
          bt_ab(n1, n2, cd_full, A, B, pos + 1, ta, tb, sa + av, sb + bv, corr))
        return true;
      memcpy(corr, saved, ms * sizeof(int));
    }
  }
  return false;
}

// ============================================================
int main(int argc, char **argv) {
  if (argc < 2) {
    cerr << "Usage: " << argv[0] << " <n> [threads]" << endl;
    return 1;
  }
  int n = atoi(argv[1]);
  G_N = n;
  int n1 = n + 1, ms = max(n1, n);

  int thr = 1;
#ifdef _OPENMP
  thr = omp_get_max_threads();
  if (argc > 2)
    thr = atoi(argv[2]);
  omp_set_num_threads(thr);
#endif

  cout << "========================================================" << endl;
  cout << "  BS(" << n1 << "," << n << ") — Wang-Zhu Algorithm" << endl;
  cout << "  arXiv:2506.20296 (2025)" << endl;
  cout << "========================================================" << endl;
  cout << "Threads: " << thr << " | 3-category NPAF pruning" << endl;
  cout << "========================================================\n" << endl;

  G_T0 = Clock::now();
  auto sigs = get_sigs(n);
  cout << "Step 1: " << sigs.size() << " sum signatures" << endl;

  long long total_bt = 0;
  int max_cd = 500000;

  for (int si = 0; si < (int)sigs.size() && !g_found.load(); si++) {
    auto &sig = sigs[si];
    g_cd_gen.store(0);
    g_cd_hall.store(0);

    vector<CDPair> cds;
    int tmpC[MX] = {}, tmpD[MX] = {}, cd_corr[MX] = {};
    gen_cd(n, n1, sig.c, sig.d, 0, tmpC, tmpD, 0, 0, cd_corr, cds, max_cd);

    if (cds.empty())
      continue;

    double el = chrono::duration<double>(Clock::now() - G_T0).count();
    cout << "Sig " << si + 1 << "/" << sigs.size() << " (" << sig.a << ","
         << sig.b << "," << sig.c << "," << sig.d << "): " << g_cd_gen.load()
         << " NPAF-ok, " << cds.size() << " Hall-ok [" << el << "s]" << endl
         << flush;

#pragma omp parallel for schedule(dynamic, 1) reduction(+ : total_bt)
    for (int ci = 0; ci < (int)cds.size(); ci++) {
      if (g_found.load(memory_order_relaxed))
        continue;

      int cd_full[MX] = {};
      for (int s = 1; s < ms; s++) {
        if (s < n)
          for (int k = 0; k < n - s; k++)
            cd_full[s] += cds[ci].C[k] * cds[ci].C[k + s] +
                          cds[ci].D[k] * cds[ci].D[k + s];
      }

      int A[MX] = {}, B[MX] = {}, ab_corr[MX] = {};
      bool ok = bt_ab(n1, n, cd_full, A, B, 0, sig.a, sig.b, 0, 0, ab_corr);
      total_bt++;

      if (ok) {
        bool valid = true;
        for (int s = 1; s < ms && valid; s++)
          if (npaf_at(A, B, n1, cds[ci].C, cds[ci].D, n, s) != 0)
            valid = false;
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
  cout << "AB backtrack: " << total_bt << endl;

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
