/*
 * BS(n+1, n) Joint-SA Solver v3
 * CP493 — Daniel Gordon
 *
 * Joint SA over the full {-1,+1}^(4n+2) manifold.
 * Cost = sum_s |NPAF(s)| + sum-constraint penalties.
 * All four sequences (A,B,C,D) are co-optimised simultaneously.
 *
 * Previous v3 bug: the CD phase used a completability penalty that was
 * mathematically always zero for n=27 (max CD correlation < threshold),
 * so CD was effectively random and AB had to match arbitrary targets.
 * Joint SA fixes this: a single cost function drives the whole search.
 *
 * Usage:  ./wz_sa_v3 <n> [seed_offset]
 * Compile: g++ -O3 -march=native -std=c++17 -fopenmp -o wz_sa_v3 src/solver/wz_sa_v3.cpp
 */

#include <algorithm>
#include <atomic>
#include <chrono>
#include <climits>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <random>
#include <sstream>
#include <tuple>
#include <vector>

#ifdef _OPENMP
#include <omp.h>
#endif

using namespace std;
using Clock = chrono::steady_clock;

static atomic<bool>      g_found{false};
static atomic<int>       g_champion_cost{INT_MAX};
static atomic<long long> g_epochs_total{0};
static int               G_N, G_N1, G_SEED_OFFSET;
static Clock::time_point G_T0;

// =====================================================================
//  Signature enumeration  a²+b²+c²+d² = 4n+2, parity + Wang-Zhu mod-4
// =====================================================================
struct Sig { int a, b, c, d; };

static vector<Sig> get_sigs(int n) {
  vector<Sig> sigs;
  int T = 4*n+2, n1 = n+1, ap = n1%2, cp = n%2;
  for (int a = 0; a <= n1; a++) {
    if (a%2 != ap || a*a > T) continue;
    for (int b = -n1; b <= n1; b++) {
      if (((b%2)+2)%2 != (unsigned)ap) continue;
      int r = T - a*a - b*b;
      if (r < 0) continue;
      for (int c = -n; c <= n; c++) {
        if (((c%2)+2)%2 != (unsigned)cp) continue;
        int d2 = r - c*c;
        if (d2 < 0) continue;
        int d = (int)round(sqrt((double)d2));
        if (d*d != d2 || d > n || ((d%2+2)%2 != (unsigned)cp)) continue;
        if (n%2 == 0) {
          if (((c-d)%4+8)%4 != 0) continue;
        } else {
          if (((a-b-2)%4+8)%4 != 0) continue;
        }
        sigs.push_back({a, b, c, d});
        if (d > 0 && (((-d)%2+2)%2 == (unsigned)cp)) {
          bool ok = true;
          if (n%2 == 0 && ((c+d)%4+8)%4 != 0) ok = false;
          if (ok) sigs.push_back({a, b, c, -d});
        }
      }
    }
  }
  sort(sigs.begin(), sigs.end(), [](auto &x, auto &y){
    return tie(x.a,x.b,x.c,x.d) < tie(y.a,y.b,y.c,y.d);
  });
  sigs.erase(unique(sigs.begin(), sigs.end(), [](auto &x, auto &y){
    return x.a==y.a && x.b==y.b && x.c==y.c && x.d==y.d;
  }), sigs.end());
  return sigs;
}

// =====================================================================
//  Hall polynomial filter + NPAF helper
// =====================================================================
static bool hall_ok(const int *X, int xl, const int *Y, int yl) {
  double lim = 4.0*G_N + 2.0;
  for (int j = 1; j <= 200; j++) {
    double th = j * M_PI / 100.0;
    double rx=0, ix_=0, ry=0, iy=0;
    for (int i = 0; i < xl; i++) { rx += X[i]*cos(i*th); ix_ += X[i]*sin(i*th); }
    for (int i = 0; i < yl; i++) { ry += Y[i]*cos(i*th); iy  += Y[i]*sin(i*th); }
    if (rx*rx + ix_*ix_ + ry*ry + iy*iy > lim + 0.5) return false;
  }
  return true;
}

static int npaf_at(const int *A, const int *B, int n1,
                   const int *C, const int *D, int n2, int s) {
  int v = 0;
  if (s < n1) for (int i = 0; i < n1-s; i++) v += A[i]*A[i+s] + B[i]*B[i+s];
  if (s < n2) for (int i = 0; i < n2-s; i++) v += C[i]*C[i+s] + D[i]*D[i+s];
  return v;
}

