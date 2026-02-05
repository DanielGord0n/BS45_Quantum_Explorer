
import sys
import itertools
try:
    import z3
except ImportError:
    print("Z3 not found")
    sys.exit(1)

# Helper for NPAF check
def check_npaf(seqs, n):
    # seqs is list of sequences
    # Check sum of autocorrelations is 0 for s=1..n-1
    for s in range(1, n):
        sum_p = 0
        for seq in seqs:
            if s < len(seq):
                # Correlate
                for i in range(len(seq) - s):
                    sum_p += seq[i] * seq[i+s]
        if sum_p != 0:
            return False
    return True

# Hardcoded Known Turyn Sequences (A,B len n; C,D len n-1)
# Sources: verified small cases + literature
known_turyn = {
    2: ([-1, -1], [-1, 1], [-1], [-1]),
    3: ([-1, -1, -1], [-1, -1, 1], [-1, 1], [-1, 1]),
    4: ([-1, -1, -1, 1], [-1, 1, 1, -1], [-1, -1, -1], [-1, 1, -1]),
    # n=5? Try brute force for 5 if not known.
    # n=7?
}

def find_turyn(n):
    if n in known_turyn:
        return known_turyn[n]
    
    # Brute force for n=5, 6, 7
    if n > 8: return None # Too slow
    
    print(f"  Searching for Turyn n={n}...")
    for A in itertools.product([-1, 1], repeat=n):
        for B in itertools.product([-1, 1], repeat=n):
            for C in itertools.product([-1, 1], repeat=n-1):
                for D in itertools.product([-1, 1], repeat=n-1):
                    if check_npaf([list(A), list(B), list(C), list(D)], n):
                        res = (list(A), list(B), list(C), list(D))
                        # Cache it
                        known_turyn[n] = res
                        return res
    return None

def find_golay(k):
    # k=1: [1], [1]
    # k=2: [1,1], [1,-1]
    # k=4: [1,1,1,-1], [1,1,-1,1]
    if k == 1: return [1], [1]
    if k == 2: return [1,1], [1,-1]
    if k == 3: return None # No Golay(3)
    if k == 4: return [1,1,1,-1], [1,1,-1,1]
    if k == 8: return [1]*8, [1]*8 # placeholder, need real Golay(8) if testing
    # Brute force
    for F in itertools.product([-1, 1], repeat=k):
        for G in itertools.product([-1, 1], repeat=k):
            if check_npaf([list(F), list(G)], k):
                return list(F), list(G)
    return None

