
import numpy as np
import sys
from pathlib import Path
import random
import math
import time

# Add current directory to path
sys.path.insert(0, str(Path(".").resolve()))

def get_turyn_n3():
    A = np.array([1, 1, 1])
    B = np.array([1, 1, -1])
    C = np.array([1, -1])
    D = [1, -1] # D is length 2
    # Wait, in original D was length 2?
    # Python code: D = np.array([1, -1])
    # Yes.
    return A, B, C, np.array(D)

def get_golay_k2():
    F = np.array([1, 1])
    G = np.array([1, -1])
    return F, G

def generate_blocks():
    # Generate the pool of 20 blocks described in Construction 2
    # Returns a list of (block_vector, descriptor)
    # block_vector is a 4xL matrix (or list of 4 arrays)
    
    A, B, C, D = get_turyn_n3()
    F, G = get_golay_k2()
    n = len(A)
    # k = 2
    
    # Vectors
    x = np.array([1, 1, 0, 0])
    y = np.array([1, -1, 0, 0])
    z = np.array([0, 0, 1, 1])
    w = np.array([0, 0, 1, -1])
    
    blocks = []
    
    def add_block(name, coeff_vec, turyn_seq):
        # coeff_vec is length 4. turyn_seq is length L.
        # Result is 4 arrays of length L.
        # block = [ row1, row2, row3, row4 ]
        row1 = coeff_vec[0] * turyn_seq
        row2 = coeff_vec[1] * turyn_seq
        row3 = coeff_vec[2] * turyn_seq
        row4 = coeff_vec[3] * turyn_seq
        blocks.append({
            'name': name,
            'data': np.array([row1, row2, row3, row4])
        })

    # Reconstruct blocks from the paper/python logic
    k = 2
    
    # Group 1: Loop j=1..k
    for j in range(1, k+1):
        # 1a
        val_f = F[k-j]; val_g = G[k-j]
        coeff = x * val_f + z * val_g
        add_block(f"G1a_j{j}", coeff, A)
        
        # 1b
        val_g_j = G[j-1]; val_f_k = F[k-j]
        coeff = x * val_g_j + z * val_f_k
        add_block(f"G1b_j{j}", coeff, C)

    # Group 2
    # {x a - z b}
    # This is slightly different structure. It's element-wise different per column?
    # The original python code construct "col" then appended to blocks.
    # Actually, my generic "add_block" assumes coeff is constant for the block.
    # In Group 2: for i in range(n): col = (x*a[i] - z*b[i])
    # This means the coefficient vector CHANGES for each column i.
    # Group 2 is effectively ONE block of length n.
    # I need to construct it manually.
    
    # 2a
    cols = []
    for i in range(n):
        col = (x * A[i] - z * B[i])
        cols.append(col)
    # transpose to 4xN
    mat = np.array(cols).T 
    blocks.append({'name': "G2", 'data': mat})
    
    # Group 3
    cols = []
    for i in range(n-1):
        col = (x * D[i] - z * C[i])
        cols.append(col)
    mat = np.array(cols).T
    blocks.append({'name': "G3", 'data': mat})
    
    # Group 4: Loop j=1..k
    for j in range(1, k+1):
        # 4a
        coeff = x * G[j-1] + z * F[k-j]
        add_block(f"G4a_j{j}", coeff, B)
        # 4b
        coeff = -x * F[j-1] + z * G[k-j]
        add_block(f"G4b_j{j}", coeff, D)
        
    # Group 5
    for j in range(1, k+1):
        # 5a
        coeff = y * F[k-j] + w * G[k-j]
        add_block(f"G5a_j{j}", coeff, A)
        # 5b
        coeff = y * G[j-1] - w * F[j-1]
        add_block(f"G5b_j{j}", coeff, C)
        
    # Group 6
    cols = []
    for i in range(n):
        col = (-y * A[i] + w * B[i])
        cols.append(col)
    mat = np.array(cols).T
    blocks.append({'name': "G6", 'data': mat})
    
    # Group 7
    cols = []
    for i in range(n-1):
        col = (y * D[i] + w * C[i])
        cols.append(col)
    mat = np.array(cols).T
    blocks.append({'name': "G7", 'data': mat})
    
    # Group 8
    for j in range(1, k+1):
        # 8a
        # Note: Original code had a comment about -b_i and logic was: coeff * B.
        # I'll stick to: coeff = y*G + w*F, term=B.
        coeff = y * G[j-1] + w * F[k-j]
        add_block(f"G8a_j{j}", coeff, B)
        # 8b
        coeff = y * F[j-1] - w * G[k-j]
        add_block(f"G8b_j{j}", coeff, D)
        
    return blocks

