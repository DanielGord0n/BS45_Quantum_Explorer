#include <iostream>
#include <cstring>
#include <cmath>
#include <algorithm>
using namespace std;
int npaf(int *A, int *B, int n1, int *C, int *D, int n2, int s) {
    int c = 0;
    if(s<n1) for(int i=0;i<n1-s;i++) c += A[i]*A[i+s] + B[i]*B[i+s];
    if(s<n2) for(int i=0;i<n2-s;i++) c += C[i]*C[i+s] + D[i]*D[i+s];
    return c;
}
int main() {
    int n = 4;
    int n1 = n+1, n2 = n;
    int A[5], B[5], C[4], D[4];
    int count = 0;
    for (int mask_a = 0; mask_a < (1<<n1); mask_a++) {
        for (int i = 0; i < n1; i++) A[i] = (mask_a >> i) & 1 ? 1 : -1;
        if (A[0] != 1) continue;
        for (int mask_b = 0; mask_b < (1<<n1); mask_b++) {
            for (int i = 0; i < n1; i++) B[i] = (mask_b >> i) & 1 ? 1 : -1;
            if (B[0] != 1) continue;
            for (int mask_c = 0; mask_c < (1<<n2); mask_c++) {
                for (int i = 0; i < n2; i++) C[i] = (mask_c >> i) & 1 ? 1 : -1;
                for (int mask_d = 0; mask_d < (1<<n2); mask_d++) {
                    for (int i = 0; i < n2; i++) D[i] = (mask_d >> i) & 1 ? 1 : -1;
                    bool ok = true;
                    for (int s = 1; s < max(n1,n2); s++) {
                        if (npaf(A,B,n1,C,D,n2,s) != 0) { ok = false; break; }
                    }
                    if (ok) {
                        count++;
                        int sa=0,sb=0,sc=0,sd=0;
                        for(int i=0;i<n1;i++){sa+=A[i];sb+=B[i];}
                        for(int i=0;i<n2;i++){sc+=C[i];sd+=D[i];}
                        if (count <= 5) {
                            cout << "sums=(" << sa << "," << sb << "," << sc << "," << sd << ")" << endl;
                            cout << " a2+b2+c2+d2=" << sa*sa+sb*sb+sc*sc+sd*sd << " target=" << 4*n+2 << endl;
                        }
                    }
                }
            }
        }
    }
    cout << "Total BS(" << n1 << "," << n2 << ") found: " << count << endl;
}
