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

## Policy (for Daniel's decision at launch; updated 2026-09-27 from Astra's policy review)

- **Canonicalization changes prefixes, not candidate sets.** Every representative of an orbit
  holds the same candidates up to a bijection (same count, same flatness multiset), but a
  truncated pass visits a representative-dependent prefix. The known n=42 solution's image under
  Q sits at in-cell flatness rank ~257k-301k, which K=50k excludes with any number of buffers
  (and K=175k too); it is not "14.5M completions deep under the front policy". No representative
  rule is known to improve capture; keep H's launch rule (min cell key). A future diversity pilot
  may alternate between the two constituent 32-orbits' minima under a recorded seed, bound to the
  manifest, only if its matched cost is within 10% of the incumbent.
- **Breadth hedge: 80% of node-days on B=1, K=50k; 20% on B=1, K=175k** (not B=2), both at
  budget 2e6, as a bounded cost pilot first (<= 2 node-days on prespecified fresh cells, cost
  within 110% of the model). Model: T_175/T_50 = 2.31 at the measured 0.476/0.524 split; the
  wider policy wins iff its 125k extra candidates carry > 52.4% of the first 50k's mean success
  probability; the mixture is 1.10x under uniform rank density and 0.89x if all mass is in the
  top 50k. With the leaf-test speedup v = 6 (pilot 61866970, deployed 09-28), T_175/T_50 = (0.476/6+1.834)/(0.476/6+0.524) = 3.17: the wider policy now needs > 3.17x the success mass, so the hedge is weaker; re-price after H's first reps show the real per-cell split.
- **Class allocation.** Node-days in proportion to tile cost N_i/r_i (equal fractional coverage;
  the robust choice with no class prior). Spread lanes over low, middle and high cell-score bands.
- **Budget.** Keep 2e6. Abort rates alone justify no change: budget b' beats b iff
  p_b'/p_b > T_b'/T_b, and abort percentages do not give the left side. No replay store.

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
