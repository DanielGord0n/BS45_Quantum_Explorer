# n=44 search-narrowing research — the record program

**Date:** 2026-07-30 · **Trigger:** BS(42,41) solved (banked, verified); Daniel greenlit
out-of-box research for BS(45,44), the open record. **Measured baseline:** the n=41 recipe
costs ~700M tested/class; ×2-3/rung ⇒ n=44 ≈ 5-20B/class × 12 admissible classes ≈
50-200B candidates ≈ 1,000-3,000 CPU node-days raw. The solver exists; the record needs
multipliers. Ranked below by (expected gain × grounding) / cost, each with its kill test.

## Today's triage data (2026-07-30, local, this session)

- **Admissible frontier enumerated: 12 classes** (a,b odd; c,d even; norm 178):
  (1,7,8,8) (1,13,2,2) (3,3,4,12) (3,5,0,12) (3,13,0,0) (5,5,8,8) (5,7,2,10)
  (5,9,6,6) (5,11,4,4) (7,7,4,8) (7,11,2,2) (9,9,0,4).
- **ALL 12 STREAM** under the full 2.11b+2.12 m6 stack (50 cands each, PROF_ORDER=0
  locally): no enumeration-bound classes at n=44. Slow streams (deprioritize): (9,9,0,4)
  242s, (3,5,0,12) 113s; the other ten reach 50 candidates in 19-28s. Cell counts 484k-1.04M.

## Lever 1 — Đoković–Kotsireas COMPRESSION as a new filter axis (novel here; likely
what WZ did not use — their paper slices only mod 2/3/6)

Grounding: "Compression of Periodic Complementary Sequences and Applications"
(Đoković & Kotsireas, arXiv:1302.0571, DCC 2013): for a PERIODIC complementary collection
of length v=dm, the m-compressed sequences (term sums at stride d) are again complementary.

Application route for BS(45,44): NPAF_A+NPAF_B+NPAF_C+NPAF_D = 0 at every shift ⇒ pad C,D
with one zero to length 45; then PAF_padded(s) = Σ_i [NPAF_i(s) + NPAF_i(45−s)] = 0, i.e.
the padded quadruple is a periodic complementary collection of length 45 = 3²·5. Compress
by d=9 (length-5 images) and d=5 (length-9 images): the compressed quadruples MUST be
periodic-complementary with entries in tiny bounded ranges — an exhaustively enumerable
necessary condition. Because compression sums entries at fixed positions mod d, this is a
POSITION-STRUCTURE constraint — a genuinely different axis from WZ's residue-class counts,
and it can be pushed to the profile/cell level exactly like 2.11b was.

- Expected gain: unknown until measured — mechanism-level reason to hope for large
  (the mod-5 slice of length 45 is untouched by any current filter).
- Kill test (1-2 days, local): (1) soundness canary — verify the banked BS(42,41) and
  WZ's published 41/42/43 all pass the compression condition (pad 41→42? general n: pad
  C,D to n+1; n+1=42=2·3·7 at n=41 — compress by 7: also new!); (2) measure rejection
  rate of the compression filter on 10k streamed candidates at n=41 and n=44. Reject
  rate >50% ⇒ build it into the stream; <10% ⇒ kill.

**⚰️ MEASURED DEAD 2026-07-30 (same session, ~1h after writing this).** Math validated
(banked BS(42,41): padded quadruple periodic-complementary, all 6 compressions PASS;
filter machinery soundness-checked — accepts the real solution). Rejection measured on
real streamed candidates (existence-of-compressed-(A,B) check, sum constraints included):
**n=41 d=7: 0/1000 = 0.0% · n=44 d=5: 2/2000 = 0.1% · n=44 d=9: 12/2000 = 0.6%** — far
below the pre-registered 10% kill line. Interpretation: survivors of 2.11a/2.11b/2.12
already satisfy the compression condition almost surely; the compressed-(A,B) space is
floppy enough to certify nearly any C,D target. Do NOT rebuild at the profile level.
Completion-level variant (compressed-PAF targets pruning the pair-DFS, AB_PROF-style)
remains possible but is DEPRIORITIZED below SAT+CAS and GPU — existence rarely failing
means its capacity cuts are likely thin. Instrument kept: `WZ_FH_DUMP` (default-off,
n=19 bit-identical verified) + `compression_filter.py` (session scratchpad).

