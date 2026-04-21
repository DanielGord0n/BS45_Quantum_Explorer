/*
 * BS(n+1, n) General-Purpose SA Solver v3
 * CP493 - Directed Research - Daniel Gordon
 *
 * Search strategy:
 *   Alternating SA over the FULL {-1,+1}^(4n+2) manifold, decomposed by
 *   signature (a,b,c,d) — the admissible sum quadruples from Lagrange identity
 *   a^2+b^2+c^2+d^2 = 4n+2 with parity constraints (see get_sigs).
 *
 *   Phase 1: solve_CD_SA — find (C,D) with sum_c=c, sum_d=d, and
 *            |corr_CD[s]| <= 2(n1-s) for every shift (AB-completability).
 *   Phase 2: solve_AB_SA — given CD fixed, find (A,B) with sum_a=a, sum_b=b,
 *            and corr_AB[s] = -corr_CD[s] for every shift (NPAF = 0).
 *
 * Differences from wz_sa_v2:
 *   - No Wang-Zhu pair-product encoding.  Init is random i.i.d. ±1; moves are
 *     single-bit flips + k-opt kicks.  This guarantees the search space
 *     contains every BS solution, not just the Williamson-style subclass.
 *
 * Differences from wz_sa_trillium:
 *   - Signature decomposition (vs. single global NPAF cost)
 *   - Per-thread std::random_device + hi-res seeding (no mt19937 correlation)
 *   - Champion state snapshot on every cost-improving step (trillium dropped this)
 *   - Shared cross-thread champion with on-disk checkpoint (survives SLURM kill)
 *   - k-opt kicks for escape from low-cost plateaus
 *
 * Usage:
 *   ./wz_sa_v3 <n> [seed_offset]
 *
 * Compile:
 *   g++ -O3 -march=native -std=c++17 -fopenmp -o wz_sa_v3 src/solver/wz_sa_v3.cpp
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

static atomic<bool>     g_found{false};
static atomic<int>      g_champion_cost{INT_MAX};
static atomic<long long> g_epochs_total{0};
static int              G_N;
static int              G_N1;
static int              G_SEED_OFFSET = 0;
static Clock::time_point G_T0;

// =====================================================================
//  Signature enumeration
//  a,b,c,d with a^2+b^2+c^2+d^2 = 4n+2 and the Lagrange parity rules.
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
//  Hall polynomial filter (spectral-sum upper bound)
// =====================================================================
static bool hall_ok(const int *X, int xlen, const int *Y, int ylen) {
  double limit = 4.0 * G_N + 2.0;
  for (int j = 1; j <= 200; j++) {
    double th = j * M_PI / 100.0;
    double rx = 0, ix = 0, ry = 0, iy = 0;
    for (int i = 0; i < xlen; i++) { rx += X[i] * cos(i * th); ix += X[i] * sin(i * th); }
    for (int i = 0; i < ylen; i++) { ry += Y[i] * cos(i * th); iy += Y[i] * sin(i * th); }
    if (rx*rx + ix*ix + ry*ry + iy*iy > limit + 0.5) return false;
  }
  return true;
}

static int npaf_at(const int *A, const int *B, int n1,
                   const int *C, const int *D, int n2, int s) {
  int c = 0;
  if (s < n1) for (int i = 0; i < n1 - s; i++) c += A[i]*A[i+s] + B[i]*B[i+s];
  if (s < n2) for (int i = 0; i < n2 - s; i++) c += C[i]*C[i+s] + D[i]*D[i+s];
  return c;
}

// =====================================================================
//  SA parameters
// =====================================================================
struct SAParams {
  double initial_temp     = 60.0;
  double cooling_rate     = 0.99997;
  int    iterations       = 2000000;
  int    restarts         = 6;
  int    reheat_threshold = 150000;
  double reheat_ratio     = 0.55;
  int    kick_after_stall = 50000;
  int    kick_max_k       = 5;
  int    kick_cost_ceiling = 48;  // trigger kick only when best_cost <= this
};

// =====================================================================
//  CD state (C,D of length n)
// =====================================================================
struct CDState {
  int C[128], D[128];
  int sum_c, sum_d;
  int corr[128];

  int cost(int tc, int td, int n1, int n) const {
    int diff_c = abs(sum_c - tc);
    int diff_d = abs(sum_d - td);
    int pen = 0;
    int ms = max(n1, n);
    for (int s = 1; s < ms; s++) {
      int max_ab = (s < n1) ? 2 * (n1 - s) : 0;
      if (abs(corr[s]) > max_ab) pen += abs(corr[s]) - max_ab;
    }
    return diff_c * 4 + diff_d * 4 + pen;
  }
};

static void cd_recompute_corr(CDState &st, int n) {
  memset(st.corr, 0, sizeof(st.corr));
  int ms = max(G_N1, n);
  for (int s = 1; s < ms; s++)
    for (int i = 0; i < n - s; i++)
      st.corr[s] += st.C[i]*st.C[i+s] + st.D[i]*st.D[i+s];
}

static void cd_random_init(CDState &st, int n, mt19937 &rng) {
  uniform_int_distribution<> bit(0, 1);
  st.sum_c = 0; st.sum_d = 0;
  for (int i = 0; i < n; i++) {
    st.C[i] = bit(rng) ? 1 : -1;
    st.D[i] = bit(rng) ? 1 : -1;
    st.sum_c += st.C[i];
    st.sum_d += st.D[i];
  }
  cd_recompute_corr(st, n);
}

// Flip a single bit on channel 0=C or 1=D at position i.
// Returns the delta_corr array through `delta_corr` (caller-provided).
static void cd_flip_apply(CDState &st, int channel, int i, int n,
                          int *delta_corr /* size >= max(n1,n) */) {
  int *arr = (channel == 0) ? st.C : st.D;
  int old = arr[i];
  int nw  = -old;
  int ms = max(G_N1, n);
  for (int s = 1; s < ms; s++) delta_corr[s] = 0;
  for (int s = 1; s < ms; s++) {
    int acc = 0;
    if (i - s >= 0) acc += arr[i - s];
    if (i + s < n)  acc += arr[i + s];
    delta_corr[s] = (nw - old) * acc;       // = -2*old*acc
  }
  arr[i] = nw;
  if (channel == 0) st.sum_c += (nw - old);
  else              st.sum_d += (nw - old);
  for (int s = 1; s < ms; s++) st.corr[s] += delta_corr[s];
}

