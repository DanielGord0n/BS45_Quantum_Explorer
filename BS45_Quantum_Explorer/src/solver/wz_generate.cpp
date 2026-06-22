/*
 * wz_generate.cpp — Wang-Zhu "generate-then-backtrack" solver for BS(n+1,n).
 *
 * Architecture (arXiv:2506.20296 Sec 3 — the method that reached n=41,42,43):
 *   1. Target signature (a,b,c,d) given on cmdline, with a^2+b^2+c^2+d^2 = 4n+2.
 *   2. GENERATE the shorter pair C,D (length n) up front under residue + spectral
 *      constraints instead of a blind joint DFS:
 *        a. Enumerate (p,q) residue-class-sum PROFILES of C,D modulo m=3 then m=6
 *           (Thm 2.3). Keep a (p,q) profile only if there EXISTS at least one valid
 *           (k,r) A,B-profile completing the norm identity
 *              sum_class K^2 + R^2 + P^2 + Q^2 = 4n+2.
 *           mod-6 tightens (a finer partition must also be consistent).
 *        b. For each surviving (p,q) profile, GENERATE the actual C,D sequences
 *           matching those mod-3 class sums, SPECTRAL-FILTERING each sequence DURING
 *           generation with the Thm 2.4 per-sequence PSD bound f_X(theta) <= 4n+2 so
 *           the full Cartesian product is never materialized. Then keep pairs with
 *           the joint hall_ok() bound f_C+f_D <= 4n+2.
 *   3. For each surviving (C,D), BACKTRACK A,B (length n1=n+1) to drive the full
 *      summed NPAF to zero. Exact NPAF==0 recheck before declaring success.
 *
 * Reuses (copied/adapted from wz_exact_t23.cpp): comb tables/init, hall_ok()
 * spectral filter, npaf_at(), the NPAF-bounds backtracking machinery.
 *
 * Usage:  ./wz_generate <n> <a> <b> <c> <d>
 *   e.g.  ./wz_generate 10 5 1 4 0
 *
 * Compile (linux):  g++ -O3 -march=native -std=c++17 -fopenmp -o wz_generate src/solver/wz_generate.cpp
 * Compile (mac):    g++ -O3 -std=c++17 -o wz_generate src/solver/wz_generate.cpp
 */
#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <cmath>
#include <functional>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <unordered_set>
#include <vector>
#ifdef _OPENMP
#include <omp.h>
#endif

using namespace std;
using Clock = chrono::steady_clock;

static int G_N, G_N1;
static long long G_GEN_CAP = 0;  // 0 = no cap; else stop a profile's gen at this many spec-ok seqs
static int G_SIG_A, G_SIG_B, G_SIG_C, G_SIG_D;
static Clock::time_point G_T0;

// Instrumentation counters (research feasibility numbers).
static long long g_prof3 = 0;       // mod-3 (p,q) profiles surviving norm test
static long long g_prof6 = 0;       // mod-6 (p,q) profiles surviving norm test
static long long g_cd_pairs = 0;    // (C,D) pairs passing joint hall_ok
static long long g_C_total = 0;     // total C class-sum candidates examined
static long long g_C_spec_ok = 0;   // C candidates passing per-seq spectral filter
static long long g_D_total = 0;
static long long g_D_spec_ok = 0;
static long long g_ab_backtracks = 0;

// ---- comb tables (from wz_exact_t23.cpp) ----
int comb16[16][4];
int comb8_pos[8][4];
int comb8_neg[8][4];
void init_combs() {
  int p = 0, ni = 0;
  for (int i = 0; i < 16; i++) {
    comb16[i][0] = (i & 8) ? 1 : -1;
    comb16[i][1] = (i & 4) ? 1 : -1;
    comb16[i][2] = (i & 2) ? 1 : -1;
    comb16[i][3] = (i & 1) ? 1 : -1;
    int prod = comb16[i][0] * comb16[i][1] * comb16[i][2] * comb16[i][3];
    if (prod == 1) {
      for (int j = 0; j < 4; j++) comb8_pos[p][j] = comb16[i][j];
      p++;
    } else {
      for (int j = 0; j < 4; j++) comb8_neg[ni][j] = comb16[i][j];
      ni++;
    }
  }
}

