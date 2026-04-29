/*
 * Wang-Zhu BS Solver v4 — Joint SA over Wang-Zhu Pair Manifold
 * CP493 - Directed Research - Daniel Gordon
 *
 * Combines:
 *   - v2's Wang-Zhu pair-encoded manifold (comb16 / comb8_pos / comb8_neg / comb4)
 *   - v3's single-loop joint SA over (A,B,C,D) — no 2-stage CD-then-AB rejection.
 *
 * State = (A[n+1], B[n+1], C[n], D[n]) all encoded as Wang-Zhu mirror-pairs.
 * Move  = resample one pair-slot (4 ±1 values) for either (A,B) or (C,D).
 * Cost  = 5*(|sum_a-sig.a|+|sum_b-sig.b|+|sum_c-sig.c|+|sum_d-sig.d|)
 *         + sum_{s=1..ms-1} |NPAF[s]|  where NPAF is the *full* joint correlation.
 *
 * Usage:  ./wz_sa_v4 <n> [seed_offset]
 *
 * macOS:  g++ -O3 -std=c++17 -Xpreprocessor -fopenmp \
 *           -I/opt/homebrew/opt/libomp/include -L/opt/homebrew/opt/libomp/lib -lomp \
 *           -o wz_sa_v4 src/solver/wz_sa_v4.cpp
 * Cluster: g++ -O3 -march=native -std=c++17 -fopenmp -o wz_sa_v4 src/solver/wz_sa_v4.cpp
 */

#include <algorithm>
#include <atomic>
#include <chrono>
#include <climits>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <random>
#include <tuple>
#include <vector>

#ifdef _OPENMP
#include <omp.h>
#endif

using namespace std;
using Clock = chrono::steady_clock;

// =====================================================================
//  Globals
// =====================================================================
static atomic<bool>      g_found{false};
static atomic<int>       g_champion_cost{INT_MAX};
static atomic<long long> g_iters_total{0};
static Clock::time_point G_T0;

// Wang-Zhu Theorem 2.4 structural tables.
static int comb16[16][4];
static int comb8_pos[8][4];
static int comb8_neg[8][4];
static const int comb4[4][2] = {{1, 1}, {1, -1}, {-1, 1}, {-1, -1}};

static void init_combs() {
  int p = 0, n_idx = 0;
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
      for (int j = 0; j < 4; j++) comb8_neg[n_idx][j] = comb16[i][j];
      n_idx++;
    }
  }
}

// =====================================================================
//  Signature enumeration (identical to v2/bs43)
// =====================================================================
struct Sig { int a, b, c, d; };

static vector<Sig> get_sigs(int n) {
  vector<Sig> sigs;
  int T = 4 * n + 2, n1 = n + 1, ap = n1 % 2, cp = n % 2;
  for (int a = 0; a <= n1; a++) {
    if (a % 2 != ap || a * a > T) continue;
    for (int b = -n1; b <= n1; b++) {
      if (((b % 2) + 2) % 2 != (unsigned)ap) continue;
      int r = T - a * a - b * b;
      if (r < 0) continue;
      for (int c = -n; c <= n; c++) {
        if (((c % 2) + 2) % 2 != (unsigned)cp) continue;
        int d2 = r - c * c;
        if (d2 < 0) continue;
        int d = (int)round(sqrt((double)d2));
        if (d * d != d2 || d > n || (((d % 2) + 2) % 2 != (unsigned)cp)) continue;
        if (n % 2 == 0) {
          if (((c - d) % 4 + 8) % 4 != 0) continue;
        } else {
          if (((a - b - 2) % 4 + 8) % 4 != 0) continue;
        }
        sigs.push_back({a, b, c, d});
        if (d > 0 && ((-d % 2 + 2) % 2 == cp)) {
          bool ok = true;
          if (n % 2 == 0 && ((c + d) % 4 + 8) % 4 != 0) ok = false;
          if (ok) sigs.push_back({a, b, c, -d});
        }
      }
    }
  }
  sort(sigs.begin(), sigs.end(), [](auto &x, auto &y) {
    return tie(x.a, x.b, x.c, x.d) < tie(y.a, y.b, y.c, y.d);
  });
  sigs.erase(unique(sigs.begin(), sigs.end(),
                    [](auto &x, auto &y) {
                      return x.a == y.a && x.b == y.b && x.c == y.c && x.d == y.d;
                    }),
             sigs.end());
  return sigs;
}

// =====================================================================
//  Joint State
// =====================================================================
struct State {
  int A[128], B[128], C[128], D[128];
  int sum_a, sum_b, sum_c, sum_d;
  int corr[128];   // joint NPAF[s] = sum over A,B,C,D autocorrelations at shift s

