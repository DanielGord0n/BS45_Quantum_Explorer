/*
 * wz_exact.cpp — Exhaustive backtracking solver for Balonin-Seberry base
 * sequences BS(n+1,n). Reimplements the COMPLETE method of Wang & Zhu
 * (arXiv:2506.20296) — the actual algorithm that produced BS(42-44).
 *
 * Replaces the simulated-annealing wz_sa_v8.cpp. SA is an INCOMPLETE method
 * (can run forever past a solution); 8 commits of SA never reproduced BS(43).
 * Exhaustive backtracking is COMPLETE: if a base sequence exists it WILL be
 * found, given enough compute.
 *
 * Method (even n only — targets are n=42 for BS(43,42) and n=44 for BS(45,44)):
 *   - A,B have length n1=n+1; C,D have length n. All entries +/-1.
 *   - Theorem 2.2 (Wang-Zhu): every mirror pair (x_i, x_{n+2-i}) of A,B and of
 *     C,D has only 8 valid sign cases (16 for the unconstrained C,D pair d=0).
 *     => the comb8_pos / comb8_neg / comb16 tables. Enumerating these tables
 *     is a COMPLETE search: Theorem 2.2 is necessary for every base sequence.
 *   - Assign mirror-pair "layers" from the outside inward. After layer d the
 *     joint NPAF at shift s = n-d is fully determined; it must equal 0 or the
 *     whole branch is pruned. This determined-NPAF prune is the search engine.
 *   - Theorem 2.4 spectral filter (hall_ok): once C,D are fully assigned,
 *     f_C(θ)+f_D(θ) must be <= 4n+2 at all θ — prunes before the A,B middle.
 *
 * NOT yet implemented: Theorem 2.3 (mod-m residue-sum decomposition). That is
 * an additional accelerator; add it if n=44 proves too slow. The search is
 * already complete and correct without it.
 *
 * Usage:  ./wz_exact <n> [combo_lo] [combo_hi]
 *   n must be even. combo_lo/hi optionally restrict the first-two-layer combo
 *   range [lo,hi) for splitting one search across SLURM array tasks.
 *
 * Compile: g++ -O3 -march=native -std=c++17 -fopenmp -o wz_exact src/solver/wz_exact.cpp
 */
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

static int G_N, G_N1, G_HALF;
static Clock::time_point G_T0;

// ---- Wang-Zhu Theorem 2.2 comb tables (same as wz_sa_v8) ----
int comb16[16][4];
int comb8_pos[8][4];  // product +1  (pair-sum 0 mod 4)
int comb8_neg[8][4];  // product -1  (pair-sum 2 mod 4)
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

// ---- Theorem 2.4 spectral filter: f_C(θ)+f_D(θ) <= 4n+2 for all θ ----
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
    if (rx * rx + ix * ix + ry * ry + iy * iy > limit + 0.5) return false;
  }
  return true;
}

// Joint aperiodic NPAF at shift s. A,B length n1; C,D length n2.
int npaf_at(const int *A, const int *B, int n1, const int *C, const int *D,
            int n2, int s) {
  int c = 0;
  if (s < n1)
    for (int i = 0; i < n1 - s; i++) c += A[i] * A[i + s] + B[i] * B[i + s];
  if (s < n2)
    for (int i = 0; i < n2 - s; i++) c += C[i] * C[i + s] + D[i] * D[i + s];
  return c;
}

static atomic<bool> g_found{false};
static atomic<long long> g_nodes{0};
static atomic<long long> g_combos_done{0};
static atomic<int> g_last_print{0};  // wall-clock second of last progress line
static int g_solA[256], g_solB[256], g_solC[256], g_solD[256];