static void cd_flip_revert(CDState &st, int channel, int i, int n,
                           const int *delta_corr) {
  int *arr = (channel == 0) ? st.C : st.D;
  int nw  = arr[i];
  int old = -nw;
  arr[i] = old;
  if (channel == 0) st.sum_c += (old - nw);
  else              st.sum_d += (old - nw);
  int ms = max(G_N1, n);
  for (int s = 1; s < ms; s++) st.corr[s] -= delta_corr[s];
}

// k-opt kick: flip k random (channel, index) pairs simultaneously.
// Cheaper to apply then recompute corr afterward.
static void cd_kopt_kick(CDState &st, int n, int k, mt19937 &rng) {
  uniform_int_distribution<> ch(0, 1);
  uniform_int_distribution<> pos(0, n - 1);
  for (int f = 0; f < k; f++) {
    int channel = ch(rng);
    int i = pos(rng);
    if (channel == 0) { st.sum_c -= 2 * st.C[i]; st.C[i] = -st.C[i]; }
    else              { st.sum_d -= 2 * st.D[i]; st.D[i] = -st.D[i]; }
  }
  cd_recompute_corr(st, n);
}

static bool solve_CD_SA(int n, int n1, int tc, int td,
                        CDState &best_state, mt19937 &rng,
                        const SAParams &sa) {
  CDState curr;
  cd_random_init(curr, n, rng);

  int current_cost = curr.cost(tc, td, n1, n);
  best_state = curr;
  int best_cost = current_cost;

  uniform_real_distribution<> prob(0.0, 1.0);
  uniform_int_distribution<> ch_dist(0, 1);
  uniform_int_distribution<> pos_dist(0, n - 1);
  int delta_corr[128];

  for (int restart = 0; restart < sa.restarts; restart++) {
    if (g_found.load(memory_order_relaxed)) return false;

    if (restart > 0) {
      cd_random_init(curr, n, rng);
      current_cost = curr.cost(tc, td, n1, n);
    }

    double temp = sa.initial_temp;
    int no_improve = 0;
    int stall_for_kick = 0;
    int accept_win = 0, attempt_win = 0;

    for (int iter = 0; iter < sa.iterations; iter++) {
      if (g_found.load(memory_order_relaxed)) return false;

      if (best_cost == 0) {
        if (hall_ok(best_state.C, n, best_state.D, n)) return true;
        current_cost += 50;
      }

      no_improve++;
      stall_for_kick++;
      attempt_win++;

      if (stall_for_kick > sa.kick_after_stall && best_cost <= sa.kick_cost_ceiling) {
        int k = 2 + uniform_int_distribution<>(0, sa.kick_max_k - 2)(rng);
        curr = best_state;
        cd_kopt_kick(curr, n, k, rng);
        current_cost = curr.cost(tc, td, n1, n);
        temp = sa.initial_temp * 0.7;
        stall_for_kick = 0;
        continue;
      }

      if (attempt_win >= 5000) {
        double acc = (double)accept_win / attempt_win;
        if (acc < 0.01 && no_improve > sa.reheat_threshold / 4) {
          temp = sa.initial_temp * sa.reheat_ratio;
          no_improve = 0;
        }
        accept_win = 0; attempt_win = 0;
      }
      if (no_improve > sa.reheat_threshold) {
        temp = sa.initial_temp * sa.reheat_ratio;
        no_improve = 0;
      }

      int channel = ch_dist(rng);
      int i = pos_dist(rng);
      cd_flip_apply(curr, channel, i, n, delta_corr);

      int new_cost = curr.cost(tc, td, n1, n);
      if (new_cost < current_cost ||
          prob(rng) < exp(-(new_cost - current_cost) / temp)) {
        current_cost = new_cost;
        accept_win++;
        if (new_cost < best_cost) {
          best_cost = new_cost;
          best_state = curr;                 // SNAPSHOT — the trillium bug fix
          no_improve = 0;
          stall_for_kick = 0;
        }
      } else {
        cd_flip_revert(curr, channel, i, n, delta_corr);
      }
      temp *= sa.cooling_rate;
    }

    if (best_cost == 0 && hall_ok(best_state.C, n, best_state.D, n)) return true;
  }
  return best_cost == 0 && hall_ok(best_state.C, n, best_state.D, n);
}

