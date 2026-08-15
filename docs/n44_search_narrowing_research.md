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

**⚰️ KILLED AT CANARY STAGE 2026-07-31 (direct-encoding variant).** Built
`tools/sat_bs_encoder.py` (XNOR product vars + exact-cardinality NPAF constraints +
nonneg-sum signature WLOG; soundness gate: banked n=29 satisfies the encoding — PASS).
Results: blind n=11 SAT in 0.9s (real solution, NPAF re-check 0) — the encoding is
CORRECT. But blind n=19: seqcounter >120s, totalizer + A0 symmetry unit **>600s, killed**
— vs firsthit's **0.2s**: a ≥3,000× deficit at a rung ~10 orders of magnitude easier than
n=44, with cardinality-heavy CNF expected to scale WORSE with n, not better. Far beyond
the pre-registered 100× kill line. Do NOT rebuild the direct encoding. Honest caveat: a
real MathCheck-style system (native pseudo-Boolean solvers, CAS callbacks,
cube-and-conquer) is a different beast than naive CNF — but that is a multi-week build
against a 3.5-order-of-magnitude canary deficit, so the question "is SAT+CAS viable for
base sequences?" is DELEGATED TO THE KOTSIREAS EMAIL (he co-leads MathCheck; one
paragraph in the brief replaces weeks of speculative engineering). Encoder kept as a
tool; the GPU spike is now the sole live throughput lever.

## Lever 3 — GPU completer (the priced ×100-1000 throughput lever)

Already the pre-registered escalation. Measure-first spike: port fh_complete_ab's
pair-DFS for ONE cell to CUDA (Fir/Rorqual have GPU partitions), two numbers:
candidates-completed/s/GPU vs the measured ~9/s/core CPU. ≥×300/GPU ⇒ n=44's 50-200B
becomes weeks on a handful of GPUs ⇒ BUILD. <×30 ⇒ the record waits on levers 1/2/4.

**MEASURED 2026-08-01/02 — VERDICT: KILLED (naive thread-per-candidate port).** Spike
job `52348541` (Fir, H100 80GB, exact CPU/GPU verdict+node cross-check PASS both
budgets — the instrument is sound). Primary (budget 1e6, 20k real n=44 (1,7,8,8)
candidates): cpu 5.74 cand/s, gpu 398.28 cand/s, **speedup 69.3× vs 1 core** =
marginal band, 1 H100 ≈ 0.36 of a 192-core node. Secondary (production budget 5e7):
cpu 3.31, gpu 19.47, **speedup 5.9× vs 1 core** = KILL band; at the budget production
lanes actually run, 1 H100 ≈ 6 cores ≈ 0.03 CPU-node — divergence eats the machine
exactly as the deep-budget caveat predicted. Per the pre-registered rule (<30× ⇒
killed): **no production GPU build.** Divergence-tolerant restructuring
(warp-per-candidate / persistent threads) was the only conceivable path back.

**MEASURED 2026-08-07 — WARP-V2 ALSO KILLED, GPU CLOSED PERMANENTLY.** Spike v2 job
`53498573` (Fir H100, production budget 5e7, 6k real n=44 candidates, exact CPU
cross-check verdicts_nodes_match=YES): warp-cooperative kernel (one candidate/warp,
32 lanes parallelize the O(L) place/prune loops) **79.88 cand/s = 24.0× vs 1 core**;
host-side flatness-sorted naive variant 2.8×; plain naive rerun 2.5×. Pre-registered
rule (≥200× build / 60–200× marginal / <60× closed): **CLOSED.** Even with
intra-candidate parallelism the deep-budget divergence keeps 1 H100 ≈ ⅛ of a
192-core CPU node — not cost-effective on any Alliance allocation. All throughput
levers are now priced dead (compression ×2, SAT-direct, GPU naive + warp, place-V2,
stabilizer); the record program = CPU canonical lanes + lever-4 class triage + the
Kotsireas collaboration.

## Lever 4 — class triage theory (search WHERE solutions can live)