// Time-gated progress print — safe to call very often (cheap early-out).
// Prints at most once every 20s regardless of how many threads call it.
static void maybe_progress() {
  double t = chrono::duration<double>(Clock::now() - G_T0).count();
  int now = (int)t, last = g_last_print.load(memory_order_relaxed);
  if (now - last < 20) return;
  if (!g_last_print.compare_exchange_strong(last, now, memory_order_relaxed))
    return;  // another thread is printing this tick
  long long nodes = g_nodes.load(memory_order_relaxed);
  long long cd = g_combos_done.load(memory_order_relaxed);
  long long rate = (t > 0) ? (long long)(nodes / t) : 0;
#pragma omp critical(report)
  cout << "[" << t << "s] nodes=" << nodes << " rate=" << rate
       << "/s combos_done=" << cd << " found="
       << (g_found.load() ? "YES" : "no") << "\n" << flush;
}

static void record_solution(const int *A, const int *B, const int *C,
                             const int *D) {
  int n = G_N, n1 = G_N1;
  // Independent re-verification before claiming anything.
  for (int s = 1; s <= n; s++)
    if (npaf_at(A, B, n1, C, D, n, s) != 0) return;  // not actually valid
  bool expected = false;
  if (!g_found.compare_exchange_strong(expected, true)) return;  // someone won
#pragma omp critical(report)
  {
    memcpy(g_solA, A, n1 * sizeof(int));
    memcpy(g_solB, B, n1 * sizeof(int));
    memcpy(g_solC, C, n * sizeof(int));
    memcpy(g_solD, D, n * sizeof(int));
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
    double t = chrono::duration<double>(Clock::now() - G_T0).count();
    cout << "\nTime: " << t << "s  nodes: " << g_nodes.load() << "\n" << flush;
  }
}

// === Lower-bound NPAF prune (branch-and-bound) ===
// Per shift s in 1..n we maintain:
//   Dnpaf[s] = sum of contributions from terms whose both endpoints are placed
//   Kund[s]  = count of terms (each one a +/-1 product) still undetermined
// The remaining undetermined sum is in [-Kund[s], +Kund[s]], so a final
// NPAF[s] = 0 is feasible only if |Dnpaf[s]| <= Kund[s]. Any shift violating
// this lets us prune the whole subtree. Strictly subsumes the old single-shift
// check (when Kund[s]==0 the bound says |Dnpaf[s]| must be 0).

static inline void init_bounds(int *Dnpaf, int *Kund) {
  int n = G_N, n1 = G_N1;
  for (int s = 0; s <= n; s++) {
    Dnpaf[s] = 0;
    int kab = (s >= 1 && s < n1) ? 2 * (n1 - s) : 0;  // A-term + B-term per i
    int kcd = (s >= 1 && s < n)  ? 2 * (n - s)  : 0;  // C-term + D-term per i
    Kund[s] = kab + kcd;
  }
}

// Update Dnpaf, Kund for the newly-placed position p in sequence X of length L.
// X[p] must already hold its new (+/-1) value; partner positions are inspected
// to see if they are also placed (non-zero) -> the term becomes determined.
static inline void update_bounds_pos(const int *X, int p, int L,
                                      int *Dnpaf, int *Kund) {
  int n = G_N;
  int v = X[p];
  for (int s = 1; s <= n; s++) {
    int j = p + s;
    if (j < L && X[j] != 0) { Dnpaf[s] += v * X[j]; Kund[s]--; }
    int i = p - s;
    if (i >= 0 && X[i] != 0) { Dnpaf[s] += X[i] * v; Kund[s]--; }
  }
}

// True iff no shift currently has |Dnpaf[s]| > Kund[s] (i.e. NPAF=0 still
// reachable for every shift).
static inline bool bounds_pass(const int *Dnpaf, const int *Kund) {
  int n = G_N;
  for (int s = 1; s <= n; s++) {
    int dv = Dnpaf[s]; if (dv < 0) dv = -dv;
    if (dv > Kund[s]) return false;
  }
  return true;
}

