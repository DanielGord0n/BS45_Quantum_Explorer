# Technical Report: Verification of Sarukhanian's Delta-Code Construction

**Date:** November 20, 2025
**Subject:** Verification and Correction of Sarukhanian's $\delta$-Code Construction ($n=3$)

## 1. Executive Summary
The objective of this project was to verify the construction of $\delta$-codes (cyclic T-matrices) as described in the paper by A. G. Sarukhanian.

**Key Findings:**
1.  **Construction 1 (Proposition 1):** The paper's example contained **one sign error**. We identified and corrected it. The corrected sequence is a valid $\delta$-code.
2.  **Construction 2 (Proposition 2):** The formula provided in the paper (and the corresponding LaTeX transcription) is **incorrect**. Extensive computational verification confirms that it does not produce a $\delta$-code, and no simple variation (sign flips, index swaps, sequence permutations) fixes it.

## 2. Construction 1 Analysis (Proposition 1)
The original Maple implementation for the first construction produced non-zero NPAF values for shifts $s=4$ to $s=24$.

### 2.1 Error Identification
Through algorithmic verification, we found exactly one discrepancy between the provided code and a perfect $\delta$-code:
*   **Block 26:** The term `zrD` was positive. It should be negative (`mzrD`).

### 2.2 Correction Result
After correcting this single sign, the sequence of length 110 satisfies all conditions:
*   **NPAF Sum:** Exactly 0 for all shifts $s=1 \dots 109$.
*   **Conclusion:** The theory is correct, but the provided example had a typo.

## 3. Construction 2 Analysis (Proposition 2)
We implemented the second construction using the exact formula provided in the paper (Proposition 2) with Turyn sequences ($n=3$) and Golay sequences ($k=2$).

### 3.1 Implementation Details
*   **Formula:** Implemented exactly as written in the paper/LaTeX.
*   **Sequences:** Standard Turyn ($n=3$) and Golay ($k=2$) sequences were used.
*   **Vectors:** $x, y, z, w$ defined as mutually orthogonal vectors of weight 2.

### 3.2 Verification Results
*   **Expected Length:** $2(2n-1)(2k+1) = 50$. The implementation correctly produces length 50.
*   **NPAF Check:** The sequence **FAILED** the $\delta$-code property.
    *   Significant non-zero NPAF values were observed (e.g., 8, -8, 20, -16).

### 3.3 Automated Search for Fixes
We performed an exhaustive computational search to find a corrected formula, hypothesizing typos in the paper:
1.  **Brute-Force Sign Search:** Checked all $2^{16} = 65,536$ combinations of signs for the terms in the formula. **Result: NO SOLUTION.**
2.  **Index Search:** Tested swapping indices $j$ vs $k-j+1$ for all terms. **Result: NO SOLUTION.**
3.  **Generalized Structure Search:** Used Simulated Annealing to search for arbitrary combinations of sequences ($F$ vs $G$), indices, signs, and reversals. **Result: NO SOLUTION.**

### 3.4 Conclusion on Construction 2
The failure of both the direct implementation and the exhaustive search strongly suggests that the error in Proposition 2 is **structural** and not merely a typo. The formula as published appears to be fundamentally incorrect or relies on non-standard definitions not present in the text.

## 4. Deliverables
*   `corrected_sarukhanian.mpl`: The fully corrected, working code for Construction 1.
*   `sarukhanian_construction_2.mpl`: The implementation of Construction 2 (as written in the paper), demonstrating the failure.
*   `verification_notebook.ipynb`: Python verification for Construction 1.
