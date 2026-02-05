# CP468 Sarukhanian Project

Investigation and verification of Sarukhanian's constructions for generating δ-codes.

## Project Structure

```
CP468_Sarukhanian_Project/
├── README.md                    # This file
├── requirements.txt             # Python dependencies
├── project_resume_summary.md    # ML techniques summary for resume
│
├── scripts/                     # All verification scripts
│   ├── verify_signs_z3.py       # Z3 sign verification
│   ├── fix_construction_z3.py   # Z3 structural swap verification
│   ├── synthesize_construction_z3.py  # Z3 free synthesis
│   ├── search_parameters_z3.py  # Parameter sensitivity analysis
│   ├── solve_construction_2.py  # Brute force + genetic algorithm
│   ├── solve_ordering_sa.py     # Simulated annealing optimization
│   ├── solve_simple_yang.py     # Working Yang's Base Sequences synthesis
│   └── fallback_generate.py     # Fallback generation script
│
├── src/                         # Core library modules
│   ├── __init__.py
│   ├── construction.py          # Construction implementations
│   ├── npaf.py                  # Non-periodic autocorrelation functions
│   ├── sequences.py             # Turyn/Golay sequence generators
│   ├── exhaustive_search.py     # Exhaustive search algorithms
│   ├── greedy_search.py         # Greedy search algorithms
│   ├── repair.py                # Repair heuristics
│   └── viz.py                   # Visualization utilities
│
├── submission/                  # Final deliverables
│   ├── construction_1/          # Construction 1 (working)
│   │   └── ...
│   └── construction_2/          # Construction 2 (disproved)
│       ├── report.md            # FINAL COMPREHENSIVE REPORT
│       ├── sarukhanian_construction_2.mpl  # Original Maple implementation
│       └── best_effort_construction_2.mpl  # Best approximation found
│
├── tests/                       # Unit tests
└── venv/                        # Python virtual environment
```

## Quick Start

```bash
# Activate virtual environment
source venv/bin/activate

# Run the working Yang synthesis
python scripts/solve_simple_yang.py

# Run parameter search verification
python scripts/search_parameters_z3.py
```

## Key Findings

1. **Construction 1:** Successfully implemented and verified
2. **Construction 2:** Proven mathematically impossible (see `submission/construction_2/report.md`)
3. **Alternative Solution:** Valid δ-code of length 52 synthesized via Yang's Base Sequences

## Dependencies

- Python 3.x
- NumPy
- Z3 Theorem Prover (`pip install z3-solver`)