// =====================================================================
//  SA parameters
// =====================================================================
struct SAParams {
  double initial_temp     = 40.0;
  double cooling_rate     = 0.99997;
  int    iterations       = 2000000;
  int    restarts         = 10;
  int    reheat_threshold = 200000;
  double reheat_ratio     = 0.65;
  int    kick_after_stall = 80000;
  int    kick_min_k       = 4;
  int    kick_max_k       = 12;
  // Compound move probabilities (single-flip = remainder)
  double p_two_flip       = 0.10;
  double p_three_flip     = 0.04;
  // ILS threshold: when best_cost <= ils_threshold, perturb best_state on restart
  int    ils_threshold    = 60;
};

// =====================================================================
//  Joint state: A,B (len n1), C,D (len n), combined npaf
// =====================================================================
static const int MAXN = 128;

struct State {
  int A[MAXN], B[MAXN];
  int C[MAXN], D[MAXN];
  int sum_a, sum_b, sum_c, sum_d;
  int npaf[MAXN];  // PAF_A[s]+PAF_B[s]+PAF_C[s]+PAF_D[s]

  int cost(const Sig &sig, int ms) const {
    int pen = 0;
    for (int s = 1; s < ms; s++) pen += abs(npaf[s]);
    pen += abs(sum_a - sig.a) * 8;
    pen += abs(sum_b - sig.b) * 8;
    pen += abs(sum_c - sig.c) * 8;
    pen += abs(sum_d - sig.d) * 8;
    return pen;
  }
};

static void state_recompute(State &st, int n1, int n) {
  memset(st.npaf, 0, sizeof(st.npaf));
  st.sum_a = st.sum_b = st.sum_c = st.sum_d = 0;
  int ms = max(n1, n);
  for (int i = 0; i < n1; i++) { st.sum_a += st.A[i]; st.sum_b += st.B[i]; }
  for (int i = 0; i < n;  i++) { st.sum_c += st.C[i]; st.sum_d += st.D[i]; }
  for (int s = 1; s < ms; s++) {
    if (s < n1) for (int i = 0; i < n1-s; i++) st.npaf[s] += st.A[i]*st.A[i+s] + st.B[i]*st.B[i+s];
    if (s < n)  for (int i = 0; i < n-s;  i++) st.npaf[s] += st.C[i]*st.C[i+s] + st.D[i]*st.D[i+s];
  }
}

static void state_random_init(State &st, int n1, int n, mt19937 &rng) {
  uniform_int_distribution<> bit(0,1);
  for (int i = 0; i < n1; i++) { st.A[i]=bit(rng)?1:-1; st.B[i]=bit(rng)?1:-1; }
  for (int i = 0; i < n;  i++) { st.C[i]=bit(rng)?1:-1; st.D[i]=bit(rng)?1:-1; }
  state_recompute(st, n1, n);
}

// Compute cost delta for flipping seq_id at idx, fill delta_npaf.
// seq_id: 0=A, 1=B, 2=C, 3=D
static int flip_delta(const State &st, int seq_id, int idx,
                      int n1, int n, const Sig &sig, int ms,
                      int *delta_npaf) {
  const int *arr;
  int len, old_sum, targ;
  switch (seq_id) {
    case 0: arr=st.A; len=n1; old_sum=st.sum_a; targ=sig.a; break;
    case 1: arr=st.B; len=n1; old_sum=st.sum_b; targ=sig.b; break;
    case 2: arr=st.C; len=n;  old_sum=st.sum_c; targ=sig.c; break;
    default:arr=st.D; len=n;  old_sum=st.sum_d; targ=sig.d; break;
  }
  int dv = -2 * arr[idx];  // new - old
  int dcost = 0;
  for (int s = 1; s < ms; s++) {
    int acc = 0;
    if (idx+s < len) acc += arr[idx+s];
    if (idx-s >= 0)  acc += arr[idx-s];
    delta_npaf[s] = dv * acc;
    dcost += abs(st.npaf[s] + delta_npaf[s]) - abs(st.npaf[s]);
  }
  int ns = old_sum + dv;
  dcost += (abs(ns - targ) - abs(old_sum - targ)) * 8;
  return dcost;
}

// Apply the flip (use delta_npaf already computed by flip_delta).
static void flip_apply(State &st, int seq_id, int idx, int ms,
                       const int *delta_npaf) {
  int *arr; int *sp;
  switch (seq_id) {
    case 0: arr=st.A; sp=&st.sum_a; break;
    case 1: arr=st.B; sp=&st.sum_b; break;
    case 2: arr=st.C; sp=&st.sum_c; break;
    default:arr=st.D; sp=&st.sum_d; break;
  }
  *sp -= 2*arr[idx];
  arr[idx] = -arr[idx];
  for (int s = 1; s < ms; s++) st.npaf[s] += delta_npaf[s];
}

