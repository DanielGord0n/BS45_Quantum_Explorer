
import sys
from pathlib import Path
sys.path.insert(0, str(Path(".").resolve()))

try:
    import z3
except ImportError:
    print("Z3 not found.")
    sys.exit(1)

def get_turyn_n3():
    A = [1, 1, 1]
    B = [1, 1, -1]
    C = [1, -1]
    D = [1, -1]
    return A, B, C, D

def get_golay_k2():
    F = [1, 1]
    G = [1, -1]
    return F, G

def solve_structural_z3():
    print("Setting up Structural Z3 solver...")
    solver = z3.Solver()
    
    A, B, C, D = get_turyn_n3()
    F, G = get_golay_k2()
    
    n = len(A)
    k = len(F)
    
    # We have 12 logical groups in the code (some loop j=1..k, some are just 1 block)
    # For each group, we allow swapping the roles of sequences.
    
    # Variables:
    # 1. Signs for each of 20 blocks (as before)
    # 2. Swap F/G for each relevant group
    # 3. Swap A/B (or C/D) for each relevant group
    # 4. Invert basis (switch x with y? z with w?) - Let's stick to sequence swaps first.
    
    # Let's count blocks again and map to groups.
    # Group 1: j loop. 2 blocks per j. (Total 2k blocks)
    # Group 2: 1 block (len n).
    # Group 3: 1 block (len n-1).
    # Group 4: j loop. 2 blocks per j.
    # Group 5: j loop. 2 blocks per j.
    # Group 6: 1 block (len n).
    # Group 7: 1 block (len n-1).
    # Group 8: j loop. 2 blocks per j.
    # Group 9: 1 block (len n)
    # Group 10: 1 block (len n-1)
    # Group 11: j loop.
    # Group 12: j loop.
    
    # Wait, my previous code had 12 groups?
    # Original paper has 8 groups. My `solve_construction_2.py` broke them down.
    # Let's use the 12-group structure from `solve_construction_2.py` / `verify_signs_z3.py`.
    
    # For each Group g in 1..12:
    #   swap_seq1_g (Bool): Swap F/G?
    #   swap_seq2_g (Bool): Swap A/B or C/D?
    #   neg_seq1_g (Bool): Negate the Golay part? (Redundant with sign, but maybe useful)
    
    # Actually, "sign" per block covers negation.
    # So we just need "Swap" variables.
    
    groups = 12
    maximize_efficiency = True # Use same swap variable for all iterations in a group? Yes.
    
    swap_golay = [z3.Bool(f'swap_golay_{i}') for i in range(groups + 1)] # 1-indexed
    swap_turyn = [z3.Bool(f'swap_turyn_{i}') for i in range(groups + 1)]
    
    # Signs for 20 blocks
    signs = [z3.Int(f's_{i}') for i in range(20)]
    for s in signs:
        solver.add(z3.Or(s == 1, s == -1))
        
    s_idx = 0
    
    row1, row2, row3, row4 = [], [], [], []
    
    # Helper to select sequence element based on swap variable
    def get_val(cond, val_norm, val_swap):
        return z3.If(cond, val_swap, val_norm)
        
    # GROUP 1: Loop j=1..k
    g = 1
    for j in range(1, k + 1):
        # Block 1a (len n)
        # Original: coeff = x * F[k-j] + z * G[k-j], term = A
        # With swap_golay: coeff = x * G[k-j] + z * F[k-j] (swapped F/G)
        # With swap_turyn: term = B (swapped A/B)
        
        # Terms
        f_val = F[k-j]
        g_val = G[k-j]
        
        # Effective values for x-part (row1/2) and z-part (row3/4)
        # Normal: x->F, z->G
        # Swapped: x->G, z->F
        x_part = get_val(swap_golay[g], g_val, f_val)
        z_part = get_val(swap_golay[g], f_val, g_val)
        
        sign = signs[s_idx]; s_idx+=1
        
        for i in range(n):
            a_val = A[i]
            b_val = B[i]
            # Normal: A, Swapped: B
            t_val = get_val(swap_turyn[g], b_val, a_val)
            
            row1.append(x_part * t_val * sign)
            row2.append(x_part * t_val * sign)
            row3.append(z_part * t_val * sign)
            row4.append(z_part * t_val * sign)
            
        # Block 1b (len n-1)
        # Original: coeff = x * G[j-1] + z * F[k-j], term = C
        # Note: In Block 1a we used k-j for both. Here G uses j-1. 
        # The swap should probably respect position.
        # Swapped: x uses whatever was in z, z uses whatever was in x?
        # OR: Swap F and G symbols entirely in the expression.
        # "F" becomes "G", "G" becomes "F".
        
        val_g_term = G[j-1]  # The thing in the x position
        val_f_term = F[k-j]  # The thing in the z position
        
        # If swapped, x gets F[j-1], z gets G[k-j]?
        # Let's assume the swap means "Replace symbol F with G and G with F"
        val_g_swapped = F[j-1]
        val_f_swapped = G[k-j]
        
        x_part = get_val(swap_golay[g], val_g_swapped, val_g_term)
        z_part = get_val(swap_golay[g], val_f_swapped, val_f_term)
        
        sign = signs[s_idx]; s_idx+=1
        
        for i in range(n-1):
            c_val = C[i]
            d_val = D[i]
            t_val = get_val(swap_turyn[g], d_val, c_val) # Swap C/D
            
            row1.append(x_part * t_val * sign)
            row2.append(x_part * t_val * sign)
            row3.append(z_part * t_val * sign)
            row4.append(z_part * t_val * sign)

    # GROUP 2: Block 2a {x a_i - z b_i}
    g = 2
    # Normal: x->A, z->B (minus handled by sign/coeff)
    # Wait, here A and B are the "Golay-like" components in the vector expression? 
    # NO. The paper says {x a_i - z b_i}. x,z are vectors. a_i, b_i are scalars from Turyn.
    # There is no "Golay" sequence here.
    # So `swap_golay` acts on A vs B? And `swap_turyn` is redundant?
    # Let's use `swap_turyn` to swap A and B usages.
    # `swap_golay` is unused here (or we force it false).
    
    sign = signs[s_idx]; s_idx+=1
    for i in range(n):
        val_a = A[i]
        val_b = B[i]
        
        term_x = get_val(swap_turyn[g], val_b, val_a)
        term_z = get_val(swap_turyn[g], val_a, val_b)
        
        # x part is positive, z part is negative (from formula)
        # We can let the sign variable handle the global sign, but the relative sign (minus) is structural.
        # Let's add a `relative_sign` variable? 
        # Z3 can handle `Int` relative sign.
        # Lets assume the relative minus is fixed, or maybe we add a variable `flip_z_sign`?
        
        row1.append(term_x * sign)
        row2.append(term_x * sign)
        row3.append(-term_z * sign) # Assuming standard - sign
        row4.append(-term_z * sign)

    # GROUP 3: {x d_i - z c_i}
    g = 3
    sign = signs[s_idx]; s_idx+=1
    for i in range(n-1):
        val_d = D[i]
        val_c = C[i]
        term_x = get_val(swap_turyn[g], val_c, val_d)
        term_z = get_val(swap_turyn[g], val_d, val_c)
        
        row1.append(term_x * sign)
        row2.append(term_x * sign)
        row3.append(-term_z * sign)
        row4.append(-term_z * sign)

    # GROUP 4: Loop j=1..k
    g = 4
    for j in range(1, k + 1):
        # Block 4a: coeff = x * G[j-1] + z * F[k-j], term = B
        v_g = G[j-1]
        v_f = F[k-j]
        # Swapped F/G
        v_g_swap = F[j-1]
        v_f_swap = G[k-j]
        
        x_part = get_val(swap_golay[g], v_g_swap, v_g)
        z_part = get_val(swap_golay[g], v_f_swap, v_f)
        
        sign = signs[s_idx]; s_idx+=1
        for i in range(n):
            a_val = A[i]; b_val = B[i]
            t_val = get_val(swap_turyn[g], a_val, b_val)
            
            row1.append(x_part * t_val * sign)
            row2.append(x_part * t_val * sign)
            row3.append(z_part * t_val * sign)
            row4.append(z_part * t_val * sign)
            
        # Block 4b: coeff = -x * F[j-1] + z * G[k-j], term = D
        v_f2 = F[j-1]
        v_g2 = G[k-j]
        v_f2_swap = G[j-1]
        v_g2_swap = F[k-j]
        
        x_part = get_val(swap_golay[g], v_f2_swap, v_f2)
        z_part = get_val(swap_golay[g], v_g2_swap, v_g2)
        
        sign = signs[s_idx]; s_idx+=1
        for i in range(n-1):
            c_val = C[i]; d_val = D[i]
            t_val = get_val(swap_turyn[g], c_val, d_val)
            
            # Note: -x
            row1.append(-x_part * t_val * sign)
            row2.append(-x_part * t_val * sign)
            row3.append(z_part * t_val * sign)
            row4.append(z_part * t_val * sign)

    # GROUP 5: Loop j=1..k (Uses y, w)
    # y=(1, -1, 0, 0), w=(0, 0, 1, -1)
    g = 5
    for j in range(1, k + 1):
        # Block 5a: coeff = y * F[k-j] + w * G[k-j], term = A
        v_f = F[k-j]; v_g = G[k-j]
        v_f_s = G[k-j]; v_g_s = F[k-j]
        
        y_part = get_val(swap_golay[g], v_f_s, v_f)
        w_part = get_val(swap_golay[g], v_g_s, v_g)
        
        sign = signs[s_idx]; s_idx+=1
        for i in range(n):
            val_a = A[i]; val_b = B[i]
            t_val = get_val(swap_turyn[g], val_b, val_a)
            
            # y contributes to row1 (+), row2 (-)
            # w contributes to row3 (+), row4 (-)
            row1.append(y_part * t_val * sign)
            row2.append(-y_part * t_val * sign)
            row3.append(w_part * t_val * sign)
            row4.append(-w_part * t_val * sign)
            
        # Block 5b: coeff = y * G[j-1] - w * F[j-1], term = C
        v_g = G[j-1]; v_f = F[j-1]
        v_g_s = F[j-1]; v_f_s = G[j-1]
        
        y_part = get_val(swap_golay[g], v_g_s, v_g)
        w_part = get_val(swap_golay[g], v_f_s, v_f)
        
        sign = signs[s_idx]; s_idx+=1
        for i in range(n-1):
            val_c = C[i]; val_d = D[i]
            t_val = get_val(swap_turyn[g], val_d, val_c)
            
            row1.append(y_part * t_val * sign)
            row2.append(-y_part * t_val * sign)
            row3.append(-w_part * t_val * sign) # -w
            row4.append(w_part * t_val * sign) # -w * -1 = +w

    # GROUP 6: {-y a_i + w b_i}
    g = 6
    sign = signs[s_idx]; s_idx+=1
    for i in range(n):
        val_a = A[i]; val_b = B[i]
        t_y = get_val(swap_turyn[g], val_b, val_a)
        t_w = get_val(swap_turyn[g], val_a, val_b)
        
        # -y a, + w b
        row1.append(-t_y * sign)
        row2.append(t_y * sign)
        row3.append(t_w * sign)
        row4.append(-t_w * sign)

    # GROUP 7: {y d_i + w c_i}
    g = 7
    sign = signs[s_idx]; s_idx+=1
    for i in range(n-1):
        val_c = C[i]; val_d = D[i]
        t_y = get_val(swap_turyn[g], val_c, val_d)
        t_w = get_val(swap_turyn[g], val_d, val_c)
        
        row1.append(t_y * sign)
        row2.append(-t_y * sign)
        row3.append(t_w * sign)
        row4.append(-t_w * sign)

    # GROUP 8: Loop j=1..k (Expanded to 11 and 12 in code?? No, let's keep it compact)
    # The previous code had groups 8, 9, 10, 11, 12. 
    # Ah, I see row 248 in `solve_construction_2.py`: "Group 8" matches Block 8 in paper.
    # Oh wait, `solve_construction_2` had Group 9, 10 etc!
    # Let me check the paper text logic from `solve_construction_2.py`.
    # Groups 1, 2, 3, 4, 5, 6, 7 are done.
    # `solve_construction_2` has Group 8 (Loop), Group 9 (1 block), Group 10 (1 block), Group 11 (Loop), Group 12 (Loop).
    # This implies the paper has more blocks than I implemented in `verify_signs_z3.py`.
    # `verify_signs_z3.py` only went up to Group 8.
    
    # CRITICAL: `solve_construction_2.py` in my previous `read_file` output shows Group 8 (loop), Group 9 (1 block), Group 10 (1 block), Group 11 (loop), Group 12 (loop).
    # I might have missed blocks in `verify_signs_z3.py`!
    # Let's count again.
    # `verify_signs_z3.py` ended at "Group 8". 
    # `solve_construction_2.py` has Groups 1..12.
    
    # If I missed blocks, that explains why it was UNSAT (wrong length? No, I checked length 50).
    # `solve_construction_2` logic:
    # 1. Loop k (2 blocks) -> 4
    # 2. Block (1) -> 1
    # 3. Block (1) -> 1
    # 4. Loop k (2 blocks) -> 4
    # 5. Loop k (2 blocks) -> 4
    # 6. Block (1) -> 1
    # 7. Block (1) -> 1
    # 8. Loop k (2 blocks) -> 4
    
    # Total so far: 4+1+1+4+4+1+1+4 = 20 blocks.
    # length = 20 blocks.
    # `solve_construction_2.py` Group 9, 10, 11, 12... wait.
    # Let's re-read `solve_construction_2.py` content carefully.
    
    # Lines 152+: Group 8 Loop.
    # Lines 179: Group 9 ?? No. It says "X, Y, Z, W = [], [], [], []". Return.
    # Ah. `best_effort_construction_2.mpl` has Groups 1..12.
    # `solve_construction_2.py` has Groups 1..8.
    
    # Wait, `solve_construction_2.py` lines:
    # 152: # 8. Loop j=1 to k
    # 173: X, Y, Z, W = ...
    # It STOPS after Group 8.
    
    # `best_effort_construction_2.mpl` lines:
    # 70: # Group 8
    # 78: # Group 9 (y A + w B)
    # 84: # Group 10 (y D + w -C)
    # 90: # Group 11 (y G + w F)
    # 98: # Group 12 (y F + w -G)
    
    # Discrepancy!
    # The python code `solve_construction_2.py` implements 8 groups.
    # The Maple code `best_effort_construction_2.mpl` implements 12 groups.
    # The report says "consisting of 8 groups of blocks".
    # BUT `best_effort` uses 12 groups?
    # Maybe `best_effort` tried adding blocks?
    # Or maybe the python code is incomplete structure?
    # The python code length calculation: "Total blocks = 8k + 4 = 20".
    # Resulting sequence length:
    # 4*k blocks of length n (or n-1).
    # n=3. n-1=2.
    # Loop j=1..k (k=2): (3+2)*2 = 10 length.
    # Groups 1,4,5,8 are Loops. 4 * 10 = 40 length.
    # Groups 2,3,6,7 are singles.
    # 2, 6 are len n=3. -> 6.
    # 3, 7 are len=2. -> 4.
    # Total = 40 + 6 + 4 = 50.
    
    # So `solve_construction_2.py` produces length 50.
    # What about Group 9,10,11,12 in Maple?
    # Let's look at Maple code lengths.
    # Groups 1,2,5,6,7,8,11,12 are loops?
    # Maple:
    # G1: loop k
    # G2: loop k
    # G3: 1 block
    # G4: 1 block
    # G5: loop k
    # G6: loop k
    # G7: loop k
    # G8: loop k
    # G9: 1 block
    # G10: 1 block
    # G11: loop k
    # G12: loop k
    
    # This is WAY MORE than 50.
    # Maple has 8 loops and 4 singles.
    # Loop = 3 or 2 length per iter.
    # This suggests `best_effort` is NOT matching `solve_construction_2`.
    
    # Report says: "consisting of 8 groups of blocks".
    # So `solve_construction_2.py` (8 groups) is likely the standard one.
    # The Maple file `best_effort` says "Heuristic Search (Best-Effort File)". Maybe it added blocks?
    
    # I will stick to the 8-group structure from `solve_construction_2.py` which matches the report description "8 groups".
    
    # Back to Group 8 implementation in `solve_structural_z3`.
    
    # GROUP 8: Loop j=1..k
    g = 8
    for j in range(1, k + 1):
        # Block 8a (len n)
        # coeff = y * G[j-1] + w * F[k-j], term = B (with minus sign potentially)
        v_g = G[j-1]; v_f = F[k-j]
        v_g_s = F[j-1]; v_f_s = G[k-j]
        
        y_part = get_val(swap_golay[g], v_g_s, v_g)
        w_part = get_val(swap_golay[g], v_f_s, v_f)
        
        sign = signs[s_idx]; s_idx+=1
        for i in range(n):
            val_b = B[i]; val_a = A[i]
            t_val = get_val(swap_turyn[g], val_a, val_b)
            
            row1.append(y_part * t_val * sign)
            row2.append(-y_part * t_val * sign)
            row3.append(w_part * t_val * sign)
            row4.append(-w_part * t_val * sign)
            
        # Block 8b (len n-1)
        # coeff = y * F[j-1] - w * G[k-j], term = D
        v_f2 = F[j-1]; v_g2 = G[k-j]
        v_f2_s = G[j-1]; v_g2_s = F[k-j]
        
        y_part = get_val(swap_golay[g], v_f2_s, v_f2)
        w_part = get_val(swap_golay[g], v_g2_s, v_g2)
        
        sign = signs[s_idx]; s_idx+=1
        for i in range(n-1):
            val_d = D[i]; val_c = C[i]
            t_val = get_val(swap_turyn[g], val_c, val_d)
            
            row1.append(y_part * t_val * sign)
            row2.append(-y_part * t_val * sign)
            row3.append(-w_part * t_val * sign) # -w
            row4.append(w_part * t_val * sign)

    # ... End of Groups
    
    N = len(row1)
    print(f"Constructed sequence length: {N}")
    
    # Constraints
    for s in range(1, N):
        terms = []
        for i in range(N - s):
            terms.append(row1[i] * row1[i+s])
            terms.append(row2[i] * row2[i+s])
            terms.append(row3[i] * row3[i+s])
            terms.append(row4[i] * row4[i+s])
        solver.add(z3.Sum(terms) == 0)
        
    print("Checking satisfiability for structural modifications...")
    result = solver.check()
    
    if result == z3.sat:
        print("SAT! Found a working structural modification.")
        model = solver.model()
        
        print("Signs:", [model[s].as_long() for s in signs])
        
        print("\nStructural Changes:")
        for g in range(1, groups+1):
            # Only print if group < 9 since we only used 8
            if g > 8: break
            sg = model[swap_golay[g]].__bool__()
            st = model[swap_turyn[g]].__bool__()
            if sg: print(f"Group {g}: Swapped Golay sequences (F <-> G)")
            if st: print(f"Group {g}: Swapped Turyn sequences (A <-> B or C <-> D)")
            
    else:
        print("UNSAT. Even with structural swaps (F/G, A/B), no solution exists.")

if __name__ == "__main__":
    solve_structural_z3()
