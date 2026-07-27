/*
 * wz_match.cpp — HASH-JOIN ("matching") base-sequence solver for BS(n+1,n).
 *
 * Companion to wz_generate.cpp. Where wz_generate.cpp re-backtracks A,B for every
 * generated (C,D) — an O(|A,B| x |C,D|) product — this solver replaces that with an
 * O(|A,B| + |C,D|) hash join, exactly the Wang-Zhu / Đoković–Kotsireas "efficient
 * matching algorithm based on hashing".
 *
 * THE ALGORITHM
 * -------------
 * A base sequence BS(n+1,n) for signature (a,b,c,d), a^2+b^2+c^2+d^2 = 4n+2, is a
 * 4-tuple of +/-1 sequences A,B (length n1 = n+1) and C,D (length n) whose summed
 * non-periodic autocorrelation (NPAF) is zero at every shift s = 1..n:
 *
 *     AB[s] + CD[s] = 0   for s = 1..n
 *
 *   AB[s] = sum_{i=0}^{n1-1-s} (A_i A_{i+s} + B_i B_{i+s})   (A,B length n1=n+1)
 *   CD[s] = sum_{i=0}^{n-1-s}  (C_i C_{i+s} + D_i D_{i+s})   (C,D length n)
 *
 * CAREFUL INDEXING (the crux of the join key):
 *   - The AB vector has n meaningful entries, s = 1..n.  AB[n] uses the single pair
 *     (A_0,A_n)+(B_0,B_n) (well-defined because A,B have length n1 = n+1).
 *   - The CD vector is only defined for s = 1..n-1; at s = n it is identically 0
 *     (C,D have length n, so there is no pair at shift n).
 *   So a solution requires
 *       AB[s] = -CD[s]   for s = 1..n-1   AND   AB[n] = 0.
 *   We encode BOTH conditions in a single length-n key:
 *       AB key  = (AB[1], ..., AB[n-1], AB[n])
 *       CD key  = (-CD[1], ..., -CD[n-1], 0)      // last entry pinned to 0
 *   Matching AB key == CD key simultaneously demands AB[s] = -CD[s] (s<n) and
 *   AB[n] = 0. autocorr_key() builds exactly these: for the CD side L=n, so the
 *   s=n term has no summands and lands at 0 automatically.
 *
 * PIPELINE
 *   1. GENERATE A,B (length n1): residue-profile-constrained (mod-3 then mod-6,
 *      Thm 2.3) and per-pair spectrally filtered (hall_ok, f_A+f_B <= 4n+2). For
 *      each surviving pair compute the length-n AB key and INSERT into a hash map
 *      keyed by that vector (value = up to WZ_HASH_KEEP (A,B) records per key).
 *   2. GENERATE C,D (length n): same residue + spectral filtering. For each pair
 *      compute the length-n lookup key (-CD[1..n-1], 0) and LOOK IT UP in the hash.
 *   3. On a hit, EXACT recheck: npaf_at over the full A,B,C,D for every s=1..n must
 *      be 0. The hash match is necessary; the recheck makes it sufficient and guards
 *      against any int16 hash collision or indexing slip. No FOUND is printed that
 *      has not passed this recheck.
 *
 * Reuses from wz_generate.cpp: hall_ok / hall_ok_single spectral filters,
 * enum_class_sums + PairNormSet residue machinery, gen_seqs_for_profile generator,
 * and npaf_at for the exact recheck.
 *
 * Usage:  ./wz_match <n> <a> <b> <c> <d>
 *   e.g.  ./wz_match 6  5 1 0 0     (BS(7,6))
 *         ./wz_match 10 5 1 4 0     (BS(11,10))
 *
 * MEMORY OPTIMIZATION (compact-key + dedup, see CHANGE notes below):
 *   - The hash key is NO LONGER the full length-n int16 autocorrelation vector. It
 *     is a single 64-bit FNV-1a hash of that vector. Collisions are harmless: every
 *     hit is still confirmed by the exact npaf_at==0 recheck over the full A,B,C,D,
 *     so a false 64-bit collision simply fails the recheck and is skipped.
 *   - We store AT MOST ONE (A,B) (or one (C,D), see below) per distinct 64-bit key.
 *     This is sound: NPAF[s] = AB[s] + CD[s] depends only on the autocorrelations,
 *     not on which specific A,B realized a given autocorrelation. So one
 *     representative per key suffices to find a solution.
 *   - We hash the SMALLER side (fewer hall_ok pairs, estimated by a quick count) and
 *     stream the larger side. The match key is symmetric: we store one side's
 *     length-n key vector (-CD[..],0 or AB[..]) and look up the IDENTICAL vector
 *     produced by the other side, because a solution requires AB == -CD entrywise.
 *
 * Env:
 *   WZ_MEASURE=1    count filtered A,B / C,D and project hash memory; skip the join.
 *   WZ_HASH_KEEP=k  IGNORED for memory (dedup keeps exactly 1 rec/key). Parsed for
 *                   backward-compat but no longer affects storage.
 *
 * Compile (linux):  g++ -O3 -march=native -std=c++17 -fopenmp -o wz_match src/solver/wz_match.cpp
 * Compile (mac):    g++ -O3 -std=c++17 -o wz_match src/solver/wz_match.cpp
 */
#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <cmath>
#include <functional>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <csignal>
#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#ifdef _OPENMP
#include <omp.h>
#endif

using namespace std;
using Clock = chrono::steady_clock;

static int G_N, G_N1;
static int G_SIG_A, G_SIG_B, G_SIG_C, G_SIG_D;
static int G_HASH_KEEP = 4;   // (A,B) records kept per autocorr key
static Clock::time_point G_T0;

// ---- hall_ok DFT basis (from wz_generate.cpp); sized for length up to 256. ----
static double G_HALL_COS[201][256];
static double G_HALL_SIN[201][256];
void init_hall_tables() {
  for (int j = 1; j <= 200; j++) {
    double th = j * M_PI / 100.0;
    for (int i = 0; i < G_N1 && i < 256; i++) {  // n1 covers both lengths n, n+1
      G_HALL_COS[j][i] = cos(i * th);
      G_HALL_SIN[j][i] = sin(i * th);
    }
  }
}

// Joint Thm 2.4 PSD bound: f_X(theta)+f_Y(theta) <= 4n+2 at theta=j*pi/100.
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

// Per-sequence Thm 2.4 bound: f_X(theta) <= 4n+2.
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

// Full summed NPAF at shift s over A,B (length n1) and C,D (length n2).
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

static int class_count(int L, int c, int m) {
  int n = 0;
  for (int p = c; p < L; p += m) n++;
  return n;
}