// Decode a flat position index into (seq_id, idx) within that sequence.
static inline void decode_flat(int p, int n1, int n, int &seq_id, int &idx) {
  if      (p < n1)       { seq_id=0; idx=p; }
  else if (p < 2*n1)     { seq_id=1; idx=p-n1; }
  else if (p < 2*n1+n)   { seq_id=2; idx=p-2*n1; }
  else                   { seq_id=3; idx=p-2*n1-n; }
}

// Compute dcost for flipping (s1,i1) and (s2,i2) together, leaving state unchanged.
static int pair_dcost(State &st, int s1, int i1, int s2, int i2,
                      int n1, int n, const Sig &sig, int ms) {
  int d1[MAXN], d2[MAXN], dundo[MAXN];
  int dc1 = flip_delta(st, s1, i1, n1, n, sig, ms, d1);
  flip_apply(st, s1, i1, ms, d1);
  int dc2 = flip_delta(st, s2, i2, n1, n, sig, ms, d2);
  // Undo s1
  flip_delta(st, s1, i1, n1, n, sig, ms, dundo);
  flip_apply(st, s1, i1, ms, dundo);
  return dc1 + dc2;
}

// Compute dcost for flipping (s1,i1), (s2,i2), (s3,i3) together, leaving state unchanged.
static int triple_dcost(State &st, int s1, int i1, int s2, int i2, int s3, int i3,
                        int n1, int n, const Sig &sig, int ms) {
  int d[MAXN];
  int dc1 = flip_delta(st, s1, i1, n1, n, sig, ms, d);
  flip_apply(st, s1, i1, ms, d);
  int dc2 = flip_delta(st, s2, i2, n1, n, sig, ms, d);
  flip_apply(st, s2, i2, ms, d);
  int dc3 = flip_delta(st, s3, i3, n1, n, sig, ms, d);
  // Undo s2 then s1
  flip_delta(st, s2, i2, n1, n, sig, ms, d);
  flip_apply(st, s2, i2, ms, d);
  flip_delta(st, s1, i1, n1, n, sig, ms, d);
  flip_apply(st, s1, i1, ms, d);
  return dc1 + dc2 + dc3;
}

static inline void apply_single(State &st, int seq_id, int idx,
                                int n1, int n, const Sig &sig, int ms) {
  int d[MAXN];
  flip_delta(st, seq_id, idx, n1, n, sig, ms, d);
  flip_apply(st, seq_id, idx, ms, d);
}

// Deterministic 2-opt descent: exhaustively try all bit-pair flips, take best improving.
// Loops until no improving pair exists. Returns true if any improvement found.
static bool two_opt_descent(State &st, int &cur_cost, int n1, int n,
                             const Sig &sig, int ms, int max_iters) {
  int total_pos = 2*n1 + 2*n;
  bool any = false;
  for (int it = 0; it < max_iters; it++) {
    int best_dc = 0, best_p1 = -1, best_p2 = -1;
    for (int p1 = 0; p1 < total_pos - 1; p1++) {
      int s1, i1; decode_flat(p1, n1, n, s1, i1);
      for (int p2 = p1 + 1; p2 < total_pos; p2++) {
        int s2, i2; decode_flat(p2, n1, n, s2, i2);
        int dc = pair_dcost(st, s1, i1, s2, i2, n1, n, sig, ms);
        if (dc < best_dc) { best_dc = dc; best_p1 = p1; best_p2 = p2; }
      }
    }
    if (best_dc < 0) {
      int s1, i1, s2, i2;
      decode_flat(best_p1, n1, n, s1, i1);
      decode_flat(best_p2, n1, n, s2, i2);
      apply_single(st, s1, i1, n1, n, sig, ms);
      apply_single(st, s2, i2, n1, n, sig, ms);
      cur_cost += best_dc;
      any = true;
      if (cur_cost == 0) return true;
    } else {
      break;
    }
  }
  return any;
}

// 3-opt EXHAUSTIVE: try all C(N,3) triples, accept best improving.
// Loops while improvements are found. Returns true if any improvement.
static bool three_opt_exhaustive(State &st, int &cur_cost, int n1, int n,
                                  const Sig &sig, int ms, int max_rounds) {
  int total_pos = 2*n1 + 2*n;
  bool any = false;
  for (int r = 0; r < max_rounds; r++) {
    int best_dc = 0;
    int best_p[3] = {-1, -1, -1};
    for (int p1 = 0; p1 < total_pos - 2; p1++) {
      int s1, i1; decode_flat(p1, n1, n, s1, i1);
      for (int p2 = p1 + 1; p2 < total_pos - 1; p2++) {
        int s2, i2; decode_flat(p2, n1, n, s2, i2);
        for (int p3 = p2 + 1; p3 < total_pos; p3++) {
          int s3, i3; decode_flat(p3, n1, n, s3, i3);
          int dc = triple_dcost(st, s1, i1, s2, i2, s3, i3, n1, n, sig, ms);
          if (dc < best_dc) { best_dc = dc; best_p[0]=p1; best_p[1]=p2; best_p[2]=p3; }
        }
      }
    }
    if (best_dc < 0) {
      for (int j = 0; j < 3; j++) {
        int sj, ij; decode_flat(best_p[j], n1, n, sj, ij);
        apply_single(st, sj, ij, n1, n, sig, ms);
      }
      cur_cost += best_dc;
      any = true;
      if (cur_cost == 0) return true;
    } else {
      break;
    }
  }
  return any;
}

