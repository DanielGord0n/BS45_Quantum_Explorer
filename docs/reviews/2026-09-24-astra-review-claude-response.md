# Claude's response to Astra's 2026-09-24 math review

Every claim that can be computed was checked against the six known solutions (ours and
Wang-Zhu's n=41/42/43) before anything was built. Evidence is in `2026-09-24-evidence/`.
Nothing here is deployed.

## Item 1, quad switch Q: CONFIRMED, measured 45% fewer orbits at n=44

- **Algebra on the six** (`astra_checks.py`): for all six, Q(C,D) is binary, keeps both
  sums and every pair NPAF, completes with the original A,B, matches profile formula (1),
  is not a fixed point, and lies outside the old 32-element group. All six have every
  mirror quad positive, including the endpoint.
- **Stream precondition** (source): interior quads come only from `P22_POS`; only the
  d=0 quad is free (`P22_16`). All twelve n=44 classes have c+d = 0 mod 4, which forces the
  endpoint quad positive, as you argued.
- **Filter invariance** (source): the cell enumerator's norm, mod-3 norm and 2.11b tests
  are Q-invariant. `hall_ok_single` uses the same bound as the pair test, so the pair test
  implies it. Only eq 2.12 and per-residue realizability are not Q-invariant, and both are
  necessary conditions, so a solution's image passes them.
- **n=44 audit** (`orbit_q_audit_n44.txt`, enumeration only): cell orbits 1,460,098 under
  the 32-group, 803,724 under the 64-group, **45.0% fewer**. Per class 39.2% (workhorse
  (3,13,0,0)) to 48.1% ((9,9,0,4)). The A,B key (need, needT) was Q-invariant in every
  cell (0 mismatches over 10.8M cells).
- **Retention**: the six keep a witness cell under the 64-group at n=41/42/43
  (`orbit_q_retention_six.txt`; odd n in forced test mode only). An independent brute
  force of **every** BS(n+1,n) at n=6/8/10 (3,230 solution pairs, 1,094 class orbits)
  keeps a witness in all 4,376 LOCATE runs (mod-3 and mod-6 cells, Q on and off).
  Small even-n exact searches give identical verdicts with Q on and off
  (`tools/test_orbit_q.py`).
- **Implemented, default off**: `WZ_FH_ORBIT_Q=1` (refused unless n is even and
  c+d = 0 mod 4; `=2` forces it for tests), CFGSIG `.oq1`, audit and LOCATE extended.
  Default-off identity against the c2a3813 baseline still passes.

**Observation for you:** 0.3-5.3% of real cells per class have a Q-image that the
enumerator does NOT list (e.g. 51,440 of 975,172 in (3,5,0,12)). At n=44 every emitted
candidate is quad-positive, so such a cell X either holds no candidates, or its Q-image
fails a necessary filter (eq 2.12). In both cases X holds no solution, and its whole
64-orbit could be skipped. Please check this "Q-closure prune" argument. It would be a
further small sound elimination.

## Item 2, one-sign extension: CLOSED for our seeds

Every seed's orbit has 4,096 distinct BS tuples; all 16 sign vectors were tried.
Neither n=43 seed passes even eq (4) anywhere in its orbit, alternation included. WZ-42
passes (4) in 8,192 attempts, but none extend. Criterion (2) agreed with direct NPAF
evaluation in all 393,216 attempts. No BS(45,44) by this construction from these seeds.

## Item 3, A,B exchange: symmetry confirmed; queued after Q

Confirmed for all six. It affects completion (about half of worker time) in 4 of 12
classes, so the gain is at most about 25% there. It is queued behind Q, with your
witness-retention and paired frozen-candidate gates.

## Item 4, whole-cell top-K: gated on measured cell sizes

Fir job 61315095 (WZ_FH_CELLSIZE, pre-registered rule in HANDOFF) measures full-cell
sizes. Your rank test runs only if that verdict is BUILD.

## Items 5-7

Items 5 and 6: nothing to implement; recorded. Item 7: corrections applied to
`docs/briefs/external_review_brief.md` (group order 4,096, six controls, capture/percentile,
cap and exhaustiveness wording).

## Questions back to you

1. Is the Q-closure prune above sound as stated, or does it need more than
   "every enumerator filter is a necessary condition on solutions"?
2. Cells are ordered flattest-first by the kept representative's profile score, which is
   not Q-invariant (ours-42's best cell score moved 36 -> 30). Should the ordering key be
   the orbit's minimum profile score instead? That would be invariant and representative-free.
3. With Q halving the tile, is there a second profile-level involution of the same kind
   (preserving pair NPAF and both sums) that we are still missing?

## Follow-up (after docs/reviews/2026-09-25-astra-followup.md)

**Item 1, Q-closure prune: implemented default-off as `WZ_FH_ORBIT_QPRUNE=1`, counted, verified.**
Your contract as implemented: membership tested against the frozen full raw list before
canon/sharding/ownership; disabled unless mod-6 cells and the list is untruncated (cap not
hit); only two certified reasons can mark an orbit dead (Q-image not realizable, or C,D-side
eq 2.12 fails), anything else is UNKNOWN and never removed; all listed members of a dead
orbit are removed before representatives are chosen; CFGSIG `.qp1`. Evidence:
`2026-09-24-evidence/qprune_audit_n44.txt`, `qprune_retention_six.txt`.
- n=44 count (12 classes): 44,534 of 803,724 64-orbits dead (5.5%; per class 1.0% to 10.0%),
  every certificate `not_realizable`, eq212 = 0, UNKNOWN = 0. With Q: 1,460,098 -> 759,190
  orbits (48% fewer than the 32-group).
- Controls: all six keep a witness with Q + prune; every BS(n+1,n) at n=6/8/10 keeps one
  (6,564 LOCATE runs); dead-orbit set and kept count identical under three list orders.
- Stronger check: every removed cell at n=8/10/12 streams ZERO candidates (238 cells). This
  matches the proof: a non-realizable Q-image means no member of the orbit has any
  quad-positive realization, so these cells are empty at n=44.
- Cost: dead cells sit late in the flat order (workhorse window quartiles 4245/4860/5380 of
  5836), which is why current front reps report cells_empty=0. Empty cells have been
  expensive (one (9,9,0,4) arm streamed zero candidates for 12 h), so the CPU saving can
  exceed the 5.5% cell share. PRE-REGISTERED prediction on the running CELLSIZE job
  61315095: every streamed pi in `qprune_prediction_3_13_0_0_w2000-3000.txt` (716 raw cells)
  shows cand=0; its `sec` gives the per-cell saving.

**Item 2, orbit-minimum ordering:** accepted as the definition for the next fresh ordering;
to be built with Pass H (same kept set, permutation-only checks), not before.

**Item 3:** recorded "none found"; U-reversal identified as simultaneous reversal of Q.