WZ proved NS(44)=∅ and NNS(44)=∅; their obstruction techniques (and the norm-form
literature) have never been applied per-signature-class to BS(45,44). Program: for each
of the 12 classes, hunt small-modulus obstructions (mod 8/16 quadratic-form conditions,
Legendre-pair-style constraints at 45's divisors) that either kill the class outright or
rank it. Every killed class = 5-20B candidates never spent. Also: flat-score prior now
has 4 deep-n data points (ours 124; WZ 140/142/134 — all ≤150) ⇒ n=44 windows sized to
score ≤~160 first.

**Class-killer variant MEASURED 2026-08-01 (negative, recorded).** Turned compression
around as a class-level EXISTENCE test: a signature class can bear a solution only if a
periodic-complementary compressed quadruple exists (right entry ranges/parities/sums,
zero-pad WLOG by rotation invariance). Positive control: the solved n=41 class (0,2,9,9)
feasible at d=2/3/6/7 ✓. Result: **ALL 12 n=44 classes FEASIBLE at d=3 and d=5 — no
class killed.** The compressed spaces are too floppy at these divisors to obstruct.
Deeper divisors (d=9/15) are enumeration-heavy and, given the d=5 floppiness, unlikely
to differ — not pursued. Class triage now rests on real obstruction theory (WZ's
NS/NNS(44) machinery) = question 1 of the Kotsireas brief. Script:
`class_triage_compression.py` (session scratchpad).

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

**⚰️ Lever 5 DEAD BY PRIOR 2026-08-04.** Symmetry audit of all 31 banked firsthit
solutions (n=32..37, 41, 42): best single-sequence palindromic/anti-palindromic match =
0.77; typical 0.5-0.65 = random baseline; ZERO solutions show symmetry structure.
Symmetry-restricted lanes would exclude everything we have ever found. Do not build.
**PROGRAM STATUS: every lever in this document is now measured** — compression (filter +
class-killer) dead, SAT-direct dead, GPU dead at production budget, symmetry dead by
prior. n=44 rides on: the lanes (10 live), obstruction-theory triage (Kotsireas Q1),
and the collaboration itself. The measured ladder (n41=0.7B, n42=3.3B, x4.7/rung) prices
n=44 at 50-70B+ — the record is a methods-or-mathematics problem now, by measurement.

## Lever 6 — C,D ORBIT CANONICALIZATION (discovered 2026-08-04): ✅ BUILT + VALIDATED,
the largest lever since flat ordering

DISCOVERY: the cell stream never canonicalized the C,D side — negC/negD/revC/revD/swap
all preserve the completion problem (targets + |sums| invariant), yet the enumeration
carries every orbit 3.8-29x redundantly (measured: n=29 3.74x, n=42 (7,11,0,0) 29.2x,
n=43 (8,-2,5,9) 3.8x, n=44 (3,13,0,0) 28.9x, (1,7,8,8) 7.9x; remaining n=44 classes to
be tabulated — batch audit run hit an unexplained empty-output issue, re-run pending).
Direct evidence it was costing node-days: the n=42 bank came from THREE lanes converging
on the same quad "up to C<->D swap" = lanes burning days on equivalent copies. Because
the flat score is invariant under the orbit group, duplicates sit ADJACENT in flat order
— dedup advances through distinct territory at the full redundancy factor.

BUILD: WZ_FH_ORBIT_CANON=1 (default OFF): keep-set = lex-min real cell per orbit
(deterministic, order-independent, always a real enumerated cell => every solution's
orbit retains a searchable representative — retention is structural). CFGSIG carries
oc flag (canon lanes = new lanes); driver lane dirs suffixed _oc1; telemetry
cells_orbit_dup. VALIDATION: (a) canon-off BIT-IDENTICAL to pre-canon binary at n=19;
(b) n=19 canon-on finds the hit (dedup 2.88x); (c) n=29 canary canon-on re-finds THE
SAME solution (rank 588, nodes 81320) at idx 15,850 vs 26,694, wall 94s vs 149s = the
predicted stream compression, live; (e) n=44 workhorse dedup 28.92x, stream flows.

IMPACT (measured factors): n=43's 18-lane ~month => ~8-10 days effective; n=44
per-class effective cost drops 8-29x => the 50-70B raw extrapolation deflates toward
single-digit billions effective — the record moves from "methods-or-mathematics only"
to "hard but in range of the fleet + patience". Cutover = wave 13 (tar-pipe + fresh
_oc1 lanes, same window structure).

**n=44 ORBIT REDUNDANCY TABLE — COMPLETE (2026-08-05, all 12 classes):**
```
n=44 (1 7 8 8): cells=977601 orbits=123994 redundancy=7.88426x
n=44 (1 13 2 2): cells=502076 orbits=62919 redundancy=7.97972x
n=44 (3 3 4 12): cells=921646 orbits=233433 redundancy=3.94822x
n=44 (3 5 0 12): cells=975172 orbits=128494 redundancy=7.58924x
n=44 (5 5 8 8): cells=972489 orbits=123337 redundancy=7.88481x
n=44 (5 7 2 10): cells=483840 orbits=120960 redundancy=4x
n=44 (5 9 6 6): cells=496428 orbits=62219 redundancy=7.97872x
n=44 (5 11 4 4): cells=1008233 orbits=127876 redundancy=7.88446x
n=44 (7 7 4 8): cells=987487 orbits=250079 redundancy=3.9487x
n=44 (7 11 2 2): cells=502076 orbits=62919 redundancy=7.97972x
n=44 (9 9 0 4): cells=968858 orbits=127943 redundancy=7.57258x
n=44 (3 13 0 0): redundancy=28.92x (measured 08-04)
```
Every class 3.9-29x redundant — the canonical fleet's gain is universal, not class luck.
V2-canary note: n=29 with WZ_FH_PLACE_V2=1 (orbit canon off) reproduces the EXACT
historical fingerprint (idx=26694, rank 588, nodes 81320, NPAF==0) — place-V2 is
search-invariant, confirmed end-to-end.

**Lever 7 — in-cell stabilizer redundancy: PRICED DEAD 2026-08-06.** Cells fixed by
nontrivial orbit-group elements could carry candidate-level duplicates the cell dedup
cannot see. Measured (STAB_AUDIT instrument): avg stabilizer 1.015-1.11, nontrivial
cells 1.4-9.3% ((8,-2,5,9) 4.5%/1.049 · (3,13,0,0) 9.3%/1.107 · (1,7,8,8) 1.4%/1.015).
Ceiling ~5-10% gain for a candidate-level canon build inside symmetric cells — not worth
the complexity. Dead by measurement, five minutes' cost. Remaining unpriced: GPU
warp-cooperative spike v2 only. No-code tweak available: orbit-count-weighted n=44 lane
allocation (smallest distinct spaces first: (3,13,0,0) 35,925 orbits · (5,9,6,6) 62,219
· (1,13,2,2)/(7,11,2,2) 62,919 vs largest 250,079).

**Lever 8 — queue-time / job-granularity (the META-Farm question): PRICED DEAD
2026-08-07, zero cluster cost.** Hypothesis: whole-node (192-core) requests wait longer
than small jobs that backfill idle cores, so splitting lanes would raise throughput.
Test: `sbatch --test-only` start estimates on Fir for the same lane at three
granularities. Result: **192-core starts 11:14:50 · 32-core 11:50:25 (35 min LATER) ·
16-core 11:15:50 (1 min later)** — whole-node is the FASTEST-scheduling option; the
by-core partition gives no advantage. Split-lane driver support (FH_SHARD_LO/HI) is
built, validated and dormant (defaults byte-identical) in case a future cluster differs.
COROLLARY (inferred, not directly confirmed): Fir would start a NEW whole-node job
within the hour while 12 existing lanes sit PD, so those lanes are limited by
fair-share PRIORITY, not node availability. Implication for planning: the fleet is at
its allocation-throughput ceiling — queueing more jobs cannot buy more compute, and
the only ways to raise output are (a) make each core-hour count more (what orbit canon
did) or (b) raise the allocation itself, which is a PI-level conversation. Confirm the
PD reason string (`squeue -o "%r"`) on a future check before acting on (b).

**Lever 9 — ordering-prior tune-up: FLAT-L1 CONFIRMED, no change (2026-08-09).**
Compared the production ordering (flat L1 = sum|NPAF_C+NPAF_D|) against L2, PSD-peak,
and max-shift by measuring what percentile each known deep solution occupies vs a
1,500-candidate background of its own class. Result: flat-L1 = 0.9% / 31.9% / 2.7%
(n=41 hit / n=42 hit / WZ-43) — the only consistently strong score; L2 worse
everywhere; max-shift never better than noise; PSD-peak catastrophic at n=41 (65%)
despite a striking single-case win at n=43 (WZ's solution is PSD-rank #1 of 1,500).
The PSD anomaly is recorded but not actionable: one case, and the locator already
placed that solution's CELL at window ~255+ where in-cell ordering is moot. Production
ordering keeps its job with its first formal confirmation. Research ledger: 16 ideas
priced — 6 shipped, 10 dead/confirmed-baseline, 0 open.


## Lever 17 (2026-08-15): n=44 WINDOW-FRONT SWEEP — deployment fix, evidence-based

Not a solver change; a board-staleness correction. Evidence: BOTH banked hits came
from mid-band windows, not the flat-front windows — n=41 was candidate RANK 1429 of
window skip-8 (the very front of a mid window); n=42 came from reverse window 4. The
n=44 board was shaped on 07-31 (before either lesson) and never revisited: workhorse
(3,13,0,0) covered only flat w0-3 (264-277M deep) + rev skip-0 (242M); (5,9,6,6) and
(5,7,2,10) only skip-0; rev skips 1+ never opened. Under flat-first ordering a
lane-day deep in w0-3 tests ranks ~270M of that window while a lane-day on a fresh
window tests ranks 0-30M — and rank ~1.4k is where n=41 lived. DEPLOYED 2026-08-15:
Fir flat (3,13,0,0) skips 4-8 + (5,9,6,6)/(5,7,2,10) skips 1-3 (11 lanes); Rorqual
rev (3,13,0,0) skips 1-5 + (1,7,8,8)/(5,5,8,8) skips 1-2 (9 lanes). All fresh CKDIRs
(no collision with running w0-3/skip-0 lanes), WZ_FH_ORBIT_CANON=1, standard budget.
Additive to the n=43 program (no lanes cannibalized); partially pre-empts the 450M
tilt criterion by growing the n=44 share +20 lanes. Measurement: first rep per
window covers its front (~15-40M); read like any wave.
