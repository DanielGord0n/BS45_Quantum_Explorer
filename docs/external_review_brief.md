# BS(45,44) search — external review brief (2026-09-22)

Self-contained context for an outside reviewer (mathematician / algorithms / HPC) asked to
find anything we are missing. Everything below is measured unless marked otherwise.
Public repo: https://github.com/DanielGord0n/BS45_Quantum_Explorer (main). Key files:
`src/solver/wz_match.cpp` (the solver, ~2,300 lines), `docs/n44_search_narrowing_research.md`
(the lever ledger with numbers), `docs/lever19_sweep_plan.md` (current program),
`docs/paper_methods_record.md` (methods + provenance), `HANDOFF.md` (daily log, newest
first), `results/champions/*.txt` (banked solutions), `tools/verify_npaf.py`.

## 1. The problem

Base sequences BS(n+1,n): four ±1 sequences A,B (length n+1) and C,D (length n) whose
non-periodic autocorrelations sum to zero at every nonzero shift:
N_A(s)+N_B(s)+N_C(s)+N_D(s)=0 for s=1..n. The base sequence conjecture (existence for all
n) was verified for n<=40 (Djokovic); Wang & Zhu (arXiv:2506.20296, v3 Feb 2026)
constructed n=41,42,43 and state n>43 is open. They also proved the two classical
shortcut families are empty at 44 (no normal sequences NS(44), no near-normal NNS(44)),
so BS(45,44) must come from the general search or new construction theory.

Signature (a,b,c,d)=(sum A, sum B, sum C, sum D) satisfies a^2+b^2+c^2+d^2=4n+2; at n=44
that is 178, with a,b odd and c,d even, giving 12 admissible classes up to sign/swap:
(1,7,8,8) (1,13,2,2) (3,3,4,12) (3,5,0,12) (3,13,0,0) (5,5,8,8) (5,7,2,10) (5,9,6,6)
(5,11,4,4) (7,7,4,8) (7,11,2,2) (9,9,0,4). Each is searched separately.

## 2. What we have found (all independently verified, all new solutions)

| n | class | found | how |
|---|---|---|---|
| 41 | (0,2,9,9) | 2026-07-29 | flat-ordered lane, window skip-8, candidate rank 1429 in its window; 212,872 completion nodes |
| 42 | (7,11,0,0) | 2026-08-03 | reverse-ordered lanes; found by 3 lanes within hours; 86,976 nodes |
| 43 | (8,-2,5,9) | 2026-08-30 | window-band sweep aimed by a "window map" (see 6); window 327, first rep, 5.8M tested, 88,616 nodes |

None equals the Wang-Zhu solution of its class under the full 1,024-element equivalence
group (independent negations/reversals of the four sequences, A-B swap, C-D swap). The
professor (I. Kotsireas) has verified them and is writing the paper; the n=44 record is
the open target.

## 3. The solver (C++17, single program `wz_match.cpp`, mode WZ_FIRSTHIT)

Per signature class:
1. **C,D stream.** Wang-Zhu Theorem 2.2 "mirror-pair" encoding enumerates (C,D) pairs
   from mod-6 residue-class-sum profile cells (a "cell" = one vector of class sums for C
   and for D). Theorem 2.3 conditions (2.11a norm identity, 2.11b residue autocorrelation,
   2.12) are applied as stream filters (8.7x cell reduction; without them a single mod-3
   profile exceeds 12 h). The Hall-polynomial / PSD bound (Theorem 2.4) is applied per
   sequence and per pair at 200 angles theta=j*pi/100.
2. **Orbit canonicalization.** The cell list carries each (C,D) equivalence orbit
   (negC, negD, revC, revD, swap) redundantly; we keep one representative per orbit
   (measured 3.8x at n=43, 4-29x per class at n=44; 28.9x on (3,13,0,0)). Verified sound:
   every known solution retains a searchable representative.
3. **Ordering.** Cells sorted by ascending sum of |class sums| ("flattest first");
   candidates within a cell buffered (500k) and completed in ascending flatness score
   sum_s |N_C(s)+N_D(s)|. Alternatives measured (L2, PSD peak, max shift) were worse on
   the three known hits (flat-L1 puts them at in-window percentiles 0.9%, 31.9%, 2.7%).
4. **A,B completion.** Backtracking over mirror-pair placements with incremental
   NPAF-difference bounds, sum bounds, per-cell profile-constrained pruning (allowed
   (k,r) class-sum lists from 2.11a/b/2.12, capacity-pruned down the DFS: 5.2-6.6x fewer
   nodes), two canonicalizations (A[0]=B[0]=+1 root, reversal-lex: ~4x each), and a
   per-candidate node budget.
5. **Per-arm checkpointing** (cell, batch, in-batch position), exact deterministic resume
   across 12 h jobs. Signature-guarded (any change to stream order invalidates a lane).
6. **Deployment.** SLURM, 1 node = 178 single-core arms (arm i takes cells congruent to i
   mod 178), 12 h jobs, 4 Alliance Canada clusters (192-core EPYC 9654/9655 and Xeon 6
   nodes), ~2.1M core-hours through Aug 31, RAC priority since.

Measured completion costs: the three known solutions needed 87k-213k nodes each; the
average tested candidate costs ~450k nodes with a fat tail. Budget was 5e7 nodes; now
2e6 (10x the largest known hit): +24-30% cells covered per lane-day, at the price of
abandoning ~21-29% of candidates at the cap (those are the deep failures). A 5e6 middle
point gave only 80% of the cells of 2e6 with 5% aborts, so 2e6 stands.

## 4. Where hits live: the "window map" and front-only search