// =====================================================================
//  AB state (A,B of length n1)
// =====================================================================
struct ABState {
  int A[128], B[128];
  int sum_a, sum_b;
  int corr[128];

  int cost(int ta, int tb, int n1, const int *cd_full) const {
    int diff_a = abs(sum_a - ta);
    int diff_b = abs(sum_b - tb);
    int pen = 0;
    int ms = max(n1, G_N);
    for (int s = 1; s < ms; s++) {
      if (corr[s] + cd_full[s] != 0) pen += abs(corr[s] + cd_full[s]);
    }
    return diff_a * 4 + diff_b * 4 + pen;
  }
};

static void ab_recompute_corr(ABState &st, int n1) {
  memset(st.corr, 0, sizeof(st.corr));
  int ms = max(n1, G_N);
  for (int s = 1; s < ms; s++)
    for (int i = 0; i < n1 - s; i++)
      st.corr[s] += st.A[i]*st.A[i+s] + st.B[i]*st.B[i+s];
}

static void ab_random_init(ABState &st, int n1, mt19937 &rng) {
  uniform_int_distribution<> bit(0, 1);
  st.sum_a = 0; st.sum_b = 0;
  for (int i = 0; i < n1; i++) {
    st.A[i] = bit(rng) ? 1 : -1;
    st.B[i] = bit(rng) ? 1 : -1;
    st.sum_a += st.A[i];
    st.sum_b += st.B[i];
  }
  ab_recompute_corr(st, n1);
}

static void ab_flip_apply(ABState &st, int channel, int i, int n1,
                          int *delta_corr) {
  int *arr = (channel == 0) ? st.A : st.B;
  int old = arr[i];
  int nw  = -old;
  int ms = max(n1, G_N);
  for (int s = 1; s < ms; s++) delta_corr[s] = 0;
  for (int s = 1; s < ms; s++) {
    int acc = 0;
    if (i - s >= 0) acc += arr[i - s];
    if (i + s < n1) acc += arr[i + s];
    delta_corr[s] = (nw - old) * acc;
  }
  arr[i] = nw;
  if (channel == 0) st.sum_a += (nw - old);
  else              st.sum_b += (nw - old);
  for (int s = 1; s < ms; s++) st.corr[s] += delta_corr[s];
}