// All length-m class-sum tuples over a +/-1 sequence of length L summing to target.
static vector<vector<int>> enum_class_sums(int L, int target, int m) {
  int cnt[8];
  for (int c = 0; c < m; c++) cnt[c] = class_count(L, c, m);
  vector<vector<int>> out;
  vector<int> cur(m, 0);
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

// Norm-set of one pair's class-sum completion (used to feasibility-test profiles).
struct PairNormSet {
  unordered_set<int> achievable;  // values of norm(K)+norm(R) for the (a,b) pair
  void build(int L, int a, int b, int m) {
    auto K = enum_class_sums(L, a, m);
    auto R = enum_class_sums(L, b, m);
    int tgt = 4 * G_N + 2;
    for (auto &kv : K) {
      int kn = norm_vec(kv);
      if (kn > tgt) continue;
      for (auto &rv : R) {
        int rn = norm_vec(rv);
        if (kn + rn <= tgt) achievable.insert(kn + rn);
      }
    }
  }
  bool feasible(int need) const { return achievable.count(need) != 0; }
};

// ===== Thm 2.3 eq 2.11b — the RESIDUE-LEVEL AUTOCORRELATION condition =======
// Added 2026-07-15 after reading arXiv:2506.20296 (docs/wz_paper_reconstruction.md).
// eq 2.11 has TWO parts; we only ever implemented the first:
//   2.11a  sum of ALL class-sum squares = 4n+2                      <- PairNormSet
//   2.11b  N_K(s)+N_R(s)+N_P(s)+N_Q(s)
//        + N_K(m-s)+N_R(m-s)+N_P(m-s)+N_Q(m-s) = 0 , s=1..[m/2]     <- THIS
// It is exactly the statement that the NPAF sum vanishes on each nonzero residue
// class mod m, so a true solution ALWAYS satisfies it (canary-verified on all six
// valid banked champions, tools/canary_thm211b.py).
//
// NOTE (proven, and it is why Wang-Zhu lift to m=6): at m=3 this is VACUOUS.
// For length-3 v, N(v,1)+N(v,2) = ((sum v)^2 - norm(v))/2, so the s=1 condition
// collapses to Thm 2.1 minus 2.11a — implied, never false (measured: 1.0x at m=3).
// m=6 is the first modulus carrying new information; only the mod-6 tighten uses it.
static bool G_THM211B = false;   // WZ_THM211B=1 to enable (A/B-able on purpose)

// eq 2.9: NON-circular autocorrelation of a residue vector, N_v(s)=sum v_i*v_{i+s}.
static int autocorr_vec(const vector<int> &v, int s) {
  int m = (int)v.size(), t = 0;
  if (s >= m) return 0;
  for (int i = 0; i + s < m; i++) t += v[i] * v[i + s];
  return t;
}
// One SIDE's eq-2.11b aggregate at shift s. The condition is AB_s + CD_s = 0.
// (At s=m/2 with m even, m-s==s and the term is counted twice on BOTH sides —
//  2X+2Y=0 <=> X+Y=0, so the literal formula stays consistent.)
static int pair_auto(const vector<int> &x, const vector<int> &y, int s) {
  int m = (int)x.size();
  return autocorr_vec(x, s) + autocorr_vec(y, s)
       + autocorr_vec(x, m - s) + autocorr_vec(y, m - s);
}
// Pack (T_1..T_[m/2], norm) into one 64-bit key. |T| <= 4*(4n+2) fits int16 with
// the +8192 bias for every n we can reach; norm <= 4n+2 fits unbiased.
static uint64_t auto_key(const vector<int> &T, int norm) {
  uint64_t k = (uint64_t)(uint16_t)norm;
  for (size_t i = 0; i < T.size() && i < 3; i++)
    k |= ((uint64_t)(uint16_t)(T[i] + 8192)) << (16 * (i + 1));
  return k;
}

// The complement side's achievable (eq-2.11b tuple, norm) pairs. Same double loop
// and same norm pruning as PairNormSet — just a richer key — so a lookup stays O(1).
static bool G_THM212 = false;   // WZ_THM212=1: Thm 2.3's mod-4 conditions (paper eq 18)

// Thm 2.3 eq (18) — "eq 2.12": mod-4 conditions coupling each class sum with
// its REFLECTED class, separable per side. Reading validated 2026-07-17
// against ALL 10 banked+reference solutions at m=3 AND m=6 (python first).
// Subtlety the fixtures caught: the paper's "j = 2..m" implicitly EXCLUDES
// the class-pair already governed by the special j=1 rule (pair {1, class of
// position n+1}), which carries ≡2 (mod 4) when n ≢ 0 (mod m). Vectors here
// are 0-indexed: class c holds 0-indexed positions ≡ c (mod m) = paper j=c+1.
static bool thm212_ok(const vector<int> &x, const vector<int> &y, int m,
                      bool abSide) {
  auto m4 = [](int v) { return ((v % 4) + 4) % 4; };
  int n = G_N;
  if (abSide) {
    int c1 = n % m;                        // 0-indexed class of position n+1
    int want = (n % m) ? 2 : 0;
    if (m4(x[0] + y[0] + x[c1] + y[c1]) != want) return false;
    for (int j = 2; j <= m; j++) {
      if (j == c1 + 1) continue;           // pair {1, c1+1}: handled above
      int c = ((n + 1 - j) % m + m) % m;   // paper class n+2-j, 0-indexed
      if (m4(x[j-1] + y[j-1] + x[c] + y[c]) != 0) return false;
    }
  } else {
    for (int j = 1; j <= m; j++) {
      int c = ((n - j) % m + m) % m;       // paper class n+1-j, 0-indexed
      if (m4(x[j-1] + y[j-1] + x[c] + y[c]) != 0) return false;
    }
  }
  return true;
}

struct PairAutoSet {
  unordered_set<uint64_t> achievable;
  void build(int L, int a, int b, int m) {
    auto K = enum_class_sums(L, a, m);
    auto R = enum_class_sums(L, b, m);
    int tgt = 4 * G_N + 2, half = m / 2;
    vector<int> T(half);
    for (auto &kv : K) {
      int kn = norm_vec(kv);
      if (kn > tgt) continue;
      for (auto &rv : R) {
        int rn = norm_vec(rv);
        if (kn + rn > tgt) continue;
        // eq 2.12 on the complement side: a witness (k,r) that violates the
        // mod-4 conditions cannot belong to any real solution.
        if (G_THM212 && !thm212_ok(kv, rv, m, L == G_N1)) continue;
        for (int s = 1; s <= half; s++) T[s - 1] = pair_auto(kv, rv, s);
        achievable.insert(auto_key(T, kn + rn));
      }
    }
  }
  // Wang-Zhu Step 3's existential test: does SOME (k,r) supply exactly `needNorm`
  // AND cancel this side's autocorrelation tuple?
  bool feasible(const vector<int> &needT, int needNorm) const {
    return achievable.count(auto_key(needT, needNorm)) != 0;
  }
};

// ============================ sequence generation ============================
//
// Generate all +/-1 sequences of length L whose mod-3 class sums equal target3,
// spectral-filtering each with the per-sequence Thm-2.4 bound. DFS prunes by
// per-class remaining capacity so the full Cartesian product is never built.
// pin0=true forces X[0]=+1 (sound when the sequence's signature sum is 0, by
// single-sequence negation symmetry).
static void gen_seqs_for_profile(int L, const vector<int> &target3, bool pin0,
                                 vector<vector<int>> &out,
                                 long long &examined, long long &spec_ok) {
  int total_in_class[3];
  for (int c = 0; c < 3; c++) total_in_class[c] = class_count(L, c, 3);

  vector<int> X(L, 0);
  function<void(int, array<int,3>, array<int,3>)> rec =
      [&](int i, array<int,3> partial, array<int,3> placed) {
    for (int c = 0; c < 3; c++) {
      int rem = total_in_class[c] - placed[c];
      int diff = target3[c] - partial[c];
      if (diff < -rem || diff > rem) return;
      if (((diff - (-rem)) & 1) != 0) return;
    }
    if (i == L) {
      examined++;
      if (hall_ok_single(X.data(), L)) { spec_ok++; out.push_back(X); }
      return;
    }
    int c = i % 3;
    int lo = -1, hi = 1;
    if (i == 0 && pin0) lo = 1;
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

// COUNT-ONLY twin of gen_seqs_for_profile: identical DFS + spectral filter but
// stores NOTHING — O(L) memory however many sequences pass. Exists because the
// materializing path OOM-kills a 192-thread node from n=29 up (measured
// 2026-07-03: canary n=29 and measure n=31 both died in the count phase, before
// any hash was built). Used by the WZ_COUNT_ONLY probe.
static void count_seqs_for_profile(int L, const vector<int> &target3, bool pin0,
                                   long long &examined, long long &spec_ok) {
  int total_in_class[3];
  for (int c = 0; c < 3; c++) total_in_class[c] = class_count(L, c, 3);

  vector<int> X(L, 0);
  function<void(int, array<int,3>, array<int,3>)> rec =
      [&](int i, array<int,3> partial, array<int,3> placed) {
    for (int c = 0; c < 3; c++) {
      int rem = total_in_class[c] - placed[c];
      int diff = target3[c] - partial[c];
      if (diff < -rem || diff > rem) return;
      if (((diff - (-rem)) & 1) != 0) return;
    }
    if (i == L) {
      examined++;
      if (hall_ok_single(X.data(), L)) spec_ok++;
      return;
    }
    int c = i % 3;
    int lo = -1, hi = 1;
    if (i == 0 && pin0) lo = 1;
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

// ============================ profile enumeration ============================
//
// Surviving mod-3 (px,py) pair-profiles for a side of length L (sigX,sigY). A
// profile survives if it fits the global norm budget (norm(px)+norm(py) <= 4n+2)
// AND the COMPLEMENT side can complete the norm identity at both mod-3 and the
// finer mod-6 (the tightening). comp3/comp6 are the complement side's PairNormSets.
struct Profile { vector<int> px, py; };

static vector<Profile> survive_profiles(int L, int sigX, int sigY,
                                        const PairNormSet &comp3,
                                        const PairNormSet &comp6,
                                        const PairAutoSet *comp6auto = nullptr) {
  int tgt = 4 * G_N + 2;
  auto PX3 = enum_class_sums(L, sigX, 3);
  auto PY3 = enum_class_sums(L, sigY, 3);
  auto PX6 = enum_class_sums(L, sigX, 6);
  auto PY6 = enum_class_sums(L, sigY, 6);
  auto reduce63 = [](const vector<int> &v6) {
    return vector<int>{v6[0]+v6[3], v6[1]+v6[4], v6[2]+v6[5]};
  };
  vector<Profile> out;
  for (auto &px : PX3) {
    int npx = norm_vec(px);
    if (npx > tgt) continue;
    for (auto &py : PY3) {
      int need = tgt - npx - norm_vec(py);
      if (need < 0) continue;
      if (!comp3.feasible(need)) continue;
      if (G_THM212 && !thm212_ok(px, py, 3, L == G_N1)) continue;  // eq 2.12 @ m=3
      // mod-6 tighten: some (px6,py6) reducing to (px,py) must admit a comp6 completion.
      bool ok6 = false;
      for (auto &px6 : PX6) {
        if (reduce63(px6) != px) continue;
        int npx6 = norm_vec(px6);
        if (npx6 > tgt) continue;
        for (auto &py6 : PY6) {
          if (reduce63(py6) != py) continue;
          int need6 = tgt - npx6 - norm_vec(py6);
          if (need6 < 0) continue;
          if (G_THM212 && !thm212_ok(px6, py6, 6, L == G_N1)) continue;  // witness lift must pass 2.12 @ m=6
          if (G_THM211B && comp6auto) {
            // Wang-Zhu Step 3, in full: the complement must not only supply the
            // remaining NORM (2.11a) but also CANCEL this pair's residue
            // autocorrelations (2.11b). Previously we only asked for the norm.
            vector<int> needT(3);
            for (int s = 1; s <= 3; s++) needT[s - 1] = -pair_auto(px6, py6, s);
            if (comp6auto->feasible(needT, need6)) { ok6 = true; break; }
          } else if (comp6.feasible(need6)) { ok6 = true; break; }
        }
        if (ok6) break;
      }
      if (!ok6) continue;
      out.push_back({px, py});
    }
  }
  return out;
}

// Modulus-m generalization of the COUNT-ONLY DFS (m <= 8). Same per-sequence
// spectral filter, class constraints at modulus m instead of 3; stores nothing.
// Used by the WZ_COUNT_MOD6 Gate-A probe (docs/wz_firsthit_plan.md).
static void count_seqs_for_profile_m(int L, const vector<int> &target, int m,
                                     bool pin0, long long &examined,
                                     long long &spec_ok) {
  int total_in_class[8];
  for (int c = 0; c < m; c++) total_in_class[c] = class_count(L, c, m);
  vector<int> X(L, 0);
  function<void(int, array<int,8>, array<int,8>)> rec =
      [&](int i, array<int,8> partial, array<int,8> placed) {
    for (int c = 0; c < m; c++) {
      int rem = total_in_class[c] - placed[c];
      int diff = target[c] - partial[c];
      if (diff < -rem || diff > rem) return;
      if (((diff - (-rem)) & 1) != 0) return;
    }
    if (i == L) {
      examined++;
      if (hall_ok_single(X.data(), L)) spec_ok++;
      return;
    }
    int c = i % m;
    int lo = -1, hi = 1;
    if (i == 0 && pin0) lo = 1;
    for (int v = hi; v >= lo; v -= 2) {
      X[i] = v;
      auto np = partial; np[c] += v;
      auto pl = placed; pl[c]++;
      rec(i + 1, np, pl);
    }
    X[i] = 0;
  };
  rec(0, array<int,8>{}, array<int,8>{});
}

// ---- Thm 2.2 pair-encoding tables (Wang-Zhu arXiv:2506.20296 Thm 2.2; matches
// wz_sa_v8's comb8/comb4 and verify_npaf.py's pair-encoding check). Symmetric
// position pairs (d, L-1-d) of (X,Y) take joint values with product +1
// (P22_POS) for d>=1; at d=0 the A,B side requires product -1 (P22_NEG), the
// C,D side is unconstrained (P22_16). Odd-L middle is free (P22_4). Every
// banked champion AND the published WZ BS(43)/BS(44) satisfy this encoding.
static int P22_16[16][4], P22_POS[8][4], P22_NEG[8][4], P22_4[4][2];
static void init_p22() {
  int np = 0, nn = 0;
  for (int i = 0; i < 16; i++) {
    int v[4] = {(i&8)?1:-1, (i&4)?1:-1, (i&2)?1:-1, (i&1)?1:-1};
    memcpy(P22_16[i], v, sizeof v);
    if (v[0]*v[1]*v[2]*v[3] == 1) memcpy(P22_POS[np++], v, sizeof v);
    else                          memcpy(P22_NEG[nn++], v, sizeof v);
  }
  int m4[4][2] = {{1,1},{1,-1},{-1,1},{-1,-1}};
  memcpy(P22_4, m4, sizeof m4);
}

// GATE A' (2026-07-08): count (X,Y) pairs of length L JOINTLY satisfying
// Thm 2.2 encoding + exact mod-3 class sums (tx, ty) + per-sequence and joint
// spectral bounds. This is the TRUE WZ-constrained pair stream. The earlier
// count instruments generated the sides INDEPENDENTLY (Thm 2.2 never applied
// in wz_match's enumeration — only in wz_sa_v8/wz_exact), so every measured
// pair-work number overstates this stream by ~2^(L/2). O(L) memory.
// Optional `sink`: called on every pair that passes ALL filters (Thm 2.2 +
// class sums + single + joint spectral). nullptr = count only (behavior of the
// validated Gate-A' instrument, unchanged). Non-null turns this DFS into the
// GENERATOR for the 2.2-joint join (WZ_JOIN22) — same filters, zero
// materialization outside what the sink itself stores.
// `m` = the modulus of the class-sum targets tx,ty (WZ Step 4 generates C,D from
// mod-6 profiles; we historically only ever generated from mod-3). Generalized
// 2026-07-15 — the DFS was already a generic class-sum walk with the modulus
// hardcoded to 3. INVARIANT: for the SAME underlying profile set, the resulting
// stream (`ok`) must be identical at m=3 and m=6 — finer profiles only regroup the
// same sequences, they never add or drop any. That invariant is the regression test.
static void count_pairs22(int L, const vector<int> &tx, const vector<int> &ty,
                          bool abSide, bool pinX, bool pinY,
                          long long &leaves, long long &ok,
                          const function<void(const vector<int>&, const vector<int>&)> *sink = nullptr,
                          int m = 3,
                          const atomic<bool> *stop = nullptr) {
  // `stop`: cooperative abort. When set and it flips true mid-DFS, the
  // enumeration winds down and leaves/ok are PARTIAL. Only pass it where a
  // partial enumeration is sound (JOIN22 resolve: a solution is already found
  // and stored, so completeness no longer matters). Without it an in-flight
  // fat profile keeps a 192-thread node pinned for hours after a FOUND —
  // job 15719454 died at walltime that way with its solution unprinted.
  int total_in_class[8];
  for (int c = 0; c < m; c++) total_in_class[c] = class_count(L, c, m);
  int half = L / 2;
  vector<int> X(L, 0), Y(L, 0);
  int px[8] = {0}, py[8] = {0}, placed[8] = {0};
  function<void(int)> rec = [&](int d) {
    if (stop && stop->load(memory_order_relaxed)) return;
    for (int c = 0; c < m; c++) {
      int rem = total_in_class[c] - placed[c];
      int dx = tx[c] - px[c], dy = ty[c] - py[c];
      if (dx < -rem || dx > rem || dy < -rem || dy > rem) return;
      if (((dx - (-rem)) & 1) != 0 || ((dy - (-rem)) & 1) != 0) return;
    }
    if (d == half) {
      auto finish = [&]() {
        leaves++;
        if (hall_ok_single(X.data(), L) && hall_ok_single(Y.data(), L) &&
            hall_ok(X.data(), L, Y.data(), L)) {
          ok++;
          if (sink) (*sink)(X, Y);
        }
      };
      auto exact = [&]() {
        for (int cc = 0; cc < m; cc++)
          if (px[cc] != tx[cc] || py[cc] != ty[cc]) return false;
        return true;
      };
      if (L % 2 == 1) {
        int mid = half, c = mid % m;
        for (int k = 0; k < 4; k++) {
          X[mid] = P22_4[k][0]; Y[mid] = P22_4[k][1];
          px[c] += X[mid]; py[c] += Y[mid]; placed[c]++;
          if (exact()) finish();
          px[c] -= X[mid]; py[c] -= Y[mid]; placed[c]--;
        }
        X[mid] = Y[mid] = 0;
      } else if (exact()) finish();
      return;
    }
    int i1 = d, i2 = L - 1 - d;
    int c1 = i1 % m, c2 = i2 % m;
    bool d0free = (d == 0 && !abSide);
    const int (*S)[4] = (d == 0) ? (abSide ? P22_NEG : P22_16) : P22_POS;
    int ns = d0free ? 16 : 8;
    for (int k = 0; k < ns; k++) {
      if (d == 0 && pinX && S[k][0] != 1) continue;
      if (d == 0 && pinY && S[k][1] != 1) continue;
      X[i1] = S[k][0]; Y[i1] = S[k][1]; X[i2] = S[k][2]; Y[i2] = S[k][3];
      px[c1] += S[k][0]; py[c1] += S[k][1]; px[c2] += S[k][2]; py[c2] += S[k][3];
      placed[c1]++; placed[c2]++;
      rec(d + 1);
      px[c1] -= S[k][0]; py[c1] -= S[k][1]; px[c2] -= S[k][2]; py[c2] -= S[k][3];
      placed[c1]--; placed[c2]--;
    }
    X[i1] = Y[i1] = X[i2] = Y[i2] = 0;
  };
  rec(0);
}

// Surviving mod-6 (px,py) pair-profiles for one side: norm budget + EXACT
// complement completion at mod-6 (same norm identity as mod-3 — parity-generic,
// audit-verified 2026-07-03). This is the GENERATION-level mod-6 constraint (the
// Wang-Zhu lift), vs survive_profiles' mod-3 profiles with existential tighten.
// Guard: bails once `cap` pairs accumulate — an exploding profile space is
// itself a Gate-A verdict, not an error.
static vector<Profile> survive_profiles6(int L, int sigX, int sigY,
                                         const PairNormSet &comp3,
                                         const PairNormSet &comp6,
                                         size_t cap = 20000000,
                                         const PairAutoSet *comp6auto = nullptr) {
  int tgt = 4 * G_N + 2;
  auto PX = enum_class_sums(L, sigX, 6);
  auto PY = enum_class_sums(L, sigY, 6);
  auto reduce63 = [](const vector<int> &v6) {
    return vector<int>{v6[0]+v6[3], v6[1]+v6[4], v6[2]+v6[5]};
  };
  vector<Profile> out;
  for (auto &px : PX) {
    int npx = norm_vec(px);
    if (npx > tgt) continue;
    auto px3 = reduce63(px);
    int npx3 = norm_vec(px3);
    if (npx3 > tgt) continue;
    for (auto &py : PY) {
      int need = tgt - npx - norm_vec(py);
      if (need < 0) continue;
      if (!comp6.feasible(need)) continue;
      if (G_THM212 && !thm212_ok(px, py, 6, L == G_N1)) continue;  // eq 2.12 @ m=6
      // A real solution satisfies the norm identity at EVERY modulus: the
      // pair's mod-3 reduction must ALSO complete (without this, mod-6
      // "survivors" include pairs mod-3 already kills — measured at n=11:
      // mod-6 pairwork exceeded mod-3, impossible for a true refinement).
      int need3 = tgt - npx3 - norm_vec(reduce63(py));
      if (need3 < 0 || !comp3.feasible(need3)) continue;
      // Thm 2.3 eq 2.11b (WZ Step 3, in full): the complement must ALSO cancel this
      // pair's residue autocorrelations, not merely supply the leftover norm.
      if (G_THM211B && comp6auto) {
        vector<int> needT(3);
        for (int s = 1; s <= 3; s++) needT[s - 1] = -pair_auto(px, py, s);
        if (!comp6auto->feasible(needT, need)) continue;
      }
      out.push_back({px, py});
      if (out.size() >= cap) {
        cout << "[mod6] WARNING: profile-pair space hit cap " << cap
             << " — mod-6 PROFILE space itself explodes (Gate-A data)\n" << flush;
        return out;
      }
    }
  }
  return out;
}

// ============================ autocorr key (compact) =========================
//
// COMPACT KEY: instead of storing the full length-n int16 autocorrelation vector
// as the map key, we hash that vector down to a single 64-bit FNV-1a value and key
// the map on that uint64_t. Collisions are sound to ignore here because every hit
// is re-validated by the exact npaf_at==0 recheck over the full A,B,C,D.
//
// The length-n vector entries are: for s=1..n, the summed autocorr term (empty for
// s>=L -> 0). For the CD side (L=n) entry s=n is 0, which pins AB[n]=0 on a match.
// The two sides hash the SAME vector convention so identical autocorrelations
// (AB[s] == -CD[s]) produce identical 64-bit keys. We mix the int16 value AND the
// shift index s so a vector and a shifted/zero-padded variant don't alias.
using Key = uint64_t;

// negate=true folds in -v (used for the CD side so its stored/lookup key matches
// the AB side's raw autocorrelation: a solution needs AB[s] == -CD[s]).
static inline uint64_t autocorr_key(const int *X, const int *Y, int L,
                                    bool negate) {
  int n = G_N;
  uint64_t h = 1469598103934665603ULL;  // FNV-1a offset basis
  for (int s = 1; s <= n; s++) {
    int v = 0;
    if (s < L)
      for (int i = 0; i < L - s; i++) v += X[i] * X[i + s] + Y[i] * Y[i + s];
    if (negate) v = -v;
    // fold the (s, value) pair in; value cast through uint16 to be sign-stable.
    h ^= (uint64_t)(uint16_t)(int16_t)v;
    h *= 1099511628211ULL;
    h ^= (uint64_t)(uint16_t)s;
    h *= 1099511628211ULL;
  }
  return h;
}

// ============================ result =========================================
static int g_solA[256], g_solB[256], g_solC[256], g_solD[256];
static atomic<bool> g_found{false};

// The probe driver kills arms with SIGTERM at its 11.5h deadline (and in the
// post-FOUND grace window). Without a handler the arm dies before its FIRSTHIT
// SUMMARY prints, and the driver's GATEB aggregation — which greps
// candidates_streamed=/budget_aborted=/total_AB_nodes= from the arm logs —
// reads 0 for every killed arm. A deep hitless run is then indistinguishable
// from an empty stream (the 07-22/23 "zero-candidate" n=41/42 waves were
// exactly this artifact: m6 stream flowing, all 178 arms deadline-killed
// pre-summary). SIGTERM now folds into the normal fh_stop path: the DFS
// unwinds at its next stop-flag check and the true summary prints, flagged
// INTERRUPTED (its counts are lower bounds, the stream was NOT exhausted).
static atomic<bool> *g_fh_stop_ptr = nullptr;
static volatile sig_atomic_t g_fh_sigterm = 0;
static void fh_on_sigterm(int) {
  g_fh_sigterm = 1;
  if (g_fh_stop_ptr) g_fh_stop_ptr->store(true);
}

// ==================== WZ_FIRSTHIT: A,B completion (Gate B/C) =================
// Ported from wz_generate.cpp (ab_search/complete_ab, 06-24, proven machinery)
// for the 2026-07-16 first-hit work order. Given fixed C,D, backtrack A,B with
// target ab[s] = -cd[s]; sound bound |t[s]-Dab[s]| <= Kab[s]. Node budget per
// candidate (FH_BUDGET): 0 = exact; >0 aborts monster dead trees (probe mode —
// NOT exhaustive per candidate; track aborts to bound what might be missed).
static long long FH_BUDGET = 0, fh_cur = 0, fh_nodes_total = 0;
static bool fh_aborted = false;
static int FH_CD_target[256];
static int FH_ABS_A = 0, FH_ABS_B = 0;

// ---- Profile-constrained A,B completion (WZ_FH_AB_PROF, 2026-07-26) --------
// The stream filters PROVE a compatible mod-m A,B class-sum profile (k,r)
// exists for every surviving C,D cell (2.11a norm identity + 2.11b residue
// cancellation + 2.12 mod-4) — then the completer ignored that proof and
// searched the whole A,B space. This constrains the pair-DFS to the cell's
// compatible profile list with count_pairs22-style per-class capacity pruning
// — the last structural gap vs WZ Step 5.
// SOUNDNESS (retention): any completion the DFS could output passes the exact
// npaf_at==0 recheck, so it IS a real solution; every real solution satisfies
// 2.11a/2.11b/2.12 at every modulus (canary tools/canary_thm211b.py + the
// thm212 fixtures, all banked + published solutions). The completer accepts
// sum(A) = ±a and sum(B) = ±b (negation isomorphism) so the allowed set
// enumerates BOTH signed targets; the reversal-canonical representative is
// itself a solution with the same sums, so its profile is enumerated too.
// Hence no branch leading to any completion is ever cut. WZ_FH_AB_PROF=0
// disables (A/B lever for validation).
struct AbpRow { int8_t k[8], r[8]; };
static const vector<AbpRow> *FH_ABP = nullptr;  // current cell's allowed (k,r)
static int FH_ABP_M = 6;                        // class modulus (m of the cell)
static int FH_ABP_TIC[8];                       // class totals of length n1 at m
static int FH_PA[8], FH_PB[8], FH_PLACED[8];    // partial class sums / counts
static vector<vector<int>> FH_ABP_STACK;        // surviving rows per DFS depth

// Filter the depth-d survivor list into depth d+1 under the current partials.
// Parity is automatic (k[c] == class-count == pa[c]+rem (mod 2) for every
// allowed row), so the test is the pure capacity box |k[c]-pa[c]| <= rem[c] —
// the same pruning shape as count_pairs22, against a SET of exact targets.
static inline bool fh_abp_filter(int d) {
  auto &src = FH_ABP_STACK[d];
  auto &dst = FH_ABP_STACK[d + 1];
  dst.clear();
  int m = FH_ABP_M;
  for (int idx : src) {
    const AbpRow &row = (*FH_ABP)[idx];
    bool ok = true;
    for (int c = 0; c < m; c++) {
      int rem = FH_ABP_TIC[c] - FH_PLACED[c];
      int dk = row.k[c] - FH_PA[c], dr = row.r[c] - FH_PB[c];
      if (dk < -rem || dk > rem || dr < -rem || dr > rem) { ok = false; break; }
    }
    if (ok) dst.push_back(idx);
  }
  return !dst.empty();
}
// Exact membership at the leaf (all positions placed, incl. an odd-L middle).
static inline bool fh_abp_leaf_ok() {
  int m = FH_ABP_M;
  for (int idx : FH_ABP_STACK[G_N1 / 2]) {
    const AbpRow &row = (*FH_ABP)[idx];
    bool eq = true;
    for (int c = 0; c < m; c++)
      if (row.k[c] != FH_PA[c] || row.r[c] != FH_PB[c]) { eq = false; break; }
    if (eq) return true;
  }
  return false;
}

// Pair-position DFS under the Thm 2.2 encoding — this is WZ Step 5 as written:
// "Definition 1.1 and Theorem 2.2 ... to truncate branches". Mirror positions
// (d, L-1-d) are placed JOINTLY from the 8 legal combos (product -1 at d=0 on
// the A,B side, +1 otherwise), which alone shrinks the raw space by ~2^(L/2)
// versus the naive 4-combos-per-position search (measured 10^5x on the stream).
// The v1 unpaired search burned its entire node budget on 100% of candidates
// at n=19 (200k/200k aborted, zero completions) — the encoding is not optional.

// Place single position p with (av,bv); update Dab/Kab against placed q's.
static inline void fh_place(int p, int av, int bv, int *A, int *B,
                            int *Dab, int *Kab, int L) {
  for (int q = 0; q < L; q++) {
    if (A[q] == 0 || q == p) continue;
    int s = q > p ? q - p : p - q;
    Dab[s] += A[q] * av + B[q] * bv;
    Kab[s] -= 2;
  }
  A[p] = av; B[p] = bv;
  int pc = p % FH_ABP_M;
  FH_PA[pc] += av; FH_PB[pc] += bv; FH_PLACED[pc]++;
}
static inline void fh_unplace(int p, int *A, int *B, int *Dab, int *Kab, int L) {
  int av = A[p], bv = B[p];
  A[p] = 0; B[p] = 0;
  int pc = p % FH_ABP_M;
  FH_PA[pc] -= av; FH_PB[pc] -= bv; FH_PLACED[pc]--;
  for (int q = 0; q < L; q++) {
    if (A[q] == 0 || q == p) continue;
    int s = q > p ? q - p : p - q;
    Dab[s] -= A[q] * av + B[q] * bv;
    Kab[s] += 2;
  }
}

// Reversal canonicalization (WZ isomorphism list, part 2 — added 2026-07-22).
// Reversing A alone (or B alone) preserves every A_i*A_{i+s} sum and the |sum|
// checks, so each completion class carries 4 mirror copies. Canonical rep:
// A >=lex cmpA*rev(A), where cmpA = A[L-1] (the sign that makes rev(A)'s
// negation-canonical form start with +1 — composes with the A[0]=B[0]=+1 root
// canon, which this REQUIRES). Tracked incrementally: while the prefix ties,
// the first strict difference must take A[d]=+1. WZ_FH_NO_CANON disables both.
static bool fh_ab_search(int d, int *A, int *B, int *Dab, int *Kab,
                         int sumA, int sumB,
                         int a_tied, int a_cmp, int b_tied, int b_cmp) {
  if (fh_aborted) return false;
  int n = G_N, L = G_N1;
  int half = L / 2;
  if (d == half) {
    auto final_ok = [&](int sA, int sB) {
      if (abs(sA) != FH_ABS_A || abs(sB) != FH_ABS_B) return false;
      for (int s = 1; s <= n; s++)
        if (Dab[s] != FH_CD_target[s]) return false;
      if (FH_ABP && !fh_abp_leaf_ok()) return false;
      return true;
    };
    if (L % 2 == 1) {  // odd L: free middle element
      int mid = half;
      for (int k = 0; k < 4; k++) {
        int av = P22_4[k][0], bv = P22_4[k][1];
        fh_place(mid, av, bv, A, B, Dab, Kab, L);
        fh_nodes_total++;
        bool ok = final_ok(sumA + av, sumB + bv);
        if (!ok) fh_unplace(mid, A, B, Dab, Kab, L);
        if (ok) return true;
      }
      return false;
    }
    return final_ok(sumA, sumB);
  }
  int i1 = d, i2 = L - 1 - d;
  const int (*S)[4] = (d == 0) ? P22_NEG : P22_POS;  // A,B side encoding
  for (int k = 0; k < 8; k++) {
    int a1 = S[k][0], b1 = S[k][1], a2 = S[k][2], b2 = S[k][3];
    // Isomorphic-transformation truncation (WZ Step 5: "the isomorphic
    // transformation of A,B sequences to truncate branches"). Negating all of
    // A (or all of B) preserves every A_i*A_{i+s} product and only flips
    // sumA (absorbed by the |a| check), so every completion class has a
    // representative with A[0]=+1 AND B[0]=+1 — fix both at the root: 2 of
    // the 8 d=0 combos survive, a sound 4x cut. WZ_FH_NO_CANON=1 disables
    // (A/B lever for validation).
    static const bool fh_canon = !getenv("WZ_FH_NO_CANON");
    if (d == 0 && fh_canon && (a1 != 1 || b1 != 1)) continue;
    // Reversal canon (requires the root canon): while the A-prefix ties its
    // reversed image, the first difference must take A[d]=+1; ditto B.
    int na_tied = a_tied, na_cmp = a_cmp, nb_tied = b_tied, nb_cmp = b_cmp;
    if (fh_canon) {
      if (d == 0) { na_cmp = a2; nb_cmp = b2; }  // ties always hold at d=0
      else {
        if (a_tied) {
          int cv = a_cmp * a2;
          if (a1 != cv) { if (a1 != 1) continue; na_tied = 0; }
        }
        if (b_tied) {
          int cv = b_cmp * b2;
          if (b1 != cv) { if (b1 != 1) continue; nb_tied = 0; }
        }
      }
    }
    fh_place(i1, a1, b1, A, B, Dab, Kab, L);
    fh_place(i2, a2, b2, A, B, Dab, Kab, L);
    fh_nodes_total++;
    if (FH_BUDGET > 0 && ++fh_cur > FH_BUDGET) {
      fh_aborted = true;
      fh_unplace(i2, A, B, Dab, Kab, L);
      fh_unplace(i1, A, B, Dab, Kab, L);
      return false;
    }
    int nsA = sumA + a1 + a2, nsB = sumB + b1 + b2;
    int rem = L - 2 * (d + 1);
    bool sum_ok = ((abs(FH_ABS_A - nsA) <= rem) || (abs(-FH_ABS_A - nsA) <= rem))
               && ((abs(FH_ABS_B - nsB) <= rem) || (abs(-FH_ABS_B - nsB) <= rem));
    bool prune = !sum_ok;
    if (!prune)
      for (int s = 1; s <= n; s++)
        if (abs(FH_CD_target[s] - Dab[s]) > Kab[s]) { prune = true; break; }
    // Profile constraint: some allowed (k,r) must remain reachable within the
    // per-class capacity left. Writes the depth-(d+1) survivor list the
    // recursion below consumes. Runs LAST — the cheap prunes go first.
    if (!prune && FH_ABP && !fh_abp_filter(d)) prune = true;
    if (!prune) {
      if (fh_ab_search(d + 1, A, B, Dab, Kab, nsA, nsB,
                       na_tied, na_cmp, nb_tied, nb_cmp))
        return true;
    }
    fh_unplace(i2, A, B, Dab, Kab, L);
    fh_unplace(i1, A, B, Dab, Kab, L);
  }
  return false;
}

// Returns: 0 = completed (g_sol* filled), 1 = rejected by cheap pre-filter
// (tail cd!=0 or |target| out of range — no backtrack entered), 2 = backtrack
// exhausted clean (no A,B), 3 = backtrack ABORTED on budget (unknown).
static int fh_complete_ab(const int *C, const int *D) {
  int n = G_N, n1 = G_N1;
  for (int s = 1; s <= n; s++) {
    int cd = 0;
    for (int i = 0; i + s < n; i++) cd += C[i] * C[i + s] + D[i] * D[i + s];
    FH_CD_target[s] = -cd;
  }
  for (int s = n1; s <= n; s++)
    if (FH_CD_target[s] != 0) return 1;
  for (int s = 1; s < n1; s++)
    if (abs(FH_CD_target[s]) > 2 * (n1 - s)) return 1;
  int A[256], B[256], Dab[256], Kab[256];
  memset(A, 0, sizeof(A)); memset(B, 0, sizeof(B));
  memset(Dab, 0, sizeof(Dab));
  for (int s = 0; s <= n; s++)
    Kab[s] = (s >= 1 && s < n1) ? 2 * (n1 - s) : 0;
  FH_ABS_A = abs(G_SIG_A);
  FH_ABS_B = abs(G_SIG_B);
  memset(FH_PA, 0, sizeof(FH_PA));
  memset(FH_PB, 0, sizeof(FH_PB));
  memset(FH_PLACED, 0, sizeof(FH_PLACED));
  fh_cur = 0;
  fh_aborted = false;
  if (!fh_ab_search(0, A, B, Dab, Kab, 0, 0, 1, 0, 1, 0))
    return fh_aborted ? 3 : 2;
  for (int s = 1; s <= n; s++)
    if (npaf_at(A, B, n1, C, D, n, s) != 0) return 2;
  memcpy(g_solA, A, n1 * sizeof(int));
  memcpy(g_solB, B, n1 * sizeof(int));
  memcpy(g_solC, C, n * sizeof(int));
  memcpy(g_solD, D, n * sizeof(int));
  return 0;
}

int main(int argc, char **argv) {
  if (argc < 6) {
    cerr << "Usage: " << argv[0] << " <n> <a> <b> <c> <d>\n"
         << "  e.g.  " << argv[0] << " 10 5 1 4 0   (BS(11,10))\n";
    return 1;
  }
  int n = atoi(argv[1]);
  // n >= 4, odd OR even. (A legacy guard rejected odd n, but every stage —
  // enum_class_sums, gen_seqs_for_profile, hall_ok, the length-n join key,
  // npaf_at — is length-generic. Odd n validated blind at n=7 and n=11
  // against the banked v3 champions, NPAF==0 independently confirmed.)
  if (n < 4) { cerr << "ERROR: n must be >= 4\n"; return 1; }
  G_N = n; G_N1 = n + 1;
  G_SIG_A = atoi(argv[2]); G_SIG_B = atoi(argv[3]);
  G_SIG_C = atoi(argv[4]); G_SIG_D = atoi(argv[5]);
  int sigsum = G_SIG_A*G_SIG_A + G_SIG_B*G_SIG_B + G_SIG_C*G_SIG_C + G_SIG_D*G_SIG_D;
  if (sigsum != 4 * n + 2) {
    cerr << "ERROR: sig norm = " << sigsum << ", expected " << (4*n+2) << "\n";
    return 1;
  }
  if (const char *hk = getenv("WZ_HASH_KEEP")) G_HASH_KEEP = atoi(hk);
  if (getenv("WZ_THM211B")) G_THM211B = true;   // Thm 2.3 eq 2.11b profile filter
  if (getenv("WZ_THM212"))  G_THM212  = true;   // Thm 2.3 eq (18) mod-4 filter
  // FIRSTHIT at n>=36: 2.11b + 2.12 are STREAM ENABLERS, not options. Measured
  // 2026-07-21 at n=41: unfiltered mod-6 cells = 2.36M, ~90% empty, ZERO
  // candidates streamed in 15 min (and 11.5-25 h on clusters); filtered =
  // 270k cells, 200 candidates in 40 s. Force both on so stale job exports
  // (e.g. Trillium's queued tickets) cannot recompile into the wall.
  if (getenv("WZ_FIRSTHIT") && G_N >= 36) { G_THM211B = true; G_THM212 = true; }
  bool measure = getenv("WZ_MEASURE") != nullptr;

  init_hall_tables();

  cout << "========================================================\n";
  cout << "  wz_match — HASH-JOIN BS(" << G_N1 << "," << n << "), sig=("
       << G_SIG_A << "," << G_SIG_B << "," << G_SIG_C << "," << G_SIG_D << ")\n";
#ifdef _OPENMP
  cout << "  OpenMP: " << omp_get_max_threads() << " threads\n";
#else
  cout << "  OpenMP: disabled (single-threaded)\n";
#endif
  cout << "========================================================\n" << flush;

  G_T0 = Clock::now();

  // --- norm-sets for both sides at both moduli (used as profile complements) ---
  PairNormSet ab3, ab6, cd3, cd6;
  ab3.build(G_N1, G_SIG_A, G_SIG_B, 3);
  ab6.build(G_N1, G_SIG_A, G_SIG_B, 6);
  cd3.build(n,    G_SIG_C, G_SIG_D, 3);
  cd6.build(n,    G_SIG_C, G_SIG_D, 6);

  // ---- WZ_PROFILE_CHECK: Gate-A soundness canary. Feed a SOLVED banner-format
  //      A/B/C/D on stdin (with the matching SIGNED sig on argv); verifies the
  //      real solution's own class-sum profiles pass the exact survival
  //      predicates the mod-3 and mod-6 paths use. FAIL = the residue lift
  //      would exclude a real solution -> do NOT trust Gate-A counts.
  //      Usage: WZ_PROFILE_CHECK=1 ./wz_match 31 0 -6 9 -3 < champion_banner.txt
  if (getenv("WZ_PROFILE_CHECK")) {
    string all, line;
    while (getline(cin, line)) all += line + "\n";
    auto parse = [&](char name, int L) {
      vector<int> v;
      size_t p = all.find(string(1, name) + " = {");
      if (p == string::npos) { cerr << "PROFILE_CHECK: '" << name << " = {' not found\n"; exit(2); }
      p += 5;
      while (p < all.size() && all[p] != '}') {
        if (all[p] == '-') { v.push_back(-1); p += 2; }
        else if (all[p] == '1') { v.push_back(1); p++; }
        else p++;
      }
      if ((int)v.size() != L) { cerr << "PROFILE_CHECK: " << name << " length " << v.size() << " != " << L << "\n"; exit(2); }
      return v;
    };
    auto A = parse('A', G_N1), B = parse('B', G_N1);
    auto C = parse('C', n),   D = parse('D', n);
    auto csums = [](const vector<int> &X, int m) {
      vector<int> s(m, 0);
      for (int i = 0; i < (int)X.size(); i++) s[i % m] += X[i];
      return s;
    };
    int tgt = 4 * n + 2;
    bool allok = true;
    auto check = [&](const char *side, const vector<int> &X, const vector<int> &Y,
                     int m, const PairNormSet &comp) {
      auto px = csums(X, m), py = csums(Y, m);
      int need = tgt - norm_vec(px) - norm_vec(py);
      bool ok = need >= 0 && comp.feasible(need);
      cout << "PROFILE_CHECK " << side << " mod-" << m << ": own-norm=" << (tgt - need)
           << " complement-needs=" << need << " -> " << (ok ? "PASS" : "FAIL") << "\n";
      allok &= ok;
    };
    check("A,B", A, B, 3, cd3); check("A,B", A, B, 6, cd6);
    check("C,D", C, D, 3, ab3); check("C,D", C, D, 6, ab6);
    // ==== 2.11b retention checks (added 2026-07-16 after the adversarial review
    // found the validation-plan step missing): assert the eq-2.11b filter KEEPS
    // this solution, through the REAL C++ predicates AND the survive_profiles*
    // membership the join/pair22 paths actually consume. G_THM211B is forced on
    // here on purpose — this mode always asserts the strictest filter.
    {
      G_THM211B = true;
      G_THM212  = true;  // retention always tests the strictest filter stack
      PairAutoSet ab6a_t, cd6a_t;
      ab6a_t.build(G_N1, G_SIG_A, G_SIG_B, 6);
      cd6a_t.build(n,    G_SIG_C, G_SIG_D, 6);
      auto check211b = [&](const char *side, const vector<int> &X, const vector<int> &Y,
                           const PairAutoSet &comp) {
        auto px = csums(X, 6), py = csums(Y, 6);
        int need = tgt - norm_vec(px) - norm_vec(py);
        vector<int> needT(3);
        for (int s = 1; s <= 3; s++) needT[s - 1] = -pair_auto(px, py, s);
        bool ok = need >= 0 && comp.feasible(needT, need);
        cout << "PC+211b " << side << " mod-6: needT=(" << needT[0] << "," << needT[1]
             << "," << needT[2] << ") need=" << need << " -> "
             << (ok ? "PASS" : "FAIL") << "\n" << flush;
        allok &= ok;
      };
      check211b("A,B", A, B, cd6a_t);
      check211b("C,D", C, D, ab6a_t);
      auto member6 = [&](const char *side, const vector<int> &X, const vector<int> &Y,
                         int L, int sX, int sY, const PairNormSet &c3,
                         const PairNormSet &c6, const PairAutoSet &c6a) {
        auto profs = survive_profiles6(L, sX, sY, c3, c6, 20000000, &c6a);
        auto px = csums(X, 6), py = csums(Y, 6);
        bool found = false;
        for (auto &pr : profs) if (pr.px == px && pr.py == py) { found = true; break; }
        cout << "PC survive_profiles6+211b " << side << ": champion mod-6 profile "
             << (found ? "KEPT" : "EXCLUDED") << " (survivors " << profs.size() << ")\n" << flush;
        allok &= found;
      };
      member6("A,B", A, B, G_N1, G_SIG_A, G_SIG_B, cd3, cd6, cd6a_t);
      member6("C,D", C, D, n,    G_SIG_C, G_SIG_D, ab3, ab6, ab6a_t);
      auto member3 = [&](const char *side, const vector<int> &X, const vector<int> &Y,
                         int L, int sX, int sY, const PairNormSet &c3,
                         const PairNormSet &c6, const PairAutoSet &c6a) {
        auto profs = survive_profiles(L, sX, sY, c3, c6, &c6a);
        auto px = csums(X, 3), py = csums(Y, 3);
        bool found = false;
        for (auto &pr : profs) if (pr.px == px && pr.py == py) { found = true; break; }
        cout << "PC survive_profiles(mod3, 211b-tighten) " << side << ": champion mod-3 profile "
             << (found ? "KEPT" : "EXCLUDED") << " (survivors " << profs.size() << ")\n" << flush;
        allok &= found;
      };
      member3("A,B", A, B, G_N1, G_SIG_A, G_SIG_B, cd3, cd6, cd6a_t);
      member3("C,D", C, D, n,    G_SIG_C, G_SIG_D, ab3, ab6, ab6a_t);
    }
    cout << (allok ? "PROFILE_CHECK: ALL PASS — residue lift keeps this solution\n"
                   : "PROFILE_CHECK: FAIL — residue lift would EXCLUDE this solution\n") << flush;
    return allok ? 0 : 1;
  }

  // --- surviving profiles for each side (complement = the OTHER side) ---
  // Thm 2.3 eq 2.11b (WZ_THM211B=1): build the complement sides' (autocorr,norm)
  // sets at m=6 — the modulus where 2.11b is not vacuous. See PairAutoSet.
  PairAutoSet ab6a, cd6a;
  if (G_THM211B) {
    ab6a.build(G_N1, G_SIG_A, G_SIG_B, 6);
    cd6a.build(n,    G_SIG_C, G_SIG_D, 6);
    cout << "[thm2.11b] ENABLED — complement (autocorr,norm) tuples: A,B side "
         << ab6a.achievable.size() << "  C,D side " << cd6a.achievable.size() << "\n" << flush;
  }
  auto abProfs = survive_profiles(G_N1, G_SIG_A, G_SIG_B, cd3, cd6, G_THM211B ? &cd6a : nullptr);
  auto cdProfs = survive_profiles(n,    G_SIG_C, G_SIG_D, ab3, ab6, G_THM211B ? &ab6a : nullptr);
  cout << "[profiles] A,B side surviving: " << abProfs.size()
       << "   C,D side surviving: " << cdProfs.size() << "\n" << flush;

  bool pinA = (G_SIG_A == 0), pinB = (G_SIG_B == 0);
  bool pinC = (G_SIG_C == 0), pinD = (G_SIG_D == 0);

  // ---- WZ_FIRSTHIT: Gate B + Gate C probe (work order 2026-07-16). Stream the
  //      Thm-2.2-constrained C,D PAIR stream in DETERMINISTIC order (profiles
  //      sequential, single thread => exact candidate index), run the bounded
  //      A,B completion per candidate, STOP at the first NPAF==0 hit.
  //      Gate C = hit index / stream size, per ordering. Gate B = per-candidate
  //      completion cost distribution. Orderings:
  //        WZ_FH_PROF_ORDER: 0 = natural DFS · 1 = flattest profile first
  //          (ascending sum |class sums|) · 2 = reverse (control)
  //        WZ_FH_SCORE_MAX: only complete candidates with sum_s|cd[s]| <= this
  //          (PSD-flatness gate — the WZ_PSD_BIAS intuition; threshold passes
  //          approximate a global flatness ordering with O(1) memory)
  //      WZ_FH_AB_BUDGET: nodes/candidate (default 200k; 0 = exact).
  //      WZ_FH_MAX_CAND: stop after this many candidates (bounded probe).
  //      Exit 0 = FOUND (banner at find time). Exit 3 = budget/stream ended,
  //      NO hit — NOT a proof of absence (aborted candidates are unknowns).
  if (getenv("WZ_FIRSTHIT")) {
    init_p22();
    // Stream source modulus. Mod-3 profiles are WALLTIME ATOMS at n>=36 — one
    // profile's DFS exceeds 12h (measured twice: the 07-15 P22 gate death, and
    // the 07-21 zero-candidate wave: 2/9 n=36 classes + ALL n>=41 jobs streamed
    // ZERO candidates in 11.5-25h). WZ's Step 4 generates C,D from MOD-6
    // profiles for exactly this reason (cells ~100-1000x smaller; the mod-6
    // union == mod-3 stream EXACTLY, invariant validated 07-15: 66/91, 1564/809).
    // Default: mod-6 at n>=36, mod-3 below. WZ_FH_M6=1/0 forces on/off (A/B).
    bool fh_m6 = (n >= 36);
    if (const char *e = getenv("WZ_FH_M6")) fh_m6 = atoi(e) != 0;
    FH_BUDGET = 200000;
    if (const char *e = getenv("WZ_FH_AB_BUDGET")) FH_BUDGET = atoll(e);
    long long max_cand = 0;
    if (const char *e = getenv("WZ_FH_MAX_CAND")) max_cand = atoll(e);
    long long score_max = 0;
    if (const char *e = getenv("WZ_FH_SCORE_MAX")) score_max = atoll(e);
    int prof_order = 0;
    if (const char *e = getenv("WZ_FH_PROF_ORDER")) prof_order = atoi(e);
    double stream_total = 0;  // banked baseline for fractional-depth print
    if (const char *e = getenv("WZ_FH_STREAM_TOTAL")) stream_total = atof(e);
    // Cluster sharding: arm i of N streams profiles pi ≡ i (mod N), in order.
    // INTERLEAVED on purpose — the JOIN22 canary measured the profile fat-tail
    // (last 29/541 profiles cost more than the first 512); contiguous slices
    // would concentrate the tail in one arm. Depth semantics per arm are
    // preserved (profiles still processed in ascending rank within the arm);
    // the global first hit ≈ min over arms by (profile_rank, idx).
    int fh_shard = 0, fh_nshard = 1;
    if (const char *e = getenv("WZ_FH_SHARD"))  fh_shard  = atoi(e);
    if (const char *e = getenv("WZ_FH_NSHARD")) fh_nshard = atoi(e);
    if (fh_nshard < 1) fh_nshard = 1;
    if (fh_shard < 0 || fh_shard >= fh_nshard) {
      cout << "[firsthit] BAD SHARD " << fh_shard << "/" << fh_nshard << "\n";
      return 2;
    }

    vector<Profile> fhProfs;
    if (fh_m6) {
      fhProfs = survive_profiles6(n, G_SIG_C, G_SIG_D, ab3, ab6, 20000000,
                                  G_THM211B ? &ab6a : nullptr);
      cout << "[firsthit] mod-6 stream source: " << fhProfs.size()
           << " C,D profile cells (mod-3 atoms exceed walltime at this n)\n" << flush;
    } else {
      fhProfs = cdProfs;
    }
    int fh_m = fh_m6 ? 6 : 3;
    auto profScore = [](const Profile &p) {
      long long s = 0;
      for (int v : p.px) s += abs(v);
      for (int v : p.py) s += abs(v);
      return s;
    };
    if (prof_order == 1)
      sort(fhProfs.begin(), fhProfs.end(), [&](const Profile &a, const Profile &b)
           { return profScore(a) < profScore(b); });
    else if (prof_order == 2)
      sort(fhProfs.begin(), fhProfs.end(), [&](const Profile &a, const Profile &b)
           { return profScore(a) > profScore(b); });
    cout << "[firsthit] profiles=" << fhProfs.size() << " m=" << fh_m
         << " order=" << prof_order
         << " ab_budget=" << FH_BUDGET << " max_cand=" << max_cand
         << " score_max=" << score_max
         << " shard=" << fh_shard << "/" << fh_nshard << "\n" << flush;

    long long cand = 0, pre_rej = 0, score_rej = 0, clean_no = 0, aborted = 0;
    long long cells_done = 0;  // fully-processed profile cells (feeds WZ_FH_PROF_SKIP)
    long long bt_entered = 0, hit_idx = -1;
    int hit_prof = -1;
    atomic<bool> fh_stop{false};
    g_fh_stop_ptr = &fh_stop;
    signal(SIGTERM, fh_on_sigterm);
    double prog_sec = 60;  // periodic progress cadence, seconds (WZ_FH_PROG_SEC)
    if (const char *e = getenv("WZ_FH_PROG_SEC")) prog_sec = atof(e);
    auto T0 = Clock::now();
    // WZ_FH_PROF_SKIP=k: skip each arm's first k profile cells — continuation
    // waves search DISJOINT depth instead of re-treading the previous window
    // (07-24: one node-day cleanly exhausts ~10-19M candidates/class at n=41;
    // expected first hit is deeper — restarting at cell 0 wastes the window).
    // Use the MIN cells-completed across the prior wave's arms: overlap is
    // idempotent, a gap would be unsound.
    int fh_skip = 0;
    if (const char *e = getenv("WZ_FH_PROF_SKIP")) fh_skip = atoi(e);
    bool cell_order = fh_m6;   // flat-first within cells (see below); =0 disables
    if (const char *e = getenv("WZ_FH_CELL_ORDER")) cell_order = atoi(e) != 0;
    // Profile-constrained A,B completion setup (see AbpRow comment block).
    // Precompute ONE map keyed by the cell invariant (need, needT): need =
    // 4n+2 - norm(cd profile) (2.11a leftover), needT = the residue-autocorr
    // tuple the A,B side must cancel (2.11b). All cells sharing a key share
    // one allowed list, so the O(|K|x|R|) sweep runs ONCE per arm, not per
    // cell. 2.12 is applied per (k,r) row. Default ON for mod-6 cells (same
    // convention as cell_order); WZ_FH_CELL_ORDER-style kill switch.
    bool ab_prof = fh_m6;
    if (const char *e = getenv("WZ_FH_AB_PROF")) ab_prof = atoi(e) != 0;
    size_t abp_cap = 20000;    // cells with bigger lists run unconstrained
    if (const char *e = getenv("WZ_FH_AB_PROF_CAP")) abp_cap = (size_t)atoll(e);
    unordered_map<uint64_t, vector<AbpRow>> abpMap;
    int half_m = fh_m / 2;
    auto cell_abp_key = [&](const Profile &p) {
      int need = 4 * n + 2 - norm_vec(p.px) - norm_vec(p.py);
      vector<int> T(half_m);
      for (int s = 1; s <= half_m; s++) T[s - 1] = -pair_auto(p.px, p.py, s);
      return auto_key(T, need);
    };
    if (ab_prof) {
      auto tb0 = Clock::now();
      for (int pi = fh_shard + fh_skip * fh_nshard; pi < (int)fhProfs.size();
           pi += fh_nshard)
        abpMap.try_emplace(cell_abp_key(fhProfs[pi]));
      // Single-side tuples over BOTH signed targets: the completer accepts
      // sum(A) = ±a (and ±b) — the A[0]=B[0]=+1 canonical representative of a
      // completion class can carry either sign, so pruning to one sign would
      // lose retention.
      struct AbpTup { vector<int> v; int norm; int ca[3]; };
      auto build_tups = [&](int sig) {
        vector<AbpTup> out;
        for (int sgn = 1; sgn >= (sig ? -1 : 1); sgn -= 2)
          for (auto &v : enum_class_sums(G_N1, sgn * sig, fh_m)) {
            AbpTup t; t.v = v; t.norm = norm_vec(v);
            if (t.norm > 4 * n + 2) continue;
            for (int s = 1; s <= half_m; s++)
              t.ca[s - 1] = autocorr_vec(v, s) + autocorr_vec(v, fh_m - s);
            out.push_back(std::move(t));
          }
        return out;
      };
      auto Kt = build_tups(G_SIG_A), Rt = build_tups(G_SIG_B);
      long long abp_rows = 0;
      size_t abp_maxlist = 0;
      vector<int> Tsum(half_m);
      for (auto &kt : Kt)
        for (auto &rt : Rt) {
          int nn2 = kt.norm + rt.norm;
          if (nn2 > 4 * n + 2) continue;
          for (int s = 0; s < half_m; s++) Tsum[s] = kt.ca[s] + rt.ca[s];
          auto it = abpMap.find(auto_key(Tsum, nn2));
          if (it == abpMap.end()) continue;
          if (!thm212_ok(kt.v, rt.v, fh_m, true)) continue;  // eq 2.12, A,B side
          AbpRow row{};
          for (int c = 0; c < fh_m; c++) {
            row.k[c] = (int8_t)kt.v[c];
            row.r[c] = (int8_t)rt.v[c];
          }
          it->second.push_back(row);
          abp_rows++;
          if (it->second.size() > abp_maxlist) abp_maxlist = it->second.size();
        }
      cout << "[abprof] K=" << Kt.size() << " R=" << Rt.size()
           << " cell_keys=" << abpMap.size() << " rows=" << abp_rows
           << " max_list=" << abp_maxlist << " cap=" << abp_cap << " build="
           << chrono::duration<double>(Clock::now() - tb0).count() << "s\n"
           << flush;
      FH_ABP_STACK.assign(G_N1 / 2 + 2, {});
    }
    FH_ABP_M = fh_m;   // partial-class bookkeeping modulus (harmless when off)
    for (int c = 0; c < fh_m; c++) FH_ABP_TIC[c] = class_count(G_N1, c, fh_m);
    long long cells_prof_dead = 0, cells_prof_uncap = 0;
    auto last_prog = T0;
    for (int pi = fh_shard + fh_skip * fh_nshard;
         pi < (int)fhProfs.size() && !fh_stop.load(); pi += fh_nshard) {
      FH_ABP = nullptr;
      if (ab_prof) {
        auto it = abpMap.find(cell_abp_key(fhProfs[pi]));
        if (it == abpMap.end() || it->second.empty()) {
          // PROVEN dead: no A,B mod-m profile satisfies 2.11a+2.11b+2.12 for
          // this cell's (need, needT), so no candidate in it can complete.
          // Counts as a fully-processed cell for PROF_SKIP resume purposes.
          cells_prof_dead++;
          cells_done++;
          continue;
        }
        if (it->second.size() <= abp_cap) {
          FH_ABP = &it->second;
          auto &root = FH_ABP_STACK[0];
          root.resize(it->second.size());
          for (size_t ri = 0; ri < root.size(); ri++) root[ri] = (int)ri;
        } else {
          cells_prof_uncap++;  // oversized list: run this cell unconstrained
        }
      }
      long long lv = 0, okc = 0;
      // Flat-first within-cell ordering (WZ_FH_CELL_ORDER=0 disables; default
      // ON for mod-6 cells): buffer the cell's candidates, sort by flatness
      // score ascending, complete in that order. Every arm gets the measured
      // ~35x flat-density enrichment as an ORDERING — no coverage loss —
      // instead of only the score-gated quarter. 500k buffer cap guards
      // monster cells (drain in sorted batches; batch-local order, exact
      // coverage). WZ's own 41-43 solutions score 140/142/134 — flat.
      struct CellCand { long long sc; vector<int> C, D; };
      vector<CellCand> cellbuf;
      auto flat_score = [&](const int *Ci, const int *Di) {
        long long sc = 0;
        for (int s = 1; s <= n; s++) {
          int cd = 0;
          for (int i = 0; i + s < n; i++) cd += Ci[i]*Ci[i+s] + Di[i]*Di[i+s];
          sc += abs(cd);
        }
        return sc;
      };
      auto complete_one = [&](const int *Ci, const int *Di) {
        long long nodes_before = fh_nodes_total;
        int r = fh_complete_ab(Ci, Di);
        if (r >= 2) bt_entered++;
        if (r == 1) pre_rej++;
        else if (r == 2) clean_no++;
        else if (r == 3) aborted++;
        else {  // r == 0: HIT
          hit_idx = cand; hit_prof = pi;
          g_found.store(true);
          fh_stop.store(true);
          double t = chrono::duration<double>(Clock::now() - T0).count();
          int n1 = G_N1;
          int sa=0,sb=0,sc2=0,sd=0;
          for (int i=0;i<n1;i++){sa+=g_solA[i];sb+=g_solB[i];}
          for (int i=0;i<n;i++){sc2+=g_solC[i];sd+=g_solD[i];}
          cout << "\n*** BS(" << n1 << "," << n << ") FOUND ***  (firsthit probe)\n";
          cout << "sig = (" << sa << "," << sb << "," << sc2 << "," << sd << ")\n";
          cout << "A = {"; for(int i=0;i<n1;i++) cout<<g_solA[i]<<(i<n1-1?",":""); cout<<"};\n";
          cout << "B = {"; for(int i=0;i<n1;i++) cout<<g_solB[i]<<(i<n1-1?",":""); cout<<"};\n";
          cout << "C = {"; for(int i=0;i<n;i++)  cout<<g_solC[i]<<(i<n-1?",":"");  cout<<"};\n";
          cout << "D = {"; for(int i=0;i<n;i++)  cout<<g_solD[i]<<(i<n-1?",":"");  cout<<"};\n";
          int maxv = 0;
          for (int s = 1; s <= n; s++) {
            int v = npaf_at(g_solA, g_solB, n1, g_solC, g_solD, n, s);
            if (abs(v) > maxv) maxv = abs(v);
          }
          cout << "VERIFY: max |NPAF[s]| over s=1.." << n << " = " << maxv
               << (maxv==0 ? "  (NPAF==0 confirmed)\n" : "  (NONZERO!)\n");
          cout << "FIRSTHIT: idx=" << hit_idx << " profile_rank=" << pi
               << " nodes_this_cand=" << (fh_nodes_total - nodes_before)
               << " score=" << flat_score(Ci, Di)
               << " elapsed=" << t << "s";
          if (stream_total > 0)
            cout << "  frac_depth=" << (double)hit_idx / stream_total;
          cout << "\n" << flush;
        }
      };
      auto drain = [&]() {
        stable_sort(cellbuf.begin(), cellbuf.end(),
                    [](const CellCand &a, const CellCand &b){ return a.sc < b.sc; });
        for (auto &cc : cellbuf) {
          if (g_found.load() || g_fh_sigterm) break;  // hits/SIGTERM abort;
          int Ci[64], Di[64];                         // max_cand still drains
          for (int i = 0; i < n; i++) { Ci[i] = cc.C[i]; Di[i] = cc.D[i]; }
          complete_one(Ci, Di);
        }
        cellbuf.clear();
      };
      function<void(const vector<int>&, const vector<int>&)> probe =
          [&](const vector<int> &C, const vector<int> &D) {
        if (fh_stop.load()) return;
        cand++;
        // Progress is TIME-based and carries the exact tokens the driver
        // aggregates: the old every-200k-cands line never printed at n>=41
        // rates (~5-20 cand/s => <200k per 11.5h shard), so killed arms left
        // logs with no counters at all. Clock checked every 256 cands. Sits
        // ABOVE the score gate — the score-rejection path returns early and
        // would starve progress on gated arms. WZ_FH_PROG_SEC overrides.
        if ((cand & 255) == 0) {
          auto nowp = Clock::now();
          if (chrono::duration<double>(nowp - last_prog).count() >= prog_sec) {
            last_prog = nowp;
            double t = chrono::duration<double>(nowp - T0).count();
            cout << "[firsthit progress] candidates_streamed=" << cand
                 << "  pre_filter_rejected=" << pre_rej
                 << "  score_rejected=" << score_rej
                 << "  clean_no_AB=" << clean_no
                 << "  budget_aborted=" << aborted
                 << "  total_AB_nodes=" << fh_nodes_total
                 << "  [" << t << "s]\n" << flush;
          }
        }
        int Ci[64], Di[64];
        for (int i = 0; i < n; i++) { Ci[i] = C[i]; Di[i] = D[i]; }
        long long sc = -1;
        if (score_max > 0 || cell_order) sc = flat_score(Ci, Di);
        if (score_max > 0 && sc > score_max) { score_rej++; return; }
        if (cell_order) {
          cellbuf.push_back({sc, C, D});
          if (cellbuf.size() >= 500000) drain();
          if (max_cand > 0 && cand >= max_cand) fh_stop.store(true);
          return;
        }
        complete_one(Ci, Di);
        if (max_cand > 0 && cand >= max_cand) fh_stop.store(true);
      };
      count_pairs22(n, fhProfs[pi].px, fhProfs[pi].py, false, pinC, pinD,
                    lv, okc, &probe, fh_m, &fh_stop);
      if (cell_order && !cellbuf.empty()) drain();  // finish the cell in order
      if (!fh_stop.load()) cells_done++;  // only cells processed to completion
    }
    double t = chrono::duration<double>(Clock::now() - T0).count();
    long long completed_tested = clean_no + aborted + (hit_idx >= 0 ? 1 : 0);
    cout << "\n=== FIRSTHIT SUMMARY (n=" << n << ", sig " << G_SIG_A << ","
         << G_SIG_B << "," << G_SIG_C << "," << G_SIG_D
         << ", shard " << fh_shard << "/" << fh_nshard << ") ===\n"
         << "candidates_streamed=" << cand << "  pre_filter_rejected=" << pre_rej
         << "  score_rejected=" << score_rej << "\n"
         << "backtracks_entered=" << completed_tested << "  clean_no_AB=" << clean_no
         << "  budget_aborted=" << aborted << "  total_AB_nodes=" << fh_nodes_total
         << "  cells_done=" << cells_done << " (skip=" << fh_skip << ")"
         << "  ab_prof=" << (ab_prof ? 1 : 0)
         << "  cells_prof_dead=" << cells_prof_dead
         << "  cells_prof_uncap=" << cells_prof_uncap << "\n"
         << (hit_idx >= 0
             ? "RESULT: FOUND at idx=" + to_string(hit_idx)
             : (g_fh_sigterm
                ? string("RESULT: INTERRUPTED (SIGTERM) — counts are LOWER "
                         "BOUNDS, stream NOT exhausted")
                : string("RESULT: NO HIT within probe budget — NOT a proof "
                         "of absence")))
         << "\nTime: " << t << "s\n" << flush;
    return hit_idx >= 0 ? 0 : 3;
  }

  // ---- WZ_JOIN22: the COMPLETE hash-join over the Thm-2.2-constrained space.
  //      Rebuilt 2026-07-09 after Gate A' measured the true streams ~10^5x
  //      smaller than the independent-side enumeration the old join used
  //      (n=29: C,D 1.74e9 / A,B 2.46e9 — hash fits a node again). Store the
  //      C,D side (smaller in every measured case, dedup one rec per 64-bit
  //      key — sound, exact npaf recheck guards collisions), stream A,B.
  //      Completeness caveats: (1) FNV-shadowing means a negative needs a
  //      perturbed-basis re-run (audit 2026-07-03); (2) completeness is
  //      relative to the Thm-2.2 space — every banked + published solution
  //      lies inside it, but a per-signature negative is "no 2.2-normalized
  //      solution with this SIGNED sig".
  if (getenv("WZ_JOIN22")) {
    init_p22();
    // v2 (2026-07-11): the v1 unordered_map<Key,Rec> hash OOM-killed the n=29
    // canary (~1.7e9 records x real map overhead >> node RAM). v2 stores ONLY
    // bare 64-bit keys in a flat open-addressed table (lock-free CAS inserts,
    // linear probing, 0 = empty sentinel): ~8 B/slot, so n=29 fits in ~34 GB
    // and n=31 (~2e10 keys, measured stream) in ~275 GB. Three phases:
    //   1. BUILD: enumerate the C,D 2.2-stream, insert keys.
    //   2. STREAM: enumerate the A,B 2.2-stream, probe; collect raw hits.
    //   3. RESOLVE: re-enumerate C,D (deterministic DFS), exact npaf recheck
    //      of every (hit A,B) x (C,D with that key); print on NPAF==0.
    // Same completeness caveats as v1 (FNV-shadowing; Thm-2.2 space).
    int slots_log2 = 32;  // 2^32 slots = 34 GB; override for bigger n
    if (const char *e = getenv("WZ_JOIN22_SLOTS_LOG2")) slots_log2 = atoi(e);
    const uint64_t nslots = 1ull << slots_log2;
    const uint64_t mask = nslots - 1;
    cout << "[join22v2] key table: 2^" << slots_log2 << " slots = "
         << (nslots * 8.0 / 1e9) << " GB\n" << flush;
    vector<uint64_t> table_v(nslots, 0);
    atomic<uint64_t> *table = reinterpret_cast<atomic<uint64_t>*>(table_v.data());
    atomic<long long> nkeys{0};
    auto fixkey = [](Key k) -> Key { return k ? k : 1; };  // 0 is the empty sentinel

    {  // ---- phase 1: BUILD key table from the C,D stream ----
      int nprof = (int)cdProfs.size();
      atomic<long long> built{0};
      double cd_stream = 0;
      #pragma omp parallel for schedule(dynamic) reduction(+:cd_stream)
      for (int pi = 0; pi < nprof; pi++) {
        long long lv = 0, okc = 0;
        function<void(const vector<int>&, const vector<int>&)> ins =
            [&](const vector<int> &C, const vector<int> &D) {
          Key k = fixkey(autocorr_key(C.data(), D.data(), n, true));
          uint64_t idx = k & mask;
          while (true) {
            uint64_t cur = table[idx].load(memory_order_relaxed);
            if (cur == k) break;  // dedup
            if (cur == 0) {
              uint64_t exp = 0;
              if (table[idx].compare_exchange_strong(exp, k, memory_order_relaxed)) {
                nkeys.fetch_add(1, memory_order_relaxed);
                break;
              }
              continue;  // lost the race; re-read this slot
            }
            idx = (idx + 1) & mask;
          }
        };
        count_pairs22(n, cdProfs[pi].px, cdProfs[pi].py, false, pinC, pinD,
                      lv, okc, &ins);
        cd_stream += (double)okc;
        long long b = ++built;
        if ((b % 32) == 0 || b == nprof) {
          #pragma omp critical
          cout << "[join22v2 build " << b << "/" << nprof << "] keys~"
               << nkeys.load() << " ["
               << chrono::duration<double>(Clock::now()-G_T0).count() << "s]\n" << flush;
        }
      }
      double load = (double)nkeys.load() / (double)nslots;
      cout << "[join22v2] C,D stream=" << cd_stream << "  distinct keys="
           << nkeys.load() << "  (dedup x" << (nkeys.load() ? cd_stream / nkeys.load() : 0)
           << ")  table load=" << load << "\n" << flush;
      if (load > 0.85)
        cout << "[join22v2] WARNING: load>0.85 — probing degrades; raise WZ_JOIN22_SLOTS_LOG2\n" << flush;
    }

    struct HitAB { Key k; int8_t A[64], B[64]; };
    vector<HitAB> hits;
    {  // ---- phase 2: STREAM A,B, probe the table, collect raw key hits ----
      int nprof = (int)abProfs.size();
      atomic<long long> streamed{0};
      #pragma omp parallel for schedule(dynamic)
      for (int pi = 0; pi < nprof; pi++) {
        function<void(const vector<int>&, const vector<int>&)> look =
            [&](const vector<int> &A, const vector<int> &B) {
          Key k = fixkey(autocorr_key(A.data(), B.data(), G_N1, false));
          uint64_t idx = k & mask;
          while (true) {
            uint64_t cur = table[idx].load(memory_order_relaxed);
            if (cur == 0) return;          // absent
            if (cur == k) break;           // hit
            idx = (idx + 1) & mask;
          }
          #pragma omp critical
          {
            HitAB h; h.k = k;
            for (int i = 0; i < G_N1; i++) { h.A[i] = (int8_t)A[i]; h.B[i] = (int8_t)B[i]; }
            hits.push_back(h);
            if (hits.size() % 100000 == 0)
              cout << "[join22v2] hits=" << hits.size() << " (unusually many)\n" << flush;
          }
        };
        long long lv = 0, okc = 0;
        count_pairs22(G_N1, abProfs[pi].px, abProfs[pi].py, true, pinA, pinB,
                      lv, okc, &look);
        long long s = ++streamed;
        if ((s % 32) == 0 || s == nprof) {
          #pragma omp critical
          cout << "[join22v2 stream " << s << "/" << nprof << "] hits="
               << hits.size() << " ["
               << chrono::duration<double>(Clock::now()-G_T0).count() << "s]\n" << flush;
        }
      }
      cout << "[join22v2] phase 2 done: " << hits.size() << " raw key hits\n" << flush;
    }

    // Full solution banner. Called TWICE on success: once at find time (inside
    // the resolve critical section, so walltime can never eat a found solution
    // again — 15719454 lost its BS(30,29) exactly there) and once at end-of-run.
    // Both blocks are identical and both carry the exact-NPAF VERIFY line.
    auto print_banner = [&](bool at_find_time) {
      int n1 = G_N1;
      int sa = 0, sb = 0, sc = 0, sd = 0;
      for (int i = 0; i < n1; i++) { sa += g_solA[i]; sb += g_solB[i]; }
      for (int i = 0; i < n; i++)  { sc += g_solC[i]; sd += g_solD[i]; }
      cout << "\n*** BS(" << n1 << "," << n << ") FOUND ***\n";
      cout << "sig = (" << sa << "," << sb << "," << sc << "," << sd << ")\n";
      cout << "A = {"; for (int i=0;i<n1;i++) cout << g_solA[i] << (i<n1-1?",":""); cout << "};\n";
      cout << "B = {"; for (int i=0;i<n1;i++) cout << g_solB[i] << (i<n1-1?",":""); cout << "};\n";
      cout << "C = {"; for (int i=0;i<n;i++)  cout << g_solC[i] << (i<n-1?",":"");  cout << "};\n";
      cout << "D = {"; for (int i=0;i<n;i++)  cout << g_solD[i] << (i<n-1?",":"");  cout << "};\n";
      int maxv = 0;
      for (int s = 1; s <= n; s++) {
        int v = npaf_at(g_solA, g_solB, n1, g_solC, g_solD, n, s);
        if (abs(v) > maxv) maxv = abs(v);
      }
      cout << "VERIFY: max |NPAF[s]| over s=1.." << n << " = " << maxv
           << (maxv == 0 ? "  (NPAF==0 confirmed)\n" : "  (NONZERO!)\n");
      if (at_find_time)
        cout << "[join22v2] banner flushed AT FIND TIME ["
             << chrono::duration<double>(Clock::now()-G_T0).count()
             << "s] — resolve loop draining, final banner + Time follow\n";
      cout << flush;
    };

    if (!hits.empty()) {  // ---- phase 3: RESOLVE — re-enumerate C,D, exact recheck ----
      unordered_multimap<Key, size_t> hitmap;
      hitmap.reserve(hits.size() * 2);
      for (size_t i = 0; i < hits.size(); i++) hitmap.emplace(hits[i].k, i);
      int nprof = (int)cdProfs.size();
      atomic<long long> resolved{0};
      #pragma omp parallel for schedule(dynamic)
      for (int pi = 0; pi < nprof; pi++) {
        if (g_found.load()) continue;
        function<void(const vector<int>&, const vector<int>&)> res =
            [&](const vector<int> &C, const vector<int> &D) {
          if (g_found.load()) return;
          Key k = fixkey(autocorr_key(C.data(), D.data(), n, true));
          auto range = hitmap.equal_range(k);
          if (range.first == range.second) return;
          int Ai[64], Bi[64], Ci[64], Di[64];
          for (int i = 0; i < n; i++) { Ci[i] = C[i]; Di[i] = D[i]; }
          for (auto it = range.first; it != range.second; ++it) {
            const HitAB &h = hits[it->second];
            for (int i = 0; i < G_N1; i++) { Ai[i] = h.A[i]; Bi[i] = h.B[i]; }
            bool okall = true;
            for (int s = 1; s <= n; s++)
              if (npaf_at(Ai, Bi, G_N1, Ci, Di, n, s) != 0) { okall = false; break; }
            if (!okall) continue;
            #pragma omp critical
            {
              if (!g_found.load()) {
                memcpy(g_solA, Ai, G_N1 * sizeof(int));
                memcpy(g_solB, Bi, G_N1 * sizeof(int));
                memcpy(g_solC, Ci, n * sizeof(int));
                memcpy(g_solD, Di, n * sizeof(int));
                g_found.store(true);
                print_banner(true);  // flush NOW — never lose a find to walltime
              }
            }
            return;
          }
        };
        long long lv = 0, okc = 0;
        count_pairs22(n, cdProfs[pi].px, cdProfs[pi].py, false, pinC, pinD,
                      lv, okc, &res, 3, &g_found);
        long long r = ++resolved;
        if ((r % 64) == 0 || r == nprof) {
          #pragma omp critical
          cout << "[join22v2 resolve " << r << "/" << nprof
               << (g_found.load() ? " FOUND" : "") << "] ["
               << chrono::duration<double>(Clock::now()-G_T0).count() << "s]\n" << flush;
        }
      }
    }

    double t = chrono::duration<double>(Clock::now() - G_T0).count();
    if (g_found.load()) {
      print_banner(false);
      cout << "Time: " << t << "s\n" << flush;
    } else {
      cout << "\n=== JOIN22 EXHAUSTED (n=" << n << ") — no solution in the "
           << "Thm-2.2 space for sig (" << G_SIG_A << "," << G_SIG_B << ","
           << G_SIG_C << "," << G_SIG_D << ") ===\n"
           << "(negative claims need a perturbed-hash re-run + signed-sig sweep; see plan)\n"
           << "Time: " << t << "s\n" << flush;
    }
    return g_found.load() ? 0 : 3;
  }

  // ---- WZ_COUNT_PAIR22: GATE A' — the TRUE Wang-Zhu-constrained stream.
  //      Joint (X,Y) generation under Thm 2.2 pair encoding + class sums +
  //      per-sequence AND joint spectral. Directly comparable to the plan's
  //      thresholds; `ok` IS the stream size (no |X|*|Y| product inflation).
  if (getenv("WZ_COUNT_PAIR22")) {
    init_p22();
    // PROFILE-RANGE SHARDING (2026-07-11). The counter was OpenMP-parallel WITHIN a
    // node but could not span nodes, so n=36 CD finished only 96/985 profiles in a
    // 12h walltime (~19.6 thread-hours/profile => ~19,300 thread-hours total). That
    // put THE gate number out of reach of any single job. WZ_PROF_LO/WZ_PROF_HI slice
    // the profile list so a SLURM array counts disjoint chunks; sum the per-shard
    // STREAM values to get the total. 20 tasks x 192 threads ~= 5h. Half-open [LO,HI).
    // Invariant (validated at n=11): sum over a partition == the unsharded total.
    auto countP = [&](vector<Profile> &profs, int L, bool abSide,
                      bool pin0, bool pin1, const char *label) -> double {
      double leaves_t = 0, ok_t = 0;
      long long done = 0;
      int nprof = (int)profs.size();
      int lo = 0, hi = nprof;
      if (const char *e = getenv("WZ_PROF_LO")) lo = max(0, atoi(e));
      if (const char *e = getenv("WZ_PROF_HI")) hi = min(nprof, atoi(e));
      if (lo > hi) lo = hi;
      int nshard = hi - lo;
      cout << "[pair22 " << label << "] shard profiles [" << lo << "," << hi
           << ") of " << nprof << "\n" << flush;
      #pragma omp parallel for schedule(dynamic)
      for (int pi = lo; pi < hi; pi++) {
        long long lv = 0, okc = 0;
        count_pairs22(L, profs[pi].px, profs[pi].py, abSide, pin0, pin1, lv, okc,
                      nullptr, (int)profs[pi].px.size());   // modulus = profile width
        #pragma omp critical
        {
          leaves_t += (double)lv; ok_t += (double)okc; done++;
          if ((done % 32) == 0 || done == nshard) {
            double t = chrono::duration<double>(Clock::now() - G_T0).count();
            cout << "[pair22 " << label << " " << done << "/" << nshard
                 << " (shard " << lo << "-" << hi << ")] leaves~" << leaves_t
                 << " stream~" << ok_t << " [" << t << "s]\n" << flush;
          }
        }
      }
      cout << "=== PAIR22 COUNT " << label << " (L=" << L << ", profiles="
           << nshard << "/" << nprof << " shard [" << lo << "," << hi << ")) ===\n"
           << "  encoding-leaves=" << leaves_t
           << "  STREAM (all filters) = " << ok_t << "\n"
           << "  SHARD_STREAM " << label << " " << lo << " " << hi << " " << ok_t
           << "\n" << flush;   // <- grep 'SHARD_STREAM' and sum column 5
      return ok_t;
    };
    // WZ_PAIR22_SIDE=CD (or AB): count only that side. The C,D stream is the
    // gate/sizing metric, and full both-side counts TIMEOUT at n>=31 because
    // the A,B side runs first — CD-only gets the decision number in one job.
    const char *sideSel = getenv("WZ_PAIR22_SIDE");
    double abS = -1, cdS = -1;
    // WZ Step 4: generate from mod-6 profiles (WZ_PAIR22_M6=1), not mod-3. The
    // modulus is taken from the profile width, so the SAME DFS serves both. With
    // WZ_THM211B=1 those mod-6 profiles are additionally cut by eq 2.11b.
    // INVARIANT (regression test): M6=1 + THM211B=0 must give the SAME stream as
    // mod-3 — finer profiles only regroup sequences. If it differs, the mod-6
    // profile set is incomplete and any 2.11b number built on it is worthless.
    if (getenv("WZ_PAIR22_M6")) {
      auto ab6P = survive_profiles6(G_N1, G_SIG_A, G_SIG_B, cd3, cd6,
                                    20000000, G_THM211B ? &cd6a : nullptr);
      auto cd6P = survive_profiles6(n,    G_SIG_C, G_SIG_D, ab3, ab6,
                                    20000000, G_THM211B ? &ab6a : nullptr);
      cout << "[pair22] GENERATING FROM MOD-6 PROFILES (WZ Step 4) — A,B "
           << ab6P.size() << "  C,D " << cd6P.size()
           << (G_THM211B ? "  [+2.11b]" : "  [norm-only]") << "\n" << flush;
      if (!sideSel || strcmp(sideSel, "CD") != 0)
        abS = countP(ab6P, G_N1, true,  pinA, pinB, "A,B");
      if (!sideSel || strcmp(sideSel, "AB") != 0)
        cdS = countP(cd6P, n,   false, pinC, pinD, "C,D");
    } else {
      if (!sideSel || strcmp(sideSel, "CD") != 0)
        abS = countP(abProfs, G_N1, true,  pinA, pinB, "A,B");
      if (!sideSel || strcmp(sideSel, "AB") != 0)
        cdS = countP(cdProfs, n,   false, pinC, pinD, "C,D");
    }
    double t = chrono::duration<double>(Clock::now() - G_T0).count();
    cout << "\n=== GATE A' SUMMARY (n=" << n << ", Thm-2.2-constrained) ===\n"
         << "TRUE streams: A,B " << abS << "   C,D " << cdS << "  (-1 = side skipped)\n"
         << "vs independent-side pair-work (earlier Gate A/count-only runs): the\n"
         << "ratio quantifies Thm 2.2's pruning power at this n.\n"
         << "Gate rule (docs/wz_firsthit_plan.md): C,D stream <= ~1e9 at n=36 PASS; >= 1e12 KILL.\n"
         << "Time: " << t << "s\n" << flush;
    return 0;
  }

  // ---- WZ_COUNT_MOD6: GATE A of docs/wz_firsthit_plan.md. Streaming counts of
  //      the MOD-6-constrained stream (the Wang-Zhu lift), zero materialization.
  //      Compare against a separate WZ_COUNT_ONLY (mod-3) run of the same sig:
  //      the reduction ratio is the gate metric. Progress lines leave partial
  //      data on TIMEOUT. Takes precedence over WZ_COUNT_ONLY if both are set.
  if (getenv("WZ_COUNT_MOD6")) {
    auto ab6Profs = survive_profiles6(G_N1, G_SIG_A, G_SIG_B, cd3, cd6,
                                      20000000, G_THM211B ? &cd6a : nullptr);
    auto cd6Profs = survive_profiles6(n,    G_SIG_C, G_SIG_D, ab3, ab6,
                                      20000000, G_THM211B ? &ab6a : nullptr);
    cout << "[mod6 profiles] A,B side: " << ab6Profs.size()
         << "   C,D side: " << cd6Profs.size() << "\n" << flush;
    auto count6 = [&](vector<Profile> &profs, int L, bool pin0, bool pin1,
                      const char *label) -> double {
      double sum_x = 0, sum_y = 0, pairwork = 0, max_pw = 0, ex_total = 0;
      long long done = 0;
      int nprof = (int)profs.size();
      long long stride = nprof > 6400 ? nprof / 100 : 32;
      #pragma omp parallel for schedule(dynamic)
      for (int pi = 0; pi < nprof; pi++) {
        long long xe = 0, xok = 0, ye = 0, yok = 0;
        count_seqs_for_profile_m(L, profs[pi].px, 6, pin0, xe, xok);
        count_seqs_for_profile_m(L, profs[pi].py, 6, pin1, ye, yok);
        double pw = (double)xok * (double)yok;
        #pragma omp critical
        {
          sum_x += (double)xok; sum_y += (double)yok;
          pairwork += pw; ex_total += (double)(xe + ye);
          if (pw > max_pw) max_pw = pw;
          done++;
          if ((done % stride) == 0 || done == nprof) {
            double t = chrono::duration<double>(Clock::now() - G_T0).count();
            cout << "[mod6 " << label << " " << done << "/" << nprof
                 << "] X~" << sum_x << " Y~" << sum_y << " pairwork~" << pairwork
                 << " [" << t << "s]\n" << flush;
          }
        }
      }
      cout << "=== MOD6 COUNT " << label << " (L=" << L << ", profiles=" << nprof
           << ") ===\n  DFS-examined=" << ex_total << "  spec-ok X=" << sum_x
           << "  Y=" << sum_y << "\n  pair-work SUM|X|*|Y| = " << pairwork
           << "  (largest profile " << max_pw << ")\n" << flush;
      return pairwork;
    };
    double ab_pw = count6(ab6Profs, G_N1, pinA, pinB, "A,B");
    double cd_pw = count6(cd6Profs, n,   pinC, pinD, "C,D");
    double t = chrono::duration<double>(Clock::now() - G_T0).count();
    cout << "\n=== GATE A SUMMARY (n=" << n << ", mod-6 constrained) ===\n"
         << "mod-6 pair-work: A,B " << ab_pw << "  C,D " << cd_pw
         << "  total " << (ab_pw + cd_pw) << "\n"
         << "Compare vs the same sig's WZ_COUNT_ONLY (mod-3) run for the reduction ratio.\n"
         << "Gate rule (docs/wz_firsthit_plan.md): C,D stream <= ~1e9 at n=36 PASS; >= 1e12 KILL.\n"
         << "Time: " << t << "s\n" << flush;
    return 0;
  }

  // ---- WZ_COUNT_ONLY probe: streaming per-profile sequence counts, O(L)/thread
  //      memory, no materialization anywhere. Usable at n where WZ_MEASURE and
  //      the join OOM (n>=29 measured). Reports the streaming-join decision
  //      numbers: per-side spec-ok counts and pair-work SUM|X|*|Y| = exact number
  //      of joint-hall_ok tests a join must run (also a pre-dedup upper bound on
  //      hash records). Progress lines every 32 profiles so a TIMEOUT still
  //      leaves partial data.
  if (getenv("WZ_COUNT_ONLY")) {
    auto count_stream = [&](vector<Profile> &profs, int L, bool pin0, bool pin1,
                            const char *label) -> double {
      double sum_x = 0, sum_y = 0, pairwork = 0, max_pw = 0, ex_total = 0;
      long long done = 0;
      int nprof = (int)profs.size();
      #pragma omp parallel for schedule(dynamic)
      for (int pi = 0; pi < nprof; pi++) {
        long long xe = 0, xok = 0, ye = 0, yok = 0;
        count_seqs_for_profile(L, profs[pi].px, pin0, xe, xok);
        count_seqs_for_profile(L, profs[pi].py, pin1, ye, yok);
        double pw = (double)xok * (double)yok;
        #pragma omp critical
        {
          sum_x += (double)xok; sum_y += (double)yok;
          pairwork += pw; ex_total += (double)(xe + ye);
          if (pw > max_pw) max_pw = pw;
          done++;
          if ((done % 32) == 0 || done == nprof) {
            double t = chrono::duration<double>(Clock::now() - G_T0).count();
            cout << "[count-only " << label << " " << done << "/" << nprof
                 << "] X~" << sum_x << " Y~" << sum_y << " pairwork~" << pairwork
                 << " [" << t << "s]\n" << flush;
          }
        }
      }
      cout << "=== COUNT-ONLY " << label << " (L=" << L << ", profiles="
           << nprof << ") ===\n"
           << "  DFS-examined=" << ex_total << "  spec-ok X=" << sum_x
           << "  Y=" << sum_y << "\n"
           << "  pair-work SUM|X|*|Y| = " << pairwork
           << "  (largest profile " << max_pw << ")\n" << flush;
      return pairwork;
    };
    double ab_pw = count_stream(abProfs, G_N1, pinA, pinB, "A,B");
    double cd_pw = count_stream(cdProfs, n,   pinC, pinD, "C,D");
    double t = chrono::duration<double>(Clock::now() - G_T0).count();
    cout << "\n=== COUNT-ONLY SUMMARY (n=" << n << ") ===\n";
    cout << "hash side would be " << (ab_pw <= cd_pw ? "A,B" : "C,D")
         << ": records (pre-dedup, pre-joint-hall_ok) <= "
         << (ab_pw <= cd_pw ? ab_pw : cd_pw) << "\n";
    cout << "join pair-tests ~= " << (ab_pw + cd_pw) << "\n";
    cout << "Time: " << t << "s\n" << flush;
    return 0;
  }

  // ---- quick hall_ok-pair COUNT for each side (no record storage) to decide
  //      which (smaller) side to hash. This regenerates sequences but stores
  //      nothing, so it is cheap in MEMORY (the thing we are optimizing).
  auto count_side = [&](vector<Profile> &profs, int L, bool pin0, bool pin1,
                        long long &examined, long long &specok) -> long long {
    long long pairs = 0;
    examined = 0; specok = 0;
    int nprof = (int)profs.size();
    #pragma omp parallel for schedule(dynamic) reduction(+:pairs,examined,specok)
    for (int pi = 0; pi < nprof; pi++) {
      auto &prof = profs[pi];
      vector<vector<int>> Xs, Ys;
      long long xEx = 0, xOk = 0, yEx = 0, yOk = 0;
      gen_seqs_for_profile(L, prof.px, pin0, Xs, xEx, xOk);
      gen_seqs_for_profile(L, prof.py, pin1, Ys, yEx, yOk);
      for (auto &X : Xs)
        for (auto &Y : Ys)
          if (hall_ok(X.data(), L, Y.data(), L)) pairs++;
      examined += xEx + yEx;
      specok += xOk + yOk;
    }
    return pairs;
  };

  long long ab_examined = 0, ab_specok = 0, ab_pairs;
  long long cd_examined = 0, cd_specok = 0, cd_pairs;
  {
    auto t0 = Clock::now();
    ab_pairs = count_side(abProfs, G_N1, pinA, pinB, ab_examined, ab_specok);
    cd_pairs = count_side(cdProfs, n,    pinC, pinD, cd_examined, cd_specok);
    double dt = chrono::duration<double>(Clock::now() - t0).count();
    cout << "[count] A,B hall_ok pairs=" << ab_pairs
         << "  C,D hall_ok pairs=" << cd_pairs
         << "  [" << dt << "s]\n" << flush;
  }

  // CHANGE 3: hash the SMALLER side, stream the larger. Roles are symmetric:
  // the stored side's key uses negate iff it is the CD side; the streamed side's
  // lookup key uses negate iff IT is the CD side. A solution needs AB[s] = -CD[s],
  // so storing raw-AB / lookup -CD  (or  store -CD / lookup raw-AB) both match.
  bool storeAB = (ab_pairs <= cd_pairs);  // hash the smaller; tie -> A,B

  // Stored-side record: P,Q are the two stored sequences (length sL each).
  struct StoreRec { vector<int8_t> P, Q; };
  unordered_map<Key, StoreRec> hashMap;   // ONE rec per distinct 64-bit key (dedup)
  size_t hash_records = 0;

  vector<Profile> *storeProfs = storeAB ? &abProfs : &cdProfs;
  vector<Profile> *streamProfs = storeAB ? &cdProfs : &abProfs;
  int sL  = storeAB ? G_N1 : n;          // length of stored sequences
  int tL  = storeAB ? n    : G_N1;       // length of streamed sequences
  bool storeNeg  = !storeAB;             // CD side keys are negated
  bool streamNeg = storeAB;              // streamed side negates iff it is CD
  bool sPin0 = storeAB ? pinA : pinC, sPin1 = storeAB ? pinB : pinD;
  bool tPin0 = storeAB ? pinC : pinA, tPin1 = storeAB ? pinD : pinB;

  // ======================= BUILD: hash the smaller side (dedup) ===============
  if (!measure) {
    auto t0 = Clock::now();
    // PARALLEL over stored-side profiles. Each thread fills a THREAD-LOCAL map
    // (deduped: at most one rec per key); then a SERIAL merge into the shared
    // map, again deduped. No concurrent writes to the shared map.
    int nthreads = 1;
#ifdef _OPENMP
    nthreads = omp_get_max_threads();
#endif
    vector<unordered_map<Key, StoreRec>> localMaps(nthreads);
    int nprof = (int)storeProfs->size();

    #pragma omp parallel for schedule(dynamic)
    for (int pi = 0; pi < nprof; pi++) {
      int tid = 0;
#ifdef _OPENMP
      tid = omp_get_thread_num();
#endif
      auto &prof = (*storeProfs)[pi];
      auto &lmap = localMaps[tid];
      vector<vector<int>> Ps, Qs;
      long long e0 = 0, o0 = 0, e1 = 0, o1 = 0;
      gen_seqs_for_profile(sL, prof.px, sPin0, Ps, e0, o0);
      gen_seqs_for_profile(sL, prof.py, sPin1, Qs, e1, o1);
      for (auto &P : Ps) {
        for (auto &Q : Qs) {
          if (!hall_ok(P.data(), sL, Q.data(), sL)) continue;
          Key key = autocorr_key(P.data(), Q.data(), sL, storeNeg);
          // DEDUP: keep at most ONE (P,Q) per key. emplace = no-op if present.
          auto ins = lmap.try_emplace(key);
          if (ins.second) {
            ins.first->second.P.assign(P.begin(), P.end());
            ins.first->second.Q.assign(Q.begin(), Q.end());
          }
        }
      }
    }

    // Serial merge of per-thread maps into the shared hash, deduped again.
    for (auto &lmap : localMaps) {
      for (auto &kv : lmap) {
        auto ins = hashMap.try_emplace(kv.first);
        if (ins.second) {
          ins.first->second = std::move(kv.second);
          hash_records++;
        }
      }
    }
    double dt = chrono::duration<double>(Clock::now() - t0).count();
    cout << "[build " << (storeAB ? "A,B" : "C,D") << "] hash keys="
         << hashMap.size() << "  records=" << hash_records
         << " (1/key dedup)  [" << dt << "s]\n" << flush;
  }

  // ======================= STREAM: larger side -> lookup ======================
  long long lookups = 0;
  if (!measure) {
    auto t0 = Clock::now();
    // PARALLEL over streamed-side profiles. The shared hashMap is READ-ONLY here,
    // so concurrent lookups are safe. On a HIT we do the exact NPAF==0 recheck
    // over the FULL A,B,C,D, then record under a critical section. g_found
    // (atomic<bool>) lets other threads stop early. The recheck makes the 64-bit
    // key SOUND: a false collision simply fails it and is skipped.
    int nprof = (int)streamProfs->size();

    #pragma omp parallel for schedule(dynamic) reduction(+:lookups)
    for (int pi = 0; pi < nprof; pi++) {
      if (g_found.load()) continue;  // early-out (can't break in omp for)
      auto &prof = (*streamProfs)[pi];
      vector<vector<int>> Us, Vs;
      long long e0 = 0, o0 = 0, e1 = 0, o1 = 0;
      gen_seqs_for_profile(tL, prof.px, tPin0, Us, e0, o0);
      gen_seqs_for_profile(tL, prof.py, tPin1, Vs, e1, o1);
      for (auto &U : Us) {
        if (g_found.load()) break;
        for (auto &V : Vs) {
          if (!hall_ok(U.data(), tL, V.data(), tL)) continue;
          Key key = autocorr_key(U.data(), V.data(), tL, streamNeg);
          lookups++;
          auto it = hashMap.find(key);
          if (it == hashMap.end()) continue;
          // HIT candidate: map A,B (length n1) and C,D (length n) to the right
          // roles, then EXACT NPAF==0 recheck over all s=1..n before accepting.
          const int8_t *Pp = it->second.P.data();
          const int8_t *Qp = it->second.Q.data();
          int A[256], B[256], C[256], D[256];
          if (storeAB) {            // stored = A,B (len n1); streamed = C,D (len n)
            for (int i = 0; i < G_N1; i++) { A[i] = Pp[i]; B[i] = Qp[i]; }
            for (int i = 0; i < n;   i++) { C[i] = U[i];  D[i] = V[i];  }
          } else {                  // stored = C,D (len n); streamed = A,B (len n1)
            for (int i = 0; i < n;   i++) { C[i] = Pp[i]; D[i] = Qp[i]; }
            for (int i = 0; i < G_N1; i++) { A[i] = U[i];  B[i] = V[i];  }
          }
          bool ok = true;
          for (int s = 1; s <= n; s++)
            if (npaf_at(A, B, G_N1, C, D, n, s) != 0) { ok = false; break; }
          if (!ok) continue;  // 64-bit hash collision, not a real solution
          #pragma omp critical(record_solution)
          {
            if (!g_found.load()) {
              memcpy(g_solA, A, G_N1 * sizeof(int));
              memcpy(g_solB, B, G_N1 * sizeof(int));
              memcpy(g_solC, C, n   * sizeof(int));
              memcpy(g_solD, D, n   * sizeof(int));
              g_found.store(true);
            }
          }
          break;
        }
      }
    }
    double dt = chrono::duration<double>(Clock::now() - t0).count();
    cout << "[stream " << (storeAB ? "C,D" : "A,B") << "] hash lookups="
         << lookups << "  [" << dt << "s]\n" << flush;
  }

  double t = chrono::duration<double>(Clock::now() - G_T0).count();

  // Hash memory estimate (compact key: 8-byte key, 1 rec/key after dedup).
  size_t key_bytes = sizeof(Key);            // 8 bytes (was n*int16)
  size_t rec_bytes = (size_t)2 * sL * sizeof(int8_t);
  double hash_mb = (hashMap.size() * (key_bytes + 48.0) +
                    hash_records * (rec_bytes + 24.0)) / 1e6;

  if (measure) {
    // Project from the side that would actually be hashed (the smaller one);
    // rec_bytes above already uses sL = the stored side's length.
    long long store_pairs = storeAB ? ab_pairs : cd_pairs;
    double proj_mb = (store_pairs * (key_bytes + 48.0) +
                      store_pairs * (rec_bytes + 24.0)) / 1e6;
    cout << "\n=== MEASURE (n=" << n << ") ===\n";
    cout << "filtered A,B (hall_ok pairs) = " << ab_pairs << "\n";
    cout << "filtered C,D (hall_ok pairs) = " << cd_pairs << "\n";
    cout << "A,B spec-ok seqs = " << ab_specok << "   C,D spec-ok seqs = " << cd_specok << "\n";
    cout << "hashed side = " << (storeAB ? "A,B" : "C,D") << " (smaller)\n";
    cout << "projected hash memory (worst case, no dedup) ~ " << proj_mb << " MB = "
         << (proj_mb / 1024.0) << " GB\n";
    cout << "Time (gen+count only): " << t << "s\n" << flush;
    return 0;
  }

  if (g_found.load()) {
    int n1 = G_N1;
    int sa = 0, sb = 0, sc = 0, sd = 0;
    for (int i = 0; i < n1; i++) { sa += g_solA[i]; sb += g_solB[i]; }
    for (int i = 0; i < n;  i++) { sc += g_solC[i]; sd += g_solD[i]; }
    cout << "\n*** BS(" << n1 << "," << n << ") FOUND ***\n";
    cout << "sig = (" << sa << "," << sb << "," << sc << "," << sd << ")\n";
    cout << "A = {"; for (int i=0;i<n1;i++) cout << g_solA[i] << (i<n1-1?",":""); cout << "};\n";
    cout << "B = {"; for (int i=0;i<n1;i++) cout << g_solB[i] << (i<n1-1?",":""); cout << "};\n";
    cout << "C = {"; for (int i=0;i<n;i++)  cout << g_solC[i] << (i<n-1?",":"");  cout << "};\n";
    cout << "D = {"; for (int i=0;i<n;i++)  cout << g_solD[i] << (i<n-1?",":"");  cout << "};\n";
    int maxv = 0;
    for (int s = 1; s <= n; s++) {
      int v = npaf_at(g_solA, g_solB, n1, g_solC, g_solD, n, s);
      if (abs(v) > maxv) maxv = abs(v);
    }
    cout << "VERIFY: max |NPAF[s]| over s=1.." << n << " = " << maxv
         << (maxv == 0 ? "  (NPAF==0 confirmed)\n" : "  (NONZERO!)\n");
    cout << "Hash memory ~ " << hash_mb << " MB\n";
    cout << "Time: " << t << "s\n" << flush;
  } else {
    cout << "\n[" << t << "s] no solution for sig (" << G_SIG_A << "," << G_SIG_B
         << "," << G_SIG_C << "," << G_SIG_D << ")\n";
    cout << "Hash memory ~ " << hash_mb << " MB\n" << flush;
  }

  return g_found.load() ? 0 : 2;
}
