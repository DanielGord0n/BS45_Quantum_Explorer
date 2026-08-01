# A new BS(42,41), a measured search frontier at n=42–44, and three methods questions

*Daniel Gordon (dangord, Alliance clusters; supervisor account def-ikotsire).
2026-08-01. One page. This is a methods conversation, not a compute request.*

## The headline result

On 2026-07-30 our solver found a **new solution for BS(42,41)** — the Wang–Zhu signature
class (0,2,9,9), but **not their published sequences**: the C,D flatness score
Σ|NPAF_C+NPAF_D| is 124 vs their 140, and the score is invariant under swap, negation,
and reversal, so the solutions are inequivalent. Independently verified (NPAF[s]=0 for
all s=1..42, norm 166, WZ pair encoding). To our knowledge this is the first independent
solution at any of Wang–Zhu's rungs since their paper (arXiv:2506.20296). Full sequences
on request.

It came from a direct-search architecture we can fully describe (and would gladly):
Thm-2.2-encoded C,D pair streaming from mod-6 residue cells with eq. 2.11a/2.11b/2.12
as stream filters (2.11b turned out to be the stream *enabler* at n≥36 — the answer to
the question an earlier draft of this brief was going to ask), flattest-cells-first
ordering with in-cell flat-first completion (motivated by measuring that all known deep
solutions are flat: ours 124, WZ's 140/142/134), profile-constrained A,B backtracking,
and per-arm candidate-level checkpointing so successive node-days accumulate exactly.
The same engine cleared n=32–37 blind in one week (every admissible class at n=34–37
attempted; 26 solutions banked, all independently verified).

## The measured frontier (numbers, not impressions)

- **n=41 (0,2,9,9): solved** at ~700M tested candidates cumulative (both stream ends).
- **n=42 (7,11,0,0):** ~1.5B+ tested and counting, hitless — consistent with ×2–3/rung
  density thinning (expected first hit 1.4–2B).
- **n=43 (8,−2,5,9):** opened; tests ~3.5× slower per node-day than n=42, with rising
  budget-abort fractions.
- **n=44 — the open problem:** we enumerated the admissible frontier (12 signature
  classes at norm 178) and stream-validated all 12 (none enumeration-bound); per-class
  completion cost is measured at 1.6–43M candidates per 12h node across the 10 fastest
  classes. Extrapolated need: ~5–20B candidates per class, with no knowledge of which
  classes bear solutions — the reason n=44 needs more than patience.

## Three questions (each touches your own programs)

1. **Class triage.** Your NS(44)/NNS(44) non-existence results close those objects; is
   any of that obstruction machinery (or norm-form arguments) sharp enough to kill or
   rank *signature classes* of the general BS(45,44) problem? Every killed class saves
   ~10¹⁰ tested candidates.
2. **Compression.** We validated the Đoković–Kotsireas compression conditions
   (arXiv:1302.0571) on our BS(42,41) by zero-padding C,D to length 42 (the padded
   quadruple is periodic-complementary; all six compressions pass exactly) — but
   measured ~0% rejection power on candidates that already survive 2.11a/b + 2.12. Is
   there a stronger way to deploy compression against *aperiodic* base sequences at
   length 45 = 9·5 that we are missing?
3. **SAT+CAS.** A direct CNF encoding (cardinality over XNOR product variables,
   CaDiCaL) is ~3,000× slower than our streaming search already at n=19. Would a
   MathCheck-grade formulation (native pseudo-Boolean, CAS-side filtering,
   cube-and-conquer over our residue cells) plausibly change that picture for BS-type
   NPAF systems, or is this family a poor fit in your experience?

We can share code, the candidate-cost tables, and all verified solutions. If any of this
is close enough to your interests for a 30-minute conversation, we would value it
greatly.