  int cost(const Sig &sig, int ms) const {
    int sd = abs(sum_a - sig.a) + abs(sum_b - sig.b)
           + abs(sum_c - sig.c) + abs(sum_d - sig.d);
    int pen = 0;
    for (int s = 1; s < ms; s++) pen += abs(corr[s]);
    return sd * 5 + pen;
  }
};

static void state_recompute(State &st, int n1, int n, int ms) {
  st.sum_a = 0; for (int i = 0; i < n1; i++) st.sum_a += st.A[i];
  st.sum_b = 0; for (int i = 0; i < n1; i++) st.sum_b += st.B[i];
  st.sum_c = 0; for (int i = 0; i < n;  i++) st.sum_c += st.C[i];
  st.sum_d = 0; for (int i = 0; i < n;  i++) st.sum_d += st.D[i];
  memset(st.corr, 0, sizeof(st.corr));
  for (int s = 1; s < ms; s++) {
    for (int i = 0; i < n1 - s; i++) st.corr[s] += st.A[i]*st.A[i+s] + st.B[i]*st.B[i+s];
    for (int i = 0; i < n  - s; i++) st.corr[s] += st.C[i]*st.C[i+s] + st.D[i]*st.D[i+s];
  }
}

// Sample a fresh AB pair-slot value (returns 4-tuple via out[]).
static inline void sample_AB(int d, mt19937 &rng, int out[4]) {
  if (d == 0) {
    const int *c = comb8_neg[uniform_int_distribution<>(0, 7)(rng)];
    out[0]=c[0]; out[1]=c[1]; out[2]=c[2]; out[3]=c[3];
  } else {
    const int *c = comb8_pos[uniform_int_distribution<>(0, 7)(rng)];
    out[0]=c[0]; out[1]=c[1]; out[2]=c[2]; out[3]=c[3];
  }
}
static inline void sample_CD(int d, mt19937 &rng, int out[4]) {
  if (d == 0) {
    const int *c = comb16[uniform_int_distribution<>(0, 15)(rng)];
    out[0]=c[0]; out[1]=c[1]; out[2]=c[2]; out[3]=c[3];
  } else {
    const int *c = comb8_pos[uniform_int_distribution<>(0, 7)(rng)];
    out[0]=c[0]; out[1]=c[1]; out[2]=c[2]; out[3]=c[3];
  }
}
static inline void sample_mid(mt19937 &rng, int out[2]) {
  const int *c = comb4[uniform_int_distribution<>(0, 3)(rng)];
  out[0]=c[0]; out[1]=c[1];
}

static void state_random_init(State &st, int n1, int n, int ms, mt19937 &rng) {
  int t[4];
  for (int d = 0; d < n1 / 2; d++) {
    int L = d, R = n1 - 1 - d;
    sample_AB(d, rng, t);
    st.A[L]=t[0]; st.B[L]=t[1]; st.A[R]=t[2]; st.B[R]=t[3];
  }
  if (n1 % 2) { int m = n1/2; int u[2]; sample_mid(rng, u); st.A[m]=u[0]; st.B[m]=u[1]; }

  for (int d = 0; d < n / 2; d++) {
    int L = d, R = n - 1 - d;
    sample_CD(d, rng, t);
    st.C[L]=t[0]; st.D[L]=t[1]; st.C[R]=t[2]; st.D[R]=t[3];
  }
  if (n % 2) { int m = n/2; int u[2]; sample_mid(rng, u); st.C[m]=u[0]; st.D[m]=u[1]; }

  state_recompute(st, n1, n, ms);
}

