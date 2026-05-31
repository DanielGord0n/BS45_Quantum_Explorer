/*
 * wz_exact_t23.cpp — Sig-targeted exhaustive backtracking for BS(n+1,n) with
 * Wang-Zhu Theorem 2.3 m=3 residue-sum pruning. Builds on wz_exact.cpp.
 *
 * The Thm 2.3 prune fires once C,D are fully placed (end of layer half-1):
 * the observed (P_actual, Q_actual) is looked up in a pre-built T23Filter,
 * yielding the small set of (K, R) m=3 triples that are consistent with a
 * valid BS solution for the target signature. The A,B middle (layer half) is
 * then constrained to comb4 options that produce a (K, R) in that compatible
 * set. Branches whose (P, Q) yields no compatible (K, R) are pruned outright.
 *
 * Search restriction: this is a SIGNATURE-TARGETED solver — it searches only
 * branches where final (sumA,sumB,sumC,sumD) = (a,b,c,d) given on the command
 * line. To search all signatures, run multiple instances (one per valid sig
 * from get_sigs(n)).
 *
 * Usage:  ./wz_exact_t23 <n> <a> <b> <c> <d> [combo_lo] [combo_hi]
 *   e.g.  ./wz_exact_t23 42 7 11 0 0     # BS(43,42) Wang-Zhu sig
 *
 * Compile: g++ -O3 -march=native -std=c++17 -fopenmp -o wz_exact_t23 src/solver/wz_exact_t23.cpp
 */
#include <array>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <unordered_map>
#include <vector>
#ifdef _OPENMP
#include <omp.h>
#endif

using namespace std;
using Clock = chrono::steady_clock;

static int G_N, G_N1, G_HALF;
static int G_SIG_A, G_SIG_B, G_SIG_C, G_SIG_D;
static Clock::time_point G_T0;

// Precomputed class counts. G_NA_CLASS[c] = positions in A (length n1) with i%3==c.
// G_PLACED_A_AFTER[d+1][c] = how many A positions in class c are filled after layer d.
// Same for C (length n). Used by sum-constraint + per-class residue prunes.
static int G_NA_CLASS[3], G_NC_CLASS[3];
static int G_PLACED_A_AFTER[64][3];
static int G_PLACED_C_AFTER[64][3];

// Symmetry-breaking pins. Negating a single sequence (A, B, C, or D) leaves
// NPAF[s] unchanged for all s (each term is a self-product within one
// sequence) and only flips that sequence's sum. So for any sequence whose
// TARGET signature component is 0, both ±versions live in the same sig and
// produce the same NPAF — we can pin that sequence's first element to +1 and
// search only one representative. For sig (7,11,0,0) this fires on C and D
// (4x reduction). Set in main() from the sig.
static bool G_PIN_A0, G_PIN_B0, G_PIN_C0, G_PIN_D0;

// ---- Wang-Zhu Theorem 2.2 comb tables (same as wz_exact) ----
int comb16[16][4];
int comb8_pos[8][4];
int comb8_neg[8][4];
int comb4[4][2] = {{1, 1}, {1, -1}, {-1, 1}, {-1, -1}};

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

// Precomputed DFT basis for hall_ok: G_HALL_COS[j][i] = cos(i * j*pi/100).
// j in 1..200, i in 0..G_N-1. Built once in main(); read-only thereafter.
// This removes ~800*n transcendental evals per hall_ok call from the hot path
// (pure speedup — identical math to the original cos/sin-in-loop version).
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

