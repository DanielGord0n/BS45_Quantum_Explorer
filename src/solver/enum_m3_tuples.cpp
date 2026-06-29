/*
 * enum_m3_tuples.cpp — Theorem 2.3 step 1.
 *
 * For a given n and signature (a,b,c,d), enumerate all valid m=3 residue-sum
 * 4-tuples (K,R,P,Q) where each entry is (x_0,x_1,x_2) — the sum of the
 * sequence over positions ≡ 0/1/2 (mod 3).
 *
 * Constraints (Wang-Zhu Thm 2.3):
 *  - Sum: x_0+x_1+x_2 = sig value for that sequence.
 *  - Parity per class: x_i ≡ class_count_i (mod 2).
 *  - Range: |x_i| ≤ class_count_i.
 *  - Norm (Eq 16): ΣK_i² + ΣR_i² + ΣP_i² + ΣQ_i² = 4n+2.
 *
 * Note on Eq (16) s=1 correlation identity: it is algebraically equivalent to
 * the norm identity given the per-sequence sum constraints — so the norm
 * filter alone implements both. (Verified by expanding (Σx_i)² and combining.)
 *
 * This is a validation tool — counts valid 4-tuples for the known BS(43,42)
 * signature (7,11,0,0). Sane count expected: hundreds to low thousands.
 */
#include <array>
#include <cstdlib>
#include <iostream>
#include <vector>

using namespace std;

static int class_count(int L, int m, int c) {
  int n = 0;
  for (int p = c; p < L; p += m) n++;
  return n;
}

// All (x_0,x_1,x_2) with x_0+x_1+x_2 = target_sum, parities/ranges per class.
static vector<array<int, 3>> enum_class_sums(int L, int target_sum) {
  vector<array<int, 3>> out;
  const int m = 3;
  int n0 = class_count(L, m, 0);
  int n1 = class_count(L, m, 1);
  int n2 = class_count(L, m, 2);
  // x_i ranges -n_i..+n_i step 2 (parity = n_i mod 2 automatically).
  for (int x0 = -n0; x0 <= n0; x0 += 2) {
    for (int x1 = -n1; x1 <= n1; x1 += 2) {
      int x2 = target_sum - x0 - x1;
      if (x2 < -n2 || x2 > n2) continue;
      // Parity safeguard (should be automatic if signature parity is right).
      if (((x2 - (-n2)) % 2 + 2) % 2 != 0) continue;
      out.push_back({x0, x1, x2});
    }
  }
  return out;
}

int main(int argc, char **argv) {
  if (argc < 6) {
    cerr << "usage: " << argv[0] << " <n> <a> <b> <c> <d>\n"
         << "  e.g.  " << argv[0] << " 42 7 11 0 0   (BS(43,42) Wang-Zhu sig)\n";
    return 1;
  }
  int n = atoi(argv[1]);
  int a = atoi(argv[2]);
  int b = atoi(argv[3]);
  int c = atoi(argv[4]);
  int d = atoi(argv[5]);
  int n1 = n + 1;

  auto K = enum_class_sums(n1, a);  // A's class sums (length n+1)
  auto R = enum_class_sums(n1, b);  // B's class sums
  auto P = enum_class_sums(n, c);   // C's class sums (length n)
  auto Q = enum_class_sums(n, d);   // D's class sums

  cout << "n=" << n << "  sig=(" << a << "," << b << "," << c << "," << d
       << ")  a²+b²+c²+d²=" << (a * a + b * b + c * c + d * d)
       << " (expect " << (4 * n + 2) << ")\n";
  cout << "Per-sequence m=3 class-sum tuple counts:\n";
  cout << "  K (A target=" << a << "): " << K.size() << "\n";
  cout << "  R (B target=" << b << "): " << R.size() << "\n";
  cout << "  P (C target=" << c << "): " << P.size() << "\n";
  cout << "  Q (D target=" << d << "): " << Q.size() << "\n";
  cout << "  Raw 4-tuple product: " << ((long long)K.size() * R.size() * P.size() * Q.size()) << "\n";

  int target_norm = 4 * n + 2;
  long long valid = 0;
  // Precompute Σx² per tuple to skip work in the inner loop.
  vector<int> K2(K.size()), R2(R.size()), P2(P.size()), Q2(Q.size());
  for (size_t i = 0; i < K.size(); i++) K2[i] = K[i][0]*K[i][0]+K[i][1]*K[i][1]+K[i][2]*K[i][2];
  for (size_t i = 0; i < R.size(); i++) R2[i] = R[i][0]*R[i][0]+R[i][1]*R[i][1]+R[i][2]*R[i][2];
  for (size_t i = 0; i < P.size(); i++) P2[i] = P[i][0]*P[i][0]+P[i][1]*P[i][1]+P[i][2]*P[i][2];
  for (size_t i = 0; i < Q.size(); i++) Q2[i] = Q[i][0]*Q[i][0]+Q[i][1]*Q[i][1]+Q[i][2]*Q[i][2];

  for (size_t i = 0; i < K.size(); i++)
    for (size_t j = 0; j < R.size(); j++) {
      int kr = K2[i] + R2[j];
      if (kr > target_norm) continue;
      for (size_t k = 0; k < P.size(); k++) {
        int krp = kr + P2[k];
        if (krp > target_norm) continue;
        int need = target_norm - krp;
        for (size_t l = 0; l < Q.size(); l++) {
          if (Q2[l] == need) valid++;
        }
      }
    }
  cout << "Valid 4-tuples after norm filter (Eq 16): " << valid << "\n";

  // Verify on known BS(43,42) sequences: their residue sums must appear in the valid set.
  // Sanity: print the Wang-Zhu tuple if sig matches.
  if (n == 42 && a == 7 && b == 11 && c == 0 && d == 0) {
    cout << "\nReference: Wang-Zhu BS(43,42) sequences yield residue sums...\n";
    cout << "(integrate later — read from verify_npaf.py's known A,B,C,D)\n";
  }
  return 0;
}
