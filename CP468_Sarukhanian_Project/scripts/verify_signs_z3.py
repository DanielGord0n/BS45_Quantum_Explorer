
import sys
from pathlib import Path
# Add local src to path if needed, though we will implement logic directly
sys.path.insert(0, str(Path(".").resolve()))

try:
    import z3
except ImportError:
    print("Z3 not found. Please install it with 'pip install z3-solver'")
    sys.exit(1)

import numpy as np

def get_turyn_n3():
    # Hardcoded standard Turyn sequences for n=3
    A = [1, 1, 1]
    B = [1, 1, -1]
    C = [1, -1]
    D = [1, -1]
    return A, B, C, D

def get_golay_k2():
    # Hardcoded standard Golay sequences for k=2
    F = [1, 1]
    G = [1, -1]
    return F, G

def solve_z3():
    print("Setting up Z3 solver...")
    solver = z3.Solver()
    
    # We have 20 distinct blocks based on the loop structure
    # Define 20 integer variables that must be 1 or -1
    signs = [z3.Int(f's_{i}') for i in range(20)]
    for s in signs:
        solver.add(z3.Or(s == 1, s == -1))
        
    A, B, C, D = get_turyn_n3()
    F, G = get_golay_k2()
    
    n = len(A)
    k = len(F)
    
    # Vectors x, y, z, w definition from paper/code
    # x = (1, 1, 0, 0)
    # y = (1, -1, 0, 0)
    # z = (0, 0, 1, 1)
    # w = (0, 0, 1, -1)
    
    # We will build 4 sequences: row1, row2, row3, row4
    # Instead of full vector arithmetic, we can compute the contribution to each row directly.
    # coeff = x * val1 + z * val2
    # row1 gets coeff[0] = 1*val1 + 0*val2 = val1
    # row2 gets coeff[1] = 1*val1 + 0*val2 = val1
    # row3 gets coeff[2] = 0*val1 + 1*val2 = val2
    # row4 gets coeff[3] = 0*val1 + 1*val2 = val2
    
    row1 = []
    row2 = []
    row3 = []
    row4 = []
    
    s_idx = 0
    
    # Helper to append a column block
    # multiplier is the sign variable for this block
    # val_x, val_y, val_z, val_w are the scalar values for the vector components
    # But wait, the formula uses x, y, z, w vectors.
    # Group 1 use x, z. 
    # Group 5 uses y, w.
    
    # Let's follow the code structure structure exactly to ensure we match the blocks.
    
    # 1. Loop j=1 to k
    for j in range(1, k + 1):
        # Block 1a (len n)
        # coeff = x * F[k-j] + z * G[k-j]
        v_f = F[k-j] # Scalar
        v_g = G[k-j] # Scalar
        
        # Row vals:
        r1_val = v_f
        r2_val = v_f
        r3_val = v_g
        r4_val = v_g
        
        sign = signs[s_idx]
        s_idx += 1
        
        for val_a in A:
            row1.append(r1_val * val_a * sign)
            row2.append(r2_val * val_a * sign)
            row3.append(r3_val * val_a * sign)
            row4.append(r4_val * val_a * sign)
            
        # Block 1b (len n-1)
        # coeff = x * G[j-1] + z * F[k-j]
        v_g_j = G[j-1]
        v_f_k = F[k-j]
        
        r1_val = v_g_j
        r2_val = v_g_j
        r3_val = v_f_k
        r4_val = v_f_k
        
        sign = signs[s_idx]
        s_idx += 1
        
        for val_c in C:
            row1.append(r1_val * val_c * sign)
            row2.append(r2_val * val_c * sign)
            row3.append(r3_val * val_c * sign)
            row4.append(r4_val * val_c * sign)

    # 2. {x a_i - z b_i}
    # Block 2a (len n)
    # coeff terms: x part is a_i, z part is -b_i
    sign = signs[s_idx]
    s_idx += 1
    for i in range(n):
        val_a = A[i]
        val_b = B[i]
        # x term = val_a, z term = -val_b
        row1.append(val_a * sign)
        row2.append(val_a * sign)
        row3.append(-val_b * sign)
        row4.append(-val_b * sign)

    # 3. {x d_i - z c_i}
    # Block 3a (len n-1)
    sign = signs[s_idx]
    s_idx += 1
    for i in range(n-1):
        val_d = D[i]
        val_c = C[i]
        row1.append(val_d * sign)
        row2.append(val_d * sign)
        row3.append(-val_c * sign)
        row4.append(-val_c * sign)
        
    # 4. Loop j=1 to k
    for j in range(1, k + 1):
        # Block 4a (len n)
        # coeff = x * G[j-1] + z * F[k-j]
        v_g = G[j-1]
        v_f = F[k-j]
        
        r1_val = v_g
        r2_val = v_g
        r3_val = v_f
        r4_val = v_f
        
        sign = signs[s_idx]
        s_idx += 1
        for val_b in B:
            row1.append(r1_val * val_b * sign)
            row2.append(r2_val * val_b * sign)
            row3.append(r3_val * val_b * sign)
            row4.append(r4_val * val_b * sign)
            
        # Block 4b (len n-1)
        # coeff = -x * F[j-1] + z * G[k-j]
        v_f_j = F[j-1]
        v_g_k = G[k-j]
        # Note the negative x
        r1_val = -v_f_j
        r2_val = -v_f_j
        r3_val = v_g_k
        r4_val = v_g_k
        
        sign = signs[s_idx]
        s_idx += 1
        for val_d in D:
            row1.append(r1_val * val_d * sign)
            row2.append(r2_val * val_d * sign)
            row3.append(r3_val * val_d * sign)
            row4.append(r4_val * val_d * sign)

    # 5. Loop j=1 to k
    # Uses y and w
    # y = (1, -1, 0, 0)
    # w = (0, 0, 1, -1)
    # coeff = y * val1 + w * val2
    # row1: 1*v1 + 0*v2 = v1
    # row2: -1*v1 + 0*v2 = -v1
    # row3: 0*v1 + 1*v2 = v2
    # row4: 0*v1 - 1*v2 = -v2
    
    for j in range(1, k + 1):
        # Block 5a (len n)
        # coeff = y * F[k-j] + w * G[k-j]
        v_f = F[k-j]
        v_g = G[k-j]
        
        sign = signs[s_idx]
        s_idx += 1
        for val_a in A:
            row1.append(v_f * val_a * sign)
            row2.append(-v_f * val_a * sign)
            row3.append(v_g * val_a * sign)
            row4.append(-v_g * val_a * sign)
            
        # Block 5b (len n-1)
        # coeff = y * G[j-1] - w * F[j-1]
        v_g_j = G[j-1]
        v_f_j = F[j-1]
        
        sign = signs[s_idx]
        s_idx += 1
        for val_c in C:
            # y part: v_g_j
            # w part: -v_f_j
            row1.append(v_g_j * val_c * sign)
            row2.append(-v_g_j * val_c * sign)
            row3.append(-v_f_j * val_c * sign)
            row4.append(-(-v_f_j) * val_c * sign) # -w part, so -(-val) = val

    # 6. {-y a_i + w b_i}
    # Block 6a (len n)
    sign = signs[s_idx]
    s_idx += 1
    for i in range(n):
        val_a = A[i]
        val_b = B[i]
        # y part: -val_a
        # w part: val_b
        row1.append(-val_a * sign)
        row2.append(-(-val_a) * sign)
        row3.append(val_b * sign)
        row4.append(-val_b * sign)
        
    # 7. {y d_i + w c_i}
    # Block 7a (len n-1)
    sign = signs[s_idx]
    s_idx += 1
    for i in range(n-1):
        val_d = D[i]
        val_c = C[i]
        # y part: val_d
        # w part: val_c
        row1.append(val_d * sign)
        row2.append(-val_d * sign)
        row3.append(val_c * sign)
        row4.append(-val_c * sign)

    # 8. Loop j=1 to k
    for j in range(1, k + 1):
        # Block 8a (len n)
        # coeff = y * G[j-1] + w * F[k-j]
        v_g = G[j-1]
        v_f = F[k-j]
        
        sign = signs[s_idx]
        s_idx += 1
        for val_b in B:
            row1.append(v_g * val_b * sign)
            row2.append(-v_g * val_b * sign)
            row3.append(v_f * val_b * sign)
            row4.append(-v_f * val_b * sign)
            
        # Block 8b (len n-1)
        # coeff = y * F[j-1] - w * G[k-j]
        v_f_j = F[j-1]
        v_g_k = G[k-j]
        
        sign = signs[s_idx]
        s_idx += 1
        for val_d in D:
            # y part: v_f_j
            # w part: -v_g_k
            row1.append(v_f_j * val_d * sign)
            row2.append(-v_f_j * val_d * sign)
            row3.append(-v_g_k * val_d * sign)
            row4.append(-(-v_g_k) * val_d * sign)

    # Verify lengths
    N = len(row1)
    print(f"Constructed sequence length: {N}")
    if N != 50:
        print("Error: Length is not 50")
    
    # Assert NPAF == 0 for all shifts s in 1..N-1
    print("Building NPAF constraints...")
    
    # NPAF(s) = sum_{i=0}^{N-1-s} (r1[i]*r1[i+s] + r2[i]*r2[i+s] + ...)
    
    # Optimization: If we find one satisfiable solution, we are good.
    
    for s in range(1, N):
        # Build expression for this shift
        terms = []
        for i in range(N - s):
            # Term for row 1
            terms.append(row1[i] * row1[i+s])
            terms.append(row2[i] * row2[i+s])
            terms.append(row3[i] * row3[i+s])
            terms.append(row4[i] * row4[i+s])
            
        # The sum of all these products must be 0
        solver.add(z3.Sum(terms) == 0)
        
    print("Checking satisfiability...")
    result = solver.check()
    
    if result == z3.sat:
        print("SAT! Found a working sign combination.")
        model = solver.model()
        final_signs = [model[s].as_long() for s in signs]
        print("Signs:", final_signs)
        
        # Verify manually
        # (This is implicitly done by the solver, but good for output)
    else:
        print("UNSAT. No combination of signs makes this a valid delta-code.")
        print("This proves the construction is structurally flawed beyond simple sign errors.")

if __name__ == "__main__":
    solve_z3()
