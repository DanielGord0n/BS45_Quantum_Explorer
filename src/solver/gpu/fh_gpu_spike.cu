// fh_gpu_spike.cu — GPU feasibility spike for the A,B completer (lever 3 of
// docs/n44_search_narrowing_research.md). MEASURE-FIRST: this is an instrument,
// not the production solver. It ports the fh_complete_ab mirror-pair DFS
// (root canon A[0]=B[0]=+1, reversal-canon tie tracking, sum bounds, Dab/Kab
// incremental pruning, node budget) to an ITERATIVE form that runs
//   (a) on the host, single thread  -> CPU baseline (identical logic), and
//   (b) as a CUDA kernel, one candidate per thread -> GPU throughput.
// Deliverable = two numbers: cands/s (1 CPU core) vs cands/s (1 GPU), same
// budget, plus an exact cross-check (verdict histogram + total nodes must
// MATCH between CPU and GPU — the DFS is deterministic).
//
// Deliberately NOT ported (spike scope): WZ_FH_AB_PROF profile constraint.
// The spike measures the architecture speedup; production levers multiply it.
//
// Build (GPU, cluster):  nvcc -O3 -arch=native -o fh_gpu_spike fh_gpu_spike.cu
// Build (CPU-only, anywhere): c++ -O3 -x c++ -DHOST_ONLY -o fh_spike_host fh_gpu_spike.cu
// Run: ./fh_gpu_spike <n> <a> <b> <c> <d> <cand_file> <budget> [max_cands] [cpu_sample]
//   cand_file = WZ_FH_DUMP output ("c c c ... | d d d ...")

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <algorithm>
#include <vector>
#include <chrono>

#ifdef HOST_ONLY
#define HD
#define DEV_CONST
#else
#define HD __host__ __device__
#define DEV_CONST __constant__
#include <cuda_runtime.h>
#endif

#define MAXL 48
#define MAXHALF 24

struct SpikeCfg {
  int n, L, half, absA, absB;
  long long budget;
  int p22pos[8][4], p22neg[8][4], p22mid[4][2];
};

struct CandResult {
  int verdict;          // 0 hit, 1 pre-reject, 2 clean-no, 3 budget-abort
  long long nodes;
  signed char A[MAXL], B[MAXL];
};