// 4-opt sampling: random quadruples, useful as last resort for hard basins.
static bool four_opt_sample(State &st, int &cur_cost, int n1, int n,
                             const Sig &sig, int ms, int samples,
                             mt19937 &rng) {
  int total_pos = 2*n1 + 2*n;
  uniform_int_distribution<> pos_dist(0, total_pos - 1);
  int best_dc = 0;
  int best_p[4] = {-1,-1,-1,-1};
  for (int k = 0; k < samples; k++) {
    int p[4];
    p[0] = pos_dist(rng);
    do { p[1] = pos_dist(rng); } while (p[1]==p[0]);
    do { p[2] = pos_dist(rng); } while (p[2]==p[0] || p[2]==p[1]);
    do { p[3] = pos_dist(rng); } while (p[3]==p[0] || p[3]==p[1] || p[3]==p[2]);
    int dc = 0;
    int d[MAXN];
    for (int j = 0; j < 4; j++) {
      int sj, ij; decode_flat(p[j], n1, n, sj, ij);
      dc += flip_delta(st, sj, ij, n1, n, sig, ms, d);
      flip_apply(st, sj, ij, ms, d);
    }
    if (dc < best_dc) { best_dc = dc; for (int j=0;j<4;j++) best_p[j]=p[j]; }
    // Undo all 4 in reverse order
    for (int j = 3; j >= 0; j--) {
      int sj, ij; decode_flat(p[j], n1, n, sj, ij);
      flip_delta(st, sj, ij, n1, n, sig, ms, d);
      flip_apply(st, sj, ij, ms, d);
    }
  }
  if (best_dc < 0) {
    for (int j = 0; j < 4; j++) {
      int sj, ij; decode_flat(best_p[j], n1, n, sj, ij);
      apply_single(st, sj, ij, n1, n, sig, ms);
    }
    cur_cost += best_dc;
    return true;
  }
  return false;
}

// Intensification: alternates 2-opt exhaustive, 3-opt exhaustive, 4-opt sample.
// Returns true if cost reaches 0.
static bool intensify(State &st, int &cur_cost, int n1, int n,
                       const Sig &sig, int ms, mt19937 &rng) {
  for (int round = 0; round < 8; round++) {
    bool i2 = two_opt_descent(st, cur_cost, n1, n, sig, ms, 200);
    if (cur_cost == 0) return true;
    bool i3 = three_opt_exhaustive(st, cur_cost, n1, n, sig, ms, 4);
    if (cur_cost == 0) return true;
    bool i4 = false;
    if (!i2 && !i3 && cur_cost <= 24) {
      i4 = four_opt_sample(st, cur_cost, n1, n, sig, ms, 200000, rng);
      if (cur_cost == 0) return true;
    }
    if (!i2 && !i3 && !i4) break;
  }
  return cur_cost == 0;
}

