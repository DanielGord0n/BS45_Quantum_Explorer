/*
 * Wang-Zhu BS Solver — Symmetric Outer-In Version
 * CP493 - Directed Research - Daniel Gordon
 *
 * This version uses THEOREM 2.2 constraints:
 * For A, B: A[i]*B[i]*A[n-i]*B[n-i] = 1 (for i>0), and -1 for i=0.
 * For C, D: C[i]*D[i]*C[n-1-i]*D[n-1-i] = 1 (for i>0, and often i=0 too).
 *
 * Generating candidates pair-by-pair (outside-in) cuts the branching
 * factor in half at every depth.
 *
 * Compile:
 *   g++ -O3 -march=native -std=c++17 -Xpreprocessor -fopenmp \
 *       -I/opt/homebrew/opt/libomp/include -L/opt/homebrew/opt/libomp/lib -lomp
 * \ -o wz_fast wz_fast.cpp
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

struct CDPair {
  int C[MX], D[MX];
};
static atomic<long long> g_cd_gen{0};

// NPAF Slack for symmetric outside-in generation.
// After depth `d`, positions 0..d and (n-1-d)..(n-1) are known.
// i.e., placed positions are: [0, d] U [n-1-d, n-1].
inline int get_slack(int s, int d, int n, int max_ab) {
  int total_pairs = (s < n) ? n - s : 0;

  // Check which pairs (i, i+s) are known.
  // A position p is known if (p <= d) OR (p >= n-1-d)
  int known = 0, mixed = 0, unknown = 0;
  for (int i = 0; i < n - s; i++) {
    bool i_known = (i <= d) || (i >= n - 1 - d);
    bool is_known = ((i + s) <= d) || ((i + s) >= n - 1 - d);
    if (i_known && is_known)
      known++;
    else if (i_known || is_known)
      mixed++;
    else
      unknown++;
  }
  return mixed + 2 * unknown + max_ab;
}

void gen_cd_sym(int n, int n1, int tc, int td, int d, int *C, int *D, int sc,
                int sd, int *corr, vector<CDPair> &out, int max_cd) {
  if (g_found.load(memory_order_relaxed))
    return;
  if ((int)out.size() >= max_cd)
    return;

  int ms = max(n1, n);

  if (d > (n - 1) / 2) {
    if (sc != tc || sd != td)
      return;
    g_cd_gen.fetch_add(1, memory_order_relaxed);
    for (int s = 1; s < ms; s++) {
      int max_ab = (s < n1) ? 2 * (n1 - s) : 0;
      if (abs(corr[s]) > max_ab)
        return;
    }
    if (!hall_ok(C, n, D, n))
      return;
    CDPair p;
    memcpy(p.C, C, n * sizeof(int));
    memcpy(p.D, D, n * sizeof(int));
    out.push_back(p);
    return;
  }

  int left = d;
  int right = n - 1 - d;

  int rem_spots = (left == right) ? 1 : 2;
  // rough sum bounds
  if (abs(tc - sc) > rem_spots + 2 * (right - left - 1))
    return;
  if (abs(td - sd) > rem_spots + 2 * (right - left - 1))
    return;

  // Branches
  for (int cL : {-1, 1}) {
    for (int dL : {-1, 1}) {
      if (left == right) {
        // center element
        int nsc = sc + cL;
        int nsd = sd + dL;
        if (nsc != tc || nsd != td)
          continue;
        C[left] = cL;
        D[left] = dL;

        int saved[MX];
        memcpy(saved, corr, ms * sizeof(int));
        for (int s = 1; s < ms; s++) {
          if (left - s >= 0)
            corr[s] += C[left - s] * cL + D[left - s] * dL;
          if (left + s < n && (left + s > right))
            corr[s] +=
                C[left] * C[left + s] + D[left] * D[left + s]; // already placed
        }

        bool ok = true;
        for (int s = 1; s < ms && ok; s++) {
          int max_ab = (s < n1) ? 2 * (n1 - s) : 0;
          int slack = get_slack(s, d, n, max_ab);
          if (abs(corr[s]) > slack)
            ok = false;
        }

        if (ok)
          gen_cd_sym(n, n1, tc, td, d + 1, C, D, nsc, nsd, corr, out, max_cd);
        memcpy(corr, saved, ms * sizeof(int));
      } else {
        for (int cR : {-1, 1}) {
          for (int dR : {-1, 1}) {
            // THEOREM 2.2 CONSTRAINT
            if (d > 0) { // i=2.. in 1-based, d=1.. in 0-based
              if (cL * dL * cR * dR != 1)
                continue;
            }

            int nsc = sc + cL + cR;
            int nsd = sd + dL + dR;

            C[left] = cL;
            D[left] = dL;
            C[right] = cR;
            D[right] = dR;

            int saved[MX];
            memcpy(saved, corr, ms * sizeof(int));

            // update cross terms involving left
            for (int s = 1; s < ms; s++) {
              if (left - s >= 0)
                corr[s] += C[left - s] * cL + D[left - s] * dL;
              if (left + s > right && left + s < n)
                corr[s] += cL * C[left + s] + dL * D[left + s];
            }
            // update cross terms involving right
            for (int s = 1; s < ms; s++) {
              if (right - s >= 0 && right - s <= left) {
                // avoid double counting if left and right are exactly s apart
                if (right - s == left) {
                  corr[s] += cL * cR + dL * dR;
                } else {
                  corr[s] += C[right - s] * cR + D[right - s] * dR;
                }
              } else if (right - s >= 0 && right - s > left &&
                         right - s < right) {
                // not placed yet, wait
              } else if (right - s >= 0 && right - s < left) {
                corr[s] += C[right - s] * cR + D[right - s] * dR;
              }
              if (right + s < n)
                corr[s] += cR * C[right + s] + dR * D[right + s];
            }

            bool ok = true;
            for (int s = 1; s < ms && ok; s++) {
              int max_ab = (s < n1) ? 2 * (n1 - s) : 0;
              int slack = get_slack(s, d, n, max_ab);
              if (abs(corr[s]) > slack)
                ok = false;
            }

            if (ok)
              gen_cd_sym(n, n1, tc, td, d + 1, C, D, nsc, nsd, corr, out,
                         max_cd);
            memcpy(corr, saved, ms * sizeof(int));
          }
        }
      }
    }
  }
}

// ============================================================
// A,B backtracking
// ============================================================
bool bt_ab_sym(int n1, int n2, const int *cd_full, int *A, int *B, int d,
               int ta, int tb, int sa, int sb, int *corr) {
  if (g_found.load(memory_order_relaxed))
    return false;
  int ms = max(n1, n2);

  if (d > (n1 - 1) / 2) {
    if (sa != ta || sb != tb)
      return false;
    for (int s = 1; s < ms; s++)
      if (corr[s] + cd_full[s] != 0)
        return false;
    return true;
  }

  int left = d;
  int right = n1 - 1 - d;

  if (left == right) {
    for (int aL : {-1, 1}) {
      for (int bL : {-1, 1}) {
        if (sa + aL != ta || sb + bL != tb)
          continue;
        A[left] = aL;
        B[left] = bL;

        int saved[MX];
        memcpy(saved, corr, ms * sizeof(int));
        for (int s = 1; s < ms; s++) {
          if (left - s >= 0)
            corr[s] += A[left - s] * aL + B[left - s] * bL;
          if (left + s < n1 && left + s > right)
            corr[s] += A[left] * A[left + s] + B[left] * B[left + s];
        }

        bool ok = true;
        for (int s = 1; s < ms && ok; s++) {
          int slack =
              get_slack(s, d, n1, 0); // max_ab is 0 for AB sequence itself
          if (abs(corr[s] + cd_full[s]) > slack)
            ok = false;
        }
        if (ok && bt_ab_sym(n1, n2, cd_full, A, B, d + 1, ta, tb, sa + aL,
                            sb + bL, corr))
          return true;
        memcpy(corr, saved, ms * sizeof(int));
      }
    }
  } else {
    for (int aL : {-1, 1}) {
      for (int bL : {-1, 1}) {
        for (int aR : {-1, 1}) {
          for (int bR : {-1, 1}) {

            // THEOREM 2.2 CONSTRAINT FOR A, B
            if (d == 0) {
              if (aL * bL * aR * bR != -1)
                continue;
            } else {
              if (aL * bL * aR * bR != 1)
                continue;
            }

            int nsa = sa + aL + aR;
            int nsb = sb + bL + bR;

            A[left] = aL;
            B[left] = bL;
            A[right] = aR;
            B[right] = bR;

            int saved[MX];
            memcpy(saved, corr, ms * sizeof(int));

            for (int s = 1; s < ms; s++) {
              if (left - s >= 0)
                corr[s] += A[left - s] * aL + B[left - s] * bL;
              if (left + s > right && left + s < n1)
                corr[s] += aL * A[left + s] + bL * B[left + s];
            }
            for (int s = 1; s < ms; s++) {
              if (right - s >= 0 && right - s <= left) {
                if (right - s == left) {
                  corr[s] += aL * aR + bL * bR;
                } else {
                  corr[s] += A[right - s] * aR + B[right - s] * bR;
                }
              } else if (right - s >= 0 && right - s < left) {
                corr[s] += A[right - s] * aR + B[right - s] * bR;
              }
              if (right + s < n1)
                corr[s] += aR * A[right + s] + bR * B[right + s];
            }

            bool ok = true;
            for (int s = 1; s < ms && ok; s++) {
              int slack = get_slack(s, d, n1, 0);
              if (abs(corr[s] + cd_full[s]) > slack)
                ok = false;
            }
            if (ok &&
                bt_ab_sym(n1, n2, cd_full, A, B, d + 1, ta, tb, nsa, nsb, corr))
              return true;
            memcpy(corr, saved, ms * sizeof(int));
          }
        }
      }
    }
  }
  return false;
}

int main(int argc, char **argv) {
  if (argc < 2) {
    cerr << "Usage: " << argv[0] << " <n> " << endl;
    return 1;
  }
  int n = atoi(argv[1]);
  G_N = n;
  int n1 = n + 1, ms = max(n1, n);

  int thr = 1;
#ifdef _OPENMP
  thr = omp_get_max_threads();
  omp_set_num_threads(thr);
#endif

  cout << "========================================================" << endl;
  cout << "  BS(" << n1 << "," << n << ") — Symmetric Accelerated Solver"
       << endl;
  cout << "========================================================" << endl;

  G_T0 = Clock::now();
  auto sigs = get_sigs(n);
  cout << "Step 1: " << sigs.size() << " sum signatures" << endl;

  long long total_bt = 0;
  int max_cd = 1000000;

  for (int si = 0; si < (int)sigs.size() && !g_found.load(); si++) {
    auto &sig = sigs[si];

    vector<CDPair> cds;
    int tmpC[MX] = {}, tmpD[MX] = {}, cd_corr[MX] = {};
    gen_cd_sym(n, n1, sig.c, sig.d, 0, tmpC, tmpD, 0, 0, cd_corr, cds, max_cd);

    if (cds.empty())
      continue;

    double el = chrono::duration<double>(Clock::now() - G_T0).count();
    cout << "Sig " << si + 1 << "/" << sigs.size() << " generated "
         << cds.size() << " CD candidates [" << el << "s]" << endl;

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
      bool ok = bt_ab_sym(n1, n, cd_full, A, B, 0, sig.a, sig.b, 0, 0, ab_corr);
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
  cout << "\nTime: " << t << "s\n" << endl;
  return g_found.load() ? 0 : 1;
}
