# Pass H plan (v3, 2026-09-26: folds in Astra's Pass H review and red-team; NOT built, NOT deployed)

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
| Namespace | CFGSIG base | CFGSIG + `.oq1.qp1` + ordering flag + ATTEMPT POLICY (K, B, budget) + list digest |

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

## Policy (for Daniel's decision at launch)

- **Breadth versus depth hedge (Astra).** Streaming costs about half of worker time and is paid
  before any completion, so completing more of each cell can win: under the measured split,
  K=175k with one buffer costs 2.31x the incumbent per cell and wins if its candidates hold more
  than 2.31x the success mass (1.52x better if success density is uniform over ranks; worse if
  it is concentrated at the very front). The data cannot decide this. Proposal: 80% of node-days
  on the incumbent (K=50k, B=1) and 20% on B=2, K=175k over disjoint fresh ranges, starting with a
  pilot of at most 2 node-days to measure its real cost. Keep it only if it captures at least one
  known witness outside the incumbent selection at a measured cost of at most 5x per cell.
- **Class allocation.** Allocate each class's node-days in proportion to its tile cost N_i/r_i, so
  every class reaches the same fraction of coverage. That is the robust choice when we do not
  know which class holds a solution. Spread lanes over low, middle and high cell-score bands.
- **Budget.** Keep 2e6. Note the budget counter charges quad trials but not middle trials, so
  "2e6 nodes" is not the literal total-node cap; report both counters.

## Performance rule (an operating policy, not a discovery claim)

At least 90% of G's kept-cells per lane-day = PASS. Below 80% = roll back to the untouched G
checkpoints. 80-90% = inconclusive, with one prespecified follow-up. The 1.92 tile ratio is a
size ratio, not a measured speed.

## Not part of the H launch (separate, each with its own pilot)

- `WZ_FH_CD_PRUNE` stream prunes: pilot Fir 61518000.
- `WZ_FH_MID_SOLVE` middle-sign pre-check: identical results, 1.8% fewer nodes at small n;
  promote only if a cluster timing shows at least 2% end-to-end.
- Outer 2/3-quad reachable-tuple tables, A,B exchange (4 classes), A,B profile reachability:
  built only after timing shows where completion time goes.