// Iterative mirror-pair DFS — logic-identical port of fh_ab_search/fh_complete_ab
// (src/solver/wz_match.cpp ~L810-935), minus the ABP profile constraint.
HD void complete_one(const SpikeCfg &cfg, const signed char *C, const signed char *D,
                     CandResult &out) {
  const int n = cfg.n, L = cfg.L, half = cfg.half;
  short target[MAXL];
  out.verdict = 2; out.nodes = 0;
  // targets + cheap pre-filter (verdict 1), as in fh_complete_ab
  for (int s = 1; s <= n; s++) {
    int cd = 0;
    for (int i = 0; i + s < n; i++) cd += C[i] * C[i + s] + D[i] * D[i + s];
    target[s] = (short)(-cd);
  }
  for (int s = 1; s < L; s++)
    if (target[s] > 2 * (L - s) || -target[s] > 2 * (L - s)) { out.verdict = 1; return; }
  signed char A[MAXL], B[MAXL];
  short Dab[MAXL], Kab[MAXL];
  for (int i = 0; i < L; i++) { A[i] = 0; B[i] = 0; }
  for (int s = 0; s <= n; s++) { Dab[s] = 0; Kab[s] = (short)((s >= 1 && s < L) ? 2 * (L - s) : 0); }
  // explicit stack
  unsigned char kk[MAXHALF + 1];
  signed char tstA[MAXHALF + 1], tcmA[MAXHALF + 1], tstB[MAXHALF + 1], tcmB[MAXHALF + 1];
  short sA[MAXHALF + 1], sB[MAXHALF + 1];
  int d = 0;
  kk[0] = 0; tstA[0] = 1; tcmA[0] = 0; tstB[0] = 1; tcmB[0] = 0; sA[0] = 0; sB[0] = 0;
  long long nodes = 0;
  bool aborted = false;

  auto place = [&](int p, int av, int bv) {
    for (int q = 0; q < L; q++) {
      if (A[q] == 0 || q == p) continue;
      int s = q > p ? q - p : p - q;
      Dab[s] = (short)(Dab[s] + A[q] * av + B[q] * bv);
      Kab[s] = (short)(Kab[s] - 2);
    }
    A[p] = (signed char)av; B[p] = (signed char)bv;
  };
  auto unplace = [&](int p) {
    int av = A[p], bv = B[p];
    A[p] = 0; B[p] = 0;
    for (int q = 0; q < L; q++) {
      if (A[q] == 0 || q == p) continue;
      int s = q > p ? q - p : p - q;
      Dab[s] = (short)(Dab[s] - (A[q] * av + B[q] * bv));
      Kab[s] = (short)(Kab[s] + 2);
    }
  };
  auto leaf_ok = [&](int fsA, int fsB) {
    int aA = fsA < 0 ? -fsA : fsA, aB = fsB < 0 ? -fsB : fsB;
    if (aA != cfg.absA || aB != cfg.absB) return false;
    for (int s = 1; s <= n; s++)
      if (Dab[s] != target[s]) return false;
    return true;
  };

  while (true) {
    if (aborted) { out.verdict = 3; out.nodes = nodes; return; }
    if (d == half) {
      bool found = false;
      if (L % 2 == 1) {
        for (int k = 0; k < 4 && !found; k++) {
          int av = cfg.p22mid[k][0], bv = cfg.p22mid[k][1];
          place(half, av, bv);
          nodes++;
          if (leaf_ok(sA[d] + av, sB[d] + bv)) found = true;
          else unplace(half);
        }
      } else {
        found = leaf_ok(sA[d], sB[d]);
      }
      if (found) {
        out.verdict = 0; out.nodes = nodes;
        for (int i = 0; i < L; i++) { out.A[i] = A[i]; out.B[i] = B[i]; }
        return;
      }
      // backtrack out of the leaf
      d--;
      if (d < 0) { out.verdict = 2; out.nodes = nodes; return; }
      unplace(L - 1 - d); unplace(d);
      kk[d]++;
      continue;
    }
    if (kk[d] >= 8) {
      d--;
      if (d < 0) { out.verdict = 2; out.nodes = nodes; return; }
      unplace(L - 1 - d); unplace(d);
      kk[d]++;
      continue;
    }
    const int (*S)[4] = (d == 0) ? cfg.p22neg : cfg.p22pos;
    int k = kk[d];
    int a1 = S[k][0], b1 = S[k][1], a2 = S[k][2], b2 = S[k][3];
    // root canon: A[0]=B[0]=+1
    if (d == 0 && (a1 != 1 || b1 != 1)) { kk[d]++; continue; }
    // reversal canon tie tracking
    int naT = tstA[d], naC = tcmA[d], nbT = tstB[d], nbC = tcmB[d];
    if (d == 0) { naC = a2; nbC = b2; }
    else {
      if (tstA[d]) {
        int cv = tcmA[d] * a2;
        if (a1 != cv) { if (a1 != 1) { kk[d]++; continue; } naT = 0; }
      }
      if (tstB[d]) {
        int cv = tcmB[d] * b2;
        if (b1 != cv) { if (b1 != 1) { kk[d]++; continue; } nbT = 0; }
      }
    }
    place(d, a1, b1);
    place(L - 1 - d, a2, b2);
    nodes++;
    if (cfg.budget > 0 && nodes > cfg.budget) {
      unplace(L - 1 - d); unplace(d);
      aborted = true;
      continue;
    }
    int nsA = sA[d] + a1 + a2, nsB = sB[d] + b1 + b2;
    int rem = L - 2 * (d + 1);
    int d1 = cfg.absA - nsA; if (d1 < 0) d1 = -d1;
    int d2 = -cfg.absA - nsA; if (d2 < 0) d2 = -d2;
    int d3 = cfg.absB - nsB; if (d3 < 0) d3 = -d3;
    int d4 = -cfg.absB - nsB; if (d4 < 0) d4 = -d4;
    bool prune = !((d1 <= rem || d2 <= rem) && (d3 <= rem || d4 <= rem));
    if (!prune)
      for (int s = 1; s <= n; s++) {
        int diff = target[s] - Dab[s]; if (diff < 0) diff = -diff;
        if (diff > Kab[s]) { prune = true; break; }
      }
    if (prune) {
      unplace(L - 1 - d); unplace(d);
      kk[d]++;
      continue;
    }
    d++;
    kk[d] = 0; tstA[d] = (signed char)naT; tcmA[d] = (signed char)naC;
    tstB[d] = (signed char)nbT; tcmB[d] = (signed char)nbC;
    sA[d] = (short)nsA; sB[d] = (short)nsB;
  }
}