// =====================================================================
//  Incremental moves (return delta_corr[] for revert)
// =====================================================================
// AB pair-slot move at d (covers L=d, R=n1-1-d).  If L==R it's a middle move.
static int try_AB_move(State &st, int n1, int n, int ms, int d, mt19937 &rng,
                        const Sig &sig, int cur_cost, int *delta_corr,
                        int *old_vals_out, bool &is_mid) {
  int L = d, R = n1 - 1 - d;
  is_mid = (L == R);
  int oA_L = st.A[L], oB_L = st.B[L];
  int oA_R = st.A[R], oB_R = st.B[R];
  int nA_L, nB_L, nA_R, nB_R;
  if (is_mid) {
    int u[2]; sample_mid(rng, u);
    nA_L = u[0]; nB_L = u[1]; nA_R = nA_L; nB_R = nB_L;
    if (oA_L == nA_L && oB_L == nB_L) return INT_MAX;
  } else {
    int u[4]; sample_AB(d, rng, u);
    nA_L = u[0]; nB_L = u[1]; nA_R = u[2]; nB_R = u[3];
    if (oA_L == nA_L && oB_L == nB_L && oA_R == nA_R && oB_R == nB_R) return INT_MAX;
  }
  old_vals_out[0]=oA_L; old_vals_out[1]=oB_L;
  old_vals_out[2]=oA_R; old_vals_out[3]=oB_R;
  old_vals_out[4]=nA_L; old_vals_out[5]=nB_L;
  old_vals_out[6]=nA_R; old_vals_out[7]=nB_R;

  // Subtract old contribution
  for (int s = 1; s < ms; s++) {
    int dlt = 0;
    if (L - s >= 0)  dlt -= st.A[L - s]*oA_L + st.B[L - s]*oB_L;
    if (L + s < n1)  dlt -= oA_L*st.A[L + s] + oB_L*st.B[L + s];
    if (!is_mid) {
      if (R - s >= 0)  dlt -= st.A[R - s]*oA_R + st.B[R - s]*oB_R;
      if (R + s < n1)  dlt -= oA_R*st.A[R + s] + oB_R*st.B[R + s];
      if (R - L == s)  dlt += oA_L*oA_R + oB_L*oB_R;  // un-double-count
    }
    delta_corr[s] = dlt;
  }

  // Apply new values
  st.A[L]=nA_L; st.B[L]=nB_L;
  if (!is_mid) { st.A[R]=nA_R; st.B[R]=nB_R; }

  // Add new contribution
  for (int s = 1; s < ms; s++) {
    int dlt = delta_corr[s];
    if (L - s >= 0)  dlt += st.A[L - s]*nA_L + st.B[L - s]*nB_L;
    if (L + s < n1)  dlt += nA_L*st.A[L + s] + nB_L*st.B[L + s];
    if (!is_mid) {
      if (R - s >= 0)  dlt += st.A[R - s]*nA_R + st.B[R - s]*nB_R;
      if (R + s < n1)  dlt += nA_R*st.A[R + s] + nB_R*st.B[R + s];
      if (R - L == s)  dlt -= nA_L*nA_R + nB_L*nB_R;
    }
    delta_corr[s] = dlt;
    st.corr[s] += dlt;
  }

  if (is_mid) {
    st.sum_a += nA_L - oA_L;
    st.sum_b += nB_L - oB_L;
  } else {
    st.sum_a += (nA_L + nA_R) - (oA_L + oA_R);
    st.sum_b += (nB_L + nB_R) - (oB_L + oB_R);
  }
  return st.cost(sig, ms);
}

static void revert_AB_move(State &st, int n1, int ms, int d,
                            const int *old_vals, const int *delta_corr,
                            bool is_mid) {
  int L = d, R = n1 - 1 - d;
  st.A[L]=old_vals[0]; st.B[L]=old_vals[1];
  if (!is_mid) { st.A[R]=old_vals[2]; st.B[R]=old_vals[3]; }
  for (int s = 1; s < ms; s++) st.corr[s] -= delta_corr[s];
  int nA_L=old_vals[4], nB_L=old_vals[5], nA_R=old_vals[6], nB_R=old_vals[7];
  int oA_L=old_vals[0], oB_L=old_vals[1], oA_R=old_vals[2], oB_R=old_vals[3];
  if (is_mid) {
    st.sum_a -= nA_L - oA_L;
    st.sum_b -= nB_L - oB_L;
  } else {
    st.sum_a -= (nA_L + nA_R) - (oA_L + oA_R);
    st.sum_b -= (nB_L + nB_R) - (oB_L + oB_R);
  }
}

