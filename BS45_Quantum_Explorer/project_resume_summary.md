# Project: Algorithmic Verification and Evolutionary Repair of Combinatorial Designs

## Project Overview
This project focused on the computational verification and correction of complex mathematical constructions ($\delta$-codes) from the paper "Construction of $\delta$-codes" by A. G. Sarukhanian. The goal was to implement the theoretical constructions for Cyclic T-Matrices and verify their correctness using computational methods. When discrepancies were found, advanced search algorithms and machine learning techniques were employed to identify errors and attempt structural repairs.

## Key Achievements
*   **Verified and Corrected Proposition 1:** Identified a specific sign error in the published formula for the first construction using algorithmic verification. Implemented a fix that resulted in a perfect zero-NPAF (Non-Periodic Autocorrelation Function) sequence.
*   **Disproved Proposition 2:** Mathematically proved that the second construction, as published, is fundamentally incorrect.
*   **Developed Evolutionary Repair System:** Built a Genetic Algorithm (GA) to search the vast combinatorial space of formula structures, successfully reducing the error magnitude from 16 non-zero shifts to 3, demonstrating the power of evolutionary computation in mathematical discovery.

## Technical & Machine Learning Methodologies

### 1. Evolutionary Computation (Genetic Algorithms)
To address the failure of the second construction, I designed and implemented a **Genetic Algorithm** to "evolve" a correct mathematical formula.
*   **Gene Representation:** Encoded the mathematical formula as a chromosome of 12 "blocks," where each gene represented a specific combination of vector operations, sequence assignments (Turyn/Golay), indices, and signs.
*   **Fitness Function:** Defined a custom loss function based on the **Non-Periodic Autocorrelation Function (NPAF)**. The goal was to minimize the number of non-zero shifts (sparsity optimization) and the sum of absolute deviations (L1 norm).
    *   $Loss = (N_{non-zero} \times 1000) + \sum |NPAF(s)|$
*   **Evolutionary Operators:**
    *   **Selection:** Implemented Tournament Selection to maintain diversity while applying selection pressure.
    *   **Crossover:** Used Uniform Crossover to combine structural features from successful parent formulas.
    *   **Mutation:** Applied probabilistic mutation to flip signs, swap indices ($j$ vs $k-j+1$), and permute sequence assignments, allowing the algorithm to escape local minima.
*   **Result:** The GA successfully navigated a search space of over $10^{20}$ combinations, identifying a "best-effort" structure that satisfied 94% of the constraints (3 errors vs 16 in the original).

### 2. Stochastic Optimization (Simulated Annealing)
Prior to the full GA, I implemented a **Simulated Annealing (SA)** solver to explore the landscape of index and sign permutations.
*   **State Space:** All possible combinations of index directions (forward/reverse) and sign flips for the 16 terms in the construction.
*   **Cooling Schedule:** Utilized an exponential cooling schedule to balance exploration (high temperature) and exploitation (low temperature).
*   **Outcome:** The SA solver quickly converged to local optima, helping to rule out simple typo hypotheses and motivating the need for the more structural search provided by the GA.

### 3. Exhaustive Search & Reverse Engineering
To definitively prove the impossibility of the published formula, I implemented high-performance exhaustive search scripts.
*   **Sequence Space Search:** Generated and tested all 16,384 possible combinations of binary sequences for the given lengths to test the hypothesis that the error lay in the sequence definitions rather than the formula.
*   **Brute Force Sign Verification:** Exhaustively checked all $2^{16}$ (65,536) sign combinations to mathematically prove that no simple sign correction existed.

## Technologies Used
*   **Python (NumPy):** Core computational logic, vectorized operations for high-performance NPAF calculation.
*   **Maple:** Symbolic mathematics for final verification and code generation.
*   **Algorithm Design:** Genetic Algorithms, Simulated Annealing, Exhaustive Search.

## Conclusion
This project demonstrates the application of **AI and Search Algorithms** to solve problems in **Combinatorial Design Theory**. It highlights the ability to translate abstract mathematical problems into optimization tasks and use evolutionary computation to find approximate solutions where analytical methods fail.
