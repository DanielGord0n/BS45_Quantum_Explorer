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
#endif
  return 0;
}
