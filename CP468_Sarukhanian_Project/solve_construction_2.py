
import numpy as np
import sys
from pathlib import Path
import random
from copy import deepcopy
import json

# Add current directory to path
sys.path.insert(0, str(Path(".").resolve()))

from src.npaf import npaf_sum_four

def get_turyn_n3():
    A = np.array([1, 1, 1])
    B = np.array([1, 1, -1])
    C = np.array([1, -1])
    D = np.array([1, -1])
    return A, B, C, D

def get_golay_k2():
    F = np.array([1, 1])
    G = np.array([1, -1])
    return F, G

def build_sequence(signs, A, B, C, D, F, G):
    n = len(A)
    k = len(F)
    
    x = np.array([1, 1, 0, 0])
    y = np.array([1, -1, 0, 0])
    z = np.array([0, 0, 1, 1])
    w = np.array([0, 0, 1, -1])
    
    blocks = []
    
    # We will use the 'signs' list to flip signs of entire blocks if needed.
    # There are many blocks.
    # Let's define the structure first, then apply signs.
    
    # To make SA effective, we need to parameterize the signs of each term in the formula.
    # The formula has 8 main groups.
    # Groups 1, 4, 5, 8 have 'k' iterations.
    # Groups 2, 3, 6, 7 have 1 iteration.
    # Total "terms" = 4*k + 4.
    # But each term inside the loop has sub-parts (length n and length n-1).
    # Let's count the distinct "atomic" blocks we are concatenating.
    # 1. k pairs of (n, n-1) -> 2k blocks
    # 2. 1 pair (n, n-1) -> 2 blocks
    # 3. k pairs -> 2k blocks
    # 4. k pairs -> 2k blocks
    # 5. 1 pair -> 2 blocks
    # 6. k pairs -> 2k blocks
    # Total blocks = 2k + 2 + 2k + 2k + 2 + 2k = 8k + 4.
    # For k=2, that's 16 + 4 = 20 blocks.
    
    # We will pass a list of 20 signs.
    
    s_idx = 0
    
    # 1. Loop j=1 to k
    for j in range(1, k + 1):
        # Block 1a (len n)
        val_f = F[k-j]
        val_g = G[k-j]
        coeff = x * val_f + z * val_g
        for val_a in A:
            blocks.append(coeff * val_a * signs[s_idx])
        s_idx += 1
            
        # Block 1b (len n-1)
        val_g_j = G[j-1]
        val_f_k = F[k-j]
        coeff = x * val_g_j + z * val_f_k
        for val_c in C:
            blocks.append(coeff * val_c * signs[s_idx])
        s_idx += 1

    # 2. {x a_i - z b_i}_{i=1}^n
    # Block 2a (len n)
    for i in range(n):
        val_a = A[i]
        val_b = B[i]
        col = (x * val_a - z * val_b) * signs[s_idx]
        blocks.append(col)
    s_idx += 1
        
    # 3. {x d_i - z c_i}_{i=1}^{n-1}
    # Block 3a (len n-1)
    for i in range(n-1):
        val_d = D[i]
        val_c = C[i]
        col = (x * val_d - z * val_c) * signs[s_idx]
        blocks.append(col)
    s_idx += 1
        
    # 4. Loop j=1 to k
    for j in range(1, k + 1):
        # Block 4a (len n)
        val_g = G[j-1]
        val_f = F[k-j]
        coeff = x * val_g + z * val_f
        for val_b in B:
            blocks.append(coeff * val_b * signs[s_idx])
        s_idx += 1
            
        # Block 4b (len n-1)
        val_f_j = F[j-1]
        val_g_k = G[k-j]
        coeff = -x * val_f_j + z * val_g_k
        for val_d in D:
            blocks.append(coeff * val_d * signs[s_idx])
        s_idx += 1
            
    # 5. Loop j=1 to k
    for j in range(1, k + 1):
        # Block 5a (len n)
        val_f = F[k-j]
        val_g = G[k-j]
        coeff = y * val_f + w * val_g
        for val_a in A:
            blocks.append(coeff * val_a * signs[s_idx])
        s_idx += 1
            
        # Block 5b (len n-1)
        val_g_j = G[j-1]
        val_f_j = F[j-1]
        coeff = y * val_g_j - w * val_f_j
        for val_c in C:
            blocks.append(coeff * val_c * signs[s_idx])
        s_idx += 1
            
    # 6. {-y a_i + w b_i}_{i=1}^n
    # Block 6a (len n)
    for i in range(n):
        val_a = A[i]
        val_b = B[i]
        col = (-y * val_a + w * val_b) * signs[s_idx]
        blocks.append(col)
    s_idx += 1
        
    # 7. {y d_i + w c_i}_{i=1}^{n-1}
    # Block 7a (len n-1)
    for i in range(n-1):
        val_d = D[i]
        val_c = C[i]
        col = (y * val_d + w * val_c) * signs[s_idx]
        blocks.append(col)
    s_idx += 1
        
    # 8. Loop j=1 to k
    for j in range(1, k + 1):
        # Block 8a (len n)
        val_g = G[j-1]
        val_f = F[k-j]
        coeff = y * val_g + w * val_f
        # Note: Original formula had -b_i. We capture that in the base logic or let SA find it.
        # Original: -b_i(y g_j + w f_{k-j+1})
        # My code here: coeff * val_b * signs[...]
        # If signs is -1, it matches.
        for val_b in B:
            blocks.append(coeff * val_b * signs[s_idx])
        s_idx += 1
            
        # Block 8b (len n-1)
        val_f_j = F[j-1]
        val_g_k = G[k-j]
        coeff = y * val_f_j - w * val_g_k
        for val_d in D:
            blocks.append(coeff * val_d * signs[s_idx])
        s_idx += 1
            
    X, Y, Z, W = [], [], [], []
    for col in blocks:
        X.append(col[0])
        Y.append(col[1])
        Z.append(col[2])
        W.append(col[3])
        
    return np.array(X), np.array(Y), np.array(Z), np.array(W)

