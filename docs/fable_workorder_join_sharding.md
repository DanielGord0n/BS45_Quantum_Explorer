# Work order for Fable — shard JOIN22 phase 2 (the only thing blocking n=31+)

*Written 2026-07-16 by Opus. Read `HANDOFF.md` newest TOP OF MIND +
`docs/wz_paper_reconstruction.md` first, then invoke the `bs45-campaign` skill.*

## Why this is the task

The complete join **finds and self-verifies solutions** — measured on a 4-core laptop:
BS(12,11) in 0.01 s, BS(20,19) in **25.9 s**, NPAF==0 confirmed. It is **deterministic and
exhaustive per signature**: it FINDS a solution or PROVES none exists for that signature. SA
cannot do the second thing, ever, and is measured to cap at n≈33-35.

The old "join dead by TIME above n≈29" verdict used **pre-Thm-2.2 independent-side** counts
(1.58e15 @ n=29). The Thm-2.2-constrained C,D stream at n=29 is **1.74e9** — six orders smaller.
HANDOFF itself flagged that the verdict was stale; nobody re-derived the frontier.

**The binding constraint is walltime, not feasibility.** Live calibration from canary `16243606`
(n=29, Rorqual, 192 cores): stream phase ~6 h and decaying (32 profiles took 972 s, the next 32
took 4268 s — 4.4× slower), plus resolve (~4 h at n=29 last run) ⇒ **~10 h at n=29**. That is
**~3.5× worse than Opus's laptop-extrapolated ~3 h** — the 2.67×/rung fit is optimistic, do not
trust it. Recalibrated: **n=31 ≈ 70 h**, past every walltime you have (Trillium 24 h max).

**So: no sharding ⇒ no n=31, ever. With sharding ⇒ n=32/33 is attemptable, and every rung above
n=31 beats the banked best DETERMINISTICALLY.**

## The task

Shard **phase 2** (the A,B stream) of `WZ_JOIN22` across a SLURM array.

- Phase 1 (BUILD the C,D key table) is comparatively cheap → let **every task rebuild it**.
  Do NOT try to share it across tasks.
- Phase 2 (STREAM A,B profiles, probe the table) is the expensive part and shards naturally
  by **A,B profile range**.
- Phase 3 (RESOLVE) — each task resolves its own hits.

**Precedent to copy:** `WZ_PROF_LO` / `WZ_PROF_HI` (half-open) already exist for the pair22
counter — same pattern, same half-open convention. Add `WZ_JOIN22_AB_LO` / `WZ_JOIN22_AB_HI`
to the phase-2 loop. Array script: model on `cluster/deploy/cluster_pair22_gate.sh`.

## Non-negotiable validation (R1 — do this BEFORE any cluster submit)

1. **Union invariant, exact:** at n=11, n=15, n=19, the union of sharded runs must find the SAME
   solution set as the unsharded run. A solution must never fall in a seam. Prove it with a
   3-way partition, like `WZ_PROF_LO/HI` was proven (n=10: 92+125+87 = 304 = unsharded total).
2. **Ground truth:** n=7 pair22 must still read **66/91** (banked exact figures).
3. **Self-verify:** every FOUND must still print `VERIFY: max |NPAF[s]| … = 0`.
4. `tools/canary_thm211b.py` must still pass 6/6 valid champions.

## Sizing (do not repeat the 34 GB mistake)

`WZ_JOIN22_SLOTS_LOG2` must be sized from **DISTINCT KEYS ≈ stream / 6**, not the raw stream.
Measured dedup: 5.7× (n=7), 7.0× (n=11), 5.8× (n=15), 5.5× (n=19). At n=29: 1.74e9/6 ≈ 2.9e8
keys × 8 B ≈ **2.4 GB**. The old "34 GB" note assumed `SLOTS_LOG2=32`, sized for the *un-deduped*
stream. Keep table load < 0.85 (the code already warns).

## Sequence

1. **WAIT for canary `16243606`.** A real `*** BS(30,29) FOUND ***` banner (NOT a
   `resolve …/342 FOUND` progress line) = the join re-finds the banked n=29 class ⇒ frontier
   re-opened, and this sharding work has a proven target. If it FAILS, report the phase it died
   in and **stop** — do not build sharding for a dead method.
2. Implement + validate the shard (above). Local only. No cluster time needed.
3. Only then: **n=31 first** (calibrates the recalibrated curve on a rung whose answer we KNOW —
   n=31 is banked, so it MUST find it: a second canary, one rung up). Then n=32, n=33.
4. One signature per job. Each rung above n=31 that FINDS = new banked best. Each that
   EXHAUSTS = a proof of absence for that signature — also a real result, and something SA
   can never produce.

## Honest scope

n=41-43 is Wang-Zhu's published result; reproducing it with our own solver would be a serious
achievement and is what "maximize the chances" actually cashes out to. **n=44 is OPEN — the
paper says so plainly — and needs new mathematics.** This ladder does not reach it. Do not
promise it, and do not let the daily loop imply it.

## Also worth your fresh eyes

`survive_profiles`/`survive_profiles6` Thm-2.3 eq-2.11b filter + the `count_pairs22` modulus
generalization are **one evening old, written by Opus**, who made two conflation errors that same
session (claimed a profile cut would carry to the stream — it does not; measured 1.0-2.3×, not
120×). Validated against n=7 ground truth and the mod-6 invariant, but a review is warranted.
Regression tests: `tools/canary_thm211b.py`, `tools/measure_thm211b_prune.py`.

**Known bad data:** `results/champions/champion_v3_n27.txt` is **NOT a valid BS(28,27)** — NPAF
nonzero at shifts 6,7,12,14,16,18,20,23,25. Banked since April in violation of verify-before-claim.
Quarantine it; any canary keyed on n=27 was chasing a non-solution.