// CD pair-slot move at d (covers L=d, R=n-1-d).
static int try_CD_move(State &st, int n1, int n, int ms, int d, mt19937 &rng,
                        const Sig &sig, int cur_cost, int *delta_corr,
                        int *old_vals_out, bool &is_mid) {
  int L = d, R = n - 1 - d;
  is_mid = (L == R);
  int oC_L = st.C[L], oD_L = st.D[L];
  int oC_R = st.C[R], oD_R = st.D[R];
  int nC_L, nD_L, nC_R, nD_R;
  if (is_mid) {
    int u[2]; sample_mid(rng, u);
    nC_L = u[0]; nD_L = u[1]; nC_R = nC_L; nD_R = nD_L;
    if (oC_L == nC_L && oD_L == nD_L) return INT_MAX;
  } else {
    int u[4]; sample_CD(d, rng, u);
    nC_L = u[0]; nD_L = u[1]; nC_R = u[2]; nD_R = u[3];
    if (oC_L == nC_L && oD_L == nD_L && oC_R == nC_R && oD_R == nD_R) return INT_MAX;
  }
  old_vals_out[0]=oC_L; old_vals_out[1]=oD_L;
  old_vals_out[2]=oC_R; old_vals_out[3]=oD_R;
  old_vals_out[4]=nC_L; old_vals_out[5]=nD_L;
  old_vals_out[6]=nC_R; old_vals_out[7]=nD_R;

  for (int s = 1; s < ms; s++) {
    int dlt = 0;
    if (L - s >= 0) dlt -= st.C[L - s]*oC_L + st.D[L - s]*oD_L;
    if (L + s < n)  dlt -= oC_L*st.C[L + s] + oD_L*st.D[L + s];
    if (!is_mid) {
      if (R - s >= 0) dlt -= st.C[R - s]*oC_R + st.D[R - s]*oD_R;
      if (R + s < n)  dlt -= oC_R*st.C[R + s] + oD_R*st.D[R + s];
      if (R - L == s) dlt += oC_L*oC_R + oD_L*oD_R;
    }
    delta_corr[s] = dlt;
  }

  st.C[L]=nC_L; st.D[L]=nD_L;
  if (!is_mid) { st.C[R]=nC_R; st.D[R]=nD_R; }

  for (int s = 1; s < ms; s++) {
    int dlt = delta_corr[s];
    if (L - s >= 0) dlt += st.C[L - s]*nC_L + st.D[L - s]*nD_L;
    if (L + s < n)  dlt += nC_L*st.C[L + s] + nD_L*st.D[L + s];
    if (!is_mid) {
      if (R - s >= 0) dlt += st.C[R - s]*nC_R + st.D[R - s]*nD_R;
      if (R + s < n)  dlt += nC_R*st.C[R + s] + nD_R*st.D[R + s];
      if (R - L == s) dlt -= nC_L*nC_R + nD_L*nD_R;
    }
    delta_corr[s] = dlt;
    st.corr[s] += dlt;
  }

  if (is_mid) {
    st.sum_c += nC_L - oC_L;
    st.sum_d += nD_L - oD_L;
  } else {
    st.sum_c += (nC_L + nC_R) - (oC_L + oC_R);
    st.sum_d += (nD_L + nD_R) - (oD_L + oD_R);
  }
  return st.cost(sig, ms);
}

static void revert_CD_move(State &st, int n, int ms, int d,
                            const int *old_vals, const int *delta_corr,
                            bool is_mid) {
  int L = d, R = n - 1 - d;
  st.C[L]=old_vals[0]; st.D[L]=old_vals[1];
  if (!is_mid) { st.C[R]=old_vals[2]; st.D[R]=old_vals[3]; }
  for (int s = 1; s < ms; s++) st.corr[s] -= delta_corr[s];
  int nC_L=old_vals[4], nD_L=old_vals[5], nC_R=old_vals[6], nD_R=old_vals[7];
  int oC_L=old_vals[0], oD_L=old_vals[1], oC_R=old_vals[2], oD_R=old_vals[3];
  if (is_mid) {
    st.sum_c -= nC_L - oC_L;
    st.sum_d -= nD_L - oD_L;
  } else {
    st.sum_c -= (nC_L + nC_R) - (oC_L + oC_R);
    st.sum_d -= (nD_L + nD_R) - (oD_L + oD_R);
  }
}

// Enumerate the candidate value-table for a slot.  Returns count and writes the
// up-to-16 4-tuples into out[] (each row is {v0,v1,v2,v3}).  For middle slots
// the row is padded with the pair's own values (since L==R).
static int enumerate_slot(int slot, int n1, int n, bool is_AB, int out[16][4]) {
  int d = slot;
  int L, R;
  if (is_AB) { L = d; R = n1 - 1 - d; }
  else       { L = d; R = n  - 1 - d; }
  if (L == R) {
    for (int i = 0; i < 4; i++) {
      out[i][0] = comb4[i][0]; out[i][1] = comb4[i][1];
      out[i][2] = comb4[i][0]; out[i][3] = comb4[i][1];
    }
    return 4;
  }
  if (is_AB) {
    if (d == 0) { for (int i = 0; i < 8; i++) memcpy(out[i], comb8_neg[i], 4*sizeof(int)); return 8; }
    else        { for (int i = 0; i < 8; i++) memcpy(out[i], comb8_pos[i], 4*sizeof(int)); return 8; }
  } else {
    if (d == 0) { for (int i = 0; i < 16; i++) memcpy(out[i], comb16[i], 4*sizeof(int)); return 16; }
    else        { for (int i = 0; i < 8; i++) memcpy(out[i], comb8_pos[i], 4*sizeof(int)); return 8; }
  }
}