def get_score(signs, A, B, C, D, F, G):
    x, y, z, w = build_sequence(signs, A, B, C, D, F, G)
    diag = npaf_sum_four(x, y, z, w)
    non_zero = np.nonzero(diag)[0]
    max_abs = np.max(np.abs(diag)) if diag.size else 0
    return non_zero.size * 1000 + max_abs

def solve():
    A, B, C, D = get_turyn_n3()
    F, G = get_golay_k2()
    
    # Initial signs: All 1s (except where formula explicitly had minus, but we simplified)
    # Actually, let's try to match the formula's explicit signs first.
    # But SA is robust, let's start with all 1s and let it flip.
    # Total blocks = 20.
    
    current_signs = [1] * 20
    
    # Manual adjustments to match formula "visual" signs if possible?
    # The formula has some explicit minuses.
    # e.g. Block 6: -y a_i.
    # Block 8a: -b_i.
    # Let's just let SA find it.
    
    current_score = get_score(current_signs, A, B, C, D, F, G)
    print(f"Initial score: {current_score}")
    
    best_signs = list(current_signs)
    best_score = current_score
    
    temp = 100.0
    cooling_rate = 0.995
    
    for i in range(50000):
        idx = random.randint(0, 19)
        neighbor_signs = list(current_signs)
        neighbor_signs[idx] *= -1
        
        neighbor_score = get_score(neighbor_signs, A, B, C, D, F, G)
        
        delta = neighbor_score - current_score
        if delta < 0 or random.random() < np.exp(-delta / temp):
            current_signs = neighbor_signs
            current_score = neighbor_score
            
            if current_score < best_score:
                best_score = current_score
                best_signs = list(current_signs)
                print(f"Iter {i}: New best score {best_score}")
                if best_score == 0:
                    print("FOUND PERFECT SOLUTION!")
                    break
        
        temp *= cooling_rate
        if temp < 0.1:
            temp = 100.0
            
    print("\nBest signs found:")
    print(best_signs)
    
    # Check against Hardcoded Best Effort from SA
    # Energy: 2404
    # Perm: [3, 15, 0, 13, 5, 19, 16, 2, 17, 18, 10, 12, 9, 8, 4, 1, 14, 6, 11, 7]
    # Signs: [-1  1 -1  1  1 -1 -1  1 -1  1  1 -1 -1  1  1 -1  1 -1 -1  1]
    
    # Verify
    x, y, z, w = build_sequence(best_signs, A, B, C, D, F, G)
    diag = npaf_sum_four(x, y, z, w)
    print(f"Final Non-zero shifts: {np.nonzero(diag)[0].size}")

if __name__ == "__main__":
    solve()