bool hall_ok(const int *X, int xlen, const int *Y, int ylen) {
  double limit = 4.0 * G_N + 2.0;
  for (int j = 1; j <= 200; j++) {
    const double *cj = G_HALL_COS[j];
    const double *sj = G_HALL_SIN[j];
    double rx = 0, ix = 0, ry = 0, iy = 0;
    for (int i = 0; i < xlen; i++) {
      rx += X[i] * cj[i];
      ix += X[i] * sj[i];
    }
    for (int i = 0; i < ylen; i++) {
      ry += Y[i] * cj[i];
      iy += Y[i] * sj[i];
    }
    if (rx * rx + ix * ix + ry * ry + iy * iy > limit + 0.5) return false;
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

// ============================ T23Filter (inlined) ============================

struct M3Triple {
  int x[3];
  bool operator==(const M3Triple &o) const {
    return x[0] == o.x[0] && x[1] == o.x[1] && x[2] == o.x[2];
  }
};
struct KRPair { M3Triple K, R; };

static inline uint64_t encode_pq(const int *P, const int *Q) {
  uint64_t k = 0;
  for (int i = 0; i < 3; i++) k = (k << 6) | uint64_t(P[i] + 32);
  for (int i = 0; i < 3; i++) k = (k << 6) | uint64_t(Q[i] + 32);
  return k;
}

class T23Filter {
public:
  T23Filter(int n, int a, int b, int c, int d) : n_(n) {
    for (int i = 0; i < 3; i++)
      for (int j = 0; j < 64; j++)
        allowed_K_set_[i][j] = allowed_R_set_[i][j] =
            allowed_P_set_[i][j] = allowed_Q_set_[i][j] = false;
    build(a, b, c, d);
  }
  const vector<KRPair> &compatible_KR(const int *P, const int *Q) const {
    auto it = by_PQ_.find(encode_pq(P, Q));
    return (it == by_PQ_.end()) ? empty_ : it->second;
  }
  size_t total_tuples() const { return total_; }
  size_t unique_PQ_keys() const { return by_PQ_.size(); }

  // Per-class reachability: each component of K, R, P, Q (over all valid
  // (K,R,P,Q) tuples) is precomputed as a bitset. class_reachable returns
  // true iff for every class c, there exists some valid K[c] (resp. R, P, Q)
  // reachable from the current partial sum within rem ±1 contributions.
  // This is a sound, looser projection of the full (K,R,P,Q) lookup; it can
  // fire at every layer (not just d==half) so it kills branches with diverging
  // class sums very early.
  bool class_reachable(const int *K, const int *R, const int *P, const int *Q,
                       const int *remA, const int *remC) const {
    for (int c = 0; c < 3; c++) {
      if (!any_reachable(allowed_K_set_[c], K[c], remA[c])) return false;
      if (!any_reachable(allowed_R_set_[c], R[c], remA[c])) return false;
      if (!any_reachable(allowed_P_set_[c], P[c], remC[c])) return false;
      if (!any_reachable(allowed_Q_set_[c], Q[c], remC[c])) return false;
    }
    return true;
  }

private:
  static bool any_reachable(const bool *set, int current, int rem) {
    for (int delta = -rem; delta <= rem; delta += 2) {
      int v = current + delta;
      if (v < -32 || v > 31) continue;
      if (set[v + 32]) return true;
    }
    return false;
  }
  static int class_count(int L, int c) {
    int n = 0;
    for (int p = c; p < L; p += 3) n++;
    return n;
  }
  static vector<M3Triple> enum_class_sums(int L, int target) {
    vector<M3Triple> out;
    int n0 = class_count(L, 0), n1 = class_count(L, 1), n2 = class_count(L, 2);
    for (int x0 = -n0; x0 <= n0; x0 += 2)
      for (int x1 = -n1; x1 <= n1; x1 += 2) {
        int x2 = target - x0 - x1;
        if (x2 < -n2 || x2 > n2) continue;
        if (((x2 - (-n2)) % 2 + 2) % 2 != 0) continue;
        out.push_back({x0, x1, x2});
      }
    return out;
  }
  static int norm(const M3Triple &t) {
    return t.x[0]*t.x[0] + t.x[1]*t.x[1] + t.x[2]*t.x[2];
  }
  void build(int a, int b, int c, int d) {
    int n1 = n_ + 1;
    auto K = enum_class_sums(n1, a);
    auto R = enum_class_sums(n1, b);
    auto P = enum_class_sums(n_, c);
    auto Q = enum_class_sums(n_, d);
    vector<int> K2(K.size()), R2(R.size()), P2(P.size()), Q2(Q.size());
    for (size_t i = 0; i < K.size(); i++) K2[i] = norm(K[i]);
    for (size_t i = 0; i < R.size(); i++) R2[i] = norm(R[i]);
    for (size_t i = 0; i < P.size(); i++) P2[i] = norm(P[i]);
    for (size_t i = 0; i < Q.size(); i++) Q2[i] = norm(Q[i]);
    int tgt = 4 * n_ + 2;
    for (size_t i = 0; i < K.size(); i++)
      for (size_t j = 0; j < R.size(); j++) {
        int kr = K2[i] + R2[j];
        if (kr > tgt) continue;
        for (size_t k = 0; k < P.size(); k++) {
          int krp = kr + P2[k];
          if (krp > tgt) continue;
          int need = tgt - krp;
          for (size_t l = 0; l < Q.size(); l++) {
            if (Q2[l] == need) {
              int pkey[3] = {P[k].x[0], P[k].x[1], P[k].x[2]};
              int qkey[3] = {Q[l].x[0], Q[l].x[1], Q[l].x[2]};
              by_PQ_[encode_pq(pkey, qkey)].push_back({K[i], R[j]});
              total_++;
              for (int ci = 0; ci < 3; ci++) {
                if (K[i].x[ci] >= -32 && K[i].x[ci] <= 31)
                  allowed_K_set_[ci][K[i].x[ci] + 32] = true;
                if (R[j].x[ci] >= -32 && R[j].x[ci] <= 31)
                  allowed_R_set_[ci][R[j].x[ci] + 32] = true;
                if (P[k].x[ci] >= -32 && P[k].x[ci] <= 31)
                  allowed_P_set_[ci][P[k].x[ci] + 32] = true;
                if (Q[l].x[ci] >= -32 && Q[l].x[ci] <= 31)
                  allowed_Q_set_[ci][Q[l].x[ci] + 32] = true;
              }
            }
          }
        }
      }
  }

  int n_;
  size_t total_ = 0;
  unordered_map<uint64_t, vector<KRPair>> by_PQ_;
  vector<KRPair> empty_;
  bool allowed_K_set_[3][64];
  bool allowed_R_set_[3][64];
  bool allowed_P_set_[3][64];
  bool allowed_Q_set_[3][64];
};

static T23Filter *G_FILTER = nullptr;

// ============================ Solver state ============================

static atomic<bool> g_found{false};
static atomic<long long> g_nodes{0};
static atomic<long long> g_combos_done{0};
static atomic<long long> g_t23_prunes{0};
static atomic<long long> g_sum_prunes{0};
static atomic<long long> g_class_prunes{0};
static atomic<long long> g_sym_skips{0};
static atomic<int> g_last_print{0};
static int g_solA[256], g_solB[256], g_solC[256], g_solD[256];

static void maybe_progress() {
  double t = chrono::duration<double>(Clock::now() - G_T0).count();
  int now = (int)t, last = g_last_print.load(memory_order_relaxed);
  if (now - last < 20) return;
  if (!g_last_print.compare_exchange_strong(last, now, memory_order_relaxed))
    return;
  long long nodes = g_nodes.load(memory_order_relaxed);
  long long cd = g_combos_done.load(memory_order_relaxed);
  long long t23 = g_t23_prunes.load(memory_order_relaxed);
  long long sumP = g_sum_prunes.load(memory_order_relaxed);
  long long clsP = g_class_prunes.load(memory_order_relaxed);
  long long rate = (t > 0) ? (long long)(nodes / t) : 0;
#pragma omp critical(report)
  cout << "[" << t << "s] nodes=" << nodes << " rate=" << rate
       << "/s combos_done=" << cd << " t23_prunes=" << t23
       << " sum_prunes=" << sumP << " class_prunes=" << clsP
       << " sym_skips=" << g_sym_skips.load(memory_order_relaxed)
       << " found=" << (g_found.load() ? "YES" : "no") << "\n" << flush;
}

static void record_solution(const int *A, const int *B, const int *C,
                             const int *D) {
  int n = G_N, n1 = G_N1;
  for (int s = 1; s <= n; s++)
    if (npaf_at(A, B, n1, C, D, n, s) != 0) return;
  bool expected = false;
  if (!g_found.compare_exchange_strong(expected, true)) return;
#pragma omp critical(report)
  {
    int sa = 0, sb = 0, sc = 0, sd = 0;
    for (int i = 0; i < n1; i++) { sa += A[i]; sb += B[i]; }
    for (int i = 0; i < n; i++) { sc += C[i]; sd += D[i]; }
    if (n >= 44)
      cout << "\n*** WORLD RECORD DISCOVERY: BS(" << n1 << "," << n
           << ") FOUND ***\n" << endl;
    else
      cout << "\n*** REPRODUCTION CONFIRMED: BS(" << n1 << "," << n
           << ") FOUND ***\n" << endl;
    cout << "sig = (" << sa << "," << sb << "," << sc << "," << sd << ")\n";
    cout << "A = {"; for (int i = 0; i < n1; i++) cout << A[i] << (i<n1-1?",":""); cout << "};\n";
    cout << "B = {"; for (int i = 0; i < n1; i++) cout << B[i] << (i<n1-1?",":""); cout << "};\n";
    cout << "C = {"; for (int i = 0; i < n; i++) cout << C[i] << (i<n-1?",":""); cout << "};\n";
    cout << "D = {"; for (int i = 0; i < n; i++) cout << D[i] << (i<n-1?",":""); cout << "};\n";
    cout << "\nTime: " << chrono::duration<double>(Clock::now()-G_T0).count() << "s\n" << flush;
  }
}

// ============================ Placement + bounds tracking ============================

// Update Dnpaf/Kund for one placed position (subset of joint NPAF). Same as wz_exact.
// Must be called AFTER X[p] is set but BEFORE the partner positions in the same
// layer are set — otherwise terms (p, partner) get double-counted via both
// the forward (j=p+s) and backward (i=p-s) branches.
static void update_bounds_pos(const int *X, int p, int L,
                              int *Dnpaf, int *Kund, int n) {
  int v = X[p];
  for (int s = 1; s <= n; s++) {
    int j = p + s;
    if (j < L && X[j] != 0) { Dnpaf[s] += v * X[j]; Kund[s]--; }
    int i = p - s;
    if (i >= 0 && X[i] != 0) { Dnpaf[s] += X[i] * v; Kund[s]--; }
  }
}

// Set + update one position at a time (interleaved, same pattern as wz_exact.cpp).
// This avoids the bidirectional-update double-counting that would happen if all
// layer positions were set first and updated after.
static inline void place_and_update_layer(int d, int abi, int cdi,
                                           int *A, int *B, int *C, int *D,
                                           int *Dnpaf, int *Kund,
                                           int *Kpar, int *Rpar,
                                           int *Ppar, int *Qpar) {
  int n = G_N, n1 = G_N1;
  const int *ab = (d == 0) ? comb8_neg[abi] : comb8_pos[abi];
  const int *cd = (d == 0) ? comb16[cdi] : comb8_pos[cdi];

  A[d] = ab[0]; update_bounds_pos(A, d, n1, Dnpaf, Kund, n);
  Kpar[d % 3] += ab[0];
  B[d] = ab[1]; update_bounds_pos(B, d, n1, Dnpaf, Kund, n);
  Rpar[d % 3] += ab[1];
  if (d != n - d) {
    A[n - d] = ab[2]; update_bounds_pos(A, n - d, n1, Dnpaf, Kund, n);
    Kpar[(n - d) % 3] += ab[2];
    B[n - d] = ab[3]; update_bounds_pos(B, n - d, n1, Dnpaf, Kund, n);
    Rpar[(n - d) % 3] += ab[3];
  }
  C[d] = cd[0]; update_bounds_pos(C, d, n, Dnpaf, Kund, n);
  Ppar[d % 3] += cd[0];
  D[d] = cd[1]; update_bounds_pos(D, d, n, Dnpaf, Kund, n);
  Qpar[d % 3] += cd[1];
  if (d != n - 1 - d) {
    C[n - 1 - d] = cd[2]; update_bounds_pos(C, n - 1 - d, n, Dnpaf, Kund, n);
    Ppar[(n - 1 - d) % 3] += cd[2];
    D[n - 1 - d] = cd[3]; update_bounds_pos(D, n - 1 - d, n, Dnpaf, Kund, n);
    Qpar[(n - 1 - d) % 3] += cd[3];
  }
}

// ============================ Search ============================

// Layer d places positions: A,B at d and n-d; C,D at d and n-1-d.
// After layer half-1, C and D are fully placed; A,B missing only the middle
// at position half. We invoke the T23 filter at that point.
static void search(int d, int *A, int *B, int *C, int *D, int *Dnpaf,
                   int *Kund, int *Kpar, int *Rpar, int *Ppar, int *Qpar) {
  if (g_found.load(memory_order_relaxed)) return;
  int n = G_N, n1 = G_N1, half = G_HALF;

  if (d == half) {
    // CD fully placed. Look up compatible (K,R) for current (P,Q).
    const vector<KRPair> &allowed = G_FILTER->compatible_KR(Ppar, Qpar);
    if (allowed.empty()) {
      g_t23_prunes.fetch_add(1, memory_order_relaxed);
      return;
    }
    // Try each comb4 middle option; commit only those that land in 'allowed'.
    int mid_class = half % 3;
    int Kpar_save = Kpar[mid_class], Rpar_save = Rpar[mid_class];
    for (int mi = 0; mi < 4 && !g_found.load(memory_order_relaxed); mi++) {
      A[half] = comb4[mi][0];
      B[half] = comb4[mi][1];
      int K0 = Kpar_save + comb4[mi][0];
      int R0 = Rpar_save + comb4[mi][1];
      // Build candidate K, R triples (only differs in class mid_class).
      M3Triple Kfinal = {{Kpar[0], Kpar[1], Kpar[2]}};
      M3Triple Rfinal = {{Rpar[0], Rpar[1], Rpar[2]}};
      Kfinal.x[mid_class] = K0; Rfinal.x[mid_class] = R0;
      bool match = false;
      for (auto &kr : allowed) {
        if (kr.K == Kfinal && kr.R == Rfinal) { match = true; break; }
      }
      if (!match) continue;
      // Final NPAF check
      bool ok = true;
      for (int s = 1; s <= n && ok; s++)
        if (npaf_at(A, B, n1, C, D, n, s) != 0) ok = false;
      if (ok) record_solution(A, B, C, D);
    }
    A[half] = B[half] = 0;
    return;
  }

  int Dsnap[256], Ksnap[256];
  memcpy(Dsnap, Dnpaf, sizeof(Dsnap));
  memcpy(Ksnap, Kund, sizeof(Ksnap));
  int Kpsnap[3], Rpsnap[3], Ppsnap[3], Qpsnap[3];
  memcpy(Kpsnap, Kpar, sizeof(Kpsnap));
  memcpy(Rpsnap, Rpar, sizeof(Rpsnap));
  memcpy(Ppsnap, Ppar, sizeof(Ppsnap));
  memcpy(Qpsnap, Qpar, sizeof(Qpsnap));

  int abN = 8, cdN = (d == 0) ? 16 : 8;
  for (int abi = 0; abi < abN && !g_found.load(memory_order_relaxed); abi++) {
    for (int cdi = 0; cdi < cdN; cdi++) {
      place_and_update_layer(d, abi, cdi, A, B, C, D,
                              Dnpaf, Kund, Kpar, Rpar, Ppar, Qpar);
      long long nc = g_nodes.fetch_add(1, memory_order_relaxed);
      if ((nc & 0x3FFFFFF) == 0) maybe_progress();

      // NPAF bounds prune
      bool prune = false;
      for (int s = 1; s <= n; s++) {
        if (abs(Dnpaf[s]) > Kund[s]) { prune = true; break; }
      }
      if (!prune) {
        // Sum-constraint prune: must be able to reach (G_SIG_A,B,C,D) given
        // partial sums and remaining ±1 contributions.
        int sumA = Kpar[0] + Kpar[1] + Kpar[2];
        int sumB = Rpar[0] + Rpar[1] + Rpar[2];
        int sumC = Ppar[0] + Ppar[1] + Ppar[2];
        int sumD = Qpar[0] + Qpar[1] + Qpar[2];
        int remA_total = n1 - 2 * (d + 1);
        int remC_total = n - 2 * (d + 1);
        bool sum_ok = (abs(G_SIG_A - sumA) <= remA_total)
                   && (abs(G_SIG_B - sumB) <= remA_total)
                   && (abs(G_SIG_C - sumC) <= remC_total)
                   && (abs(G_SIG_D - sumD) <= remC_total);
        if (!sum_ok) {
          g_sum_prunes.fetch_add(1, memory_order_relaxed);
        } else {
          // Per-class residue prune via T23 filter projections.
          int remA_c[3] = {G_NA_CLASS[0] - G_PLACED_A_AFTER[d + 1][0],
                           G_NA_CLASS[1] - G_PLACED_A_AFTER[d + 1][1],
                           G_NA_CLASS[2] - G_PLACED_A_AFTER[d + 1][2]};
          int remC_c[3] = {G_NC_CLASS[0] - G_PLACED_C_AFTER[d + 1][0],
                           G_NC_CLASS[1] - G_PLACED_C_AFTER[d + 1][1],
                           G_NC_CLASS[2] - G_PLACED_C_AFTER[d + 1][2]};
          if (!G_FILTER->class_reachable(Kpar, Rpar, Ppar, Qpar,
                                          remA_c, remC_c)) {
            g_class_prunes.fetch_add(1, memory_order_relaxed);
          } else {
            if (d == half - 1) {
              // C, D fully placed. Apply Thm 2.4 spectral filter.
              if (hall_ok(C, n, D, n)) {
                search(d + 1, A, B, C, D, Dnpaf, Kund, Kpar, Rpar, Ppar, Qpar);
              }
            } else {
              search(d + 1, A, B, C, D, Dnpaf, Kund, Kpar, Rpar, Ppar, Qpar);
            }
          }
        }
      }

      // Restore everything.
      A[d] = B[d] = 0;
      if (d != n - d) { A[n - d] = B[n - d] = 0; }
      C[d] = D[d] = 0;
      if (d != n - 1 - d) { C[n - 1 - d] = D[n - 1 - d] = 0; }
      memcpy(Dnpaf, Dsnap, sizeof(Dsnap));
      memcpy(Kund, Ksnap, sizeof(Ksnap));
      memcpy(Kpar, Kpsnap, sizeof(Kpsnap));
      memcpy(Rpar, Rpsnap, sizeof(Rpsnap));
      memcpy(Ppar, Ppsnap, sizeof(Ppsnap));
      memcpy(Qpar, Qpsnap, sizeof(Qpsnap));
    }
  }
}

int main(int argc, char **argv) {
  if (argc < 6) {
    cerr << "Usage: " << argv[0] << " <n> <a> <b> <c> <d> [combo_lo] [combo_hi]\n"
         << "  e.g.  " << argv[0] << " 42 7 11 0 0   (BS(43,42) Wang-Zhu sig)\n";
    return 1;
  }
  int n = atoi(argv[1]);
  if (n < 4 || n % 2 != 0) {
    cerr << "ERROR: n must be even and >= 4\n";
    return 1;
  }
  G_N = n;
  G_N1 = n + 1;
  G_HALF = n / 2;
  G_SIG_A = atoi(argv[2]);
  G_SIG_B = atoi(argv[3]);
  G_SIG_C = atoi(argv[4]);
  G_SIG_D = atoi(argv[5]);
  // Sanity: sig must satisfy a²+b²+c²+d² = 4n+2.
  int sigsum = G_SIG_A*G_SIG_A + G_SIG_B*G_SIG_B + G_SIG_C*G_SIG_C + G_SIG_D*G_SIG_D;
  if (sigsum != 4 * n + 2) {
    cerr << "ERROR: sig norm = " << sigsum << ", expected " << (4 * n + 2) << "\n";
    return 1;
  }
  init_combs();
  init_hall_tables();

  // Single-sequence-negation symmetry pins: only sound when the sequence's
  // target sum is 0 (so the negated copy keeps the same signature).
  G_PIN_A0 = (G_SIG_A == 0);
  G_PIN_B0 = (G_SIG_B == 0);
  G_PIN_C0 = (G_SIG_C == 0);
  G_PIN_D0 = (G_SIG_D == 0);

  // Precompute class counts and cumulative placement-per-layer arrays.
  for (int c = 0; c < 3; c++) { G_NA_CLASS[c] = 0; G_NC_CLASS[c] = 0; }
  for (int i = 0; i < G_N1; i++) G_NA_CLASS[i % 3]++;
  for (int i = 0; i < G_N;  i++) G_NC_CLASS[i % 3]++;
  for (int c = 0; c < 3; c++) {
    G_PLACED_A_AFTER[0][c] = 0;
    G_PLACED_C_AFTER[0][c] = 0;
  }
  for (int d = 0; d < G_HALF; d++) {
    for (int c = 0; c < 3; c++) {
      G_PLACED_A_AFTER[d + 1][c] = G_PLACED_A_AFTER[d][c];
      G_PLACED_C_AFTER[d + 1][c] = G_PLACED_C_AFTER[d][c];
    }
    G_PLACED_A_AFTER[d + 1][d % 3]++;
    if (d != G_N - d) G_PLACED_A_AFTER[d + 1][(G_N - d) % 3]++;
    G_PLACED_C_AFTER[d + 1][d % 3]++;
    if (d != G_N - 1 - d) G_PLACED_C_AFTER[d + 1][(G_N - 1 - d) % 3]++;
  }

  long long total_combos = 128LL * 64LL * 64LL;
  long long lo = (argc >= 7) ? atoll(argv[6]) : 0;
  long long hi = (argc >= 8) ? atoll(argv[7]) : total_combos;
  if (lo < 0) lo = 0;
  if (hi > total_combos) hi = total_combos;

  int thr = 1;
#ifdef _OPENMP
  thr = omp_get_max_threads();
#endif

  cout << "========================================================\n";
  cout << "  wz_exact_t23 — sig-targeted BS(" << G_N1 << "," << n
       << "), sig=(" << G_SIG_A << "," << G_SIG_B << "," << G_SIG_C << "," << G_SIG_D << ")\n";
  cout << "  threads=" << thr << "  combos[" << lo << "," << hi << ")/"
       << total_combos << "\n";
  cout << "  sym_pins: A0=" << G_PIN_A0 << " B0=" << G_PIN_B0
       << " C0=" << G_PIN_C0 << " D0=" << G_PIN_D0;
  {
    int pins = (int)G_PIN_A0 + G_PIN_B0 + G_PIN_C0 + G_PIN_D0;
    cout << "  (=> " << (1 << pins) << "x reduction)\n";
  }
  cout << "========================================================" << endl;

  // Build T23Filter for this sig.
  cout << "Building T23Filter..." << flush;
  G_FILTER = new T23Filter(n, G_SIG_A, G_SIG_B, G_SIG_C, G_SIG_D);
  cout << " " << G_FILTER->total_tuples() << " valid 4-tuples, "
       << G_FILTER->unique_PQ_keys() << " (P,Q) keys\n" << flush;

  G_T0 = Clock::now();

#pragma omp parallel for schedule(dynamic, 64)
  for (long long combo = lo; combo < hi; combo++) {
    if (g_found.load(memory_order_relaxed)) continue;
    int ab0 = (int)(combo & 7);
    int cd0 = (int)((combo >> 3) & 15);
    int ab1 = (int)((combo >> 7) & 7);
    int cd1 = (int)((combo >> 10) & 7);
    int ab2 = (int)((combo >> 13) & 7);
    int cd2 = (int)((combo >> 16) & 7);

    // Symmetry-breaking pins on layer-0 first elements (A[0],B[0] in comb8_neg;
    // C[0],D[0] in comb16). Sound only for sequences whose target sum is 0.
    // Skips ~3/4 of combos for sig (7,11,0,0) before any allocation.
    if ((G_PIN_A0 && comb8_neg[ab0][0] != 1) ||
        (G_PIN_B0 && comb8_neg[ab0][1] != 1) ||
        (G_PIN_C0 && comb16[cd0][0] != 1) ||
        (G_PIN_D0 && comb16[cd0][1] != 1)) {
      g_sym_skips.fetch_add(1, memory_order_relaxed);
      g_combos_done.fetch_add(1, memory_order_relaxed);
      continue;
    }

    int A[256], B[256], C[256], D[256];
    int Dnpaf[256], Kund[256];
    int Kpar[3] = {0, 0, 0}, Rpar[3] = {0, 0, 0};
    int Ppar[3] = {0, 0, 0}, Qpar[3] = {0, 0, 0};
    memset(A, 0, sizeof(A)); memset(B, 0, sizeof(B));
    memset(C, 0, sizeof(C)); memset(D, 0, sizeof(D));
    memset(Dnpaf, 0, sizeof(Dnpaf));
    for (int s = 0; s <= G_N; s++) {
      if (s == 0) Kund[s] = 0;
      else if (s < G_N) Kund[s] = 2 * (G_N1 - s) + 2 * (G_N - s);
      else if (s == G_N) Kund[s] = 2 * (G_N1 - s);
      else Kund[s] = 0;
    }

    // Inline place + update for layers 0, 1, 2 with prune checks.
    auto place_and_check = [&](int d, int abi, int cdi) -> bool {
      place_and_update_layer(d, abi, cdi, A, B, C, D,
                              Dnpaf, Kund, Kpar, Rpar, Ppar, Qpar);
      g_nodes.fetch_add(1, memory_order_relaxed);
      for (int s = 1; s <= G_N; s++)
        if (abs(Dnpaf[s]) > Kund[s]) return false;
      // Sum-constraint prune.
      int sumA = Kpar[0] + Kpar[1] + Kpar[2];
      int sumB = Rpar[0] + Rpar[1] + Rpar[2];
      int sumC = Ppar[0] + Ppar[1] + Ppar[2];
      int sumD = Qpar[0] + Qpar[1] + Qpar[2];
      int remA_total = G_N1 - 2 * (d + 1);
      int remC_total = G_N - 2 * (d + 1);
      if (abs(G_SIG_A - sumA) > remA_total ||
          abs(G_SIG_B - sumB) > remA_total ||
          abs(G_SIG_C - sumC) > remC_total ||
          abs(G_SIG_D - sumD) > remC_total) {
        g_sum_prunes.fetch_add(1, memory_order_relaxed);
        return false;
      }
      // Per-class residue prune.
      int remA_c[3] = {G_NA_CLASS[0] - G_PLACED_A_AFTER[d + 1][0],
                       G_NA_CLASS[1] - G_PLACED_A_AFTER[d + 1][1],
                       G_NA_CLASS[2] - G_PLACED_A_AFTER[d + 1][2]};
      int remC_c[3] = {G_NC_CLASS[0] - G_PLACED_C_AFTER[d + 1][0],
                       G_NC_CLASS[1] - G_PLACED_C_AFTER[d + 1][1],
                       G_NC_CLASS[2] - G_PLACED_C_AFTER[d + 1][2]};
      if (!G_FILTER->class_reachable(Kpar, Rpar, Ppar, Qpar, remA_c, remC_c)) {
        g_class_prunes.fetch_add(1, memory_order_relaxed);
        return false;
      }
      return true;
    };

    bool ok0 = place_and_check(0, ab0, cd0);
    if (ok0) {
      bool ok1 = place_and_check(1, ab1, cd1);
      if (ok1) {
        bool ok2 = place_and_check(2, ab2, cd2);
        if (ok2) {
          search(3, A, B, C, D, Dnpaf, Kund, Kpar, Rpar, Ppar, Qpar);
        }
      }
    }

    long long done = g_combos_done.fetch_add(1, memory_order_relaxed) + 1;
    if (done % 64 == 0 || done == hi - lo) {
#pragma omp critical(report)
      {
        double t = chrono::duration<double>(Clock::now() - G_T0).count();
        cout << "[" << t << "s] COMBO DONE " << done << "/" << (hi - lo)
             << "  nodes=" << g_nodes.load() << "  t23_prunes=" << g_t23_prunes.load()
             << "  sum_prunes=" << g_sum_prunes.load()
             << "  class_prunes=" << g_class_prunes.load()
             << "  found=" << (g_found.load() ? "YES" : "no") << "\n" << flush;
      }
    }
  }

  double t = chrono::duration<double>(Clock::now() - G_T0).count();
  if (!g_found.load()) {
    cout << "\n[" << t << "s] exhausted combo range [" << lo << "," << hi
         << ") for sig (" << G_SIG_A << "," << G_SIG_B << "," << G_SIG_C
         << "," << G_SIG_D << "). nodes=" << g_nodes.load()
         << " t23_prunes=" << g_t23_prunes.load()
         << " sum_prunes=" << g_sum_prunes.load()
         << " class_prunes=" << g_class_prunes.load()
         << " sym_skips=" << g_sym_skips.load() << "\n";
  }
  delete G_FILTER;
  return g_found.load() ? 0 : 2;
}
