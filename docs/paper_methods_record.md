# Paper input: complete methods and provenance record

**Purpose:** self-contained input for drafting the paper on the BS(42,41) and BS(43,42)
solutions (and, if it lands in time, BS(44,43)). Compiled 2026-08-06 from the project's
audit trail (HANDOFF.md + archive + per-wave telemetry). Prof. Kotsireas has verified
both solutions independently and is contributing the derived-combinatorial-objects
section. Everything below is measured, with job-level provenance.

## 1. Results

**BS(42,41)** — found 2026-07-29, banked and independently verified 2026-07-30.
Signature (a,b,c,d) = (0, 2, 9, 9), norm 166 = 4n+2. NPAF[s] = 0 for all s = 1..42
(verified by an independent Python checker, separate code path from the solver).
NOT Wang-Zhu's published solution: exhaustive comparison against the full 1,024-variant
equivalence orbit of their Table 1 sequences (independent negation/reversal of all four
sequences + A-B swap + C-D swap) shows no match; the C,D flatness score
sum_s |N_C(s)+N_D(s)| — invariant under the entire orbit — is 124 vs their 140.

```
A: 1 1 1 1 1 1 -1 -1 1 -1 -1 -1 -1 1 -1 -1 1 -1 1 -1 -1 1 1 -1 1 -1 -1 1 1 1 -1 -1 -1 1 1 -1 1 -1 -1 1 1 -1
B: 1 -1 -1 1 -1 -1 -1 -1 1 1 -1 1 1 1 -1 1 -1 -1 1 1 1 -1 -1 -1 1 1 1 1 1 -1 1 -1 1 1 1 -1 -1 1 -1 -1 -1 1
C: -1 -1 -1 -1 1 1 1 1 1 1 1 -1 1 -1 1 1 -1 1 1 1 1 1 -1 1 -1 1 -1 1 1 1 -1 -1 1 -1 1 1 -1 1 -1 1 -1
D: -1 -1 1 1 1 1 1 1 1 -1 -1 1 1 1 1 1 1 -1 1 -1 1 -1 -1 -1 1 1 -1 -1 1 -1 1 1 1 -1 1 1 -1 -1 1 1 -1
```

**BS(43,42)** — found 2026-08-03, banked and independently verified 2026-08-04.
Signature (-7, 11, 0, 0), norm 170 = 4n+2. NPAF[s] = 0 for all s = 1..43. NOT
Wang-Zhu's published solution (same 1,024-variant orbit check; flatness 150 vs their
142). Found independently by THREE separate search lanes within hours (Trillium jobs
2007533 then 2007532, and Rorqual 18266737), all converging on the same quadruple up
to C-D swap — an internal replication.

```
A: 1 -1 1 1 1 -1 1 -1 1 -1 -1 1 1 -1 1 1 1 -1 -1 1 -1 1 -1 -1 1 1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 1 1 1 -1
B: 1 1 1 -1 1 1 -1 1 1 1 1 -1 -1 -1 -1 1 -1 1 -1 -1 1 1 1 1 1 -1 1 -1 1 -1 1 1 1 1 -1 1 1 1 -1 -1 1 -1 1
C: 1 -1 1 -1 1 1 1 -1 -1 -1 -1 -1 1 -1 1 1 -1 -1 1 -1 1 1 -1 1 1 1 -1 -1 1 1 -1 -1 1 -1 -1 1 1 -1 1 1 -1 -1
D: 1 -1 1 -1 1 1 -1 1 1 1 -1 -1 -1 -1 -1 -1 1 1 -1 1 1 1 1 -1 -1 -1 1 1 1 -1 -1 -1 -1 1 1 -1 1 -1 1 1 -1 -1
```

Context: the base sequence conjecture was verified for n <= 40 in prior literature
(Djokovic classification program); Wang & Zhu (arXiv:2506.20296) first constructed
n = 41, 42, 43. These are, to our knowledge, the first independent solutions at any
of those rungs, and are new (inequivalent) solutions in both cases.

## 2. Search method (the solver as it found both solutions)

Single C++ program (wz_match.cpp, WZ_FIRSTHIT mode), first-hit search: stop at the
first NPAF-verified completion. Pipeline per signature class:

