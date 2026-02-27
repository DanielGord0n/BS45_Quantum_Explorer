# CP468: Base Sequence Solver Project

## Overview
This project implements the state-of-the-art Wang-Zhu algorithm (arXiv:2506.20296) for discovering Base Sequences $BS(n+1,n)$. Base sequences are four sequences of $\pm 1$ and lengths $n+1, n+1, n, n$ with zero non-periodic autocorrelation (NPAF).

The repository includes:
1. **A highly-optimized C++ solver** that exhaustively searches and verifies Base Sequences up to $n \le 15$ in seconds.
2. **A verifier script** containing the exact, mathematically proven sequences for $BS(43,42)$ and $BS(44,43)$ extracted directly from the Wang-Zhu paper's raw LaTeX source.

---

## Directory Structure
- `bin/`: Compiled executables.
- `src/solver/`: Source code for the Wang-Zhu base sequence solver.
- `src/verifier/`: Source code for decoding and verifying the $n=43, 44$ sequences.
- `report/`: Project documentation and write-ups.

---

## Building the Tools

This project requires a C++17 compatible compiler with OpenMP support (for multi-threading).

### macOS (Apple Silicon)
Assuming Homebrew `libomp` is installed (`brew install libomp`):
```bash
# Compile Solver
g++ -O3 -march=native -std=c++17 -Xpreprocessor -fopenmp -I/opt/homebrew/opt/libomp/include -L/opt/homebrew/opt/libomp/lib -lomp -o bin/wz_solver src/solver/wang_zhu_final.cpp

# Compile Verifier
g++ -O3 src/verifier/verify_bs43.cpp -o bin/verify_bs43
```

---

## Usage

### 1. The Solver
The solver (`wz_solver`) implements Sum Signatures (Theorem 2.1), NPAF Feasibility Pruning, and Hall Polynomial filtering (Theorem 2.4). It completes exhaustive search for small $n$ and falls back to probabilstic sampling for large $n$.

```bash
# Usage: ./wz_solver <n_length> [threads]
./bin/wz_solver 4
./bin/wz_solver 12
```

### 2. The Verifier
The verifier (`verify_bs43`) mathematically tests the specific $BS(43,42)$ and $BS(44,43)$ strings extracted from the arXiv payload, ensuring their NPFA sum is identically $0$ at all shifts.

```bash
./bin/verify_bs43
```

## Results
- The solver instantly verifies known base sequences like $BS(5,4)$ and finds $BS(13,12)$ in under 6 seconds on an M4 Pro.
- The verifier mathematically proves that the exact sequences for $BS(43,42)$ and $BS(44,43)$ satisfy the Zero-Autocorrelation property, confirming the state of the art.
