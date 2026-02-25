import numpy as np
import time
import sys

def calculate_npaf_energy(A, B, C, D, n):
    energy = 0
    for s in range(1, n):
        corr = sum(A[:n-s] * A[s:]) + sum(B[:n-s] * B[s:]) + \
               sum(C[:n-s] * C[s:]) + sum(D[:n-s] * D[s:])
        energy += corr * corr
    return energy

def calculate_npaf_energy_fast(seqs, n):
    energy = 0
    for s in range(1, n):
        corr = 0
        for seq in seqs:
            corr += np.sum(seq[:n-s] * seq[s:])
        energy += corr * corr
    return energy

def tabu_search_bs(n, max_iterations=10000, tabu_tenure=50):
    # Initialize randomly with +/- 1
    seqs = np.random.choice([-1, 1], size=(4, n))
    current_energy = calculate_npaf_energy_fast(seqs, n)
    best_energy = current_energy
    best_seqs = np.copy(seqs)
    
    # Tabu list: stores (seq_idx, pos) and the iteration it expires
    tabu_list = np.zeros((4, n), dtype=int)
    
    start_time = time.time()
    
    for iteration in range(max_iterations):
        if best_energy == 0:
            break
            
        best_neighbor_energy = float('inf')
        best_move = None
        
        # Evaluate all possible 1-flip neighborhood
        for seq_idx in range(4):
            for i in range(n):
                # Flip bit
                seqs[seq_idx, i] *= -1
                
                # Full evaluation (can be optimized with delta updates, but keeping simple/fast in numpy)
                new_energy = calculate_npaf_energy_fast(seqs, n)
                
                # Check if tabu
                is_tabu = tabu_list[seq_idx, i] > iteration
                
                # Aspiration criterion: if it beats the global best, ignore tabu
                if new_energy < best_neighbor_energy and (not is_tabu or new_energy < best_energy):
                    best_neighbor_energy = new_energy
                    best_move = (seq_idx, i)
                
                # Revert flip
                seqs[seq_idx, i] *= -1
        
        # Apply the best move
        if best_move is not None:
            seq_idx, i = best_move
            seqs[seq_idx, i] *= -1
            current_energy = best_neighbor_energy
            
            # Update Tabu list
            tabu_list[seq_idx, i] = iteration + tabu_tenure
            
            if current_energy < best_energy:
                best_energy = current_energy
                best_seqs = np.copy(seqs)
                print(f"Iter {iteration}: New Best Energy = {best_energy}")
                
        if iteration % 1000 == 0:
            print(f"Iteration {iteration}, Current Energy: {current_energy}, Best: {best_energy}")
            
    elapsed = time.time() - start_time
    return best_seqs, best_energy, elapsed

if __name__ == "__main__":
    n = 50
    if len(sys.argv) > 1:
        n = int(sys.argv[1])
        
    print(f"Starting Tabu Search for BS({n})")
    print(f"Search space size: 2^{4*n} = {2**(4*n):.2e} combinations")
    
    best_seqs, best_energy, elapsed = tabu_search_bs(n, max_iterations=5000, tabu_tenure=min(50, n))
    
    print(f"\nSearch finished in {elapsed:.2f} seconds.")
    print(f"Best Energy Found: {best_energy}")
    if best_energy == 0:
        print("SUCCESS! Found mathematically perfect Base Sequences.")
        print("A =", list(best_seqs[0]))
        print("B =", list(best_seqs[1]))
        print("C =", list(best_seqs[2]))
        print("D =", list(best_seqs[3]))
    else:
        print(f"FAILED to reach 0 energy. Best was {best_energy}.")
        print("This highlights the immense difficulty of finding new Base Sequences.")
