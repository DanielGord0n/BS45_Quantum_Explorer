
import sys
from pathlib import Path
try:
    import z3
except ImportError:
    print("Z3 not found")
    sys.exit(1)

def solve_synthesis():
    solver = z3.Solver()
    
    # Base Sequences
    # Turyn (n=3)
    A = [1, 1, 1]
    B = [1, 1, -1]
    C = [1, -1]
    D = [1, -1] # Treating D as length 2 based on previous likelihood
    
    # Golay (k=2)
    F = [1, 1]
    G = [1, -1]
    
    # Helper: symbolic choice from list
    def choose_from(options, name_prefix):
        # options is a list of values (e.g. lists or scalars)
        # We create integer index variable
        idx = z3.Int(f'{name_prefix}_idx')
        solver.add(idx >= 0, idx < len(options))
        # Return the symbolic value? 
        # Z3 doesn't support "list indexing by variable" easily without array logic.
        # Instead, we construct the value using If tree.
        val = options[0]
        for i in range(1, len(options)):
            val = z3.If(idx == i, options[i], val)
        return val, idx

    # We need to construct a sequence of length 50.
    # The structure in the report/code had 12 "Groups" (or 8, with 4 loops).
    # Loops iterate k=2 times.
    # Total distinct atomic blocks:
    # Groups 1,4,5,8,11,12 are loops of k=2 -> 2 blocks each.
    # Groups 2,3,6,7,9,10 are singles -> 1 block each.
    # Wait, previous analysis said 8 groups.
    # Let's use a "Bag of Blocks" approach.
    # We need to fill a sequence of length 50.
    # We have slots.
    # Let's assume the *lengths* of the slots are fixed by the paper structure (because index arithmetic depends on it).
    # Structure:
    # 20 blocks.
    # Lengths:
    # 1. 3 (n)
    # 2. 2 (n-1)
    # 3. 3
    # 4. 2
    # 5. 3 (Group 2)
    # 6. 2 (Group 3)
    # ...
    # This assumes the order in `solve_construction_2.py` was roughly correct about lengths.
    
    # Let's define the 20 slots with their fixed lengths.
    # Based on `solve_construction_2.py`:
    # Loop j=1..2: 3, 2
    # Group 2: 3
    # Group 3: 2
    # Loop j=1..2: 3, 2
    # Loop j=1..2: 3, 2
    # Group 6: 3
    # Group 7: 2
    # Loop j=1..2: 3, 2
    
    # Sequence of lengths:
    # 3, 2, 3, 2 (Group 1)
    # 3 (Group 2)
    # 2 (Group 3)
    # 3, 2, 3, 2 (Group 4)
    # 3, 2, 3, 2 (Group 5)
    # 3 (Group 6)
    # 2 (Group 7)
    # 3, 2, 3, 2 (Group 8)
    
    lengths = [3,2,3,2, 3, 2, 3,2,3,2, 3,2,3,2, 3, 2, 3,2,3,2]
    # Sum = 4*5 + 3 + 2 + 4*5 + 4*5 + 3 + 2 + 4*5 = 20 + 5 + 20 + 20 + 5 + 20 = 90 ??
    # Wait.
    # Group 1: j=1 (3,2), j=2 (3,2). Sum = 10.
    # Group 2: 3.
    # Group 3: 2.
    # Group 4: 10.
    # Group 5: 10.
    # Group 6: 3.
    # Group 7: 2.
    # Group 8: 10.
    # Total = 10+3+2 + 10 + 10 + 3+2 + 10 = 50.
    # PERFECT.
    
    # So we have 20 slots.
    # For each slot, we must synthesize the block content.
    # Content is (linear_combination_of_x_y_z_w) * (Turyn_Sequence).
    # Linear combo coefficients come from Golay F,G.
    
    # Variables for each slot i in 0..19:
    # 1. Turyn Choice: A, B, C, D. (Constrained by length! 3->A/B, 2->C/D).
    # 2. Vector X coeff: value from {F, G, -F, -G, 0, 1, -1} ?
    #    Actually, the structure is usually:
    #    coeff = (v1 * scalar1 + v2 * scalar2)
    #    where v1,v2 in {x,y,z,w}.
    #    scalar1, scalar2 in {F[idx], G[idx]}.
    
    # Let's generalize:
    # For each slot, we compute 4 rows.
    # For each row r in 1..4:
    #   multiplier_r = scalar choice from {F elements, G elements, 0, 1} * Sign?
    #   The Turyn sequence is multiplied by multiplier_r.
    #   Actually, all rows use the SAME Turyn sequence (A or B etc).
    
    # Optimization:
    # Each slot `i`:
    #   Choose `seq_i` from {A, B} if len=3, {C, D} if len=2.
    #   Choose `mult_r_i` for r=1..4.
    #   `mult_r_i` is an integer variable.
    #   To keep search space finite, constrain `mult_r_i` to be composed of F/G values.
    #   Possible values in the paper: F[k-j], G[k-j], F[j-1], G[j-1], etc.
    #   Given n=3, k=2.
    #   F = [1, 1], G = [1, -1].
    #   Possible scalar values are {1, -1}.
    #   So `mult_r_i` can be strictly -1 or 1 (or 0?).
    #   Wait, if F/G are only +/-1, then we just need to choose +/-1 for each row?
    #   YES.
    #   The complex formula just results in +/-1 factors for x,y,z,w components!
    #   E.g. x * F[..] -> x * 1.
    #   So really, for each slot, we just need to assign a Sign (+/-1) to each of the 4 rows?
    #   AND choose which sequence t use.
    
    # Variables:
    # For each slot i=0..19:
    #   seq_choice_i (Bool): 0->A/C, 1->B/D.
    #   row_signs_i (Array of 4 Ints): Each is +1 or -1. (Or 0?)
    #   Wait, x=(1,1,0,0). So x contributes to rows 1,2.
    #   z=(0,0,1,1). z contributes to rows 3,4.
    #   So x*F + z*G means rows 1,2 get F, rows 3,4 get G.
    #   Since F,G are +/-1, this means independent signs for (1,2) and (3,4).
    #   Actually, x and y cover rows 1,2.
    #   x=(1,1), y=(1,-1).
    #   So we can generate any pattern (u, v) on rows 1,2 by linear combo of x,y?
    #   Yes, (1,0) = (x+y)/2. (0,1) = (x-y)/2.
    #   But the coefficients in the paper are integers (F, G).
    #   x*F + y*G = (F+G, F-G).
    #   Since F,G are +/-1, F+G is in {2, 0, -2}.
    #   This suggests values can be non-unit?
    #   Let's check the paper values.
    #   x*F (x=1,1, F=1) -> (1,1).
    #   x*F + z*G -> (1, 1, 1, -1) (if G=1,-1... no G is scalar).
    
    #   Okay, let's look at the VALUES of F, G.
    #   F=[1,1], G=[1,-1].
    #   They are strictly {-1, 1}.
    #   So `x*F` is either `(1,1,0,0)` or `(-1,-1,0,0)`.
    #   `z*G` is `(0,0,1,1)` or `(0,0,-1,-1)`.
    #   So `coeff` is a vector of 4 values, each is +/-1 (or 0).
    #   Since the formula ALWAYS combines an (x,y) part with a (z,w) part...
    #   e.g. x*F + z*G -> (F, F, G, G). All +/-1.
    #   x*G + z*F -> (G, G, F, F). All +/-1.
    #   y*F + w*G -> y=(1,-1). yp*F = (F, -F). w=(0,0,1,-1). wp*G = (G, -G).
    #   Result: (F, -F, G, -G). All +/-1.
    
    #   Conclusion: Every row in every block has a multiplier of +1 or -1.
    #   (Assuming no term uses x and y simultaneously? The groups seem separated).
    #   Wait, `solve_construction_2.py` Group 2: `x*A - z*B`.
    #   Here we sum vectors? No. `x` is vector, `A` is scalar in the loop.
    #   `x * A[i]` -> vector (A[i], A[i], 0, 0).
    #   So Group 2 constructs columns where rows 1,2 get A[i], rows 3,4 get -B[i].
    #   This fits the pattern: Row k gets some Turyn element with some sign.
    
    #   REVISED SYNTHESIS MODEL:
    #   For each slot i=0..19:
    #     Length L is fixed (3 or 2).
    #     For each row r=1..4:
    #       We choose a "Source Sequence" S_r from {A, B} (if L=3) or {C,D} (if L=2).
    #       We choose a "Sign" s_r from {+1, -1}.
    #       Row r content = S_r * s_r.
    
    #   Constraint: The columns must be "complementary" in some way?
    #   No, just let Z3 find ANY combination that works.
    #   This is a relaxed superset of the construction.
    #   We have variables:
    #     For i=0..19:
    #       For r=0..3:
    #         choice_ir (Bool): 0->Use First Seq (A/C), 1->Use Second (B/D)
    #         sign_ir (Bool): 0->Positive, 1->Negative
    
    #   This seems solvable. 20 * 4 * 2 = 160 bits. Search space 2^160 is too big for brute force but Z3 might handle it if constraints propagate well.
    #   BUT we must calculate NPAF symbolically.
    #   NPAF is sum of products. Quadratic. Z3 handles quadratic integers okayish.
    
    #   Let's try it.
    
    cols = []
    
    # Base Seqs
    A_val = [1, 1, 1]; B_val = [1, 1, -1]
    C_val = [1, -1]; D_val = [1, -1]
    
    # Variables
    row_seqs = [] # List of 4 lists. Each list has 50 z3 Ints.
    for r in range(4):
        row_seqs.append([])
        
    for i in range(20):
        L = lengths[i]
        
        # Decide choices for this block, for each row
        # To make it more like the construction, we might constrain rows 1,2 to be related?
        # In the construction, rows 1,2 usually identical or negated. 3,4 identical or negated.
        # Let's enforce that constraint to reduce search space.
        # R1, R2 related to x/y.
        # R3, R4 related to z/w.
        # If x is used: R1=v, R2=v.
        # If y is used: R1=v, R2=-v.
        # If z is used: R3=v, R4=v.
        # If w is used: R3=v, R4=-v.
        
        # Let's define "Type 1" (x/y) vars and "Type 2" (z/w) vars for each block.
        # type1_mode: 0->x (1,1), 1->y (1,-1)
        # type2_mode: 0->z (1,1), 1->w (1,-1)
        # But wait, a block might be ALL x? or x+z?
        # The construction always fills all 4 rows.
        # So we assume it's always (Type 1 on top, Type 2 on bottom).
        
        # Vars for block i:
        # top_mode: Bool (x or y pattern)
        # bot_mode: Bool (z or w pattern)
        # top_seq: Bool (A vs B / C vs D)
        # bot_seq: Bool (A vs B / C vs D) ... Wait, construction sometimes mixes A and B in same block?
        # e.g. Group 2: x*A - z*B. Top gets A, Bot gets B.
        # So yes, independent sequence choices.
        # top_sign: Int (+/- 1)
        # bot_sign: Int (+/- 1)
        
        top_mode = z3.Bool(f'b{i}_tm')
        bot_mode = z3.Bool(f'b{i}_bm')
        top_seq_sel = z3.Bool(f'b{i}_ts')
        bot_seq_sel = z3.Bool(f'b{i}_bs')
        top_sign = z3.Int(f'b{i}_tsgn')
        bot_sign = z3.Int(f'b{i}_bsgn')
        
        solver.add(z3.Or(top_sign == 1, top_sign == -1))
        solver.add(z3.Or(bot_sign == 1, bot_sign == -1))
        
        # Values
        if L == 3:
            s1 = A_val; s2 = B_val
        else:
            s1 = C_val; s2 = D_val
            
        # We need to append L integers to each row list
        # We can't append "lists of z3 expressions" easily inside the loop if they depend on Bools
        # We must formulate the expression for each position k in 0..L-1
        
        for k_idx in range(L):
            # Top Val
            # If top_seq_sel false -> s1[k], else s2[k]
            val_s1 = s1[k_idx]
            val_s2 = s2[k_idx]
            
            # Logic: t_val = (top_seq_sel ? val_s2 : val_s1) * top_sign
            t_base = z3.If(top_seq_sel, val_s2, val_s1)
            t_val = t_base * top_sign
            
            # Apply x/y pattern to R1, R2
            # x (0): R1=val, R2=val
            # y (1): R1=val, R2=-val
            r1_val = t_val
            r2_val = z3.If(top_mode, -t_val, t_val)
            
            # Bot Val
            b_base = z3.If(bot_seq_sel, val_s2, val_s1)
            b_val = b_base * bot_sign
            
            # Apply z/w pattern
            # z (0): R3=val, R4=val
            # w (1): R3=val, R4=-val
            r3_val = b_val
            r4_val = z3.If(bot_mode, -b_val, b_val)
            
            row_seqs[0].append(r1_val)
            row_seqs[1].append(r2_val)
            row_seqs[2].append(r3_val)
            row_seqs[3].append(r4_val)

    # Now we have 4 rows of length 50.
    N = 50
    print("Building NPAF constraints...")
    
    # NPAF sum = 0 for s=1..N-1
    for s in range(1, N):
        terms = []
        for r in range(4):
            # Sum_{i=0}^{N-1-s} row[i]*row[i+s]
            for i in range(N - s):
                terms.append(row_seqs[r][i] * row_seqs[r][i+s])
        
        solver.add(z3.Sum(terms) == 0)
        
    print("Checking...")
    res = solver.check()
    if res == z3.sat:
        print("SAT! Found a working construction.")
        m = solver.model()
        # Extract solution
        final_rows = [[], [], [], []]
        for r in range(4):
            for i in range(N):
                val = m.evaluate(row_seqs[r][i]).as_long()
                final_rows[r].append(val)
        print("Rows generated.")
        # Print first few elements
        print("R1:", final_rows[0][:10])
    else:
        print("UNSAT. Even with this relaxed model, it is impossible.")

if __name__ == "__main__":
    solve_synthesis()
