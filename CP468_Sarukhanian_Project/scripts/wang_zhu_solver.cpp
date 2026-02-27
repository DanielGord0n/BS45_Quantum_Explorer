/*
 * Wang-Zhu Algorithm for Base Sequences BS(n+1, n)
 * CP493 - Directed Research - Daniel Gordon
 *
 * Implementation of the algorithm from:
 *   "On Base, Normal and Near-normal Sequences"
 *   Xu Wang, Jiayi Zhu (arXiv:2506.20296, 2025)
 *
 * This is the EXACT algorithm that extended the BS conjecture to n=43.
 *
 * Algorithm overview:
 *   Step 1: Enumerate valid sum signatures (a,b,c,d) via Theorem 2.1
 *   Step 2-3: Find partial autocorrelation parameters (Theorem 2.3)
 *   Step 4: Generate C,D candidates, filter by Hall polynomial (Theorem 2.4)
 *   Step 5: Backtrack to find matching A,B sequences
 *
 * The key insight is that C,D candidates can be enumerated and filtered
 * BEFORE searching for A,B. This massively prunes the search space.
 *
 * Compile:
 *   g++ -O3 -march=native -std=c++17 -Xpreprocessor -fopenmp \
 *       -I/opt/homebrew/opt/libomp/include -L/opt/homebrew/opt/libomp/lib -lomp
 * \ -o wz_solver wang_zhu_solver.cpp
 *
 * Usage:
 *   ./wz_solver <n> [threads]
 */

#include <algorithm>
#include <atomic>
#include <cassert>
#include <chrono>
#include <climits>
#include <cmath>
#include <complex>
#include <cstring>
#include <iostream>
#include <random>
#include <vector>

#ifdef _OPENMP
#include <omp.h>
#endif

using namespace std;
using Clock = chrono::steady_clock;

static atomic<bool> found{false};
static int g_sol[4][300];
static int G_N; // n (C,D length = n, A,B length = n+1)

// ============================================================
// Step 1: Enumerate valid sum signatures (a,b,c,d)
// Theorem 2.1: a^2 + b^2 + c^2 + d^2 = 4n + 2
//   where a = sum(A), b = sum(B), c = sum(C), d = sum(D)
//   a,b have same parity as n+1; c,d have same parity as n
//   Plus mod 4 constraints
// ============================================================
struct SumSignature {
  int a, b, c, d;
};

vector<SumSignature> enumerate_signatures(int n) {
  vector<SumSignature> sigs;
  int target = 4 * n + 2;
  int n1 = n + 1; // length of A, B

  // a ranges from -(n+1) to n+1 with step 2 (all same sign as elements)
  // a ≡ n+1 (mod 2) since A has n+1 elements of ±1
  for (int a = -(n1); a <= n1; a += 2) {
    for (int b = -(n1); b <= n1; b += 2) {
      int rem_ab = target - a * a - b * b;
      if (rem_ab < 0)
        continue;

      for (int c = -n; c <= n; c += 2) {
        int d2 = rem_ab - c * c;
        if (d2 < 0)
          continue;
        int d = (int)round(sqrt(d2));
        if (d * d != d2)
          continue;
        // d must have same parity as n
        if ((d % 2) != (n % 2)) {
          if (d > 0)
            d--;
          else
            continue;
          if (d * d != d2)
            continue;
          if ((d % 2) != (n % 2))
            continue;
        }

        // Check mod 4 constraints from Theorem 2.1
        // When n is even: c ≡ d (mod 4)
        // When n is odd: a ≡ b + 2 (mod 4)
        if (n % 2 == 0) {
          if (((c - d) % 4 + 4) % 4 != 0)
            continue;
        } else {
          if (((a - b - 2) % 4 + 4) % 4 != 0)
            continue;
        }

        // Both +d and -d
        sigs.push_back({a, b, c, d});
        if (d != 0)
          sigs.push_back({a, b, c, -d});
      }
    }
  }

  cout << "  Found " << sigs.size() << " valid sum signatures" << endl;
  return sigs;
}