// Structured kick: identify which shifts have nonzero NPAF, perturb bits
// that contribute to those shifts. More targeted than random kick.
static void structured_kick(State &st, int n1, int n, int ms, int k, mt19937 &rng) {
  // Score each bit by how many violated shifts it touches.
  int total_pos = 2*n1 + 2*n;
  vector<int> score(total_pos, 0);
  vector<int> violated_shifts;
  for (int s = 1; s < ms; s++) if (st.npaf[s] != 0) violated_shifts.push_back(s);

  if (violated_shifts.empty()) {
    // Fall through to random kick if nothing is violated (shouldn't happen at low cost)
    uniform_int_distribution<> flat(0, total_pos - 1);
    for (int f = 0; f < k; f++) {
      int p = flat(rng);
      int sid, idx; decode_flat(p, n1, n, sid, idx);
      int *arr; int *sp;
      switch (sid) { case 0: arr=st.A; sp=&st.sum_a; break;
                     case 1: arr=st.B; sp=&st.sum_b; break;
                     case 2: arr=st.C; sp=&st.sum_c; break;
                     default:arr=st.D; sp=&st.sum_d; break; }
      *sp -= 2*arr[idx]; arr[idx] = -arr[idx];
    }
    state_recompute(st, n1, n);
    return;
  }

  for (int p = 0; p < total_pos; p++) {
    int sid, idx; decode_flat(p, n1, n, sid, idx);
    int len = (sid <= 1) ? n1 : n;
    for (int s : violated_shifts) {
      if (idx + s < len || idx - s >= 0) score[p]++;
    }
  }

  // Build weighted distribution: bits scoring > 0 get probability proportional to score.
  // Pick k bits without replacement, biased by score.
  vector<int> picked;
  vector<bool> used(total_pos, false);
  for (int f = 0; f < k; f++) {
    int total_w = 0;
    for (int p = 0; p < total_pos; p++) if (!used[p]) total_w += max(1, score[p]);
    if (total_w == 0) break;
    int r = uniform_int_distribution<>(0, total_w - 1)(rng);
    int acc = 0, chosen = -1;
    for (int p = 0; p < total_pos; p++) {
      if (used[p]) continue;
      acc += max(1, score[p]);
      if (acc > r) { chosen = p; break; }
    }
    if (chosen < 0) break;
    used[chosen] = true;
    picked.push_back(chosen);
  }

  for (int p : picked) {
    int sid, idx; decode_flat(p, n1, n, sid, idx);
    int *arr; int *sp;
    switch (sid) { case 0: arr=st.A; sp=&st.sum_a; break;
                   case 1: arr=st.B; sp=&st.sum_b; break;
                   case 2: arr=st.C; sp=&st.sum_c; break;
                   default:arr=st.D; sp=&st.sum_d; break; }
    *sp -= 2*arr[idx]; arr[idx] = -arr[idx];
  }
  state_recompute(st, n1, n);
}

// k-opt kick: flip k random positions across all 4 sequences, recompute npaf.
static void kopt_kick(State &st, int n1, int n, int ms, int k, mt19937 &rng) {
  uniform_int_distribution<> flat_dist(0, 2*n1+2*n-1);
  for (int f = 0; f < k; f++) {
    int p = flat_dist(rng);
    int seq_id, idx;
    if      (p < n1)       { seq_id=0; idx=p; }
    else if (p < 2*n1)     { seq_id=1; idx=p-n1; }
    else if (p < 2*n1+n)   { seq_id=2; idx=p-2*n1; }
    else                   { seq_id=3; idx=p-2*n1-n; }
    int *arr; int *sp;
    switch (seq_id) {
      case 0: arr=st.A; sp=&st.sum_a; break;
      case 1: arr=st.B; sp=&st.sum_b; break;
      case 2: arr=st.C; sp=&st.sum_c; break;
      default:arr=st.D; sp=&st.sum_d; break;
    }
    *sp -= 2*arr[idx];
    arr[idx] = -arr[idx];
  }
  state_recompute(st, n1, n);  // recompute npaf after batch flips
}