## Lever 2 — SAT+CAS (MathCheck-style): a second, independent engine

Grounding: Bright–Kotsireas–Ganesh, "Applying computer algebra systems with SAT solvers
to the Williamson conjecture" (JSC; uwaterloo.ca/mathcheck) — SAT+CAS enumerated ALL
Williamson matrices of even order ≤ 70, a record for a sibling complementary-sequence
class. Kotsireas CO-LEADS MathCheck — this dovetails with the collaboration email.

Route: encode NPAF(A,B,C,D)=0 as pseudo-Boolean constraints, inject our canon (A[0]=B[0]=1,
reversal-lex) as symmetry-breaking clauses and Thm 2.2/2.3 residue facts as CAS-derived
blocking clauses; run CaDiCaL/kissat with cube-and-conquer (cubes = our signature classes
/ profile cells — the infrastructure maps 1:1).

- Expected gain: unknown; different search dynamics entirely (CDCL learns structure DFS
  cannot). Records in the sibling class are proof of concept.
- Kill test (weekend, laptop/1 node): encode n=29 with a known solution — SAT should
  re-find it; then blind n=38 (we know solutions exist, our probe found them in ~min-hours)
  — if SAT is within 10× of the probe's wall on n=38, scale it; if 100× slower, kill.

## Lever 3 — GPU completer (the priced ×100-1000 throughput lever)

Already the pre-registered escalation. Measure-first spike: port fh_complete_ab's
pair-DFS for ONE cell to CUDA (Fir/Rorqual have GPU partitions), two numbers:
candidates-completed/s/GPU vs the measured ~9/s/core CPU. ≥×300/GPU ⇒ n=44's 50-200B
becomes weeks on a handful of GPUs ⇒ BUILD. <×30 ⇒ the record waits on levers 1/2/4.

## Lever 4 — class triage theory (search WHERE solutions can live)

WZ proved NS(44)=∅ and NNS(44)=∅; their obstruction techniques (and the norm-form
literature) have never been applied per-signature-class to BS(45,44). Program: for each
of the 12 classes, hunt small-modulus obstructions (mod 8/16 quadratic-form conditions,
Legendre-pair-style constraints at 45's divisors) that either kill the class outright or
rank it. Every killed class = 5-20B candidates never spent. Also: flat-score prior now
has 4 deep-n data points (ours 124; WZ 140/142/134 — all ≤150) ⇒ n=44 windows sized to
score ≤~160 first.

## Lever 5 — symmetry-restricted lanes (cheap lottery tickets, run alongside)

Đoković-school trick: restrict to structured subspaces (skew A / symmetric B /
near-palindromic C,D). Cuts each side's space quadratically at the risk that no n=44
solution is that symmetric. Cost: a stream-mode flag; run as a MINORITY lane (1 job/class)
so it never displaces the main search. n≤37 banked solutions can be audited TODAY for
partial symmetry (free prior evidence).

## What this is NOT

No composition/construction routes (measured-dead 2026-07-17: Yang/Turyn shapes are
wrong; 44 is non-Golay); no exhaustive completeness; no promise of the record — honest
frame: WZ stopped at 43 with a national HPC center; our edge must come from levers they
did not use (1, 2) multiplied by throughput (3) and aim (4).

## Sources

- Đoković & Kotsireas, Compression of Periodic Complementary Sequences and Applications —
  https://arxiv.org/abs/1302.0571 · https://link.springer.com/article/10.1007/s10623-013-9862-z
- Bright, Kotsireas, Ganesh — Williamson via SAT+CAS: https://cs.uwaterloo.ca/~cbright/reports/jsc-willsat.pdf
  · MathCheck: https://uwaterloo.ca/mathcheck/publications
- New results on periodic Golay pairs (compression in current use): https://arxiv.org/html/2408.15611
