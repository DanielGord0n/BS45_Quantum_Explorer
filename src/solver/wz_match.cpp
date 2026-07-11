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
                                        const PairNormSet &comp6) {
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
          if (comp6.feasible(need6)) { ok6 = true; break; }
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
static void count_pairs22(int L, const vector<int> &tx, const vector<int> &ty,
                          bool abSide, bool pinX, bool pinY,
                          long long &leaves, long long &ok,
                          const function<void(const vector<int>&, const vector<int>&)> *sink = nullptr) {
  int total_in_class[3];
  for (int c = 0; c < 3; c++) total_in_class[c] = class_count(L, c, 3);
  int half = L / 2;
  vector<int> X(L, 0), Y(L, 0);
  int px[3] = {0,0,0}, py[3] = {0,0,0}, placed[3] = {0,0,0};
  function<void(int)> rec = [&](int d) {
    for (int c = 0; c < 3; c++) {
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
        for (int cc = 0; cc < 3; cc++)
          if (px[cc] != tx[cc] || py[cc] != ty[cc]) return false;
        return true;
      };
      if (L % 2 == 1) {
        int mid = half, c = mid % 3;
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
    int c1 = i1 % 3, c2 = i2 % 3;
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
                                         size_t cap = 20000000) {
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
      // A real solution satisfies the norm identity at EVERY modulus: the
      // pair's mod-3 reduction must ALSO complete (without this, mod-6
      // "survivors" include pairs mod-3 already kills — measured at n=11:
      // mod-6 pairwork exceeded mod-3, impossible for a true refinement).
      int need3 = tgt - npx3 - norm_vec(reduce63(py));
      if (need3 < 0 || !comp3.feasible(need3)) continue;
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
    cout << (allok ? "PROFILE_CHECK: ALL PASS — residue lift keeps this solution\n"
                   : "PROFILE_CHECK: FAIL — residue lift would EXCLUDE this solution\n") << flush;
    return allok ? 0 : 1;
  }

  // --- surviving profiles for each side (complement = the OTHER side) ---
  auto abProfs = survive_profiles(G_N1, G_SIG_A, G_SIG_B, cd3, cd6);
  auto cdProfs = survive_profiles(n,    G_SIG_C, G_SIG_D, ab3, ab6);
  cout << "[profiles] A,B side surviving: " << abProfs.size()
       << "   C,D side surviving: " << cdProfs.size() << "\n" << flush;

  bool pinA = (G_SIG_A == 0), pinB = (G_SIG_B == 0);
  bool pinC = (G_SIG_C == 0), pinD = (G_SIG_D == 0);

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
              }
            }
            return;
          }
        };
        long long lv = 0, okc = 0;
        count_pairs22(n, cdProfs[pi].px, cdProfs[pi].py, false, pinC, pinD,
                      lv, okc, &res);
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
        count_pairs22(L, profs[pi].px, profs[pi].py, abSide, pin0, pin1, lv, okc);
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
    if (!sideSel || strcmp(sideSel, "CD") != 0)
      abS = countP(abProfs, G_N1, true,  pinA, pinB, "A,B");
    if (!sideSel || strcmp(sideSel, "AB") != 0)
      cdS = countP(cdProfs, n,   false, pinC, pinD, "C,D");
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
    auto ab6Profs = survive_profiles6(G_N1, G_SIG_A, G_SIG_B, cd3, cd6);
    auto cd6Profs = survive_profiles6(n,    G_SIG_C, G_SIG_D, ab3, ab6);
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