// ---- hall_ok DFT basis (from wz_exact_t23.cpp), sized for n up to 256 ----
static double G_HALL_COS[201][256];
static double G_HALL_SIN[201][256];
void init_hall_tables() {
  for (int j = 1; j <= 200; j++) {
    double th = j * M_PI / 100.0;
    for (int i = 0; i < G_N && i < 256; i++) {
      G_HALL_COS[j][i] = cos(i * th);
      G_HALL_SIN[j][i] = sin(i * th);
    }
  }
}

// Joint Thm 2.4 PSD bound: f_C(theta)+f_D(theta) <= 4n+2 at theta=j*pi/100.
bool hall_ok(const int *X, int xlen, const int *Y, int ylen) {
  double limit = 4.0 * G_N + 2.0;
  for (int j = 1; j <= 200; j++) {
    const double *cj = G_HALL_COS[j];
    const double *sj = G_HALL_SIN[j];
    double rx = 0, ix = 0, ry = 0, iy = 0;
    for (int i = 0; i < xlen; i++) { rx += X[i] * cj[i]; ix += X[i] * sj[i]; }
    for (int i = 0; i < ylen; i++) { ry += Y[i] * cj[i]; iy += Y[i] * sj[i]; }
    if (rx * rx + ix * ix + ry * ry + iy * iy > limit + 0.5) return false;
  }
  return true;
}

// Per-sequence Thm 2.4 bound: f_X(theta) <= 4n+2 (the half of the joint bound
// applicable to a single sequence; since f_C,f_D >= 0, f_C <= f_C+f_D <= 4n+2).
bool hall_ok_single(const int *X, int xlen) {
  double limit = 4.0 * G_N + 2.0;
  for (int j = 1; j <= 200; j++) {
    const double *cj = G_HALL_COS[j];
    const double *sj = G_HALL_SIN[j];
    double rx = 0, ix = 0;
    for (int i = 0; i < xlen; i++) { rx += X[i] * cj[i]; ix += X[i] * sj[i]; }
    if (rx * rx + ix * ix > limit + 0.5) return false;
  }
  return true;
}

int npaf_at(const int *A, const int *B, int n1, const int *C, const int *D,
            int n2, int s) {
  int c = 0;
  if (s < n1)
    for (int i = 0; i < n1 - s; i++) c += A[i] * A[i + s] + B[i] * B[i + s];
  if (s < n2)
    for (int i = 0; i < n2 - s; i++) c += C[i] * C[i + s] + D[i] * D[i + s];
  return c;
}

// ============================ Residue-profile (Thm 2.3) machinery ============

// class_count(L,c,m) = #positions i in [0,L) with i%m==c.
static int class_count(int L, int c, int m) {
  int n = 0;
  for (int p = c; p < L; p += m) n++;
  return n;
}

// Enumerate all class-sum tuples (length m) over a +/-1 sequence of length L
// whose total sum == target. Generalizes enum_class_sums to arbitrary modulus m.
static vector<vector<int>> enum_class_sums(int L, int target, int m) {
  int cnt[8];
  for (int c = 0; c < m; c++) cnt[c] = class_count(L, c, m);
  vector<vector<int>> out;
  vector<int> cur(m, 0);
  // recursive fill of classes 0..m-2, last class forced by target.
  function<void(int, int)> rec = [&](int c, int acc) {
    if (c == m - 1) {
      int last = target - acc;
      if (last < -cnt[c] || last > cnt[c]) return;
      if (((last - (-cnt[c])) & 1) != 0) return;  // parity
      cur[c] = last;
      out.push_back(cur);
      return;
    }
    for (int v = -cnt[c]; v <= cnt[c]; v += 2) {
      cur[c] = v;
      rec(c + 1, acc + v);
    }
  };
  rec(0, 0);
  return out;
}

static int norm_vec(const vector<int> &v) {
  int s = 0;
  for (int x : v) s += x * x;
  return s;
}