// Apply a forced value to a slot (no SA accept/reject).  Returns true if applied (state changed).
// Used by exhaustive descent.  Caller must save old values to revert.
static bool apply_forced(State &st, int n1, int n, int ms, int slot, bool is_AB,
                          const int v[4], int *delta_corr, int *old_vals) {
  int L, R;
  if (is_AB) { L = slot; R = n1 - 1 - slot; }
  else       { L = slot; R = n  - 1 - slot; }
  bool is_mid = (L == R);
  int *X1 = is_AB ? st.A : st.C;
  int *Y1 = is_AB ? st.B : st.D;
  int N  = is_AB ? n1 : n;

  int oX_L = X1[L], oY_L = Y1[L];
  int oX_R = X1[R], oY_R = Y1[R];
  int nX_L = v[0], nY_L = v[1], nX_R = v[2], nY_R = v[3];
  if (is_mid && oX_L == nX_L && oY_L == nY_L) return false;
  if (!is_mid && oX_L == nX_L && oY_L == nY_L && oX_R == nX_R && oY_R == nY_R) return false;

  old_vals[0]=oX_L; old_vals[1]=oY_L; old_vals[2]=oX_R; old_vals[3]=oY_R;

  for (int s = 1; s < ms; s++) {
    int dlt = 0;
    if (L - s >= 0) dlt -= X1[L - s]*oX_L + Y1[L - s]*oY_L;
    if (L + s < N)  dlt -= oX_L*X1[L + s] + oY_L*Y1[L + s];
    if (!is_mid) {
      if (R - s >= 0) dlt -= X1[R - s]*oX_R + Y1[R - s]*oY_R;
      if (R + s < N)  dlt -= oX_R*X1[R + s] + oY_R*Y1[R + s];
      if (R - L == s) dlt += oX_L*oX_R + oY_L*oY_R;
    }
    delta_corr[s] = dlt;
  }
  X1[L]=nX_L; Y1[L]=nY_L;
  if (!is_mid) { X1[R]=nX_R; Y1[R]=nY_R; }
  for (int s = 1; s < ms; s++) {
    int dlt = delta_corr[s];
    if (L - s >= 0) dlt += X1[L - s]*nX_L + Y1[L - s]*nY_L;
    if (L + s < N)  dlt += nX_L*X1[L + s] + nY_L*Y1[L + s];
    if (!is_mid) {
      if (R - s >= 0) dlt += X1[R - s]*nX_R + Y1[R - s]*nY_R;
      if (R + s < N)  dlt += nX_R*X1[R + s] + nY_R*Y1[R + s];
      if (R - L == s) dlt -= nX_L*nX_R + nY_L*nY_R;
    }
    delta_corr[s] = dlt;
    st.corr[s] += dlt;
  }
  if (is_AB) {
    if (is_mid) { st.sum_a += nX_L - oX_L; st.sum_b += nY_L - oY_L; }
    else        { st.sum_a += (nX_L+nX_R)-(oX_L+oX_R); st.sum_b += (nY_L+nY_R)-(oY_L+oY_R); }
  } else {
    if (is_mid) { st.sum_c += nX_L - oX_L; st.sum_d += nY_L - oY_L; }
    else        { st.sum_c += (nX_L+nX_R)-(oX_L+oX_R); st.sum_d += (nY_L+nY_R)-(oY_L+oY_R); }
  }
  return true;
}

static void revert_forced(State &st, int n1, int n, int ms, int slot, bool is_AB,
                           const int *old_vals, const int *delta_corr) {
  int L, R;
  if (is_AB) { L = slot; R = n1 - 1 - slot; }
  else       { L = slot; R = n  - 1 - slot; }
  bool is_mid = (L == R);
  int *X1 = is_AB ? st.A : st.C;
  int *Y1 = is_AB ? st.B : st.D;
  int oX_L = old_vals[0], oY_L = old_vals[1];
  int oX_R = old_vals[2], oY_R = old_vals[3];
  int nX_L = X1[L], nY_L = Y1[L];
  int nX_R = X1[R], nY_R = Y1[R];
  X1[L]=oX_L; Y1[L]=oY_L;
  if (!is_mid) { X1[R]=oX_R; Y1[R]=oY_R; }
  for (int s = 1; s < ms; s++) st.corr[s] -= delta_corr[s];
  if (is_AB) {
    if (is_mid) { st.sum_a -= nX_L - oX_L; st.sum_b -= nY_L - oY_L; }
    else        { st.sum_a -= (nX_L+nX_R)-(oX_L+oX_R); st.sum_b -= (nY_L+nY_R)-(oY_L+oY_R); }
  } else {
    if (is_mid) { st.sum_c -= nX_L - oX_L; st.sum_d -= nY_L - oY_L; }
    else        { st.sum_c -= (nX_L+nX_R)-(oX_L+oX_R); st.sum_d -= (nY_L+nY_R)-(oY_L+oY_R); }
  }
}

