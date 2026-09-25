#include <random>
int main(int argc,char**argv){
 int n=argc>1?atoi(argv[1]):13; G_N=n;G_N1=n+1;G_SIG_A=2;G_SIG_B=4;G_SIG_C=n==13?3:5;G_SIG_D=n==13?5:1;
 init_p22();init_hall_tables();FH_ABP=nullptr;FH_ABP_M=6;FH_BUDGET=0;
 std::mt19937 rng(1234567);int tested=0,hits=0;unsigned long long digest=1469598103934665603ULL;long long sumNodes=0;
 int C[256],D[256];auto t=Clock::now();
 while(tested<2000){for(int d=0;d<n/2;d++){int k=rng()%8;C[d]=P22_POS[k][0];D[d]=P22_POS[k][1];C[n-1-d]=P22_POS[k][2];D[n-1-d]=P22_POS[k][3];} if(n%2){C[n/2]=(rng()%2)?1:-1;D[n/2]=(rng()%2)?1:-1;}
 int cs=0,ds=0;for(int i=0;i<n;i++){cs+=C[i];ds+=D[i];}if(cs!=G_SIG_C||ds!=G_SIG_D||!hall_ok(C,n,D,n))continue;
 int result=fh_complete_ab(C,D);hits+=(result==0);digest=(digest^(unsigned)result)*1099511628211ULL; if(result==0){for(int i=0;i<n+1;i++){digest=(digest^(unsigned)(g_solA[i]+2))*1099511628211ULL;digest=(digest^(unsigned)(g_solB[i]+2))*1099511628211ULL;}}
 tested++;
 }
 printf("n=%d tested=%d hits=%d nodes=%lld digest=%llu seconds=%.6f\n",n,tested,hits,fh_nodes_total,digest,chrono::duration<double>(Clock::now()-t).count());
}