// =====================================================================
//  Joint SA: optimise all four sequences simultaneously
// =====================================================================
static bool solve_SA(int n1, int n, const Sig &sig,
                     State &best_state, mt19937 &rng,
                     const SAParams &sa) {
  int ms = max(n1, n);
  int total_pos = 2*n1 + 2*n;
  State curr;
  state_random_init(curr, n1, n, rng);
  int cur_cost = curr.cost(sig, ms);
  best_state = curr;
  int best_cost = cur_cost;

  uniform_real_distribution<> prob(0.0, 1.0);
  uniform_int_distribution<> pos_dist(0, total_pos-1);
  int delta_npaf[MAXN];

  auto decode_pos = [&](int p, int &seq_id, int &idx) {
    if      (p < n1)     { seq_id=0; idx=p; }
    else if (p < 2*n1)   { seq_id=1; idx=p-n1; }
    else if (p < 2*n1+n) { seq_id=2; idx=p-2*n1; }
    else                 { seq_id=3; idx=p-2*n1-n; }
  };

  for (int restart = 0; restart < sa.restarts; restart++) {
    if (g_found.load(memory_order_relaxed)) return false;
    if (restart > 0) {
      // ILS: perturb best_state when we are already close, otherwise random restart.
      if (best_cost > 0 && best_cost <= sa.ils_threshold) {
        curr = best_state;
        int base_k = max(sa.kick_min_k + 2, n/4);
        int kk = base_k + uniform_int_distribution<>(0, n/4 + 2)(rng);
        kopt_kick(curr, n1, n, ms, kk, rng);
      } else {
        state_random_init(curr, n1, n, rng);
      }
      cur_cost = curr.cost(sig, ms);
    }

    double temp = sa.initial_temp;
    int no_improve = 0, stall = 0;
    int acc_win = 0, att_win = 0;

    int delta1[MAXN], delta2[MAXN], delta3[MAXN], dummy[MAXN];

    for (int iter = 0; iter < sa.iterations; iter++) {
      if (g_found.load(memory_order_relaxed)) return false;
      if (best_cost == 0) return true;

      no_improve++; stall++; att_win++;

      // When stalled near a basin: run intensification, then structured kick
      if (stall > sa.kick_after_stall) {
        // If close to a solution, run intensify on best_state IMMEDIATELY
        if (best_cost > 0 && best_cost <= 32) {
          State work = best_state;
          int wc = best_cost;
          if (intensify(work, wc, n1, n, sig, ms, rng)) {
            best_state = work; best_cost = 0;
            return true;
          }
          if (wc < best_cost) {
            best_state = work; best_cost = wc;
          }
        }
        int kspan = sa.kick_max_k - sa.kick_min_k;
        int k = sa.kick_min_k + uniform_int_distribution<>(0, max(1, kspan))(rng);
        curr = best_state;
        // Structured kick near a basin, random kick when far
        if (best_cost <= 24) structured_kick(curr, n1, n, ms, k, rng);
        else                 kopt_kick(curr, n1, n, ms, k, rng);
        cur_cost = curr.cost(sig, ms);
        temp = sa.initial_temp;
        stall = 0;
        no_improve = 0;
        continue;
      }

      // Adaptive reheat
      if (att_win >= 5000) {
        if ((double)acc_win/att_win < 0.01 && no_improve > sa.reheat_threshold/4) {
          temp = sa.initial_temp * sa.reheat_ratio;
          no_improve = 0;
        }
        acc_win = 0; att_win = 0;
      }
      if (no_improve > sa.reheat_threshold) {
        temp = sa.initial_temp * sa.reheat_ratio;
        no_improve = 0;
      }

      // Choose move type. Compound moves only kick in once we are close to the basin.
      double mt = prob(rng);
      bool use_compound = (best_cost > 0 && best_cost <= sa.ils_threshold);
      double p2 = use_compound ? sa.p_two_flip   : 0.0;
      double p3 = use_compound ? sa.p_three_flip : 0.0;

      if (mt < p3) {
        // 3-flip: pick three distinct positions; tentatively apply, decide together.
        int p1 = pos_dist(rng);
        int q1 = pos_dist(rng); while (q1 == p1) q1 = pos_dist(rng);
        int r1 = pos_dist(rng); while (r1 == p1 || r1 == q1) r1 = pos_dist(rng);
        int s1, i1, s2, i2, s3, i3;
        decode_pos(p1, s1, i1); decode_pos(q1, s2, i2); decode_pos(r1, s3, i3);

        int dc1 = flip_delta(curr, s1, i1, n1, n, sig, ms, delta1);
        flip_apply(curr, s1, i1, ms, delta1);
        int dc2 = flip_delta(curr, s2, i2, n1, n, sig, ms, delta2);
        flip_apply(curr, s2, i2, ms, delta2);
        int dc3 = flip_delta(curr, s3, i3, n1, n, sig, ms, delta3);

        int dcost = dc1 + dc2 + dc3;
        if (dcost < 0 || prob(rng) < exp(-(double)dcost / temp)) {
          flip_apply(curr, s3, i3, ms, delta3);
          cur_cost += dcost;
          acc_win++;
          if (cur_cost < best_cost) {
            best_cost = cur_cost; best_state = curr;
            no_improve = 0; stall = 0;
          }
        } else {
          // Roll back: undo s2 then s1.
          flip_delta(curr, s2, i2, n1, n, sig, ms, dummy);
          flip_apply(curr, s2, i2, ms, dummy);
          flip_delta(curr, s1, i1, n1, n, sig, ms, dummy);
          flip_apply(curr, s1, i1, ms, dummy);
        }
      } else if (mt < p3 + p2) {
        // 2-flip
        int p1 = pos_dist(rng);
        int q1 = pos_dist(rng); while (q1 == p1) q1 = pos_dist(rng);
        int s1, i1, s2, i2;
        decode_pos(p1, s1, i1); decode_pos(q1, s2, i2);

        int dc1 = flip_delta(curr, s1, i1, n1, n, sig, ms, delta1);
        flip_apply(curr, s1, i1, ms, delta1);
        int dc2 = flip_delta(curr, s2, i2, n1, n, sig, ms, delta2);

        int dcost = dc1 + dc2;
        if (dcost < 0 || prob(rng) < exp(-(double)dcost / temp)) {
          flip_apply(curr, s2, i2, ms, delta2);
          cur_cost += dcost;
          acc_win++;
          if (cur_cost < best_cost) {
            best_cost = cur_cost; best_state = curr;
            no_improve = 0; stall = 0;
          }
        } else {
          flip_delta(curr, s1, i1, n1, n, sig, ms, dummy);
          flip_apply(curr, s1, i1, ms, dummy);
        }
      } else {
        // Single flip
        int p = pos_dist(rng);
        int seq_id, idx;
        decode_pos(p, seq_id, idx);

        int dcost = flip_delta(curr, seq_id, idx, n1, n, sig, ms, delta_npaf);

        if (dcost < 0 || prob(rng) < exp(-(double)dcost / temp)) {
          flip_apply(curr, seq_id, idx, ms, delta_npaf);
          cur_cost += dcost;
          acc_win++;
          if (cur_cost < best_cost) {
            best_cost = cur_cost; best_state = curr;
            no_improve = 0; stall = 0;
          }
        }
      }
      temp *= sa.cooling_rate;
    }

    // Post-restart intensification: if we are near a basin, exhaust 2-opt / 3-opt.
    if (best_cost > 0 && best_cost <= 32) {
      State work = best_state;
      int wc = best_cost;
      if (intensify(work, wc, n1, n, sig, ms, rng)) {
        best_state = work; best_cost = 0;
        return true;
      }
      if (wc < best_cost) { best_state = work; best_cost = wc; }
    }
    if (best_cost == 0) return true;
  }

  // Final intensification attempt on whatever we ended with.
  if (best_cost > 0 && best_cost <= 48) {
    State work = best_state;
    int wc = best_cost;
    if (intensify(work, wc, n1, n, sig, ms, rng)) {
      best_state = work; best_cost = 0;
      return true;
    }
    if (wc < best_cost) { best_state = work; best_cost = wc; }
  }
  return best_cost == 0;
}

