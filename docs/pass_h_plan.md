# Pass H plan (draft 2026-09-25, NOT built, NOT deployed)

Pass H replaces Pass G on all 12 n=44 classes. It is built only after the Q cluster canary
(Fir 61331128) PASSES, and deployed only on Daniel's go. Background:
`docs/reviews/2026-09-24-astra-review-claude-response.md` and HANDOFF 2026-09-24.

## What changes versus Pass G

| Component | Pass G (running) | Pass H |
|---|---|---|
| Cell group | 32 (neg/rev each, swap) | 64 (+ quad switch Q), `WZ_FH_ORBIT_Q=1` |
| Orbit elimination | none | Q-closure prune, `WZ_FH_ORBIT_QPRUNE=1` (all removed cells provably empty) |
| Cell order | flat: kept representative's profile score s(X) | orbit minimum over listed members, min(s(X), s(QX)); ties by orbit id |
| Kept orbits (12 classes) | 1,460,098 | 759,190 (-48%) |
| In-cell policy | front-only K=50k of the first 500k buffer, budget 2e6, early check on | unchanged |
| Checkpoint namespace | CFGSIG base | CFGSIG + `.oq1.qp1` + ordering flag; driver CKDIR gains `_oq1_qp1_om1` |

The in-cell stream and selection are unchanged, so the only effects are which cells are
kept, and in what order.

## Mechanics

1. **Ordering.** After canonicalization, stable-sort the raw list by (orbit-min score, orbit id,
   old position). The kept representative set must be identical with and without this sort
   (permutation-only check). Lanes keep addressing raw positions, so LOCATE, CELLSIZE and
   ownership code are unchanged.
2. **Ownership.** Keep S = 1000 raw windows for the workhorse and S = 300/150 elsewhere,
   exactly as in Pass G. Each window now holds about 48% fewer kept cells, so a lane finishes
   its range about 1.9x sooner. Lane count and names are H-prefixed (`F44h<k>`, `F44hr<k>`
   reversed).
3. **Transition.** Cancel Pass G jobs that are PENDING. Let RUNNING reps finish (their
   checkpoints stay valid under the old namespace). Submit Pass H lanes in their place, with
   the same per-cluster class assignment. Pass G coverage so far is 0-10/178 per lane, so
   re-searching those orbits costs little.
4. **Gates before any lane.** (a) Q canary PASS; (b) local: default-off identity,
   permutation-only kept set, six-control and n=6/8/10 retention, every pruned cell empty,
   and all existing suites; (c) the CELLSIZE prediction: all 716 predicted-empty cells in
   61315095's window stream cand=0.
5. **Pre-registered read rule.** Count kept cells completed per lane-day. PASS if it is at
   least 90% of Pass G's rate: per-cell cost is unchanged, so coverage per orbit is ~1.9x.
   FAIL (revert to Pass G) if it falls below 80%, or if any control or identity check fails.

## Open questions (for the math review)

- Is the orbit-min order safe to combine with Q and the prune in one namespace? Any
  interaction with front-only selection or with ownership?
- Any reason to prefer "kept-cells-only compact list" over raw-position windows?
- Anything in the transition that could leave an orbit unowned by every lane?