def solve_param(n, k):
    print(f"Testing n={n}, k={k}...")
    
    # 1. Get Sequences
    turyn = find_turyn(n)
    if not turyn:
        print(f"  No Turyn sequences found for n={n}")
        return False
    A, B, C, D = turyn
    # print(f"  Found Turyn: A={A}, ...") 
    
    golay = find_golay(k)
    if not golay:
        print(f"  No Golay sequences found for k={k}")
        return False
    F, G = golay
    
    # 2. Build Z3 Model
    solver = z3.Solver()
    
    # Groups 1,4,5,8 are LOOPS k. Groups 2,3,6,7 are singles.
    # Total blocks = 4*(2k) + 4 = 8k+4.
    num_blocks = 8*k + 4
    signs = [z3.Int(f's_{i}') for i in range(num_blocks)]
    for s in signs:
        solver.add(z3.Or(s==1, s==-1))
        
    row1, row2, row3, row4 = [], [], [], []
    s_idx = 0
    
    def add(d, s):
        row1.extend([v*s for v in d[0]])
        row2.extend([v*s for v in d[1]])
        row3.extend([v*s for v in d[2]])
        row4.extend([v*s for v in d[3]])

    # Reconstruct dynamic logic
    
    # Group 1 (j=1..k)
    for j in range(1, k+1):
        v_f=F[k-j]; v_g=G[k-j]
        d=[[v_f*v for v in A],[v_f*v for v in A],[v_g*v for v in A],[v_g*v for v in A]]
        add(d, signs[s_idx]); s_idx+=1
        
        v_g=G[j-1]; v_f=F[k-j]
        d=[[v_g*v for v in C],[v_g*v for v in C],[v_f*v for v in C],[v_f*v for v in C]]
        add(d, signs[s_idx]); s_idx+=1
        
    # Group 2
    d=[[],[],[],[]]
    for i in range(n):
        d[0].append(A[i]); d[1].append(A[i]); d[2].append(-B[i]); d[3].append(-B[i])
    add(d, signs[s_idx]); s_idx+=1
    
    # Group 3
    d=[[],[],[],[]]
    for i in range(n-1):
        d[0].append(D[i]); d[1].append(D[i]); d[2].append(-C[i]); d[3].append(-C[i])
    add(d, signs[s_idx]); s_idx+=1

    # Group 4
    for j in range(1, k+1):
        v_g=G[j-1]; v_f=F[k-j]
        d=[[v_g*v for v in B],[v_g*v for v in B],[v_f*v for v in B],[v_f*v for v in B]]
        add(d, signs[s_idx]); s_idx+=1
        
        v_f=F[j-1]; v_g=G[k-j]
        d=[[-v_f*v for v in D],[-v_f*v for v in D],[v_g*v for v in D],[v_g*v for v in D]]
        add(d, signs[s_idx]); s_idx+=1

    # Group 5
    for j in range(1, k+1):
        v_f=F[k-j]; v_g=G[k-j]
        r1=[v_f*v for v in A]; r2=[-v_f*v for v in A]
        r3=[v_g*v for v in A]; r4=[-v_g*v for v in A]
        add([r1,r2,r3,r4], signs[s_idx]); s_idx+=1
        
        v_g=G[j-1]; v_f=F[j-1]
        r1=[v_g*v for v in C]; r2=[-v_g*v for v in C]
        r3=[-v_f*v for v in C]; r4=[v_f*v for v in C]
        add([r1,r2,r3,r4], signs[s_idx]); s_idx+=1

    # Group 6
    d=[[],[],[],[]]
    for i in range(n):
        d[0].append(-A[i]); d[1].append(A[i]); d[2].append(B[i]); d[3].append(-B[i])
    add(d, signs[s_idx]); s_idx+=1

    # Group 7
    d=[[],[],[],[]]
    for i in range(n-1):
        d[0].append(D[i]); d[1].append(-D[i]); d[2].append(C[i]); d[3].append(-C[i])
    add(d, signs[s_idx]); s_idx+=1
    
    # Group 8
    for j in range(1, k+1):
        v_g=G[j-1]; v_f=F[k-j]
        r1=[v_g*v for v in B]; r2=[-v_g*v for v in B]
        r3=[v_f*v for v in B]; r4=[-v_f*v for v in B]
        add([r1,r2,r3,r4], signs[s_idx]); s_idx+=1
        
        v_f=F[j-1]; v_g=G[k-j]
        r1=[v_f*v for v in D]; r2=[-v_f*v for v in D]
        r3=[-v_g*v for v in D]; r4=[v_g*v for v in D]
        add([r1,r2,r3,r4], signs[s_idx]); s_idx+=1
        
    N_seq = len(row1)
    
    # NPAF
    for s in range(1, N_seq):
        terms = []
        for i in range(N_seq - s):
            terms.append(row1[i]*row1[i+s])
            terms.append(row2[i]*row2[i+s])
            terms.append(row3[i]*row3[i+s])
            terms.append(row4[i]*row4[i+s])
        solver.add(z3.Sum(terms) == 0)
        
    if solver.check() == z3.sat:
        print(f"  SAT! Works for n={n}, k={k} (Length {N_seq})")
        return True
    else:
        print(f"  UNSAT for n={n}, k={k} (Length {N_seq})")
        return False

if __name__ == "__main__":
    # Extensive test
    solve_param(2, 1)
    solve_param(3, 1)
    solve_param(3, 2)
    solve_param(4, 1)
    # New ones
    solve_param(5, 1)
    solve_param(5, 2)
    # solve_param(7, 1)