// =====================================================================
//  Cross-thread champion + disk checkpoint
// =====================================================================
struct Champion {
  int sig_a, sig_b, sig_c, sig_d;
  int A[MAXN], B[MAXN], C[MAXN], D[MAXN];
  int cost, n, n1;
  bool valid;
};

static Champion g_champion_state{};
#ifdef _OPENMP
static omp_lock_t g_champion_lock;
#endif

static void champion_update(const Sig &sig, const State &st, int cost, int n, int n1) {
  if (cost >= g_champion_cost.load(memory_order_relaxed)) return;
#ifdef _OPENMP
  omp_set_lock(&g_champion_lock);
#endif
  if (cost < g_champion_cost.load(memory_order_relaxed)) {
    g_champion_state = {sig.a, sig.b, sig.c, sig.d, {}, {}, {}, {}, cost, n, n1, true};
    memcpy(g_champion_state.A, st.A, sizeof(int)*n1);
    memcpy(g_champion_state.B, st.B, sizeof(int)*n1);
    memcpy(g_champion_state.C, st.C, sizeof(int)*n);
    memcpy(g_champion_state.D, st.D, sizeof(int)*n);
    g_champion_cost.store(cost, memory_order_relaxed);

    ostringstream path;
    path << ".champion_v3_n" << n << ".txt";
    ofstream out(path.str());
    if (out.is_open()) {
      out << n << " " << n1 << " " << cost << "\n"
          << sig.a << " " << sig.b << " " << sig.c << " " << sig.d << "\n";
      for (int i = 0; i < n1; i++) out << st.A[i] << (i+1<n1?" ":"\n");
      for (int i = 0; i < n1; i++) out << st.B[i] << (i+1<n1?" ":"\n");
      for (int i = 0; i < n;  i++) out << st.C[i] << (i+1<n?" ":"\n");
      for (int i = 0; i < n;  i++) out << st.D[i] << (i+1<n?" ":"\n");
    }
  }
#ifdef _OPENMP
  omp_unset_lock(&g_champion_lock);
#endif
}

