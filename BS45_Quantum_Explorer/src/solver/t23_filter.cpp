/*
 * t23_filter.cpp — Theorem 2.3 m=3 precomputation + (P,Q)→(K,R) lookup.
 *
 * For a given n and signature (a,b,c,d), enumerates all valid m=3 residue-sum
 * 4-tuples (K,R,P,Q) and indexes them so that, given an observed (P,Q) from
 * a fully-placed C,D pair, the compatible (K,R) candidates can be retrieved
 * in O(1). Empty result => prune the A,B subtree.
 *
 * Standalone test mode: prints index statistics + verifies that the known
 * Wang-Zhu BS(43,42) sequences' (K,R,P,Q) is present in the precomputed set.
 *
 * Compile: g++ -O3 -std=c++17 -o t23_filter src/solver/t23_filter.cpp
 * Test:    ./t23_filter 42 7 11 0 0
 */
#include <array>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <unordered_map>
#include <vector>

using namespace std;

struct M3Triple {
  int x[3];
  bool operator==(const M3Triple &o) const {
    return x[0] == o.x[0] && x[1] == o.x[1] && x[2] == o.x[2];
  }
};

struct KRPair { M3Triple K, R; };

// Encode a (P,Q) pair as a 64-bit key for hash lookup.
// Each value is in [-21, 21] (worst case for n=44); add 32 to make non-negative,
// fits in 6 bits. 6 values * 6 bits = 36 bits, fits in uint64.
static inline uint64_t encode_pq(const M3Triple &P, const M3Triple &Q) {
  uint64_t k = 0;
  for (int i = 0; i < 3; i++) k = (k << 6) | uint64_t(P.x[i] + 32);
  for (int i = 0; i < 3; i++) k = (k << 6) | uint64_t(Q.x[i] + 32);
  return k;
}

class T23Filter {
public:
  T23Filter(int n, int a, int b, int c, int d) : n_(n) {
    build(a, b, c, d);
  }
  const vector<KRPair> &compatible_KR(const M3Triple &P,
                                       const M3Triple &Q) const {
    auto it = by_PQ_.find(encode_pq(P, Q));
    return (it == by_PQ_.end()) ? empty_ : it->second;
  }
  size_t total_tuples() const { return total_; }
  size_t unique_PQ_keys() const { return by_PQ_.size(); }
  // Stats for reporting.
  size_t max_KR_per_PQ() const {
    size_t m = 0;
    for (auto &kv : by_PQ_) m = max(m, kv.second.size());
    return m;
  }
  double avg_KR_per_PQ() const {
    if (by_PQ_.empty()) return 0.0;
    return double(total_) / by_PQ_.size();
  }

private:
  static int class_count(int L, int m, int c) {
    int n = 0;
    for (int p = c; p < L; p += m) n++;
    return n;
  }
  static vector<M3Triple> enum_class_sums(int L, int target_sum) {
    vector<M3Triple> out;
    int n0 = class_count(L, 3, 0);
    int n1 = class_count(L, 3, 1);
    int n2 = class_count(L, 3, 2);
    for (int x0 = -n0; x0 <= n0; x0 += 2)
      for (int x1 = -n1; x1 <= n1; x1 += 2) {
        int x2 = target_sum - x0 - x1;
        if (x2 < -n2 || x2 > n2) continue;
        if (((x2 - (-n2)) % 2 + 2) % 2 != 0) continue;
        out.push_back({x0, x1, x2});
      }
    return out;
  }
  static int norm(const M3Triple &t) {
    return t.x[0] * t.x[0] + t.x[1] * t.x[1] + t.x[2] * t.x[2];
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
    int target_norm = 4 * n_ + 2;
    for (size_t i = 0; i < K.size(); i++)
      for (size_t j = 0; j < R.size(); j++) {
        int kr = K2[i] + R2[j];
        if (kr > target_norm) continue;
        for (size_t k = 0; k < P.size(); k++) {
          int krp = kr + P2[k];
          if (krp > target_norm) continue;
          int need = target_norm - krp;
          for (size_t l = 0; l < Q.size(); l++) {
            if (Q2[l] == need) {
              by_PQ_[encode_pq(P[k], Q[l])].push_back({K[i], R[j]});
              total_++;
            }
          }
        }
      }
  }