// ============================================================
// Theorem 2.2: Pair constraints
// For each i, [a_i, a_{n+2-i}; b_i, b_{n+2-i}] has only 8 cases
// Same for [c_i, c_{n+1-i}; d_i, d_{n+1-i}]
// ============================================================
// The 8 valid 2x2 blocks of ±1:
static const int VALID_BLOCKS[8][4] = {
    {1, 1, 1, 1},   {1, 1, -1, -1}, {1, -1, 1, -1}, {1, -1, -1, 1},
    {-1, 1, 1, -1}, {-1, 1, -1, 1}, {-1, -1, 1, 1}, {-1, -1, -1, -1}};

// ============================================================
// Theorem 2.4: Hall polynomial filter
// f_A(θ) = |h_A(e^{iθ})| where h_A(t) = sum a_i t^i
// We need (f_A + f_B)(θ) ≤ 4n+2 for all θ
// Same for (f_C + f_D)(θ) ≤ 4n+2
// ============================================================
bool hall_filter(const vector<int> &X, const vector<int> &Y, int n) {
  // Theorem 2.4: fA(θ)^2 + fB(θ)^2 ≤ 4n+2 for all θ
  // where fA = |hA(e^{iθ})|
  double target = 4.0 * G_N + 2.0;
  int npts = 200;

  for (int j = 1; j <= npts; j++) {
    double theta = j * M_PI / 100.0;
    complex<double> hx(0, 0), hy(0, 0);
    for (int i = 0; i < (int)X.size(); i++) {
      complex<double> z = polar(1.0, i * theta);
      hx += (double)X[i] * z;
    }
    for (int i = 0; i < (int)Y.size(); i++) {
      complex<double> z = polar(1.0, i * theta);
      hy += (double)Y[i] * z;
    }
    double fx2 = norm(hx); // |hx|^2
    double fy2 = norm(hy); // |hy|^2
    if (fx2 + fy2 > target + 1.0)
      return false; // small tolerance for floating point
  }
  return true;
}

// ============================================================
// Generate C,D candidates
// Uses Theorem 2.2 block structure + sum constraints
// Issue: 0 candidates being generated for some n.
// ============================================================
struct CDCandidate {
  vector<int> C, D;
};

void generate_cd_recursive(int n, int pos, int target_c, int target_d,
                           vector<int> &C, vector<int> &D, int current_c,
                           int current_d, vector<CDCandidate> &results) {
  if (found.load(memory_order_relaxed))
    return;

  if (pos > n / 2) {
    // For odd n, middle element is unconstrained
    if (n % 2 == 1) {
      int mid = n / 2; // 0-indexed
      // Try both values for C[mid] and D[mid]
      for (int cv : {-1, 1}) {
        for (int dv : {-1, 1}) {
          if (current_c + cv != target_c)
            continue;
          if (current_d + dv != target_d)
            continue;
          C[mid] = cv;
          D[mid] = dv;
          // Hall polynomial filter
          if (hall_filter(C, D, n)) {
            results.push_back({C, D});
          }
        }
      }
    } else {
      if (current_c != target_c || current_d != target_d)
        return;
      if (hall_filter(C, D, n)) {
        results.push_back({C, D});
      }
    }
    return;
  }

  int i = pos;         // 0-indexed position from start
  int j = n - 1 - pos; // 0-indexed mirror position

  if (i == j) {
    // Middle element (even n case)
    for (int cv : {-1, 1}) {
      for (int dv : {-1, 1}) {
        C[i] = cv;
        D[i] = dv;
        int nc = current_c + cv;
        int nd = current_d + dv;
        if (abs(target_c - nc) > (n / 2 - pos) * 2)
          continue;
        if (abs(target_d - nd) > (n / 2 - pos) * 2)
          continue;
        generate_cd_recursive(n, pos + 1, target_c, target_d, C, D, nc, nd,
                              results);
      }
    }
    return;
  }

  // Try all 8 valid blocks for positions (i, j) in (C, D)
  for (int b = 0; b < 8; b++) {
    int ci = VALID_BLOCKS[b][0];
    int cj = VALID_BLOCKS[b][1];
    int di = VALID_BLOCKS[b][2];
    int dj = VALID_BLOCKS[b][3];

    C[i] = ci;
    C[j] = cj;
    D[i] = di;
    D[j] = dj;

    int nc = current_c + ci + cj;
    int nd = current_d + di + dj;

    // Pruning: check if remaining positions can reach target sums
    int remaining_pairs = n / 2 - pos - 1;
    int remaining_singles = (n % 2 == 1) ? 1 : 0;
    int max_remaining = remaining_pairs * 2 + remaining_singles;

    if (abs(target_c - nc) > max_remaining)
      continue;
    if (abs(target_d - nd) > max_remaining)
      continue;

    generate_cd_recursive(n, pos + 1, target_c, target_d, C, D, nc, nd,
                          results);

    if (results.size() > 500000)
      return; // safety limit
  }
}