// Exhaustive 1-pair descent: for every (slot, value-combo), apply if improving.
// Loop until no improvement.  Returns true if cost reached 0.
static bool one_pair_descent(State &st, int &cur_cost, int n1, int n, int ms,
                              const Sig &sig) {
  int ab_slots = (n1 + 1) / 2;
  int cd_slots = (n  + 1) / 2;
  int total = ab_slots + cd_slots;
  bool improved = true;
  int delta_corr[128];
  int old_vals[8];
  int combos[16][4];
  while (improved && cur_cost > 0) {
    improved = false;
    for (int slot = 0; slot < total; slot++) {
      bool is_AB = (slot < ab_slots);
      int local_slot = is_AB ? slot : slot - ab_slots;
      int nc = enumerate_slot(local_slot, n1, n, is_AB, combos);
      for (int ci = 0; ci < nc; ci++) {
        if (!apply_forced(st, n1, n, ms, local_slot, is_AB, combos[ci], delta_corr, old_vals)) continue;
        int nc2 = st.cost(sig, ms);
        if (nc2 < cur_cost) {
          cur_cost = nc2;
          improved = true;
          if (cur_cost == 0) return true;
          break;
        } else {
          revert_forced(st, n1, n, ms, local_slot, is_AB, old_vals, delta_corr);
        }
      }
    }
  }
  return cur_cost == 0;
}

// Exhaustive 2-pair descent: for every pair (slot_i, slot_j) and every (combo_i, combo_j),
// apply if improving.  Loop until no improvement.  Returns true if cost reached 0.
static bool two_pair_descent(State &st, int &cur_cost, int n1, int n, int ms,
                              const Sig &sig) {
  int ab_slots = (n1 + 1) / 2;
  int cd_slots = (n  + 1) / 2;
  int total = ab_slots + cd_slots;
  bool improved = true;
  int delta1[128], delta2[128];
  int oldv1[8], oldv2[8];
  int combos_i[16][4], combos_j[16][4];
  while (improved && cur_cost > 0) {
    improved = false;
    for (int si = 0; si < total && !improved; si++) {
      bool i_AB = (si < ab_slots);
      int li = i_AB ? si : si - ab_slots;
      int nci = enumerate_slot(li, n1, n, i_AB, combos_i);
      for (int sj = si + 1; sj < total && !improved; sj++) {
        bool j_AB = (sj < ab_slots);
        int lj = j_AB ? sj : sj - ab_slots;
        int ncj = enumerate_slot(lj, n1, n, j_AB, combos_j);
        for (int ci = 0; ci < nci && !improved; ci++) {
          if (!apply_forced(st, n1, n, ms, li, i_AB, combos_i[ci], delta1, oldv1)) continue;
          for (int cj = 0; cj < ncj; cj++) {
            if (!apply_forced(st, n1, n, ms, lj, j_AB, combos_j[cj], delta2, oldv2)) continue;
            int nc = st.cost(sig, ms);
            if (nc < cur_cost) {
              cur_cost = nc;
              improved = true;
              if (cur_cost == 0) return true;
              break;
            }
            revert_forced(st, n1, n, ms, lj, j_AB, oldv2, delta2);
          }
          if (!improved) revert_forced(st, n1, n, ms, li, i_AB, oldv1, delta1);
        }
      }
    }
  }
  return cur_cost == 0;
}

// Intensification: run 1-pair descent, then 2-pair descent if needed.  Returns true if cost reached 0.
static bool intensify(State &st, int &cur_cost, int n1, int n, int ms, const Sig &sig) {
  if (one_pair_descent(st, cur_cost, n1, n, ms, sig)) return true;
  if (cur_cost <= 24 && two_pair_descent(st, cur_cost, n1, n, ms, sig)) return true;
  if (one_pair_descent(st, cur_cost, n1, n, ms, sig)) return true;
  return cur_cost == 0;
}

// k-opt kick: resample k random pair slots simultaneously (full reset of corr afterward).
static void kopt_kick(State &st, int n1, int n, int ms, int k, mt19937 &rng) {
  int ab_slots = (n1 + 1) / 2;   // includes mid for odd n1
  int cd_slots = (n  + 1) / 2;
  int total = ab_slots + cd_slots;
  for (int kk = 0; kk < k; kk++) {
    int s = uniform_int_distribution<>(0, total - 1)(rng);
    int t[4]; int u[2];
    if (s < ab_slots) {
      int d = s;
      int L = d, R = n1 - 1 - d;
      if (L == R) { sample_mid(rng, u); st.A[L]=u[0]; st.B[L]=u[1]; }
      else        { sample_AB(d, rng, t); st.A[L]=t[0]; st.B[L]=t[1]; st.A[R]=t[2]; st.B[R]=t[3]; }
    } else {
      int d = s - ab_slots;
      int L = d, R = n - 1 - d;
      if (L == R) { sample_mid(rng, u); st.C[L]=u[0]; st.D[L]=u[1]; }
      else        { sample_CD(d, rng, t); st.C[L]=t[0]; st.D[L]=t[1]; st.C[R]=t[2]; st.D[R]=t[3]; }
    }
  }
  state_recompute(st, n1, n, ms);
}

