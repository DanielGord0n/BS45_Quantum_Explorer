// Debug: test AB backtracking with a known solution
#include <iostream>
#include <cstring>
#include <cmath>
#include <algorithm>
using namespace std;
static const int MX = 128;

bool bt_ab(int n1, int n2, const int *cd_full,
           int *A, int *B, int pos, int ta, int tb, int sa, int sb,
           int *corr_ab, int depth) {
    if (pos == n1) {
        if (sa != ta || sb != tb) return false;
        int ms = max(n1, n2);
        for (int s = 1; s < ms; s++) {
            if (corr_ab[s] + cd_full[s] != 0) return false;
        }
        return true;
    }
    int remaining = n1 - pos - 1;
    for (int av : {-1, 1}) {
        if (abs(ta - (sa+av)) > remaining) continue;
        A[pos] = av;
        for (int bv : {-1, 1}) {
            if (abs(tb - (sb+bv)) > remaining) continue;
            B[pos] = bv;
            int ms = max(n1, n2);
            int saved[MX];
            memcpy(saved, corr_ab, ms * sizeof(int));
            for (int s = 1; s <= pos && s < n1; s++)
                corr_ab[s] += A[pos-s]*av + B[pos-s]*bv;
            bool ok = true;
            for (int s = 1; s < ms && ok; s++) {
                int total = corr_ab[s] + cd_full[s];
                int rem = max(0, n1 - s - pos - 1) * 2;
                if (rem == 0) {
                    if (total != 0) {
                        if (depth < 3) cout << "  PRUNE pos=" << pos << " s=" << s
                            << " total=" << total << " rem=0" << endl;
                        ok = false;
                    }
                } else {
                    if (abs(total) > rem) {
                        if (depth < 3) cout << "  PRUNE pos=" << pos << " s=" << s
                            << " total=" << total << " rem=" << rem << endl;
                        ok = false;
                    }
                }
            }
            if (ok) {
                if (bt_ab(n1, n2, cd_full, A, B, pos+1, ta, tb, sa+av, sb+bv, corr_ab, depth+1))
                    return true;
            }
            memcpy(corr_ab, saved, ms * sizeof(int));
        }
    }
    return false;
}

int main() {
    // Known solution: BS(5,4) from our v2 solver:
    // A = [-1,-1,-1,-1,1], B = [-1,-1,1,-1,-1]
    // C = [-1,1,-1,1], D = [1,1,-1,-1]
    // sum(A)=-3, sum(B)=-1, sum(C)=0... wait
    int Ak[] = {-1,-1,-1,-1,1};
    int Bk[] = {-1,-1,1,-1,-1};
    int Ck[] = {-1,1,-1,1};
    int Dk[] = {1,1,-1,-1};
    int n1 = 5, n2 = 4;

    // Compute sums
    int sa=0,sb=0,sc=0,sd=0;
    for(int i=0;i<n1;i++) { sa+=Ak[i]; sb+=Bk[i]; }
    for(int i=0;i<n2;i++) { sc+=Ck[i]; sd+=Dk[i]; }
    cout << "Known solution sums: a=" << sa << " b=" << sb << " c=" << sc << " d=" << sd << endl;

    // Verify
    int ms = max(n1,n2);
    for (int s = 1; s < ms; s++) {
        int c = 0;
        for (int i=0;i<n1-s;i++) c += Ak[i]*Ak[i+s] + Bk[i]*Bk[i+s];
        for (int i=0;i<n2-s;i++) c += Ck[i]*Ck[i+s] + Dk[i]*Dk[i+s];
        cout << "  NPAF(" << s << ") = " << c << endl;
    }

    // Compute cd_full
    int cd_full[MX] = {};
    for (int s = 1; s < ms; s++) {
        if (s < n2) for (int k=0;k<n2-s;k++)
            cd_full[s] += Ck[k]*Ck[k+s] + Dk[k]*Dk[k+s];
    }
    cout << "CD autocorrelations: ";
    for (int s=1;s<ms;s++) cout << cd_full[s] << " ";
    cout << endl;

    // Now try to find A,B with the backtracker
    // Note: the solver uses a>=0 symmetry reduction, so ta=-3 won't be tried
    // Let's try with the actual sums
    int A[MX]={}, B[MX]={}, corr_ab[MX]={};
    cout << "\nSearching with ta=" << sa << " tb=" << sb << endl;
    bool found = bt_ab(n1, n2, cd_full, A, B, 0, sa, sb, 0, 0, corr_ab, 0);
    cout << "Found: " << (found ? "YES" : "NO") << endl;
    if (found) {
        cout << "A=["; for(int i=0;i<n1;i++) cout<<(i?",":"")<<A[i]; cout<<"]"<<endl;
        cout << "B=["; for(int i=0;i<n1;i++) cout<<(i?",":"")<<B[i]; cout<<"]"<<endl;
    }
}
