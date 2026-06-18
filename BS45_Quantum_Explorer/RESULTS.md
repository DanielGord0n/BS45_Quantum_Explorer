# Results — wz_exact_t23 base-sequence solver

A from-scratch C++ solver for **base sequences BS(n+1,n)** (four ±1 sequences A,B of
length n+1 and C,D of length n whose nonperiodic autocorrelations sum to zero — i.e. a
δ-code). It implements the published state-of-the-art search: **signature targeting +
Wang–Zhu Theorem 2.3 residue prune + Theorem 2.4 (`hall_ok`) spectral filter +
C,D-then-A,B backtracking**, parallelized with OpenMP and resumable via bitmap
checkpoints.

## Verified results

**1. Blind discovery (no seed — found from scratch):**

| Target | Signature | Result |
|--------|-----------|--------|
| BS(7,6)   | (5,1,0,0) | found + `verify_npaf` PASS |
| BS(11,10) | (5,1,4,0) | found + `verify_npaf` PASS |
| BS(19,18) | (7,5,0,0) | found from scratch in ~1.3 s, `verify_npaf` PASS |

Run any of these: `./wz_exact_t23 <n> <a> <b> <c> <d>` (e.g. `./wz_exact_t23 18 7 5 0 0`).

**2. Reproduction of the published BS(43,42) δ-code (independently verified):**

Blind discovery at n=42 is infeasible with this method (search-tree wall — see
`HANDOFF.md`). The solver instead fixes the first K of the 21 layers of the known
published solution and **blindly searches the remaining (21−K)**, then `verify_npaf.py`
(separate code path) confirms the result:

```
$ ./reproduce_bs43.sh           # K=13: fix 13 layers, search 8 blind (~15 s)
*** REPRODUCTION CONFIRMED: BS(43,42) FOUND ***
sig = (7,11,0,0)
A = {...}  B = {...}  C = {...}  D = {...}
=== independent verification (verify_npaf.py) ===
Lengths: A=43, B=43, C=42, D=42
Signature: (a=7, b=11, c=0, d=0)  a²+b²+c²+d²=170 (expect 170)
PASS: NPAF[s]=0 for all s=1..43
PASS: fits Wang-Zhu pair encoding
```

`./reproduce_bs43.sh K` lowers the seed (smaller K = more layers searched blind).
On one core, **K=13 (~15 s) is the practical floor**; K≤12 searches ≥9 layers blind and
exceeds 10 min — a concrete demonstration of the per-layer exponential growth. No cluster
or HPC access is needed for this reproduction; it runs on a laptop.

## Honest scope

- **Solver correctness is established** (blind small cases + verified n=42 reproduction).
- **Blind n=42 and the BS(45,44) "world record" (n=44) are not reachable** by this
  brute-force method — the search tree grows exponentially and the per-combo subtrees
  exceed any walltime (the "monster-combo wall"); the feasibility frontier is ~8 freely
  searched layers. Every prune lever was tried (mod-3 residue, (P,Q)-reachability,
  partial-CD spectral) — all net-zero/negative. Full analysis + the n=41/42/43-are-
  recent / n=44-is-open literature is in `HANDOFF.md` (2026-06-18 entries).
- **n=44 is a genuine open problem** for the whole field (verified only up to n=43,
  Đoković/Kotsireas), so reaching it needs new mathematics, not more compute.