// A mod-m profile (p,q) for (C,D) is FEASIBLE if there exists an A,B profile (k,r)
// of the right sums with sum of squared class sums over all four == 4n+2. We
// precompute, for the A,B sums (a,b), the SET of achievable values of
// (norm(K)+norm(R)). Then a (p,q) is feasible iff (4n+2 - norm(P)-norm(Q)) is in
// that set. This is exactly the Thm-2.3 norm identity restricted to class sums.
struct ABNormSet {
  unordered_set<int> achievable;  // values of norm(K)+norm(R)
  void build(int n1, int a, int b, int m) {
    auto K = enum_class_sums(n1, a, m);
    auto R = enum_class_sums(n1, b, m);
    vector<int> K2(K.size()), R2(R.size());
    for (size_t i = 0; i < K.size(); i++) K2[i] = norm_vec(K[i]);
    for (size_t i = 0; i < R.size(); i++) R2[i] = norm_vec(R[i]);
    int tgt = 4 * G_N + 2;
    for (int kv : K2)
      for (int rv : R2)
        if (kv + rv <= tgt) achievable.insert(kv + rv);
  }
  bool feasible(int need) const { return achievable.count(need) != 0; }
};

// ============================ C,D generation =================================

// Generate all +/-1 sequences of length L whose mod-3 class sums equal `target`
// (a length-3 vector), spectral-filtering each completed sequence with the
// per-sequence Thm-2.4 bound. To avoid materializing the full Cartesian product
// we prune during the DFS: track per-class remaining capacity (a class sum can
// still be reached only if |target[c]-partial[c]| <= remaining[c]) so dead
// branches die immediately. Optionally pin X[0]=+1 (sound when the sequence's
// signature sum is 0, by single-sequence negation symmetry).
//
// Results are appended to `out` (each is a length-L vector). `examined` counts
// fully-built sequences; `spec_ok` counts those that pass the spectral filter.
static void gen_seqs_for_profile(int L, const vector<int> &target3, bool pin0,
                                 vector<vector<int>> &out,
                                 long long &examined, long long &spec_ok) {
  // remaining[c] = positions still unassigned in class c (mod 3) from index i on.
  int total_in_class[3];
  for (int c = 0; c < 3; c++) total_in_class[c] = class_count(L, c, 3);

  vector<int> X(L, 0);
  // partial[c], placed[c] tracked incrementally.
  function<void(int, array<int,3>, array<int,3>)> rec =
      [&](int i, array<int,3> partial, array<int,3> placed) {
    // Feasibility prune: for every class, the remaining positions must be able to
    // bridge target - partial in parity and magnitude.
    for (int c = 0; c < 3; c++) {
      int rem = total_in_class[c] - placed[c];
      int diff = target3[c] - partial[c];
      if (diff < -rem || diff > rem) return;
      if (((diff - (-rem)) & 1) != 0) return;
    }
    if (i == L) {
      examined++;
      if (hall_ok_single(X.data(), L)) {
        spec_ok++;
        out.push_back(X);
      }
      return;
    }
    // Optional cap on spectrally-OK sequences kept (feasibility probe only).
    if (G_GEN_CAP > 0 && spec_ok >= G_GEN_CAP) return;
    int c = i % 3;
    int lo = -1, hi = 1;
    if (i == 0 && pin0) lo = 1;  // symmetry pin
    for (int v = hi; v >= lo; v -= 2) {
      X[i] = v;
      auto np = partial; np[c] += v;
      auto pl = placed; pl[c]++;
      rec(i + 1, np, pl);
    }
    X[i] = 0;
  };
  rec(0, {0, 0, 0}, {0, 0, 0});
}

// ============================ A,B backtracking ===============================
//
// Given fixed C,D (length n), find A,B (length n1) with the full summed NPAF
// zero at every shift s>=1. We fix the CD-only contribution once:
//   cd[s] = sum_i C_i C_{i+s} + D_i D_{i+s}
// then need ab[s] = sum_i A_i A_{i+s} + B_i B_{i+s} = -cd[s] for all s in [1,n].
// (For s>=n1 the AB part is 0, but a real BS solution has total NPAF 0, so cd[s]
// must already be 0 there; we check it.)
//
// Backtrack A,B together, position pos = 0..n1-1, placing A[pos],B[pos] (4 combos)
// at each step. Bounds prune: Dab[s] is the running ab-correlation accumulated
// from placed positions; Kab[s] is how many product terms at shift s are still
// undetermined. Target for shift s is t[s] = -cd[s]. Feasible iff
//   |t[s] - Dab[s]| <= Kab[s]   for all s.
// Each undetermined term contributes +/-1 so this is the standard sound bound.

static int g_solA[256], g_solB[256], g_solC[256], g_solD[256];
static atomic<bool> g_found{false};