  int n_;
  size_t total_ = 0;
  unordered_map<uint64_t, vector<KRPair>> by_PQ_;
  vector<KRPair> empty_;
};

// =========================== Standalone test ===========================

static int parse_pm(const string &s) {
  int v = 0;
  for (char ch : s) {
    if (ch == '+') v++;
    else if (ch == '-') v--;
  }
  return v;
}

// Compute m=3 residue sum triple for a ±1 sequence.
static M3Triple residue_sums(const vector<int> &seq) {
  M3Triple t = {{0, 0, 0}};
  for (size_t i = 0; i < seq.size(); i++) t.x[i % 3] += seq[i];
  return t;
}

int main(int argc, char **argv) {
  if (argc < 6) {
    cerr << "usage: " << argv[0] << " <n> <a> <b> <c> <d>\n";
    return 1;
  }
  int n = atoi(argv[1]);
  int a = atoi(argv[2]), b = atoi(argv[3]), c = atoi(argv[4]), d = atoi(argv[5]);

  cout << "Building T23Filter for n=" << n << " sig=(" << a << "," << b
       << "," << c << "," << d << ")...\n";
  T23Filter f(n, a, b, c, d);
  cout << "  total valid 4-tuples: " << f.total_tuples() << "\n";
  cout << "  unique (P,Q) keys:    " << f.unique_PQ_keys() << "\n";
  cout << "  max (K,R) per (P,Q):  " << f.max_KR_per_PQ() << "\n";
  cout << "  avg (K,R) per (P,Q):  " << f.avg_KR_per_PQ() << "\n";

  // For sig (7,11,0,0) at n=42, verify the Wang-Zhu BS(43,42) tuple is present.
  if (n == 42 && a == 7 && b == 11 && c == 0 && d == 0) {
    cout << "\n--- Verifying known Wang-Zhu BS(43,42) sequences ---\n";
    // Parsed inline from verify_bs43.cpp / verify_npaf.py.
    auto parse_seq = [](const string &s1, const string &s2) {
      vector<int> v;
      for (char ch : s1 + s2) {
        if (ch == '+') v.push_back(1);
        else if (ch == '-') v.push_back(-1);
      }
      return v;
    };
    auto A = parse_seq("++--++--+-+++-+--+-++-", "-+----+++-+-+-+++++++");
    auto B = parse_seq("+++++++++---+-+-+++--+", "+-+-+-++++--++-++--+-");
    auto C = parse_seq("+++++----+++--+++-++-+", "--+----+--+--+++--+-");
    auto D = parse_seq("++------+---++++---+-+", "-+++--+-++-+-++-+++-");
    M3Triple K = residue_sums(A);
    M3Triple R = residue_sums(B);
    M3Triple P = residue_sums(C);
    M3Triple Q = residue_sums(D);
    cout << "  K = (" << K.x[0] << "," << K.x[1] << "," << K.x[2] << ")\n";
    cout << "  R = (" << R.x[0] << "," << R.x[1] << "," << R.x[2] << ")\n";
    cout << "  P = (" << P.x[0] << "," << P.x[1] << "," << P.x[2] << ")\n";
    cout << "  Q = (" << Q.x[0] << "," << Q.x[1] << "," << Q.x[2] << ")\n";
    const auto &candidates = f.compatible_KR(P, Q);
    cout << "  compatible (K,R) for this (P,Q): " << candidates.size() << "\n";
    bool found = false;
    for (auto &kr : candidates) {
      if (kr.K == K && kr.R == R) {
        found = true;
        break;
      }
    }
    cout << "  Wang-Zhu (K,R) present in compatible set: "
         << (found ? "YES ✓" : "NO ✗") << "\n";
  }
  return 0;
}