// =====================================================================
//  SA parameters
// =====================================================================
struct SAParams {
  double initial_temp     = 8.0;
  double cooling_rate     = 0.9999;
  int    iterations       = 80000;
  int    restarts         = 4;
  int    reheat_threshold = 8000;
  double reheat_ratio     = 0.6;
  int    kick_after_stall = 3000;
};

// =====================================================================
//  Per-signature joint SA
// =====================================================================
static bool solve_joint_SA(int n, int n1, int ms, const Sig &sig,
                            State &best_state, mt19937 &rng) {
  State curr;
  state_random_init(curr, n1, n, ms, rng);
  int cur_cost = curr.cost(sig, ms);
  best_state = curr;
  int best_cost = cur_cost;

  SAParams sa;
  uniform_real_distribution<> prob(0.0, 1.0);
  int ab_slots = (n1 + 1) / 2;
  int cd_slots = (n  + 1) / 2;
  int total_slots = ab_slots + cd_slots;

  int delta_corr[128];
  int old_vals[8];

  for (int restart = 0; restart < sa.restarts; restart++) {
    if (g_found.load(memory_order_relaxed)) return false;

    if (restart > 0) {
      // ILS-style restart: perturb best_state with a strong kick (k = total_slots/4).
      curr = best_state;
      int k = max(2, total_slots / 4);
      kopt_kick(curr, n1, n, ms, k, rng);
      cur_cost = curr.cost(sig, ms);
    }

    // Post-restart deterministic descent: cheap and helps every sig.
    {
      State work = curr;
      int wc = cur_cost;
      one_pair_descent(work, wc, n1, n, ms, sig);
      if (wc < cur_cost) { curr = work; cur_cost = wc; }
      if (wc < best_cost) { best_state = work; best_cost = wc; }
      if (best_cost == 0) return true;
    }

    double temp = sa.initial_temp;
    int no_improve = 0;
    int stall_for_kick = 0;

    for (int iter = 0; iter < sa.iterations; iter++) {
      g_iters_total.fetch_add(1, memory_order_relaxed);
      if ((iter & 0xFFF) == 0 && g_found.load(memory_order_relaxed)) return false;
      if (best_cost == 0) return true;

      no_improve++;
      stall_for_kick++;

      if (stall_for_kick > sa.kick_after_stall) {
        // Try exhaustive descent from best_state before kicking.
        if (best_cost > 0 && best_cost <= 32) {
          State work = best_state;
          int wc = best_cost;
          if (intensify(work, wc, n1, n, ms, sig)) {
            best_state = work;
            best_cost = 0;
            return true;
          }
          if (wc < best_cost) {
            best_state = work;
            best_cost = wc;
          }
        }
        curr = best_state;
        int k = 2 + uniform_int_distribution<>(0, 3)(rng);
        kopt_kick(curr, n1, n, ms, k, rng);
        cur_cost = curr.cost(sig, ms);
        temp = sa.initial_temp * 0.6;
        stall_for_kick = 0;
        continue;
      }

      if (no_improve > sa.reheat_threshold) {
        temp = sa.initial_temp * sa.reheat_ratio;
        no_improve = 0;
      }

      // Pick a slot uniformly across (AB ∪ CD).
      int slot = uniform_int_distribution<>(0, total_slots - 1)(rng);
      bool is_mid;
      int new_cost;
      if (slot < ab_slots) {
        int d = slot;
        new_cost = try_AB_move(curr, n1, n, ms, d, rng, sig, cur_cost,
                                delta_corr, old_vals, is_mid);
      } else {
        int d = slot - ab_slots;
        new_cost = try_CD_move(curr, n1, n, ms, d, rng, sig, cur_cost,
                                delta_corr, old_vals, is_mid);
      }
      if (new_cost == INT_MAX) { temp *= sa.cooling_rate; continue; }

      bool accept = (new_cost < cur_cost) ||
                    (prob(rng) < exp(-(new_cost - cur_cost) / temp));
      if (accept) {
        cur_cost = new_cost;
        if (new_cost < best_cost) {
          best_cost = new_cost;
          best_state = curr;
          no_improve = 0;
          stall_for_kick = 0;
        }
      } else {
        if (slot < ab_slots) revert_AB_move(curr, n1, ms, slot, old_vals, delta_corr, is_mid);
        else                 revert_CD_move(curr, n,  ms, slot - ab_slots, old_vals, delta_corr, is_mid);
      }

      temp *= sa.cooling_rate;
    }
  }

  return best_cost == 0;
}