#ifndef HOST_ONLY
DEV_CONST SpikeCfg d_cfg;
__global__ void spike_kernel(const signed char *Cs, const signed char *Ds,
                             int ncand, int n, int *verdicts, long long *nodes) {
  int idx = blockIdx.x * blockDim.x + threadIdx.x;
  if (idx >= ncand) return;
  CandResult r;
  complete_one(d_cfg, Cs + (size_t)idx * n, Ds + (size_t)idx * n, r);
  verdicts[idx] = r.verdict;
  nodes[idx] = r.nodes;
}
#endif
#ifndef HOST_ONLY

// ---- v2 (2026-08-06): WARP-COOPERATIVE kernel — one candidate per WARP.
// Lane 0 owns DFS control flow; all 32 lanes parallelize the O(L) node work
// (fused place/unplace update over shift s, prune scan via ballot). Targets
// the deep-budget divergence that killed thread-per-candidate (5.9x at 5e7).
__global__ void spike_kernel_warp(const signed char *Cs, const signed char *Ds,
                                  int ncand, int n, int *verdicts, long long *nodes,
                                  SpikeCfg cfg) {
  const int wpb = blockDim.x / 32;
  const int warp = blockIdx.x * wpb + (threadIdx.x / 32);
  const int lane = threadIdx.x & 31;
  if (warp >= ncand) return;
  const int L = cfg.L, half = cfg.half;
  extern __shared__ int smem[];
  const int woff = (threadIdx.x / 32) * (4 * MAXL + 8 * MAXHALF + 16);
  int *A   = smem + woff;
  int *B   = A + MAXL;
  int *Dab = B + MAXL;
  int *Kab = Dab + MAXL;
  int *kk  = Kab + MAXL;             // per-depth branch index
  int *sA  = kk + MAXHALF + 2;
  int *sB  = sA + MAXHALF + 2;
  int *tie = sB + MAXHALF + 2;       // packed: tstA|tcmA+2|tstB|tcmB+2 nibbles
  short target[MAXL];
  const signed char *C = Cs + (size_t)warp * n;
  const signed char *D = Ds + (size_t)warp * n;
  // targets + pre-filter (parallel over s, ballot verdict)
  bool pre_bad = false;
  for (int s = 1 + lane; s <= n; s += 32) {
    int cd = 0;
    for (int i = 0; i + s < n; i++) cd += C[i] * C[i + s] + D[i] * D[i + s];
    target[s] = (short)(-cd);
    if (target[s] > 2 * (L - s) || -target[s] > 2 * (L - s)) pre_bad = true;
  }
  // share targets: each lane computed a subset; broadcast via shared Dab scratch
  for (int s = 1 + lane; s <= n; s += 32) Dab[s] = target[s];
  __syncwarp();
  for (int s = 1; s <= n; s++) target[s] = (short)Dab[s];
  if (__any_sync(0xffffffffu, pre_bad)) {
    if (lane == 0) { verdicts[warp] = 1; nodes[warp] = 0; }
    return;
  }
  for (int i = lane; i < L; i += 32) { A[i] = 0; B[i] = 0; }
  for (int s = lane; s <= n; s += 32) {
    Dab[s] = 0;
    Kab[s] = (s >= 1 && s < L) ? 2 * (L - s) : 0;
  }
  __syncwarp();
  long long nd = 0;
  int d = 0, verdict = 2;
  if (lane == 0) { kk[0] = 0; sA[0] = 0; sB[0] = 0; tie[0] = 1 | (0 + 2) << 4 | 1 << 8 | (0 + 2) << 12; }
  __syncwarp();
  auto upd = [&](int p, int av, int bv, int sgn) {
    // fused: for every shift s, both mirror contributions; lanes take disjoint s
    for (int s = 1 + lane; s < L; s += 32) {
      int acc = 0, kacc = 0;
      if (s <= p)     { acc += A[p - s] * av + B[p - s] * bv; kacc += (A[p - s] != 0); }
      if (s < L - p)  { acc += A[p + s] * av + B[p + s] * bv; kacc += (A[p + s] != 0); }
      Dab[s] += sgn * acc;
      Kab[s] -= sgn * 2 * kacc;
    }
    __syncwarp();
    if (lane == 0) { A[p] = (sgn > 0) ? av : 0; B[p] = (sgn > 0) ? bv : 0; }
    __syncwarp();
  };
  while (true) {
    if (d == half) {
      bool found = false;
      if (L % 2 == 1) {
        for (int k = 0; k < 4 && !found; k++) {
          int av = cfg.p22mid[k][0], bv = cfg.p22mid[k][1];
          // place mid (A[p] must be set BEFORE, mirror-loop skips self via A==0)
          upd(half, av, bv, +1);
          nd++;
          int fsA = sA[d] + av, fsB = sB[d] + bv;
          bool bad = (abs(fsA) != cfg.absA || abs(fsB) != cfg.absB);
          bool lb = false;
          for (int s = 1 + lane; s <= n; s += 32) if (Dab[s] != target[s]) lb = true;
          bad |= __any_sync(0xffffffffu, lb);
          if (!bad) found = true;
          else { if (lane == 0) { A[half] = 0; B[half] = 0; } __syncwarp(); upd(half, av, bv, -1); }
        }
      } else {
        int fsA = sA[d], fsB = sB[d];
        bool bad = (abs(fsA) != cfg.absA || abs(fsB) != cfg.absB);
        bool lb = false;
        for (int s = 1 + lane; s <= n; s += 32) if (Dab[s] != target[s]) lb = true;
        found = !(bad | __any_sync(0xffffffffu, lb));
      }
      if (found) { verdict = 0; break; }
      d--;
      if (d < 0) { verdict = 2; break; }
      { int i2 = L - 1 - d, i1 = d;
        int av2 = A[i2], bv2 = B[i2];
        if (lane == 0) { A[i2] = 0; B[i2] = 0; } __syncwarp();
        upd(i2, av2, bv2, -1);
        int av1 = A[i1], bv1 = B[i1];
        if (lane == 0) { A[i1] = 0; B[i1] = 0; } __syncwarp();
        upd(i1, av1, bv1, -1); }
      if (lane == 0) kk[d]++; __syncwarp();
      continue;
    }
    if (kk[d] >= 8) {
      d--;
      if (d < 0) { verdict = 2; break; }
      { int i2 = L - 1 - d, i1 = d;
        int av2 = A[i2], bv2 = B[i2];
        if (lane == 0) { A[i2] = 0; B[i2] = 0; } __syncwarp();
        upd(i2, av2, bv2, -1);
        int av1 = A[i1], bv1 = B[i1];
        if (lane == 0) { A[i1] = 0; B[i1] = 0; } __syncwarp();
        upd(i1, av1, bv1, -1); }
      if (lane == 0) kk[d]++; __syncwarp();
      continue;
    }
    const int (*S)[4] = (d == 0) ? cfg.p22neg : cfg.p22pos;
    int k = kk[d];
    int a1 = S[k][0], b1 = S[k][1], a2 = S[k][2], b2 = S[k][3];
    int t = tie[d];
    int tA = t & 1, cA = ((t >> 4) & 15) - 2, tB = (t >> 8) & 1, cB = ((t >> 12) & 15) - 2;
    bool skip = false;
    int nA = tA, ncA = cA, nB = tB, ncB = cB;
    if (d == 0) {
      if (a1 != 1 || b1 != 1) skip = true;
      ncA = a2; ncB = b2;
    } else {
      if (!skip && tA) { int cv = cA * a2; if (a1 != cv) { if (a1 != 1) skip = true; else nA = 0; } }
      if (!skip && tB) { int cv = cB * b2; if (b1 != cv) { if (b1 != 1) skip = true; else nB = 0; } }
    }
    if (skip) { if (lane == 0) kk[d]++; __syncwarp(); continue; }
    upd(d, a1, b1, +1);
    upd(L - 1 - d, a2, b2, +1);
    nd++;
    if (cfg.budget > 0 && nd > cfg.budget) { verdict = 3; break; }
    int nsA = sA[d] + a1 + a2, nsB = sB[d] + b1 + b2;
    int rem = L - 2 * (d + 1);
    int d1 = abs(cfg.absA - nsA), d2 = abs(-cfg.absA - nsA);
    int d3 = abs(cfg.absB - nsB), d4 = abs(-cfg.absB - nsB);
    bool prune = !((d1 <= rem || d2 <= rem) && (d3 <= rem || d4 <= rem));
    bool lp = false;
    for (int s = 1 + lane; s <= n; s += 32) {
      int diff = target[s] - Dab[s]; if (diff < 0) diff = -diff;
      if (diff > Kab[s]) lp = true;
    }
    prune |= __any_sync(0xffffffffu, lp);
    if (prune) {
      { int i2 = L - 1 - d, i1 = d;
        int av2 = A[i2], bv2 = B[i2];
        if (lane == 0) { A[i2] = 0; B[i2] = 0; } __syncwarp();
        upd(i2, av2, bv2, -1);
        int av1 = A[i1], bv1 = B[i1];
        if (lane == 0) { A[i1] = 0; B[i1] = 0; } __syncwarp();
        upd(i1, av1, bv1, -1); }
      if (lane == 0) kk[d]++; __syncwarp();
      continue;
    }
    if (lane == 0) {
      d++;
      kk[d] = 0; sA[d] = nsA; sB[d] = nsB;
      tie[d] = nA | (ncA + 2) << 4 | nB << 8 | (ncB + 2) << 12;
      d--;
    }
    __syncwarp();
    d++;
    __syncwarp();
  }
  if (lane == 0) { verdicts[warp] = verdict; nodes[warp] = nd; }
}