static int CD_target[256];  // t[s] = -(C,D npaf at s), s in [1,n]
static int G_ABS_A = 0, G_ABS_B = 0;  // |target sumA|, |target sumB|

// Per-(C,D) AB-backtrack node budget. 0 = unlimited (exact). When >0, a single
// (C,D)'s backtrack aborts once it spends this many nodes without a hit; the
// driver then moves to the next (C,D). For a KNOWN-solvable target this turns the
// search into a fast "find any solution" probe (the solution-bearing C,D completes
// well under budget) while skipping monster dead AB trees. Not exhaustive.
static long long G_AB_BUDGET = 0;
static long long g_ab_cur = 0;        // nodes spent on the current (C,D)
static bool g_ab_aborted = false;     // current (C,D) hit the budget

static bool ab_search(int pos, int *A, int *B, int *Dab, int *Kab,
                      int sumA, int sumB) {
  if (g_found.load(memory_order_relaxed)) return true;
  if (g_ab_aborted) return false;
  int n = G_N, n1 = G_N1;
  // Sum-reachability prune: with rem = n1-pos positions left, sumA can still land
  // on +/-|a| only if |(+/-|a|) - sumA| <= rem for one of the two signs. Same B.
  int rem = n1 - pos;
  bool a_ok = (abs(G_ABS_A - sumA) <= rem) || (abs(-G_ABS_A - sumA) <= rem);
  bool b_ok = (abs(G_ABS_B - sumB) <= rem) || (abs(-G_ABS_B - sumB) <= rem);
  if (!a_ok || !b_ok) return false;
  if (pos == n1) {
    // Final sum must equal +/-|a| (and +/-|b|).
    if (abs(sumA) != G_ABS_A || abs(sumB) != G_ABS_B) return false;
    // All AB placed; verify ab[s] == t[s] for all s in [1, n].
    for (int s = 1; s <= n; s++) {
      int ab = 0;
      if (s < n1)
        for (int i = 0; i < n1 - s; i++) ab += A[i] * A[i + s] + B[i] * B[i + s];
      if (ab != CD_target[s]) return false;
    }
    return true;
  }
  // Snapshot bounds for restore.
  int Dsnap[256], Ksnap[256];
  int span = n + 1;
  memcpy(Dsnap, Dab, span * sizeof(int));
  memcpy(Ksnap, Kab, span * sizeof(int));

  for (int ai = 0; ai < 2; ai++) {
    for (int bi = 0; bi < 2; bi++) {
      int av = ai ? 1 : -1, bv = bi ? 1 : -1;
      A[pos] = av; B[pos] = bv;
      // Update Dab/Kab for terms (i,pos) with i<pos already placed (shift pos-i).
      for (int i = 0; i < pos; i++) {
        int s = pos - i;
        Dab[s] += A[i] * av + B[i] * bv;
        Kab[s] -= 2;  // one A-term, one B-term become determined
      }
      g_ab_backtracks++;
      if (G_AB_BUDGET > 0 && ++g_ab_cur > G_AB_BUDGET) {
        g_ab_aborted = true;
        memcpy(Dab, Dsnap, span * sizeof(int));
        memcpy(Kab, Ksnap, span * sizeof(int));
        A[pos] = B[pos] = 0;
        return false;
      }
      bool prune = false;
      for (int s = 1; s <= n; s++) {
        if (abs(CD_target[s] - Dab[s]) > Kab[s]) { prune = true; break; }
      }
      if (!prune) {
        if (ab_search(pos + 1, A, B, Dab, Kab, sumA + av, sumB + bv))
          return true;
      }
      // restore
      memcpy(Dab, Dsnap, span * sizeof(int));
      memcpy(Kab, Ksnap, span * sizeof(int));
      A[pos] = B[pos] = 0;
    }
  }
  return false;
}