A lane at PROF_SKIP=k starts each arm k cells into its list ("window k"). Locating the
five known solutions (ours 41/42/43 + Wang-Zhu 41/42/43) in our ordering:
WZ-42 window 0; ours-41 window ~8; WZ-43 windows 255-571; WZ-41 windows 499-842;
ours-42 at the far reverse end (reverse window 4). So the cell-level flatness prior is
weak (tie blocks of 4k-61k cells), solutions sit anywhere in the ordering, and cells
never exhaust (a 12 h arm covers ~1-2 cells when uncapped). This produced the n=43 hit
within 24 h of aiming at its band after weeks of deep search in the wrong windows.

All three hits surfaced inside the FIRST or SECOND sorted 500k buffer of their cell
(stream index 500,000 / 1,000,000 / 500,000) at in-cell flatness percentiles 0.9% /
31.9% / 2.7%. Hence "front-only" mode: complete only the K flattest of each cell's
first buffer (K=50,000, 10%), abandon the rest, advance to the next cell: ~10x cells per
lane-day; control re-found our n=43 in 4.9 h (5e7) / 3.8 h (2e6). A second independent
front per cell comes from reversing the enumeration order inside the cell (validated:
same candidate set, different order).

## 5. The current program (Pass F / FR / F2)

Workhorse class (3,13,0,0): 1,038,952 cells, 35,925 orbits, 5,836 windows. Class
inventory (cells / windows / orbits / dedup): (5,9,6,6) 496k/2788/62k/8.0x; (5,7,2,10)
484k/2718/121k/4.0x; (1,7,8,8) 978k/5492/124k/7.9x; (3,3,4,12) 922k/5177/233k/3.9x;
(5,5,8,8) 972k/5463/123k/7.9x; (5,11,4,4) 1.01M/5664/128k/7.9x; (7,7,4,8)
987k/5547/250k/3.9x; (7,11,2,2) 502k/2820/63k/8.0x; (1,13,2,2) same; (9,9,0,4)
969k/5443/128k/7.6x; (3,5,0,12) 975k/5478/128k/7.6x.

- Pass F: front-only lanes every 8 windows (an arm advances ~8 cells per rep) — the
  workhorse forward tile is 80% assigned (Sept 21), hitless so far; ~600 cells/lane-rep,
  ~32M candidates tested per lane-rep, aborts 24-29%.
- Pass FR: same offsets with reversed in-cell order (second front): 60 lanes read,
  100 queued (Nibi).
- Pass F2 (next): first TWO buffers, top 35% each (catches an n=42-like case).
- Then the same sequence over the other 11 classes. Uncapped deep lanes (windows 0-8,
  300-430M candidates each) were retired.

Throughput limit today is scheduler admission (RAC fairshare depleted after ~250 whole-
node jobs/night; Nibi waits ~13 days per job); short/small jobs did not backfill (0
admissions), so lanes stay 12 h whole-node.

## 6. Levers priced (with kill rules) — do not re-propose without a new argument

Shipped: 2.11b+2.12 stream filters (8.7x); profile-constrained completion (5.2-6.6x);
flat-first ordering (7-10x throughput); dual canon (root + reversal, ~4x each); orbit
canonicalization (3.8-29x); checkpointed lanes; window concentration; stratified window
sweep (found n=43); front-only top-K (~10x cells/day); budget 2e6 (+24-30%); reversed
in-cell order (second front). Measured dead: Djokovic-Kotsireas compression filter
(0.0-0.6% extra rejection after 2.11/2.12); SAT/CaDiCaL encoding (>=3,000x slower at
n=19); GPU completer (thread-per-candidate 5.9x, warp-cooperative 24x vs one core at
production budget, under the 60x break-even vs 192-core nodes); PSD bias above n=30;
hash-join above n~29; in-cell stabilizer dedup (<=1.1x); symmetry-restricted lanes (0 of
31 banked solutions symmetric); branchless placement (-4% on x86); L2/PSD/max-shift
orderings; stream-wall timeout (+0.1%); 3 h backfill and split-lane small jobs (0
admissions on saturated clusters); exhaustive enumeration at n>=36 (walltime).

## 7. What we want from you

1. **Mathematics we are missing.** Any necessary condition on (C,D), on cells, or on
   whole signature classes at n=44 beyond 2.11a/2.11b/2.12, the 200-angle Hall/PSD bound
   and compression — especially anything that could prove a class empty cheaply or
   cut the C,D stream by a large factor. Any construction/composition route to BS(45,44)
   given NS(44)=NNS(44)=empty.
2. **The completer.** The A,B backtracking is the cost (~450k nodes/candidate, fat tail).
   Better bounds, better variable/value ordering, meet-in-the-middle for A,B given the
   (C,D) target, exploiting more symmetry inside the completion, or an argument that a
   much smaller node budget is safe.
3. **Search policy.** Given hit depths (first/second buffer, in-cell percentiles 0.9-32%),
   window positions (0 to 842 and the far reverse end) and per-lane throughput, what
   allocation of lane-days across F/FR/F2, K, buffer size and the 12 classes maximizes
   the probability of a hit per node-day? An expected-time model would help.
4. **Code.** Constant-factor wins in `wz_match.cpp` hot loops (NPAF-difference updates,
   candidate scoring, sorting), correctness risks, anything unsound in orbit
   canonicalization, checkpointing or the front-only cap.
5. **Flaws.** Anything above that is wrong, unjustified, or an artifact of how we measure.

Constraints: CPU clusters only (GPU measured dead), 12 h jobs, 178 arms per node,
deterministic resume must survive; changes to stream order invalidate checkpoints, so
propose them as new lane types. We validate every idea against re-finding the three
known solutions before fleet use, with a pre-registered pass/fail rule.