// =====================================================================
//  main
// =====================================================================
int main(int argc, char **argv) {
  if (argc < 2) { cerr << "Usage: " << argv[0] << " <n> [seed_offset]\n"; return 1; }
  int n = atoi(argv[1]);
  int seed_offset = (argc >= 3) ? atoi(argv[2]) : 0;
  G_N = n; G_N1 = n+1; G_SEED_OFFSET = seed_offset;
  int n1 = n+1, ms = max(n1, n);

#ifdef _OPENMP
  omp_init_lock(&g_champion_lock);
#endif
  int thr = 1;
#ifdef _OPENMP
  thr = omp_get_max_threads();
#endif

  cout << "========================================================\n"
       << "  BS(" << n1 << "," << n << ") v3 joint SA\n"
       << "  [ Threads: " << thr << " | Seed offset: " << seed_offset << " ]\n"
       << "========================================================\n";

  G_T0 = Clock::now();
  auto sigs = get_sigs(n);
  cout << "Loaded " << sigs.size() << " valid signatures.\n";

  // Scale SA parameters with problem size. Smaller iterations + faster
  // stall-trigger so intensification fires often.
  SAParams sa;
  if (n <= 15) {
    sa.iterations = 300000;  sa.restarts = 8;  sa.initial_temp = 15.0;
    sa.kick_min_k = 3; sa.kick_max_k = 6;  sa.ils_threshold = 24;
    sa.kick_after_stall = 25000;
  } else if (n <= 25) {
    sa.iterations = 600000;  sa.restarts = 12; sa.initial_temp = 28.0;
    sa.kick_min_k = 4; sa.kick_max_k = 10; sa.ils_threshold = 40;
    sa.kick_after_stall = 35000;
  } else if (n <= 35) {
    sa.iterations = 800000;  sa.restarts = 20; sa.initial_temp = 40.0;
    sa.kick_min_k = 5; sa.kick_max_k = 14; sa.ils_threshold = 60;
    sa.kick_after_stall = 40000;
  } else {
    sa.iterations = 1200000; sa.restarts = 24; sa.initial_temp = 55.0;
    sa.kick_min_k = 6; sa.kick_max_k = 18; sa.ils_threshold = 90;
    sa.kick_after_stall = 50000;
  }

  vector<atomic<int>> sig_fails(sigs.size());
  for (auto &a : sig_fails) a.store(0, memory_order_relaxed);

  auto last_log = Clock::now();

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
      (uint32_t)tid,  (uint32_t)(tid >> 16),
      (uint32_t)seed_offset, (uint32_t)(seed_offset >> 16),
      (uint32_t)ns,   (uint32_t)(ns >> 32)
    };
    mt19937 rng(seq);

    while (!g_found.load(memory_order_relaxed)) {
      // Signature selection: ε-greedy with fail-count penalty
      int si;
      while (true) {
        si = uniform_int_distribution<>(0, (int)sigs.size()-1)(rng);
        int fails = sig_fails[si].load(memory_order_relaxed);
        if (fails < 3) break;
        double p = 1.0 / (1.0 + 0.3*(fails-2));
        if (uniform_real_distribution<>(0.0,1.0)(rng) < p + 0.1) break;
      }
      auto &sig = sigs[si];

      State best_st;
      bool found = solve_SA(n1, n, sig, best_st, rng, sa);

      int joint_cost = found ? 0 : best_st.cost(sig, ms);
      champion_update(sig, best_st, joint_cost, n, n1);

      if (found) {
        bool valid = true;
        for (int s = 1; s < ms && valid; s++)
          if (npaf_at(best_st.A, best_st.B, n1, best_st.C, best_st.D, n, s) != 0)
            valid = false;
        if (valid && hall_ok(best_st.C, n, best_st.D, n) &&
                     hall_ok(best_st.A, n1, best_st.B, n1)) {
          g_found.store(true);
#pragma omp critical(output)
          {
            cout << "\n*** " << (n >= 44 ? "WORLD RECORD" : "REPRODUCTION CONFIRMED")
                 << ": BS(" << n1 << "," << n << ") FOUND ***\n";
            cout << "sig=(" << sig.a << "," << sig.b << "," << sig.c << "," << sig.d << ")\n";
            cout << "A={"; for (int i=0;i<n1;i++) cout<<best_st.A[i]<<(i<n1-1?",":""); cout<<"};\n";
            cout << "B={"; for (int i=0;i<n1;i++) cout<<best_st.B[i]<<(i<n1-1?",":""); cout<<"};\n";
            cout << "C={"; for (int i=0;i<n; i++) cout<<best_st.C[i]<<(i<n-1? ",":""); cout<<"};\n";
            cout << "D={"; for (int i=0;i<n; i++) cout<<best_st.D[i]<<(i<n-1? ",":""); cout<<"};\n";
            double t = chrono::duration<double>(Clock::now()-G_T0).count();
            cout << "Time: " << t << "s  Seed offset: " << seed_offset << "\n";
          }
        }
      } else {
        sig_fails[si].fetch_add(1, memory_order_relaxed);
      }

      g_epochs_total.fetch_add(1, memory_order_relaxed);

      if (tid == 0) {
        auto now = Clock::now();
        if (chrono::duration<double>(now - last_log).count() > 30.0) {
          last_log = now;
          double t = chrono::duration<double>(now - G_T0).count();
          long long tot = g_epochs_total.load(memory_order_relaxed);
          int ch = g_champion_cost.load(memory_order_relaxed);
          cout << "[" << t << "s] Epochs: " << tot
               << " Speed: " << (t>0 ? tot/t : 0)
               << " Champion cost: " << (ch==INT_MAX ? -1 : ch) << "\n" << flush;
        }
      }
    }
  }

#ifdef _OPENMP
  omp_destroy_lock(&g_champion_lock);
#endif
  return g_found.load() ? 0 : 1;
}