// Try to complete A,B for a fixed (C,D). Returns true and fills g_sol* on success.
static bool complete_ab(const int *C, const int *D) {
  int n = G_N, n1 = G_N1;
  // CD-only correlation.
  for (int s = 1; s <= n; s++) {
    int cd = 0;
    for (int i = 0; i + s < n; i++) cd += C[i] * C[i + s] + D[i] * D[i + s];
    CD_target[s] = -cd;
  }
  // For s in [n1, n]: AB part is identically 0, so cd[s] must be 0 already.
  for (int s = n1; s <= n; s++)
    if (CD_target[s] != 0) return false;
  // Cheap feasibility pre-filter: ab[s] is a sum of 2*(n1-s) terms of +/-1, so
  // |ab[s]| <= 2*(n1-s). If any required CD_target[s] exceeds that, NO A,B can
  // match this (C,D) -> skip before the expensive backtrack. (Sound: a real
  // solution has ab[s] = CD_target[s] within this range.)
  for (int s = 1; s < n1; s++)
    if (abs(CD_target[s]) > 2 * (n1 - s)) return false;

  // AB sum check: total sum_A must equal sig a, sum_B == b. The backtrack does
  // not pin the sum but the signature target is implied by the norm identity;
  // we let the NPAF bounds drive it and verify sig at the end.
  int A[256], B[256], Dab[256], Kab[256];
  memset(A, 0, sizeof(A)); memset(B, 0, sizeof(B));
  memset(Dab, 0, sizeof(Dab));
  for (int s = 0; s <= n; s++) {
    if (s == 0) Kab[s] = 0;
    else if (s < n1) Kab[s] = 2 * (n1 - s);  // A-terms + B-terms at shift s
    else Kab[s] = 0;
  }
  G_ABS_A = abs(G_SIG_A);
  G_ABS_B = abs(G_SIG_B);
  g_ab_cur = 0;
  g_ab_aborted = false;
  if (!ab_search(0, A, B, Dab, Kab, 0, 0)) return false;

  // Verify full NPAF == 0 and record.
  for (int s = 1; s <= n; s++)
    if (npaf_at(A, B, n1, C, D, n, s) != 0) return false;
  memcpy(g_solA, A, n1 * sizeof(int));
  memcpy(g_solB, B, n1 * sizeof(int));
  memcpy(g_solC, C, n * sizeof(int));
  memcpy(g_solD, D, n * sizeof(int));
  return true;
}

// ============================ Driver =========================================