#endif

static void build_p22(SpikeCfg &cfg) {
  int npos = 0, nneg = 0;
  for (int m = 0; m < 16; m++) {
    int v[4];
    for (int j = 0; j < 4; j++) v[j] = (m >> j & 1) ? 1 : -1;
    if (v[0] * v[1] * v[2] * v[3] == 1) memcpy(cfg.p22pos[npos++], v, sizeof v);
    else                                memcpy(cfg.p22neg[nneg++], v, sizeof v);
  }
  const int m4[4][2] = {{1, 1}, {1, -1}, {-1, 1}, {-1, -1}};
  memcpy(cfg.p22mid, m4, sizeof m4);
}

int main(int argc, char **argv) {
  if (argc < 8) {
    fprintf(stderr, "usage: %s n a b c d cand_file budget [max_cands] [cpu_sample]\n", argv[0]);
    return 1;
  }
  SpikeCfg cfg{};
  cfg.n = atoi(argv[1]); cfg.L = cfg.n + 1; cfg.half = cfg.L / 2;
  cfg.absA = abs(atoi(argv[2])); cfg.absB = abs(atoi(argv[3]));
  cfg.budget = atoll(argv[7]);
  build_p22(cfg);
  int max_cands = argc > 8 ? atoi(argv[8]) : 1000000;
  int cpu_sample = argc > 9 ? atoi(argv[9]) : 500;

  std::vector<signed char> Cs, Ds;
  {
    FILE *f = fopen(argv[6], "r");
    if (!f) { fprintf(stderr, "no cand file\n"); return 1; }
    char line[4096];
    while (fgets(line, sizeof line, f) && (int)(Cs.size() / cfg.n) < max_cands) {
      char *bar = strchr(line, '|');
      if (!bar) continue;
      *bar = 0;
      int v, cnt = 0; char *p = line;
      while (sscanf(p, "%d%n", &v, &cnt) == 1) { Cs.push_back((signed char)v); p += cnt; }
      p = bar + 1;
      while (sscanf(p, "%d%n", &v, &cnt) == 1) { Ds.push_back((signed char)v); p += cnt; }
    }
    fclose(f);
  }
  int ncand = (int)(Cs.size() / cfg.n);
  printf("[spike] n=%d ncand=%d budget=%lld\n", cfg.n, ncand, cfg.budget);

  // ---- CPU baseline: identical logic, single thread, first cpu_sample cands
  int csamp = ncand < cpu_sample ? ncand : cpu_sample;
  long long cpu_nodes = 0; int cpu_hist[4] = {0, 0, 0, 0};
  auto t0 = std::chrono::steady_clock::now();
  for (int i = 0; i < csamp; i++) {
    CandResult r;
    complete_one(cfg, &Cs[(size_t)i * cfg.n], &Ds[(size_t)i * cfg.n], r);
    cpu_hist[r.verdict]++; cpu_nodes += r.nodes;
  }
  double cpu_wall = std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();
  double cpu_rate = csamp / cpu_wall;
  printf("[spike] CPU 1-core: %d cands in %.2fs = %.2f cands/s  (hit=%d pre=%d clean=%d abort=%d nodes=%lld)\n",
         csamp, cpu_wall, cpu_rate, cpu_hist[0], cpu_hist[1], cpu_hist[2], cpu_hist[3], cpu_nodes);

#ifdef HOST_ONLY
  printf("[spike] HOST_ONLY build — GPU pass skipped (this run validates the ported logic).\n");
#else
  signed char *dC, *dD; int *dV; long long *dN;
  cudaMalloc(&dC, Cs.size()); cudaMalloc(&dD, Ds.size());
  cudaMalloc(&dV, ncand * sizeof(int)); cudaMalloc(&dN, ncand * sizeof(long long));
  cudaMemcpy(dC, Cs.data(), Cs.size(), cudaMemcpyHostToDevice);
  cudaMemcpy(dD, Ds.data(), Ds.size(), cudaMemcpyHostToDevice);
  cudaMemcpyToSymbol(d_cfg, &cfg, sizeof cfg);
  int bs = 128, nb = (ncand + bs - 1) / bs;
  cudaDeviceSynchronize();
  t0 = std::chrono::steady_clock::now();
  spike_kernel<<<nb, bs>>>(dC, dD, ncand, cfg.n, dV, dN);
  cudaError_t err = cudaDeviceSynchronize();
  double gpu_wall = std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();
  if (err != cudaSuccess) { printf("[spike] CUDA ERROR: %s\n", cudaGetErrorString(err)); return 2; }
  std::vector<int> hv(ncand); std::vector<long long> hn(ncand);
  cudaMemcpy(hv.data(), dV, ncand * sizeof(int), cudaMemcpyDeviceToHost);
  cudaMemcpy(hn.data(), dN, ncand * sizeof(long long), cudaMemcpyDeviceToHost);
  int ghist[4] = {0, 0, 0, 0}; long long gnodes = 0;
  for (int i = 0; i < ncand; i++) { ghist[hv[i]]++; gnodes += hn[i]; }
  double gpu_rate = ncand / gpu_wall;
  printf("[spike] GPU: %d cands in %.2fs = %.2f cands/s  (hit=%d pre=%d clean=%d abort=%d nodes=%lld)\n",
         ncand, gpu_wall, gpu_rate, ghist[0], ghist[1], ghist[2], ghist[3], gnodes);
  // exact cross-check on the CPU sample
  bool match = true; long long gs_nodes = 0; int gs_hist[4] = {0, 0, 0, 0};
  for (int i = 0; i < csamp; i++) { gs_hist[hv[i]]++; gs_nodes += hn[i]; }
  for (int j = 0; j < 4; j++) if (gs_hist[j] != cpu_hist[j]) match = false;
  if (gs_nodes != cpu_nodes) match = false;
  printf("GPU_SPIKE: cpu_cands_per_s=%.2f gpu_cands_per_s=%.2f speedup_vs_1core=%.1f verdicts_nodes_match=%s\n",
         cpu_rate, gpu_rate, gpu_rate / cpu_rate, match ? "YES" : "NO");

  // ---- v2a: naive kernel, candidates SORTED by flatness (divergence reduction,
  // host-side only). Same set of candidates => histogram must match unsorted.
  {
    std::vector<int> ord(ncand);
    for (int i = 0; i < ncand; i++) ord[i] = i;
    std::vector<long long> score(ncand);
    for (int i = 0; i < ncand; i++) {
      const signed char *C = &Cs[(size_t)i * cfg.n], *D = &Ds[(size_t)i * cfg.n];
      long long sc = 0;
      for (int sh = 1; sh < cfg.n; sh++) {
        int cd = 0;
        for (int j = 0; j + sh < cfg.n; j++) cd += C[j] * C[j + sh] + D[j] * D[j + sh];
        sc += cd < 0 ? -cd : cd;
      }
      score[i] = sc;
    }
    std::sort(ord.begin(), ord.end(), [&](int a, int b){ return score[a] < score[b]; });
    std::vector<signed char> Cs2(Cs.size()), Ds2(Ds.size());
    for (int i = 0; i < ncand; i++) {
      memcpy(&Cs2[(size_t)i * cfg.n], &Cs[(size_t)ord[i] * cfg.n], cfg.n);
      memcpy(&Ds2[(size_t)i * cfg.n], &Ds[(size_t)ord[i] * cfg.n], cfg.n);
    }
    cudaMemcpy(dC, Cs2.data(), Cs2.size(), cudaMemcpyHostToDevice);
    cudaMemcpy(dD, Ds2.data(), Ds2.size(), cudaMemcpyHostToDevice);
    cudaDeviceSynchronize();
    auto ts = std::chrono::steady_clock::now();
    spike_kernel<<<nb, bs>>>(dC, dD, ncand, cfg.n, dV, dN);
    cudaError_t e2 = cudaDeviceSynchronize();
    double w2 = std::chrono::duration<double>(std::chrono::steady_clock::now() - ts).count();
    if (e2 != cudaSuccess) printf("V2A CUDA ERROR: %s\n", cudaGetErrorString(e2));
    else {
      cudaMemcpy(hv.data(), dV, ncand * sizeof(int), cudaMemcpyDeviceToHost);
      int h2[4] = {0,0,0,0};
      for (int i = 0; i < ncand; i++) h2[hv[i]]++;
      bool hm = true;
      for (int j = 0; j < 4; j++) if (h2[j] != ghist[j]) hm = false;
      printf("GPU_SPIKE_V2A_SORTED: cands_per_s=%.2f speedup_vs_1core=%.1f histogram_match=%s\n",
             ncand / w2, (ncand / w2) / cpu_rate, hm ? "YES" : "NO");
    }
  }
  // ---- v2b: WARP-COOPERATIVE kernel (original candidate order).
  {
    cudaMemcpy(dC, Cs.data(), Cs.size(), cudaMemcpyHostToDevice);
    cudaMemcpy(dD, Ds.data(), Ds.size(), cudaMemcpyHostToDevice);
    int tpb = 256, wpb = tpb / 32;
    int nb2 = (ncand + wpb - 1) / wpb;
    size_t shmem = (size_t)wpb * (4 * MAXL + 8 * MAXHALF + 16) * sizeof(int);
    cudaDeviceSynchronize();
    auto tw = std::chrono::steady_clock::now();
    spike_kernel_warp<<<nb2, tpb, shmem>>>(dC, dD, ncand, cfg.n, dV, dN, cfg);
    cudaError_t e3 = cudaDeviceSynchronize();
    double w3 = std::chrono::duration<double>(std::chrono::steady_clock::now() - tw).count();
    if (e3 != cudaSuccess) printf("V2B CUDA ERROR: %s\n", cudaGetErrorString(e3));
    else {
      cudaMemcpy(hv.data(), dV, ncand * sizeof(int), cudaMemcpyDeviceToHost);
      cudaMemcpy(hn.data(), dN, ncand * sizeof(long long), cudaMemcpyDeviceToHost);
      int h3[4] = {0,0,0,0}; long long n3 = 0;
      for (int i = 0; i < ncand; i++) { h3[hv[i]]++; n3 += hn[i]; }
      // exact per-candidate cross-check on the CPU sample
      bool wm = true; long long ws_nodes = 0; int ws_hist[4] = {0,0,0,0};
      for (int i = 0; i < csamp; i++) { ws_hist[hv[i]]++; ws_nodes += hn[i]; }
      for (int j = 0; j < 4; j++) if (ws_hist[j] != cpu_hist[j]) wm = false;
      if (ws_nodes != cpu_nodes) wm = false;
      printf("GPU_SPIKE_V2B_WARP: cands_per_s=%.2f speedup_vs_1core=%.1f verdicts_nodes_match=%s (hist %d/%d/%d/%d nodes=%lld)\n",
             ncand / w3, (ncand / w3) / cpu_rate, wm ? "YES" : "NO", h3[0], h3[1], h3[2], h3[3], n3);
    }
  }
#endif
  return 0;
}