1. **C,D candidate stream**: Wang-Zhu Theorem 2.2 mirror-pair encoding streams C,D
   pairs from mod-6 residue class-sum profile cells; Theorem 2.3 conditions (2.11a
   norm identity, 2.11b residue autocorrelation, 2.12) applied as stream filters.
   Measured: 2.11b+2.12 are stream ENABLERS at n >= 36 (8.7x cell reduction; without
   them one mod-3 profile's DFS exceeds 12 h walltime).
2. **Flat-first ordering** at two levels: profile cells sorted by ascending
   sum |class sums| ("flattest cells first"), and candidates within each cell
   completed in ascending flatness score sum_s |N_C(s)+N_D(s)| (500k-candidate sort
   buffer). Motivation (measured): solutions concentrate at flat scores — the known
   deep solutions score 124-150 vs typical candidate median ~160.
3. **A,B completion**: backtracking over mirror-pair placements (Thm 2.2 encoding)
   with incremental NPAF-difference bounds, sum bounds, per-cell profile-constrained
   pruning (allowed (k,r) class-sum lists from 2.11a/2.11b/2.12, capacity-pruned down
   the DFS — measured 5.2-6.6x node reduction at n=41/42), two canonicalizations
   (A[0]=B[0]=+1 root canon, 4x; reversal-lex canon, ~4x), and a per-candidate node
   budget of 5e7 (abort counted, not silently dropped).
4. **Orbit canonicalization** (added after both hits; relevant to n=43+ searches and
   the paper's scaling analysis): the cell list carries each C,D equivalence orbit
   (negC/negD/revC/revD/swap) redundantly — measured 3.8-29.2x per class; the solver
   now streams one representative per orbit.
5. **Checkpointed lanes**: every arm persists (cell, batch, in-batch position)
   atomically; successive 12 h jobs resume exactly. Deterministic order makes resume
   exact (validated: interrupt/resume ledgers match one-shot runs candidate-for-
   candidate).
6. **Verification discipline**: a solution is claimed only after the solver banner,
   an independent NPAF checker pass (separate implementation), and banking with full
   provenance.

Deployment: SLURM, one 192-core node per job, FH_NARMS=178 single-core arms per node,
each arm streaming an interleaved shard (profiles congruent to arm index mod 178),
12 h walltime, --requeue.

## 3. Provenance of the two hits

**BS(42,41):** Fir job 51517707 (wave 6, the first checkpointed-lane wave), n=41 class
(0,2,9,9), flat profile order, window selector skip=8 (each arm's 9th-flattest cell),
arm shard 5/178. Hit at stream index 500,000 of that arm, profile rank 1429, 212,872
completion nodes for the hit candidate, elapsed 17,708.9 s (~4.9 h) into the lane.
That job alone tested 12.9M candidates (178 arms); cumulative campaign effort on the
class to that point ~0.7B raw candidates across 6 waves (both stream ends).

**BS(43,42):** reverse-order lanes, window skip=4-region; first banner Trillium job
2007533 at ~38 minutes into its lane (2026-08-03 ~20:24 EDT), independently also
Trillium 2007532 and Rorqual 18266737 (same quadruple up to C-D swap). Cumulative
campaign effort on class (7,11,0,0) ~3.3B raw candidates.

**Ladder validation** (same engine, one week, 2026-07-17 to 07-21): cleared n=32
through n=37 blind — every admissible signature class attempted at n=34-37; 29
champion solutions banked at n=32-37, all independently NPAF-verified. (n <= 40 known
to the literature; these served as engine validation, not novelty claims.)

## 4. Cost and scaling (measured, with the redundancy correction)

Raw tested-candidate counts are inflated by class-dependent orbit redundancy; the
paper should report both:

| Rung | Class | Raw candidates to first hit | Orbit redundancy | Distinct orbits (effective) |
|---|---|---|---|---|
| n=41 | (0,2,9,9) | ~0.7B | 7.53x | ~93M |
| n=42 | (7,11,0,0) | ~3.3B | 29.18x | ~113M |
| n=43 | (8,-2,5,9) | ~2B and searching | 3.81x | ~525M so far |

Key methods finding: the apparent 4.7x cost step from n=41 to n=42 is almost entirely
a redundancy artifact — in distinct orbits the two rungs cost within 1.2x of each
other. n=44 planning numbers: 12 admissible signature classes at norm 178 (a,b odd,
c,d even), all confirmed non-empty streams; per-class orbit redundancies 3.9-28.9x;
distinct-space sizes range 35,925 orbits ((3,13,0,0)) to 250,079 ((7,7,4,8)).

Throughput (production, per 192-core node-day): ~20-100M candidates tested per class
depending on class; completion rate ~3-9 candidates/s/core at budget 5e7.

## 5. Hardware and compute

Four Digital Research Alliance of Canada clusters. Login-node measurements
(2026-08-06) and compute-node status:
- Rorqual: dual AMD EPYC 9654 (96-core), 192 cores/node, ~755 GB (login matches
  compute-class spec).
- Trillium: dual AMD EPYC 9655 (96-core), 192 cores/node, ~755 GB (login matches
  compute-class spec).
- Fir: compute nodes are 192-core (job telemetry); login node reads dual EPYC 9135 —
  compute-node model to be confirmed (driver now logs lscpu per job; next wave's
  outputs will carry the authoritative model).
- Nibi: login node reads Intel Xeon Platinum 8480+; compute-node core count/model to
  be confirmed the same way.
GPU experiments: NVIDIA H100 80GB HBM3 (Fir). Jobs compiled on-node with gcc 12.3,
-O3 -march=native, StdEnv/2023.
Search jobs: 1 node x 12 h x 178 single-core arms. GPU experiments used one NVIDIA
H100 80GB (Fir). Approximate total campaign compute through 2026-08-06: order of
10-15 core-years across the four clusters (job-level records available for an exact
figure).

## 6. Negative results worth reporting (all measured, pre-registered thresholds)

- **Compression filter** (Djokovic-Kotsireas compression, applied to the zero-padded
  quadruple at length n+1): mathematically valid on all our solutions (all divisor
  compressions pass exactly), but rejection power on candidates already filtered by
  2.11a/b+2.12 measured at 0.0-0.6% — no additional pruning at the profile level.
  Class-level existence variant: all 12 n=44 classes feasible at d=3, d=5 — no class
  eliminations.
- **Direct SAT encoding** (cardinality-over-XNOR CNF, CaDiCaL): correct (re-finds
  known small-n solutions) but >=3,000x slower than the streaming search already at
  n=19.
- **GPU (thread-per-candidate CUDA port)**: exact verdict/node parity with CPU;
  69.3x vs one core at light budget but only 5.9x at the production budget (deep-tree
  warp divergence) — below cost-effectiveness vs 192-core CPU nodes.
- **GPU (warp-cooperative kernel, one candidate per warp)**: 32 lanes parallelize
  the O(L) place/prune inner loops (fused shift-partitioned updates + ballots);
  exact verdict/node parity with CPU. 24.0x vs one core at the production budget
  (host-side flatness sorting of the naive kernel: 2.8x) — under the pre-registered
  60x line, so one H100 stays below ~1/8 of a 192-core CPU node and the GPU route
  was closed permanently.
- **Branchless/vectorizable placement loop**: bit-identical, +6% on ARM but -4% on
  the production x86 target — dropped.
- **Symmetry-restricted search**: 0 of 31 banked solutions show palindromic or
  anti-palindromic structure (best single-sequence symmetry 0.77 vs random ~0.5-0.65)
  — restriction would exclude every solution ever found by this program.
- **In-cell stabilizer dedup**: average in-cell redundancy 1.015-1.11 — ceiling too
  low to justify candidate-level canonicalization.

## 7. Honest framing and open items

- n=41-43 are replications-with-new-solutions of rungs first constructed by Wang-Zhu;
  the open frontier is n=44 (BS(45,44)), for which the base sequence conjecture is
  unverified. NS(44) and NNS(44) are known empty (Wang-Zhu); the general BS(45,44)
  object is open — a solution would settle the next rung of the conjecture.
- n=43 search in progress (18-24 lanes, both stream ends, windows 0-11). n=44 search
  running in parallel (10 lanes, all fast classes, both ends).
- To insert when available: exact CPU models per cluster; exact core-year total from
  job accounting; n=43 solution and its provenance if found before submission.