int main(int argc, char **argv) {
  if (argc < 6) {
    cerr << "Usage: " << argv[0] << " <n> <a> <b> <c> <d>\n"
         << "  e.g.  " << argv[0] << " 10 5 1 4 0   (BS(11,10))\n";
    return 1;
  }
  int n = atoi(argv[1]);
  if (n < 4 || n % 2 != 0) { cerr << "ERROR: n even and >=4\n"; return 1; }
  G_N = n; G_N1 = n + 1;
  G_SIG_A = atoi(argv[2]); G_SIG_B = atoi(argv[3]);
  G_SIG_C = atoi(argv[4]); G_SIG_D = atoi(argv[5]);
  int sigsum = G_SIG_A*G_SIG_A + G_SIG_B*G_SIG_B + G_SIG_C*G_SIG_C + G_SIG_D*G_SIG_D;
  if (sigsum != 4 * n + 2) {
    cerr << "ERROR: sig norm = " << sigsum << ", expected " << (4*n+2) << "\n";
    return 1;
  }
  bool instrument = getenv("WZ_INSTRUMENT") != nullptr;
  if (const char *gc = getenv("WZ_GEN_CAP")) G_GEN_CAP = atoll(gc);
  if (const char *ab = getenv("WZ_AB_BUDGET")) G_AB_BUDGET = atoll(ab);

  init_combs();
  init_hall_tables();

  cout << "========================================================\n";
  cout << "  wz_generate — generate-then-backtrack BS(" << G_N1 << "," << n
       << "), sig=(" << G_SIG_A << "," << G_SIG_B << "," << G_SIG_C << ","
       << G_SIG_D << ")\n";
  cout << "========================================================\n" << flush;

  G_T0 = Clock::now();

  // ---- Step 2a: enumerate mod-3 (then mod-6 tighten) (p,q) profiles, keep only
  // those with a valid (k,r) A,B completion of the norm identity. ----
  ABNormSet ab3, ab6;
  ab3.build(G_N1, G_SIG_A, G_SIG_B, 3);
  ab6.build(G_N1, G_SIG_A, G_SIG_B, 6);
  int tgt = 4 * n + 2;

  // mod-3 profiles for C and D.
  auto P3 = enum_class_sums(n, G_SIG_C, 3);
  auto Q3 = enum_class_sums(n, G_SIG_D, 3);
  // mod-6 profiles for C and D (for the tightening test).
  auto P6 = enum_class_sums(n, G_SIG_C, 6);
  auto Q6 = enum_class_sums(n, G_SIG_D, 6);

  // Index mod-6 profiles by their mod-3 reduction so we can test mod-6 feasibility
  // for a given mod-3 (p,q). p3[c] = p6[c] + p6[c+3].
  auto reduce63 = [](const vector<int> &v6) {
    return vector<int>{v6[0] + v6[3], v6[1] + v6[4], v6[2] + v6[5]};
  };

  // Surviving mod-3 (p,q) profiles (after the mod-6 tightening too).
  struct Profile { vector<int> p3, q3; };
  vector<Profile> surviving;

  for (auto &p3 : P3) {
    int np = norm_vec(p3);
    if (np > tgt) continue;
    for (auto &q3 : Q3) {
      int need = tgt - np - norm_vec(q3);
      if (need < 0) continue;
      if (!ab3.feasible(need)) continue;
      g_prof3++;
      // mod-6 tighten: there must exist mod-6 profiles (p6,q6) reducing to (p3,q3)
      // whose norms admit an A,B mod-6 completion.
      bool ok6 = false;
      for (auto &p6 : P6) {
        if (reduce63(p6) != p3) continue;
        int np6 = norm_vec(p6);
        if (np6 > tgt) continue;
        for (auto &q6 : Q6) {
          if (reduce63(q6) != q3) continue;
          int need6 = tgt - np6 - norm_vec(q6);
          if (need6 < 0) continue;
          if (ab6.feasible(need6)) { ok6 = true; break; }
        }
        if (ok6) break;
      }
      if (!ok6) continue;
      g_prof6++;
      surviving.push_back({p3, q3});
    }
  }

  cout << "[profiles] mod-3 (p,q) surviving norm test: " << g_prof3
       << "; after mod-6 tighten: " << g_prof6 << "\n" << flush;

  // ---- Step 2b + 3: for each surviving profile generate C,D and backtrack A,B. ----
  bool pinC = (G_SIG_C == 0);
  bool pinD = (G_SIG_D == 0);

  // MEASURE mode (env WZ_MEASURE): for feasibility study only. Generate C,D for a
  // limited number of profiles (env WZ_MEASURE = how many, default 1), report
  // per-profile spec-filter survival and the joint (C,D) hall_ok count, then
  // estimate the total generated (C,D) set across ALL surviving profiles WITHOUT
  // running the AB backtrack. Use for n=42 where the full run is not attempted.
  if (const char *mz = getenv("WZ_MEASURE")) {
    int kprof = atoi(mz); if (kprof < 1) kprof = 1;
    if (kprof > (int)surviving.size()) kprof = surviving.size();
    cout << "[MEASURE] sampling " << kprof << " of " << surviving.size()
         << " surviving profiles\n" << flush;
    double sumPairsPerProf = 0;
    for (int pi = 0; pi < kprof; pi++) {
      auto &prof = surviving[pi];
      vector<vector<int>> Cs, Ds;
      long long cEx = 0, cOk = 0, dEx = 0, dOk = 0;
      auto t0 = Clock::now();
      gen_seqs_for_profile(n, prof.p3, pinC, Cs, cEx, cOk);
      gen_seqs_for_profile(n, prof.q3, pinD, Ds, dEx, dOk);
      // Joint hall_ok pair count (this is the real generated CD set for this prof).
      long long pairs = 0;
      for (auto &C : Cs) for (auto &D : Ds)
        if (hall_ok(C.data(), n, D.data(), n)) pairs++;
      double dt = chrono::duration<double>(Clock::now() - t0).count();
      sumPairsPerProf += pairs;
      cout << "  [prof " << pi << "] p3=(" << prof.p3[0] << "," << prof.p3[1]
           << "," << prof.p3[2] << ") q3=(" << prof.q3[0] << "," << prof.q3[1]
           << "," << prof.q3[2] << ")\n"
           << "      C spec-ok " << cOk << "/" << cEx << "  D spec-ok " << dOk
           << "/" << dEx << "  joint (C,D) hall_ok pairs=" << pairs
           << "  [" << dt << "s]\n" << flush;
    }
    double avg = sumPairsPerProf / kprof;
    cout << "[MEASURE] avg joint (C,D) pairs/profile = " << avg
         << "  => est. total over " << surviving.size()
         << " profiles ~ " << (long long)(avg * surviving.size()) << "\n";
    cout << "[MEASURE] (AB backtrack NOT run; this is a generation-set size probe)\n"
         << flush;
    return 0;
  }

  // Cache C-generation per distinct p3 profile (the same p3 may recur with
  // different q3). Same for D per q3.
  unordered_set<int> done_profiles;  // not strictly needed; iterate directly.

  long long profiles_done = 0;
  for (auto &prof : surviving) {
    if (g_found.load()) break;
    vector<vector<int>> Cs, Ds;
    long long cEx = 0, cOk = 0, dEx = 0, dOk = 0;
    gen_seqs_for_profile(n, prof.p3, pinC, Cs, cEx, cOk);
    gen_seqs_for_profile(n, prof.q3, pinD, Ds, dEx, dOk);
    g_C_total += cEx; g_C_spec_ok += cOk;
    g_D_total += dEx; g_D_spec_ok += dOk;

    if (instrument && profiles_done < 3) {
      cout << "  [prof " << profiles_done << "] p3=(" << prof.p3[0] << ","
           << prof.p3[1] << "," << prof.p3[2] << ") q3=(" << prof.q3[0] << ","
           << prof.q3[1] << "," << prof.q3[2] << ")  C: " << cOk << "/" << cEx
           << " spec-ok   D: " << dOk << "/" << dEx << " spec-ok\n" << flush;
    }

    // Joint hall_ok filter + AB backtrack.
    for (auto &C : Cs) {
      if (g_found.load()) break;
      for (auto &D : Ds) {
        if (!hall_ok(C.data(), n, D.data(), n)) continue;
        g_cd_pairs++;
        if (complete_ab(C.data(), D.data())) { g_found.store(true); break; }
      }
    }
    profiles_done++;
  }

  double t = chrono::duration<double>(Clock::now() - G_T0).count();

  if (g_found.load()) {
    int n1 = G_N1;
    int sa = 0, sb = 0, sc = 0, sd = 0;
    for (int i = 0; i < n1; i++) { sa += g_solA[i]; sb += g_solB[i]; }
    for (int i = 0; i < n; i++) { sc += g_solC[i]; sd += g_solD[i]; }
    cout << "\n*** BS(" << n1 << "," << n << ") FOUND ***\n";
    cout << "sig = (" << sa << "," << sb << "," << sc << "," << sd << ")\n";
    cout << "A = {"; for (int i=0;i<n1;i++) cout << g_solA[i] << (i<n1-1?",":""); cout << "};\n";
    cout << "B = {"; for (int i=0;i<n1;i++) cout << g_solB[i] << (i<n1-1?",":""); cout << "};\n";
    cout << "C = {"; for (int i=0;i<n;i++) cout << g_solC[i] << (i<n-1?",":""); cout << "};\n";
    cout << "D = {"; for (int i=0;i<n;i++) cout << g_solD[i] << (i<n-1?",":""); cout << "};\n";
    // Independent NPAF verification.
    int maxv = 0;
    for (int s = 1; s <= n; s++) {
      int v = npaf_at(g_solA, g_solB, n1, g_solC, g_solD, n, s);
      if (abs(v) > maxv) maxv = abs(v);
    }
    cout << "VERIFY: max |NPAF[s]| over s=1.." << n << " = " << maxv
         << (maxv == 0 ? "  (NPAF==0 confirmed)\n" : "  (NONZERO!)\n");
    cout << "Time: " << t << "s\n" << flush;
  } else {
    cout << "\n[" << t << "s] no solution for sig (" << G_SIG_A << "," << G_SIG_B
         << "," << G_SIG_C << "," << G_SIG_D << ")\n";
  }

  cout << "\n=== FEASIBILITY INSTRUMENTATION ===\n";
  cout << "mod-3 profiles surviving: " << g_prof3 << "\n";
  cout << "mod-6 profiles surviving: " << g_prof6 << "\n";
  cout << "C candidates: " << g_C_spec_ok << " spec-ok / " << g_C_total
       << " examined\n";
  cout << "D candidates: " << g_D_spec_ok << " spec-ok / " << g_D_total
       << " examined\n";
  cout << "(C,D) pairs passing joint hall_ok: " << g_cd_pairs << "\n";
  cout << "AB backtrack node count: " << g_ab_backtracks << "\n" << flush;

  return g_found.load() ? 0 : 2;
}