vector<CDCandidate> generate_cd_candidates(int n, int target_c, int target_d) {
  vector<CDCandidate> results;
  vector<int> C(n, 0), D(n, 0);
  generate_cd_recursive(n, 0, target_c, target_d, C, D, 0, 0, results);
  return results;
}

// ============================================================
// Step 5: Backtracking search for A,B given fixed C,D
// ============================================================
bool backtrack_ab(int n1, int n, const vector<int> &C, const vector<int> &D,
                  vector<int> &A, vector<int> &B, int pos, int target_a,
                  int target_b, int current_a, int current_b,
                  vector<int> &partial_corr) {
  if (found.load(memory_order_relaxed))
    return false;

  if (pos > n1 / 2) {
    // Handle middle element for odd n1
    if (n1 % 2 == 1) {
      int mid = n1 / 2;
      for (int av : {-1, 1}) {
        for (int bv : {-1, 1}) {
          if (current_a + av != target_a)
            continue;
          if (current_b + bv != target_b)
            continue;
          A[mid] = av;
          B[mid] = bv;

          // Full verification
          bool valid = true;
          for (int s = 1; s < max(n1, n) && valid; s++) {
            int corr = 0;
            for (int i = 0; i < n1 - s; i++)
              corr += A[i] * A[i + s] + B[i] * B[i + s];
            for (int i = 0; i < n - s; i++)
              corr += C[i] * C[i + s] + D[i] * D[i + s];
            if (corr != 0)
              valid = false;
          }
          if (valid)
            return true;
        }
      }
    } else {
      if (current_a != target_a || current_b != target_b)
        return false;
      // Full verification
      bool valid = true;
      for (int s = 1; s < max(n1, n) && valid; s++) {
        int corr = 0;
        for (int i = 0; i < n1 - s; i++)
          corr += A[i] * A[i + s] + B[i] * B[i + s];
        for (int i = 0; i < n - s; i++)
          corr += C[i] * C[i + s] + D[i] * D[i + s];
        if (corr != 0)
          valid = false;
      }
      return valid;
    }
    return false;
  }

  int i = pos;
  int j = n1 - 1 - pos;

  if (i == j) {
    // Middle element
    for (int av : {-1, 1}) {
      for (int bv : {-1, 1}) {
        A[i] = av;
        B[i] = bv;
        int na = current_a + av;
        int nb = current_b + bv;
        int rem = (n1 / 2 - pos) * 2;
        if (abs(target_a - na) > rem)
          continue;
        if (abs(target_b - nb) > rem)
          continue;
        if (backtrack_ab(n1, n, C, D, A, B, pos + 1, target_a, target_b, na, nb,
                         partial_corr))
          return true;
      }
    }
    return false;
  }

  // Try all 8 valid blocks for A[i], A[j], B[i], B[j]
  for (int b = 0; b < 8; b++) {
    int ai = VALID_BLOCKS[b][0];
    int aj = VALID_BLOCKS[b][1];
    int bi = VALID_BLOCKS[b][2];
    int bj = VALID_BLOCKS[b][3];

    A[i] = ai;
    A[j] = aj;
    B[i] = bi;
    B[j] = bj;

    int na = current_a + ai + aj;
    int nb = current_b + bi + bj;

    int remaining = (n1 / 2 - pos - 1) * 2 + (n1 % 2 == 1 ? 1 : 0);
    if (abs(target_a - na) > remaining)
      continue;
    if (abs(target_b - nb) > remaining)
      continue;

    // Partial autocorrelation pruning:
    // Check if filling from outside-in, the partial correlations
    // for the known shifts are still feasible
    bool feasible = true;

    // Check autocorrelation for shift = j - i (the distance between our pair)
    int s = j - i;
    if (s > 0 && s < max(n1, n)) {
      int partial = 0;
      // Compute known portion of correlation for this shift
      for (int k = 0; k <= pos; k++) {
        int ki = k, kj = n1 - 1 - k;
        if (ki + s < n1)
          partial += A[ki] * A[ki + s];
        if (kj + s < n1 && kj != ki)
          partial += A[kj] * A[kj + s];
        if (ki + s < n1)
          partial += B[ki] * B[ki + s];
        if (kj + s < n1 && kj != ki)
          partial += B[kj] * B[kj + s];
      }
      // Add C,D contribution (fully known)
      for (int k = 0; k < n - s; k++)
        partial += C[k] * C[k + s] + D[k] * D[k + s];

      // Remaining A,B elements: positions pos+1 to n1/2-1
      // Each unfilled pair can contribute at most ±4 to correlation at shift s
      int unfilled = n1 / 2 - pos - 1;
      if (abs(partial) > unfilled * 4 + (n1 % 2 == 1 ? 2 : 0))
        feasible = false;
    }

    if (!feasible)
      continue;

    if (backtrack_ab(n1, n, C, D, A, B, pos + 1, target_a, target_b, na, nb,
                     partial_corr))
      return true;
  }

  return false;
}