def calc_npaf(seq_matrix):
    # seq_matrix is 4 x N
    # returns sum of squares NPAF
    rows, N = seq_matrix.shape
    total_energy = 0
    # Manually compute autocorrelation for each row and sum
    for r in range(rows):
        row = seq_matrix[r]
        # autocorr
        # correlate in 'valid' mode gives the dot products
        # We need NPAF for s=1..N-1
        # np.correlate(row, row, mode='full') gives it
        full_corr = np.correlate(row, row, mode='full')
        # midpoint is index N-1 (0 lag).
        # We want lags 1 to N-1.
        # Indices N to 2N-2.
        lags = full_corr[N:]
        total_energy += np.sum(lags**2)
        
    return total_energy

def solve_sa():
    blocks_pool = generate_blocks()
    num_blocks = len(blocks_pool)
    print(f"Total blocks: {num_blocks}")
    
    # State: 
    # 1. Permutation of indices [0..19]
    # 2. Signs [1, 1, ..]
    
    perm = list(range(num_blocks))
    signs = np.ones(num_blocks, dtype=int)
    
    # Construct initial sequence
    def build_seq(p, s):
        # p is permutation list
        # s is signs array
        concatenated = []
        for idx in range(num_blocks):
            block_idx = p[idx]
            sign = s[idx]
            data = blocks_pool[block_idx]['data'] * sign
            concatenated.append(data)
        
        # Concatenate along axis 1 (columns)
        return np.concatenate(concatenated, axis=1)
        
    current_seq = build_seq(perm, signs)
    current_energy = calc_npaf(current_seq)
    
    print(f"Initial Energy: {current_energy}")
    
    best_energy = current_energy
    best_perm = list(perm)
    best_signs = np.array(signs)
    
    T = 1000.0
    cooling = 0.9995
    steps = 100000
    
    for step in range(steps):
        # Mutate
        new_perm = list(perm)
        new_signs = np.array(signs)
        
        r = random.random()
        if r < 0.4:
            # Flip sign of one block
            idx = random.randint(0, num_blocks-1)
            new_signs[idx] *= -1
        elif r < 0.7:
            # Swap two blocks
            i1, i2 = random.sample(range(num_blocks), 2)
            new_perm[i1], new_perm[i2] = new_perm[i2], new_perm[i1]
        else:
            # Move block
            i1 = random.randint(0, num_blocks-1)
            i2 = random.randint(0, num_blocks-1)
            item = new_perm.pop(i1)
            new_perm.insert(i2, item)
            
            # also move sign? No signs are attached to position in array, 
            # but structurally they attach to the block.
            # We should probably permute the signs too if we permute the blocks to keep valid signs attached?
            # Or just let SA learn new signs for positions.
            # Let's permute signs too to keep "good" blocks intact.
            s_item = new_signs[i1] # actually this is harder with numpy array.
            # Let's just reconstruct.
            # Actually, separating signs from blocks is confusing.
            # Let's say: State is list of (block_index, sign).
            pass # The "Move" logic above is buggy for signs.
            # Let's stick to Swap only for permutation.
            # Revert move attempt if needed or implement correctly.
            
            # Use Swap logic only for now for simplicity of implementation
            i1, i2 = random.sample(range(num_blocks), 2)
            new_perm[i1], new_perm[i2] = new_perm[i2], new_perm[i1]
            # Swap signs too?
            # No, let's keep disjoint.
            # Actually, if a block works well with -1, we should keep it -1.
            # So signs should be attached to the block index roughly.
            # But "signs array" is indexed by Position 0..19.
            # If I swap blocks at pos 0 and 1, the signs at pos 0 and 1 stay.
            # This is fine. The optimizer handles it.
            
        new_seq = build_seq(new_perm, new_signs)
        new_energy = calc_npaf(new_seq)
        
        delta = new_energy - current_energy
        
        if delta < 0 or random.random() < math.exp(-delta / T):
            perm = new_perm
            signs = new_signs
            current_energy = new_energy
            
            if current_energy < best_energy:
                best_energy = current_energy
                best_perm = list(perm)
                best_signs = np.array(signs)
                print(f"Step {step}: New Best: {best_energy}")
                if best_energy == 0:
                    print("FOUND PERFECT SOLUTION!")
                    break
        
        T *= cooling
        if T < 0.01: T = 1.0 # Reheat
        
    print("Final Best Energy:", best_energy)
    print("Permutation:", best_perm)
    print("Signs:", best_signs)
    
    # Save solution to file if good
    if best_energy == 0:
        names = [blocks_pool[i]['name'] for i in best_perm]
        print("Block Names:", names)

if __name__ == "__main__":
    solve_sa()
