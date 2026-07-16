# Base-sequence search: results and a measured frontier — brief for Prof. Kotsireas

*Daniel Gordon, 2026-07-07. One page. This is a methods question, not a compute request.*

## What we have found (blind metaheuristic, all independently NPAF-verified)

Simulated annealing (`wz_sa_v8`: phased CD-then-AB with coupled refinement), run as
8-task × 192-thread SLURM arrays (~1,536 chains/cluster) across four Alliance Canada
clusters, found — **blind, no prefix, signature discovered by search**:

| Sequence | sig (a,b,c,d) | Found | Verification |
|---|---|---|---|
| BS(30,29) | (4,−10,1,1) | 2026-06-29 | NPAF[s]=0 ∀s, independent checker |
| BS(30,29) | (0,6,9,1) | 2026-06-30 | ″ |
| BS(31,30) | (1,−7,6,6) | 2026-06-30 | ″ |
| BS(32,31) | (0,−6,9,−3) | 2026-07-04 | ″ |

Full ±1 sequences available on request (`results/champions/`). Empirical cost is a steep
ladder: n=30 fell after ~4 node-days×1,536 chains, n=31 after ~9; n=32 is in progress. We
do not represent these as new to the literature — they are capability results for a blind
solver, and the interesting part is what we measured at the frontier.

## The measured frontier (why our complete method stops near n≈29)

We also built the complete hash-join ("matching") solver in the Đoković–Kotsireas style:
mod-3→mod-6 residue class-sum profiles (Thm 2.3-type) + the 200-point spectral bound
f ≤ 4n+2 (Thm 2.4-type) as generation filters, 64-bit-key join on the autocorrelation
vector, exact NPAF recheck on hits. It blindly reproduces BS(19,18) in 51 s on one
192-core node, and passes an exhaustive small-n ground-truth audit (280/280
solution-admitting signatures at n=5–9, odd n included).

Streaming (O(1)-memory) counting of the filtered candidate space then gives hard numbers:

| n | sig | spec-ok C,D candidates | join pair-tests required |
|---|---|---|---|
| 29 | (0,6,9,1) | 1.8×10⁸ × 5.4×10⁸ | **1.6×10¹⁵** per signature |
| 31 | (6,4,7,5) | 2.3×10⁹ × 3.1×10⁹ | **4.0×10¹⁶** per signature |

At ~10¹⁰ pair-tests/s/node that is months per signature at n=31, growing ~5–30× per rung —
infeasible regardless of memory. Our conclusion: **with Thm 2.3/2.4-strength filters used
as we use them, the complete route is closed above n≈29 on this hardware; the gap to
Wang-Zhu's n=41–43 constructions (arXiv:2506.20296) is filter strength, not compute.**
Their per-sequence filtering during construction appears effectively ~10³× tighter than
what we reconstruct from the paper.

One more measurement (2026-07-08): we implemented the "extend to modulus 6" step as mod-6
class-sum profiles constrained by the norm identity at both moduli (validated: all our banked
solutions pass it; exact agreement with brute-force ground truth at small n). At n=31 it
reduces the filtered stream by **0.15%** over mod-3; at n=36 the mod-6 profile space alone
is ~1.7×10⁶ pairs and the stream exceeds 4×10¹⁷. So the class-sum reading of the mod-6 lift
does essentially no pruning at scale — whatever carries Wang-Zhu to n=41–43 must be a
different (or additional) constraint.

## The questions

1. What does the Wang-Zhu-style generate step actually cost per signature at n≈40 — and
   which constraint does the heavy lifting? Measured: it is not the mod-6 class-sum lift
   (0.15% at n=31) and not the published Thm 2.3/2.4 forms as we apply them.
2. Is a faithful first-hit reproduction (generate constrained C,D → backtrack A,B → stop at
   first solution) realistically within reach of ~4×192-core clusters at n=42–43, and would
   a careful reimplementation be a useful replication?
3. In your view, is n=44 reachable by any residue/constructive refinement of this family of
   methods, or is it new-mathematics-only?

We have four clusters' worth of validated tooling and would value 30 minutes on methods.