// Place mirror-pair "layer" d (AB positions d,n-d; CD positions d,n-1-d) AND
// update Dnpaf/Kund incrementally. Sets positions one at a time so each
// update_bounds_pos sees the partner's current placed/unplaced state correctly,
// avoiding double-counting a term whose two endpoints both belong to this layer.
static inline void place_and_update_layer(int d, int abi, int cdi,
                                           int *A, int *B, int *C, int *D,
                                           int *Dnpaf, int *Kund) {
  int n = G_N, n1 = G_N1;
  const int *ab = (d == 0) ? comb8_neg[abi] : comb8_pos[abi];
  const int *cd = (d == 0) ? comb16[cdi] : comb8_pos[cdi];
  A[d] = ab[0]; update_bounds_pos(A, d, n1, Dnpaf, Kund);
  B[d] = ab[1]; update_bounds_pos(B, d, n1, Dnpaf, Kund);
  if (d != n - d) {
    A[n - d] = ab[2]; update_bounds_pos(A, n - d, n1, Dnpaf, Kund);
    B[n - d] = ab[3]; update_bounds_pos(B, n - d, n1, Dnpaf, Kund);
  }
  C[d] = cd[0]; update_bounds_pos(C, d, n, Dnpaf, Kund);
  D[d] = cd[1]; update_bounds_pos(D, d, n, Dnpaf, Kund);
  if (d != n - 1 - d) {
    C[n - 1 - d] = cd[2]; update_bounds_pos(C, n - 1 - d, n, Dnpaf, Kund);
    D[n - 1 - d] = cd[3]; update_bounds_pos(D, n - 1 - d, n, Dnpaf, Kund);
  }
}

// Zero out the positions written by place_and_update_layer at depth d.
// (Used on backtrack so other iterations see "unplaced" for those slots.)
static inline void zero_layer_positions(int d, int *A, int *B, int *C, int *D) {
  int n = G_N;
  A[d] = B[d] = 0;
  if (d != n - d) { A[n - d] = B[n - d] = 0; }
  C[d] = D[d] = 0;
  if (d != n - 1 - d) { C[n - 1 - d] = D[n - 1 - d] = 0; }
}

// Recursive exhaustive search starting at layer d (layers 0..d-1 already set,
// Dnpaf/Kund reflect that state).
static void search(int d, int *A, int *B, int *C, int *D,
                   int *Dnpaf, int *Kund) {
  if (g_found.load(memory_order_relaxed)) return;
  int n = G_N, n1 = G_N1, half = G_HALF;

  // Snapshot once per call; restore at end of each iteration.
  int Dsave[256], Ksave[256];
  memcpy(Dsave, Dnpaf, (n + 1) * sizeof(int));
  memcpy(Ksave, Kund,  (n + 1) * sizeof(int));

  if (d == half) {
    // C,D complete; only A,B middle position left (assumes even n -> n1 odd).
    for (int mi = 0; mi < 4 && !g_found.load(memory_order_relaxed); mi++) {
      A[half] = comb4[mi][0]; update_bounds_pos(A, half, n1, Dnpaf, Kund);
      B[half] = comb4[mi][1]; update_bounds_pos(B, half, n1, Dnpaf, Kund);
      // After full assignment Kund should be 0 everywhere; solution iff all Dnpaf are 0.
      bool ok = true;
      for (int s = 1; s <= n; s++) if (Dnpaf[s] != 0) { ok = false; break; }
      if (ok) record_solution(A, B, C, D);
      A[half] = B[half] = 0;
      memcpy(Dnpaf, Dsave, (n + 1) * sizeof(int));
      memcpy(Kund,  Ksave, (n + 1) * sizeof(int));
    }
    return;
  }

  int abN = 8;
  int cdN = (d == 0) ? 16 : 8;
  for (int abi = 0; abi < abN && !g_found.load(memory_order_relaxed); abi++) {
    for (int cdi = 0; cdi < cdN; cdi++) {
      place_and_update_layer(d, abi, cdi, A, B, C, D, Dnpaf, Kund);
      long long nc = g_nodes.fetch_add(1, memory_order_relaxed);
      if ((nc & 0x3FFFFFF) == 0) maybe_progress();

      if (bounds_pass(Dnpaf, Kund)) {
        // After the last mirror layer C,D are complete -> Theorem 2.4 spectral prune.
        if (d != half - 1 || hall_ok(C, n, D, n))
          search(d + 1, A, B, C, D, Dnpaf, Kund);
      }

      zero_layer_positions(d, A, B, C, D);
      memcpy(Dnpaf, Dsave, (n + 1) * sizeof(int));
      memcpy(Kund,  Ksave, (n + 1) * sizeof(int));
    }
  }
}