static void ab_flip_revert(ABState &st, int channel, int i, int n1,
                           const int *delta_corr) {
  int *arr = (channel == 0) ? st.A : st.B;
  int nw  = arr[i];
  int old = -nw;
  arr[i] = old;
  if (channel == 0) st.sum_a += (old - nw);
  else              st.sum_b += (old - nw);
  int ms = max(n1, G_N);
  for (int s = 1; s < ms; s++) st.corr[s] -= delta_corr[s];
}

static void ab_kopt_kick(ABState &st, int n1, int k, mt19937 &rng) {
  uniform_int_distribution<> ch(0, 1);
  uniform_int_distribution<> pos(0, n1 - 1);
  for (int f = 0; f < k; f++) {
    int channel = ch(rng);
    int i = pos(rng);
    if (channel == 0) { st.sum_a -= 2 * st.A[i]; st.A[i] = -st.A[i]; }
    else              { st.sum_b -= 2 * st.B[i]; st.B[i] = -st.B[i]; }
  }
  ab_recompute_corr(st, n1);
}

static bool solve_AB_SA(int n1, int ta, int tb, const int *cd_full,
                        ABState &best_state, mt19937 &rng,
                        const SAParams &sa) {
  ABState curr;
  ab_random_init(curr, n1, rng);

  int current_cost = curr.cost(ta, tb, n1, cd_full);
  best_state = curr;
  int best_cost = current_cost;

  uniform_real_distribution<> prob(0.0, 1.0);
  uniform_int_distribution<> ch_dist(0, 1);
  uniform_int_distribution<> pos_dist(0, n1 - 1);
  int delta_corr[128];

  for (int restart = 0; restart < sa.restarts; restart++) {
    if (g_found.load(memory_order_relaxed)) return false;

    if (restart > 0) {
      ab_random_init(curr, n1, rng);
      current_cost = curr.cost(ta, tb, n1, cd_full);
    }

    double temp = sa.initial_temp;
    int no_improve = 0;
    int stall_for_kick = 0;
    int accept_win = 0, attempt_win = 0;

    for (int iter = 0; iter < sa.iterations; iter++) {
      if (g_found.load(memory_order_relaxed)) return false;
      if (best_cost == 0) return true;

      no_improve++;
      stall_for_kick++;
      attempt_win++;

      if (stall_for_kick > sa.kick_after_stall && best_cost <= sa.kick_cost_ceiling) {
        int k = 2 + uniform_int_distribution<>(0, sa.kick_max_k - 2)(rng);
        curr = best_state;
        ab_kopt_kick(curr, n1, k, rng);
        current_cost = curr.cost(ta, tb, n1, cd_full);
        temp = sa.initial_temp * 0.7;
        stall_for_kick = 0;
        continue;
      }

      if (attempt_win >= 5000) {
        double acc = (double)accept_win / attempt_win;
        if (acc < 0.01 && no_improve > sa.reheat_threshold / 4) {
          temp = sa.initial_temp * sa.reheat_ratio;
          no_improve = 0;
        }
        accept_win = 0; attempt_win = 0;
      }
      if (no_improve > sa.reheat_threshold) {
        temp = sa.initial_temp * sa.reheat_ratio;
        no_improve = 0;
      }

      int channel = ch_dist(rng);
      int i = pos_dist(rng);
      ab_flip_apply(curr, channel, i, n1, delta_corr);

      int new_cost = curr.cost(ta, tb, n1, cd_full);
      if (new_cost < current_cost ||
          prob(rng) < exp(-(new_cost - current_cost) / temp)) {
        current_cost = new_cost;
        accept_win++;
        if (new_cost < best_cost) {
          best_cost = new_cost;
          best_state = curr;                 // SNAPSHOT
          no_improve = 0;
          stall_for_kick = 0;
        }
      } else {
        ab_flip_revert(curr, channel, i, n1, delta_corr);
      }
      temp *= sa.cooling_rate;
    }
    if (best_cost == 0) return true;
  }
  return best_cost == 0;
}

// =====================================================================
//  Cross-thread champion (best partial result so far) + disk checkpoint
// =====================================================================
struct Champion {
  int sig_a, sig_b, sig_c, sig_d;
  int A[128], B[128], C[128], D[128];
  int cost;
  int n, n1;
  bool valid;
};

static Champion g_champion_state{};
#ifdef _OPENMP
static omp_lock_t g_champion_lock;
#endif

