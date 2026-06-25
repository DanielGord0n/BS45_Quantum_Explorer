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
  if (n < 4 || n % 2 != 0) { cerr << "ERROR: n even and >=4\n"; return 1; }
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

  // --- surviving profiles for each side (complement = the OTHER side) ---
  auto abProfs = survive_profiles(G_N1, G_SIG_A, G_SIG_B, cd3, cd6);
  auto cdProfs = survive_profiles(n,    G_SIG_C, G_SIG_D, ab3, ab6);
  cout << "[profiles] A,B side surviving: " << abProfs.size()
       << "   C,D side surviving: " << cdProfs.size() << "\n" << flush;

  bool pinA = (G_SIG_A == 0), pinB = (G_SIG_B == 0);
  bool pinC = (G_SIG_C == 0), pinD = (G_SIG_D == 0);

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
    double proj_mb = (ab_pairs * (key_bytes + 48.0) +
                      ab_pairs * (rec_bytes + 24.0)) / 1e6;
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