int main(int argc, char **argv) {
  if (argc < 2) {
    cerr << "Usage: " << argv[0] << " <n even> [combo_lo] [combo_hi]" << endl;
    return 1;
  }
  int n = atoi(argv[1]);
  if (n < 4 || n % 2 != 0) {
    cerr << "ERROR: n must be even and >= 4 (got " << n << ")" << endl;
    return 1;
  }
  G_N = n;
  G_N1 = n + 1;
  G_HALF = n / 2;
  init_combs();

  // First THREE layers are flattened into a combo list for parallel splitting.
  // Layer 0: 8 (AB comb8_neg) x 16 (CD comb16) = 128.  Layers 1,2: 8 x 8 = 64.
  // Bit layout (LSB): ab0(3) cd0(4) ab1(3) cd1(3) ab2(3) cd2(3) — 19 bits.
  long long total_combos = 128LL * 64LL * 64LL;  // 524288
  long long lo = (argc >= 3) ? atoll(argv[2]) : 0;
  long long hi = (argc >= 4) ? atoll(argv[3]) : total_combos;
  if (lo < 0) lo = 0;
  if (hi > total_combos) hi = total_combos;

  int thr = 1;
#ifdef _OPENMP
  thr = omp_get_max_threads();
#endif
  cout << "========================================================\n";
  cout << "  wz_exact — exhaustive backtracking for BS(" << G_N1 << "," << n << ")\n";
  cout << "  threads=" << thr << "  combos[" << lo << "," << hi << ")/"
       << total_combos << "\n";
  cout << "========================================================" << endl;
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

    int A[256], B[256], C[256], D[256];
    int Dnpaf[256], Kund[256];
    memset(A, 0, sizeof(A)); memset(B, 0, sizeof(B));
    memset(C, 0, sizeof(C)); memset(D, 0, sizeof(D));
    init_bounds(Dnpaf, Kund);

    // Three layers; the bounds prune fires across ALL shifts (not just the
    // single fully-determined one) so each layer kills many more branches.
    place_and_update_layer(0, ab0, cd0, A, B, C, D, Dnpaf, Kund);
    g_nodes.fetch_add(1, memory_order_relaxed);
    if (bounds_pass(Dnpaf, Kund)) {
      place_and_update_layer(1, ab1, cd1, A, B, C, D, Dnpaf, Kund);
      g_nodes.fetch_add(1, memory_order_relaxed);
      if (bounds_pass(Dnpaf, Kund)) {
        place_and_update_layer(2, ab2, cd2, A, B, C, D, Dnpaf, Kund);
        g_nodes.fetch_add(1, memory_order_relaxed);
        if (bounds_pass(Dnpaf, Kund))
          search(3, A, B, C, D, Dnpaf, Kund);
      }
    }

    long long done = g_combos_done.fetch_add(1, memory_order_relaxed) + 1;
    // Print every 64 combos (tasks have ~13k combos with the 3-layer split,
    // so this is a few hundred lines over a 24h run). Combos completing is
    // the clearest signal the search depth is tractable.
    if (done % 64 == 0 || done == hi - lo) {
#pragma omp critical(report)
      {
        double t = chrono::duration<double>(Clock::now() - G_T0).count();
        cout << "[" << t << "s] COMBO DONE " << done << "/" << (hi - lo)
             << "  nodes=" << g_nodes.load() << "  found="
             << (g_found.load() ? "YES" : "no") << "\n" << flush;
      }
    }
  }

  double t = chrono::duration<double>(Clock::now() - G_T0).count();
  if (!g_found.load()) {
    cout << "\n[" << t << "s] search of combo range [" << lo << "," << hi
         << ") exhausted — no BS(" << G_N1 << "," << n
         << ") in this slice. nodes=" << g_nodes.load() << endl;
  }
  return g_found.load() ? 0 : 2;
}
