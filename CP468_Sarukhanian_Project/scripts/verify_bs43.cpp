#include <iostream>
#include <string>
#include <vector>

using namespace std;

vector<int> parse_seq(string s1, string s2) {
  string s = s1 + s2;
  vector<int> res;
  for (char c : s) {
    if (c == '+')
      res.push_back(1);
    else if (c == '-')
      res.push_back(-1);
  }
  return res;
}

bool verify_bs(const string &name, const vector<int> &A, const vector<int> &B,
               const vector<int> &C, const vector<int> &D) {
  int n1 = A.size();
  if (B.size() != n1) {
    cout << "Error: A and B lengths differ for " << name << endl;
    return false;
  }
  int n = C.size();
  if (D.size() != n) {
    cout << "Error: C and D lengths differ for " << name << endl;
    return false;
  }
  if (n1 != n + 1) {
    cout << "Error: A length is not C length + 1 for " << name << endl;
    return false;
  }

  cout << "Verifying " << name << " (Lengths A:" << n1 << " C:" << n << ")..."
       << endl;

  bool valid = true;
  for (int s = 1; s <= n1; s++) {
    int c = 0;
    if (s < n1) {
      for (int i = 0; i < n1 - s; i++)
        c += A[i] * A[i + s] + B[i] * B[i + s];
    }
    if (s < n) {
      for (int i = 0; i < n - s; i++)
        c += C[i] * C[i + s] + D[i] * D[i + s];
    }
    if (c != 0) {
      cout << "  FAILED: NPAF at shift " << s << " is " << c << " (not 0)!"
           << endl;
      valid = false;
    }
  }

  if (valid)
    cout << "  SUCCESS: " << name << " is a valid Base Sequence!" << endl;
  return valid;
}

int main() {
  // BS(43,42)
  vector<int> A43 =
      parse_seq("++--++--+-+++-+--+-++-", "-+----+++-+-+-+++++++");
  vector<int> B43 =
      parse_seq("+++++++++---+-+-+++--+", "+-+-+-++++--++-++--+-");
  vector<int> C43 = parse_seq("+++++----+++--+++-++-+", "--+----+--+--+++--+-");
  vector<int> D43 = parse_seq("++------+---++++---+-+", "-+++--+-++-+-++-+++-");
  verify_bs("BS(43,42)", A43, B43, C43, D43);

  // BS(44,43)
  vector<int> A44 =
      parse_seq("+++-+++++---+--+-++-++", "---+++-++-+-+-+-+++--+");
  vector<int> B44 =
      parse_seq("+++-++--++-+-+--+--+--", "+++---++-+-----++++---");
  vector<int> C44 =
      parse_seq("++---+--++++-+-+-+----", "+--+-+-++-+++++-+-+++");
  vector<int> D44 =
      parse_seq("--++++---++--+++++-++-", "-+-+++++++++-++--+---");
  verify_bs("BS(44,43)", A44, B44, C44, D44);

  cout << "\nRaw sequences for BS(43,42):" << endl;
  cout << "A = {";
  for (size_t i = 0; i < A43.size(); i++)
    cout << A43[i] << (i + 1 < A43.size() ? "," : "");
  cout << "};" << endl;
  cout << "B = {";
  for (size_t i = 0; i < B43.size(); i++)
    cout << B43[i] << (i + 1 < B43.size() ? "," : "");
  cout << "};" << endl;
  cout << "C = {";
  for (size_t i = 0; i < C43.size(); i++)
    cout << C43[i] << (i + 1 < C43.size() ? "," : "");
  cout << "};" << endl;
  cout << "D = {";
  for (size_t i = 0; i < D43.size(); i++)
    cout << D43[i] << (i + 1 < D43.size() ? "," : "");
  cout << "};" << endl;

  return 0;
}
