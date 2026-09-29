# Pass H plan (v4, 2026-09-28: BUILT and gated; launch waits for Daniel's go)

Pass H replaces Pass G on all 12 n=44 classes. Build only after the Q canary (v2: Fir 61516887)
PASSES; deploy only on Daniel's go. Reviews: `docs/reviews/2026-09-25-astra-passh.md`,
`docs/reviews/2026-09-26-astra-redteam.md`.

## What changes versus Pass G

| Component | Pass G (running) | Pass H |
|---|---|---|
| Cell group | 32 (neg/rev each, swap) | 64 (+ quad switch Q), `WZ_FH_ORBIT_Q=1` |
| Orbit elimination | none | Q-closure prune, `WZ_FH_ORBIT_QPRUNE=1`; every removed orbit carries a written certificate |
| Cell order | flat: kept representative's profile score, ties by `std::sort` (toolchain-dependent) | (orbit-min score, orbit id, cell key): identical on every toolchain |
| Kept orbits (12 classes) | 1,460,098 | 759,190 (-48%) |
| In-cell policy | K=50k of the first 500k buffer, budget 2e6, early check on | unchanged for the incumbent (see Policy for the hedge) |
| Namespace | CFGSIG base | CFGSIG + `ord3` + `.oq1.qp1` + `.bud<budget>` + `.dg<list>:<kept>` (both digest halves, Astra 09-28) |

## Mechanics

1. **Deterministic ordered list.** Freeze the full untruncated raw list; the first-hit mode refuses
   to search a truncated list (built 2026-09-26). Sort by (orbit-min score over listed members,
   orbit id, cell key). Every worker prints the digest of the ordered list and of the kept-position
   bitmap; the driver refuses to search or resume on a digest mismatch.
2. **Ownership manifest.** Per class and direction, partition windows `[0, ceil(N_raw/178))` into
   disjoint half-open lane ranges, computed from the raw-list length. Assert that every kept raw
   index i has exactly one owner `(class, direction, range, arm = i mod 178)`. Commit the manifest
   and have it reviewed before Daniel's go.
3. **Checkpoint meaning.** The namespace includes the attempt policy (K, B, node budget) so that
   a policy change can never resume as if earlier attempts were made under the new policy. A
   deliberate re-attempt of old aborts would be a separately named replay lane.
4. **Transition.** H ranges start at their own lower bounds with fresh H checkpoints, independent
   of G job status. Running G reps may finish in the old namespace; G is no longer resubmitted.
   The loop reconciles the manifest against queued, running and finished jobs, so a cancelled or
   failed H unit shows as uncompleted, never as an invisible gap.

## Gates before any lane

(a) Q canary v2 PASS; (b) local: default-off identity, permutation-only kept set, six-control and
n=6/8/10 retention with Q + prune + order, every pruned cell streams empty, resume-boundary and all
existing suites; (c) the Fir-order dead-cell prediction on CELLSIZE 61315095: a finished stream
with cand=0 corroborates; timeouts/partials/unvisited are inconclusive; any emitted candidate
blocks launch; (d) the manifest check in Mechanics 2.

## Policy (for Daniel's decision at launch; updated 2026-09-27 from Astra's policy review)

- **Canonicalization changes prefixes, not candidate sets.** Every representative of an orbit
  holds the same candidates up to a bijection (same count, same flatness multiset), but a
  truncated pass visits a representative-dependent prefix. The known n=42 solution's image under
  Q sits at in-cell flatness rank ~257k-301k, which K=50k excludes with any number of buffers
  (and K=175k too); it is not "14.5M completions deep under the front policy". No representative
  rule is known to improve capture; keep H's launch rule (min cell key). A future diversity pilot
  may alternate between the two constituent 32-orbits' minima under a recorded seed, bound to the
  manifest, only if its matched cost is within 10% of the incumbent.
- **Breadth hedge: NOT at launch** (Astra 09-28: with the 6x stream, T_175/T_50 = 3.17, so K=175k needs
  > 3.17x the success mass; an 80/20 mix gains 2-2.4% under uniform density and loses 14% if the mass is
  in the top 50k). Launch 100% incumbent (B=1, K=50k, 2e6); after H's first reps give real timings,
  reconsider a <= 2 node-day K=175k cost pilot in its own namespace. Earlier text kept for the record:
  80% of node-days on B=1, K=50k; 20% on B=1, K=175k (not B=2), both at budget 2e6, as a bounded cost pilot first (<= 2 node-days on prespecified fresh cells, cost
  within 110% of the model). Model: T_175/T_50 = 2.31 at the measured 0.476/0.524 split; the
  wider policy wins iff its 125k extra candidates carry > 52.4% of the first 50k's mean success
  probability; the mixture is 1.10x under uniform rank density and 0.89x if all mass is in the
  top 50k. With the leaf-test speedup v = 6 (pilot 61866970, deployed 09-28), T_175/T_50 = (0.476/6+1.834)/(0.476/6+0.524) = 3.17: the wider policy now needs > 3.17x the success mass, so the hedge is weaker; re-price after H's first reps show the real per-cell split.
