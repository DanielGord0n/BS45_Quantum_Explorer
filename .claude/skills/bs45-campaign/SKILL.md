---
name: bs45-campaign
description: Use for ANY work on the BS45 base-sequence campaign — reading cluster checker output, interpreting bestAB/FOUND, verifying and banking solutions, refilling the SA blitz, or deciding the next experiment. Encodes the campaign playbook, the verification discipline, and what is measured-dead vs proven-to-work.
---

# BS45 Campaign Playbook

**HANDOFF.md is the canonical STATE** (read its QUICK REFERENCE + newest TOP OF MIND first,
every session — chats get lost, the doc doesn't). **This skill is the PROCEDURE and the
judgment.** Update HANDOFF whenever checker results change; fill job IDs the moment submits echo.

## Prime constraints (non-negotiable)

1. **You cannot SSH to the clusters — Daniel can.** Every cluster action = a paste-ready command
   block for him (each `ssh` costs him a Duo push on his phone; batch commands per cluster).
2. **No heavy solvers on the laptop.** Local compute is for: compiling, small-n validation
   (n≤13, seconds), and `tools/verify_npaf.py`. Clusters do everything else.
3. **Verify before claiming — always.** A result exists only after: solver banner
   (`*** REPRODUCTION CONFIRMED ... FOUND ***`) → independent `python3 tools/verify_npaf.py`
   PASS → champion file written with full provenance → HANDOFF updated → committed.
   "Should work" and unverified banners are banned claims.
4. **When Daniel delegates ("do whatever's smartest"): pick one option, state why in a
   sentence, give exact paste commands.** No menus of options.

## The daily loop

1. Daniel pastes checker output (script lives in HANDOFF QUICK REFERENCE; keep the
   `grep -vE "<banked-file-IDs>"` filter current so `NEW FOUND?` only shows news).
2. Interpret (see traps below). 3. On a hit: banner dump → verify → bank → next rung.
4. Refill idle clusters: fresh **disjoint** `WZ_SEED_BASE` per run (ledger in HANDOFF; stride
   3,000,000; NEVER reuse — `SEED = base + task*100000`, same base = identical trajectories,
   a wasted duplicate). 5. Record round verdict + new job IDs in HANDOFF.

Refill template (script already on clusters at the BARE path, not the repo path):
```
ssh dangord@<cluster>.alliancecan.ca 'cd $SCRATCH/bs45 && sbatch --requeue --export=ALL,WZ_N=<n>,WZ_SEED_BASE=<next-base> ./cluster_sa_ladder.sh'
```
Nibi additionally needs `--account=def-ikotsire_cpu`. Ship NEW/patched source via tar-pipe from
repo root (scp does NOT expand $SCRATCH): `tar -cf - <files> | ssh ... 'cd $SCRATCH/bs45 && tar -xf - && sbatch ...'`.

## Output-reading traps (each cost real time once)

- **Trust only the banner, never the progress line.** The periodic `bestAB=` counter is
  per-signature and STALE — it reads 8 on files whose banner holds a real solution.
- **`grep -l FOUND` matches old banked files** — keep the exclusion filter updated after
  every bank, or every checker run cries wolf.
- **`TIMEOUT` at full walltime = SUCCESSFUL run** (12h shot completed). `CANCELLED ... slurmstepd`
  tails on finished files = the array hitting walltime/preemption — also normal. `--requeue`
  restarts preempted fragments automatically.
- **Odd bestAB values (e.g. 9) under WZ_PSD_BIAS** = the logged cost includes the bias term
  (true penalty is always even). Reported floors under bias are inflated by 0–3; only
  bestAB=0/FOUND is exact.
- **OOM ~20–30 s after "[profiles] ..."** in wz_match = the count-phase materialization wall,
  NOT the hash. Use `WZ_COUNT_ONLY=1` (streaming, cannot OOM) for any feasibility question.

## Measured-dead — do NOT rebuild or re-run (asking "can't we just...?" costs a round)

- **Exhaustive/complete search at n≥36**: 6–15 orders of magnitude short (proven 2026-06-27).
- **Hash-join (wz_match) above n≈29**: pair-work measured 1.58e15 at n=29, 4.0e16 at n=31
  (worst sig) — dead by TIME; memory OOM is just the first symptom. Streaming rewrite NOT justified.
- **Incremental PSD/PAF pruning in the backtracker**: built, A/B-tested, net-negative (06-27).
- **WZ_PSD_BIAS above n=30**: cracked n=30 (floor 4→0) but plain==bias floors at n≥31 (measured
  three ways). Not the lever anymore.
- **Long walltimes**: 24h × 1,536 chains at n=31 → same floor 8, no hit. Hits arrive at random
  restart times (41 min, 3.9 h, 11 h). 12h arrays maximize schedulability. Never request more.

## Proven-to-work: ticket volume

`wz_sa_v8` via `cluster_sa_ladder.sh`, 8-task arrays × 192 threads ≈ 1,536 chains/cluster,
fresh seeds every round. Ladder record: n=28 → 29 (×2) → 30 → 31, every rung blind and
independently verified. Cost curve: n=30 ≈ 4 arrays, n=31 ≈ 9 arrays; expect each rung to
cost several× the last. The winning arm is unpredictable (n=31 fell to the PLAIN arm on
Nibi — the "unreliable" cluster — 41 minutes into a run). Buy tickets everywhere.

## Decision doctrine (how this campaign avoids wasting weeks)

- **Measure before building.** The count-only probe (~100 lines, one day) killed a
  multi-week streaming-join build with two numbers. Always find the cheap measurement first.
- **Pre-register the decision rule** ("≲1e13 build / ≳1e15 dead") BEFORE results arrive, in
  HANDOFF — prevents motivated reasoning when numbers land in between.
- **Canary before trusting negatives.** Any new solver path must first re-find a BANKED
  solution (its signature class) before a "no solution" from it means anything.
- **Suspect guards, verify math.** The wz_match even-n guard was legacy, not mathematics —
  found by testing, confirmed by adversarial audit + exhaustive small-n ground truth
  (280/280). When a limit looks arbitrary: small-n empirical test + independent audit.
- **One validated change per round.** Ship nothing to clusters that didn't pass small-n
  validation locally the same day.
- **Honest framing, always:** these finds are solver-capability results — the sequences for
  n≤40 are known to the literature (Wang-Zhu constructed 41–43). The world record is n=44 and
  needs NEW MATHEMATICS. Never promise it; never call a ladder rung a "record".

## Escalation ladder (when the current rung stalls)

1. **Now:** SA blitz at the current rung, fresh seeds each round (this is the engine).
2. **Ready:** the Kotsireas methods conversation — brief at `docs/kotsireas_brief.md`
   (results + measured frontier + the filter-gap question). A methods ask, NOT a compute ask.
3. **Research-grade gamble (weeks, uncertain, decide explicitly):** reimplement the Wang-Zhu
   first-hit architecture — generate mod-6-residue-constrained C,D with their per-sequence
   filter DURING construction, backtrack A,B per survivor, stop at first hit. Our filter is
   ~10³× looser (measured); closing that gap is the only known route to n=42–43 territory.
4. **Not a lever:** more compute on current methods above the SA ceiling (~n≈33–35).