// =====================================================================
//  Main
// =====================================================================
int main(int argc, char **argv) {
  if (argc < 2) {
    cerr << "Usage: " << argv[0] << " <n> [seed_offset]" << endl;
    return 1;
  }
  int n = atoi(argv[1]);
  int seed_offset = (argc >= 3) ? atoi(argv[2]) : 0;
  int n1 = n + 1;
  int ms = max(n1, n);

  init_combs();

  int thr = 1;
#ifdef _OPENMP
  thr = omp_get_max_threads();
#endif

  cout << "========================================================" << endl;
  cout << "  BS(" << n1 << "," << n << ") v4 Solver — Joint Wang-Zhu SA" << endl;
  cout << "  [ Threads: " << thr << " | Seed offset: " << seed_offset << " ]" << endl;
  cout << "========================================================" << endl;

  G_T0 = Clock::now();
  auto sigs = get_sigs(n);
  cout << "Loaded " << sigs.size() << " valid sum signatures." << endl << endl;

  vector<atomic<int>> sig_fails(sigs.size());
  for (auto &a : sig_fails) a.store(0, memory_order_relaxed);

  Clock::time_point last_log = Clock::now();

#pragma omp parallel
  {
    int tid = 0;
#ifdef _OPENMP
    tid = omp_get_thread_num();
#endif
    std::random_device rd;
    uint64_t ns = chrono::high_resolution_clock::now().time_since_epoch().count();
    std::seed_seq seq{
      (uint32_t)rd(), (uint32_t)rd(), (uint32_t)rd(), (uint32_t)rd(),
      (uint32_t)tid, (uint32_t)(tid >> 16),
      (uint32_t)seed_offset, (uint32_t)(seed_offset >> 16),
      (uint32_t)ns, (uint32_t)(ns >> 32)
    };
    mt19937 rng(seq);

    while (!g_found.load(memory_order_relaxed)) {
      // ε-greedy signature selection.
      int si;
      while (true) {
        si = uniform_int_distribution<>(0, (int)sigs.size() - 1)(rng);
        int fails = sig_fails[si].load(memory_order_relaxed);
        if (fails < 3) break;
        double p = 1.0 / (1.0 + 0.4 * (fails - 2));
        if (uniform_real_distribution<>(0.0, 1.0)(rng) < p + 0.15) break;
      }
      const Sig &sig = sigs[si];

      State best;
      bool found = solve_joint_SA(n, n1, ms, sig, best, rng);
      int cost = found ? 0 : best.cost(sig, ms);

      int prev = g_champion_cost.load(memory_order_relaxed);
      while (cost < prev &&
             !g_champion_cost.compare_exchange_weak(prev, cost, memory_order_relaxed)) {}

      if (found) {
        // Final NPAF=0 verification.
        bool valid = true;
        for (int s = 1; s < ms && valid; s++) if (best.corr[s] != 0) valid = false;
        valid = valid && abs(best.sum_a - sig.a) == 0 && abs(best.sum_b - sig.b) == 0
                      && abs(best.sum_c - sig.c) == 0 && abs(best.sum_d - sig.d) == 0;
        if (valid) {
          g_found.store(true);
#pragma omp critical(output)
          {
            if (n >= 44)
              cout << "\n*** WORLD RECORD DISCOVERY: BS(" << n1 << "," << n << ") FOUND ***\n" << endl;
            else
              cout << "\n*** REPRODUCTION CONFIRMED: BS(" << n1 << "," << n << ") FOUND ***\n" << endl;
            cout << "sig = (" << sig.a << "," << sig.b << "," << sig.c << "," << sig.d << ")" << endl;
            cout << "A = {"; for (int i=0;i<n1;i++) cout<<best.A[i]<<(i<n1-1?",":""); cout<<"};\n";
            cout << "B = {"; for (int i=0;i<n1;i++) cout<<best.B[i]<<(i<n1-1?",":""); cout<<"};\n";
            cout << "C = {"; for (int i=0;i<n; i++) cout<<best.C[i]<<(i<n-1 ?",":""); cout<<"};\n";
            cout << "D = {"; for (int i=0;i<n; i++) cout<<best.D[i]<<(i<n-1 ?",":""); cout<<"};\n";
            double t = chrono::duration<double>(Clock::now() - G_T0).count();
            cout << "\nTime: " << t << "s\nSeed offset: " << seed_offset << endl;
          }
        }
      } else {
        sig_fails[si].fetch_add(1, memory_order_relaxed);
      }

      // Logging from tid 0.
      if (tid == 0) {
        auto now = Clock::now();
        if (chrono::duration<double>(now - last_log).count() > 15.0) {
          last_log = now;
          double t = chrono::duration<double>(now - G_T0).count();
          long long it = g_iters_total.load(memory_order_relaxed);
          int champ = g_champion_cost.load(memory_order_relaxed);
          cout << "[" << t << "s] Iters: " << it
               << " Speed: " << (long long)(it / max(t, 1e-3))
               << " Champion: " << (champ == INT_MAX ? -1 : champ)
               << "\n" << flush;
        }
      }
    }
  }

  return g_found.load() ? 0 : 1;
}