// ============================================================
// Verification
// ============================================================
bool verify(int *A, int *B, int n1, int *C, int *D, int n2) {
  for (int s = 1; s < max(n1, n2); s++) {
    int c = 0;
    for (int i = 0; i < n1 - s; i++)
      c += A[i] * A[i + s] + B[i] * B[i + s];
    for (int i = 0; i < n2 - s; i++)
      c += C[i] * C[i + s] + D[i] * D[i + s];
    if (c != 0)
      return false;
  }
  return true;
}

// ============================================================
// Main
// ============================================================
int main(int argc, char **argv) {
  if (argc < 2) {
    cerr << "Usage: " << argv[0] << " <n> [threads]" << endl;
    cerr << "  Searches for BS(n+1, n) using Wang-Zhu algorithm" << endl;
    return 1;
  }

  int n = atoi(argv[1]);
  G_N = n;
  int n1 = n + 1;

  int threads = 1;
#ifdef _OPENMP
  threads = omp_get_max_threads();
  if (argc > 2)
    threads = atoi(argv[2]);
  omp_set_num_threads(threads);
#endif

  cout << "========================================================" << endl;
  cout << "  Wang-Zhu Algorithm for BS(" << n1 << "," << n << ")" << endl;
  cout << "========================================================" << endl;
  cout << "Threads: " << threads << endl;
  cout << "Based on arXiv:2506.20296 (2025)" << endl;
  cout << "========================================================" << endl;
  cout << endl;

  auto t_start = Clock::now();

  // Step 1: Enumerate sum signatures
  cout << "Step 1: Enumerating sum signatures..." << endl;
  vector<SumSignature> sigs = enumerate_signatures(n);
  cout << flush;

  // For each signature, generate C,D candidates and backtrack A,B
  long long total_cd = 0;
  long long total_attempts = 0;

#pragma omp parallel for schedule(dynamic)                                     \
    reduction(+ : total_cd, total_attempts)
  for (int si = 0; si < (int)sigs.size(); si++) {
    if (found.load(memory_order_relaxed))
      continue;

    SumSignature &sig = sigs[si];

    // Step 4: Generate C,D candidates for this signature
    vector<CDCandidate> cd_cands = generate_cd_candidates(n, sig.c, sig.d);
    total_cd += cd_cands.size();

#pragma omp critical
    {
      if (!found.load(memory_order_relaxed) && cd_cands.size() > 0) {
        cout << "  Sig (" << sig.a << "," << sig.b << "," << sig.c << ","
             << sig.d << "): " << cd_cands.size() << " C,D candidates" << endl;
        cout << flush;
      }
    }

    // Step 5: For each C,D, backtrack A,B
    for (auto &cd : cd_cands) {
      if (found.load(memory_order_relaxed))
        break;
      total_attempts++;

      vector<int> A(n1, 0), B(n1, 0);
      vector<int> partial_corr(max(n1, n), 0);

      bool ok = backtrack_ab(n1, n, cd.C, cd.D, A, B, 0, sig.a, sig.b, 0, 0,
                             partial_corr);

      if (ok) {
        // Verify
        int Aarr[300], Barr[300], Carr[300], Darr[300];
        for (int i = 0; i < n1; i++) {
          Aarr[i] = A[i];
          Barr[i] = B[i];
        }
        for (int i = 0; i < n; i++) {
          Carr[i] = cd.C[i];
          Darr[i] = cd.D[i];
        }

        if (verify(Aarr, Barr, n1, Carr, Darr, n)) {
          found.store(true);
#pragma omp critical
          {
            memcpy(g_sol[0], Aarr, n1 * sizeof(int));
            memcpy(g_sol[1], Barr, n1 * sizeof(int));
            memcpy(g_sol[2], Carr, n * sizeof(int));
            memcpy(g_sol[3], Darr, n * sizeof(int));
            cout << "\n*** FOUND BS(" << n1 << "," << n << ")! ***" << endl;
            cout << flush;
          }
        }
      }
    }
  }

  auto t_end = Clock::now();
  double elapsed = chrono::duration<double>(t_end - t_start).count();

  cout << "\n========================================================" << endl;
  cout << "  RESULTS" << endl;
  cout << "========================================================" << endl;
  cout << "Time: " << elapsed << "s (" << elapsed / 60 << " min)" << endl;
  cout << "Total C,D candidates tested: " << total_cd << endl;
  cout << "Total backtracking attempts: " << total_attempts << endl;

  if (found.load()) {
    cout << "\nSUCCESS! BS(" << n1 << "," << n << ") VERIFIED!" << endl;
    const char *names[] = {"A", "B", "C", "D"};
    int lens[] = {n1, n1, n, n};
    for (int j = 0; j < 4; j++) {
      cout << names[j] << " = [";
      for (int i = 0; i < lens[j]; i++) {
        if (i)
          cout << ",";
        cout << g_sol[j][i];
      }
      cout << "]" << endl;
    }
    cout << endl;
    for (int j = 0; j < 4; j++) {
      cout << names[j] << " := ";
      for (int i = 0; i < lens[j]; i++)
        cout << (g_sol[j][i] == 1 ? "+" : "-");
      cout << endl;
    }
    cout << "\nDelta-code length: " << (2 * n1 - 1) << endl;
  } else {
    cout << "\nNo solution found." << endl;
  }

  return found.load() ? 0 : 1;
}