- **Class allocation.** Node-days in proportion to tile cost N_i/r_i (equal fractional coverage;
  the robust choice with no class prior). Spread lanes over low, middle and high cell-score bands.
- **Budget.** Keep 2e6. Abort rates alone justify no change: budget b' beats b iff
  p_b'/p_b > T_b'/T_b, and abort percentages do not give the left side. No replay store.

## Arm count must be odd (2026-09-29, from the first seven Fir H reps)

Every one of the first seven H reps (61997672-678) ended with `range_done=89/178` and
`arms_interrupted=89`. Cause, proven locally on the manifest's own lists (digests identical;
docs/reviews/evidence/passh_arm_stripe_2026-09-29.txt): under ORDER=3 the raw list is sorted by
(orbit-min score, orbit id, cell key), so each 64-group orbit is one contiguous block, and every
block has even size (orbit sizes divide 64); the kept representative is the block's first cell, so
EVERY kept cell sits at an even raw index. The arm interleave is `arm = i mod FH_NARMS`; with 178
(even) the 89 odd arms own nothing and exit at once with RANGE EXHAUSTED while the 89 even arms
carry the whole lane: half of every node idles. Classes with an odd-sized orbit flip parity mid-
list ((1,7,8,8) is mixed), but the workhorse (3,13,0,0) and (9,9,0,4) are pure-even. Per BUSY arm
the H reps did ~7.5 kept cells per 12 h against ~4.6 for G (the 1.6x HALL_FAST gain), so the
policy performs and only the striping halves it.

Fix (tools only, no solver change): `FH_NARMS=177` (odd), regenerated by
`tools/passh_manifest.py` (default now 177; the lint refuses an even count). An odd count spreads
the even indices over every residue (measured 177/177 arms busy in every workhorse lane, 8-32 kept
cells per arm). The digests do not depend on the arm count; window counts and lane ranges do, so
every H lane gets a fresh CKDIR (`..._177_ord3_...`) and the 178-arm checkpoints are orphaned
(Fir's seven first reps, ~7 node-days; nothing else had started). Balanced kept-rank sharding
(each arm owns kept cells by kept rank) would remove the residual per-arm imbalance but needs a
solver change and a review; it is not part of this fix.

## Performance rule (an operating policy, not a discovery claim)

At least 90% of G's kept-cells per lane-day = PASS. Below 80% = roll back to the untouched G
checkpoints. 80-90% = inconclusive, with one prespecified follow-up. The 1.92 tile ratio is a
size ratio, not a measured speed.

## Not part of the H launch (separate, each with its own pilot)

- `WZ_FH_HALL_FAST`: PASS (6.03x) and deployed fleet-wide 09-28 (build f611904).
- `WZ_FH_MID_SOLVE` middle-sign pre-check: identical results, 1.8% fewer nodes at small n;
  promote only if a cluster timing shows at least 2% end-to-end.
- Outer 2/3-quad reachable-tuple tables, A,B exchange (4 classes), A,B profile reachability:
  built only after timing shows where completion time goes.
- CLOSED by measurement: whole-cell top-K (KILL: every finished cell >= 8x the prefix),
  `WZ_FH_CD_PRUNE` DFS prunes (CLOSE: 12% fewer visits, 1% wall).

## Launch checklist (2026-09-28, after Astra's launch review)

- Solver: full `list:kept` digest in CFGSIG; ord3 search refuses without `WZ_FH_EXPECT_DIGEST`;
  raw cell keys asserted unique; driver unsets every measurement/locate variable.
- Submit lines: explicit `WZ_FH_STREAM_REV=0|1`, `WZ_FH_DRAIN_BATCHES=1`, class digest on every line
  (`tools/passh_manifest.py`; lint: `tools/test_passh_submit.py`).
- Gates re-run on the launch build: `test_passh_order.py` (incl. forged-list-digest resume refusal),
  default-off identity, resume boundary, six-control retention under ORDER=3 + Q + prune.
- Transition: per cluster, sha-checked redeploy (solver + driver), `scancel` PENDING G (checkpoints
  kept), submit all H units at their lower bounds; running G reps finish and are read; G never
  resubmitted. Any `RESULT: DIGEST MISMATCH` in an arm log = STOP.
- Follow-ups, separate from launch: shared C,D stream for (7,11,2,2)/(1,13,2,2) (sound, ~7% of that
  pair's cost, pilot only if >= 5% matched CPU saving); K=175k cost pilot after H timings.
