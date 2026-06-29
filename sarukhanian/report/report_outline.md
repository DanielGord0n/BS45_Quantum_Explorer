# Report Outline Template

## 1. Introduction
- Context on Hadamard/δ-codes and why zero-sum NPAFs matter.
- Explicit goal: construct four ±1 sequences of length 110 with zero summed NPAFs.

## 2. Background
- Definition of nonperiodic autocorrelation (NPAF) and the zero-sum criterion.
- Description of base sequences (BS(3,2)) and pattern columns (x, y, z, w).
- Brief overview of Sarukhanian's Maple construction and known bug.

## 3. Methods
- Maple-to-Python translation strategy and block plan representation.
- Configuration dict and helper APIs for tweaking signs/order.
- Diagnostic tooling: verification function, plots, and saved artifacts.

## 4. Results
- Baseline construction stats (non-zero shifts, worst deviations).
- Repair attempts: manual tweaks, local search, and whether zero-sum achieved.
- Pointer to generated figures and tables.

## 5. Discussion
- Interpretation of what changed between buggy and repaired plans.
- Limitations of current tooling or search coverage.

## 6. Conclusion & Next Steps
- Key takeaways and immediate future work items.

## Appendix A – Key Functions
- Summaries of `build_sarukhanian_110`, `verify_four_sequences`, `auto_local_search`, etc.

## Appendix B – Block Plan Listing
- Full table/listing of the plan (pattern, seq, sign) for reproducibility.