static void champion_update(const Sig &sig, const ABState *ab, const CDState *cd,
                            int cost, int n, int n1) {
  int prev = g_champion_cost.load(memory_order_relaxed);
  if (cost >= prev) return;
#ifdef _OPENMP
  omp_set_lock(&g_champion_lock);
#endif
  if (cost < g_champion_cost.load(memory_order_relaxed)) {
    g_champion_state.sig_a = sig.a;
    g_champion_state.sig_b = sig.b;
    g_champion_state.sig_c = sig.c;
    g_champion_state.sig_d = sig.d;
    g_champion_state.cost = cost;
    g_champion_state.n = n;
    g_champion_state.n1 = n1;
    g_champion_state.valid = true;
    if (ab) {
      memcpy(g_champion_state.A, ab->A, sizeof(int) * n1);
      memcpy(g_champion_state.B, ab->B, sizeof(int) * n1);
    }
    if (cd) {
      memcpy(g_champion_state.C, cd->C, sizeof(int) * n);
      memcpy(g_champion_state.D, cd->D, sizeof(int) * n);
    }
    g_champion_cost.store(cost, memory_order_relaxed);

    ostringstream path;
    path << ".champion_v3_n" << n << ".txt";
    ofstream out(path.str());
    if (out.is_open()) {
      out << n << " " << n1 << " " << cost << "\n";
      out << sig.a << " " << sig.b << " " << sig.c << " " << sig.d << "\n";
      if (ab) {
        for (int i = 0; i < n1; i++) out << ab->A[i] << (i + 1 < n1 ? " " : "\n");
        for (int i = 0; i < n1; i++) out << ab->B[i] << (i + 1 < n1 ? " " : "\n");
      } else {
        for (int i = 0; i < n1; i++) out << 0 << (i + 1 < n1 ? " " : "\n");
        for (int i = 0; i < n1; i++) out << 0 << (i + 1 < n1 ? " " : "\n");
      }
      if (cd) {
        for (int i = 0; i < n; i++) out << cd->C[i] << (i + 1 < n ? " " : "\n");
        for (int i = 0; i < n; i++) out << cd->D[i] << (i + 1 < n ? " " : "\n");
      } else {
        for (int i = 0; i < n; i++) out << 0 << (i + 1 < n ? " " : "\n");
        for (int i = 0; i < n; i++) out << 0 << (i + 1 < n ? " " : "\n");
      }
    }
  }
#ifdef _OPENMP
  omp_unset_lock(&g_champion_lock);
#endif
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
  G_N = n;
  G_N1 = n + 1;
  G_SEED_OFFSET = seed_offset;
  int n1 = n + 1;
  int ms = max(n1, n);

#ifdef _OPENMP
  omp_init_lock(&g_champion_lock);
#endif

  int thr = 1;
#ifdef _OPENMP
  thr = omp_get_max_threads();
  omp_set_num_threads(thr);
#endif

  cout << "========================================================" << endl;
  cout << "  BS(" << n1 << "," << n << ") v3 — General SA (unconstrained)" << endl;
  cout << "  [ Threads: " << thr << " | Seed offset: " << seed_offset << " ]" << endl;
  cout << "========================================================" << endl;

  G_T0 = Clock::now();
  auto sigs = get_sigs(n);
  cout << "Loaded " << sigs.size() << " valid sum signatures." << endl;

  vector<atomic<int>> sig_fails(sigs.size());
  for (auto &a : sig_fails) a.store(0, memory_order_relaxed);

  // Scale SA parameters with problem size so small cases finish fast.
  SAParams sa_base;
  if (n <= 15)      { sa_base.iterations = 400000;  sa_base.restarts = 4; }
  else if (n <= 25) { sa_base.iterations = 800000;  sa_base.restarts = 5; }
  else if (n <= 35) { sa_base.iterations = 1500000; sa_base.restarts = 6; }
  else              { sa_base.iterations = 2500000; sa_base.restarts = 8; }

  auto last_log = Clock::now();

#pragma omp parallel
  {
    int tid = 0;
#ifdef _OPENMP
    tid = omp_get_thread_num();
#endif

    std::random_device rd;
    uint64_t ns_epoch = chrono::high_resolution_clock::now().time_since_epoch().count();
    std::seed_seq seq{
      (uint32_t)rd(), (uint32_t)rd(), (uint32_t)rd(), (uint32_t)rd(),
      (uint32_t)tid, (uint32_t)(tid >> 16),
      (uint32_t)seed_offset, (uint32_t)(seed_offset >> 16),
      (uint32_t)ns_epoch, (uint32_t)(ns_epoch >> 32)
    };
    mt19937 rng(seq);

    while (!g_found.load(memory_order_relaxed)) {
      int si;
      while (true) {
        si = uniform_int_distribution<>(0, (int)sigs.size() - 1)(rng);
        int fails = sig_fails[si].load(memory_order_relaxed);
        if (fails < 3) break;
        double p_accept = 1.0 / (1.0 + 0.3 * (fails - 2));
        if (uniform_real_distribution<>(0.0, 1.0)(rng) < p_accept + 0.15) break;
      }
      auto &sig = sigs[si];

      CDState best_cd;
      bool found_cd = solve_CD_SA(n, n1, sig.c, sig.d, best_cd, rng, sa_base);

      if (!found_cd) {
        sig_fails[si].fetch_add(1, memory_order_relaxed);
        int cd_only_cost = best_cd.cost(sig.c, sig.d, n1, n);
        champion_update(sig, nullptr, &best_cd, cd_only_cost + 1000, n, n1);
        g_epochs_total.fetch_add(1, memory_order_relaxed);
        goto log_check;
      }

      {
        int cd_full[128] = {0};
        for (int s = 1; s < ms; s++)
          for (int k = 0; k < n - s; k++)
            cd_full[s] += best_cd.C[k]*best_cd.C[k+s] + best_cd.D[k]*best_cd.D[k+s];

        ABState best_ab;
        bool found_ab = solve_AB_SA(n1, sig.a, sig.b, cd_full, best_ab, rng, sa_base);

        int joint_cost = found_ab ? 0 : best_ab.cost(sig.a, sig.b, n1, cd_full);
        champion_update(sig, &best_ab, &best_cd, joint_cost, n, n1);

        if (found_ab) {
          bool valid = true;
          for (int s = 1; s < ms && valid; s++) {
            if (npaf_at(best_ab.A, best_ab.B, n1, best_cd.C, best_cd.D, n, s) != 0)
              valid = false;
          }
          if (valid && hall_ok(best_cd.C, n, best_cd.D, n) &&
              hall_ok(best_ab.A, n1, best_ab.B, n1)) {
            g_found.store(true);
#pragma omp critical(output)
            {
              if (n >= 44)
                cout << "\n*** WORLD RECORD DISCOVERY: BS(" << n1 << "," << n << ") FOUND ***\n" << endl;
              else
                cout << "\n*** REPRODUCTION CONFIRMED: BS(" << n1 << "," << n << ") FOUND ***\n" << endl;
              cout << "sig = (" << sig.a << "," << sig.b << "," << sig.c << "," << sig.d << ")" << endl;
              cout << "A = {";
              for (int i = 0; i < n1; i++) cout << best_ab.A[i] << (i < n1 - 1 ? "," : "");
              cout << "};" << endl;
              cout << "B = {";
              for (int i = 0; i < n1; i++) cout << best_ab.B[i] << (i < n1 - 1 ? "," : "");
              cout << "};" << endl;
              cout << "C = {";
              for (int i = 0; i < n; i++) cout << best_cd.C[i] << (i < n - 1 ? "," : "");
              cout << "};" << endl;
              cout << "D = {";
              for (int i = 0; i < n; i++) cout << best_cd.D[i] << (i < n - 1 ? "," : "");
              cout << "};" << endl;
              double t = chrono::duration<double>(Clock::now() - G_T0).count();
              cout << "\nTime: " << t << "s" << endl;
              cout << "Seed offset: " << seed_offset << endl;
            }
          }
        } else {
          sig_fails[si].fetch_add(1, memory_order_relaxed);
        }
      }
      g_epochs_total.fetch_add(1, memory_order_relaxed);

    log_check:
      if (tid == 0) {
        auto now = Clock::now();
        if (chrono::duration<double>(now - last_log).count() > 30.0) {
          last_log = now;
          double t = chrono::duration<double>(now - G_T0).count();
          long long total = g_epochs_total.load(memory_order_relaxed);
          double speed = (t > 0) ? (total / t) : 0.0;
          int champ = g_champion_cost.load(memory_order_relaxed);
          cout << "[" << t << "s] Epochs: " << total
               << " Speed: " << speed
               << " Champion cost: " << (champ == INT_MAX ? -1 : champ)
               << "\n" << flush;
        }
      }
    }
  }

#ifdef _OPENMP
  omp_destroy_lock(&g_champion_lock);
#endif
  return g_found.load() ? 0 : 1;
}
