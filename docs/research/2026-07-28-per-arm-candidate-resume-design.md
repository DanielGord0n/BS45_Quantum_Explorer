# Per-arm candidate-level resume (flat-first-preserving) — design

**Date:** 2026-07-28 · **Status:** approved (approach B) · **Target:** `src/solver/wz_match.cpp` (FIRSTHIT path) + `cluster/deploy/cluster_firsthit_probe.sh`

## Problem

Waves at n≥41 die mid-drain of their first profile cell (`cells_done=0` on every arm, all
jobs): the completer tests the flattest ~370k of each 500k-streamed batch at ~9/s and the
walltime ends. Cell-level resume (`WZ_FH_PROF_SKIP` as a *resume*) is therefore dead — no
arm ever finishes cell 0. Every new wave re-pays for already-tested ground unless it uses
PROF_SKIP as a *window selector* (wave 5), which burns whole windows to guarantee
disjointness. The fix: resume each arm exactly where it stopped, at candidate granularity.

**Key constraint discovered in code review:** with `WZ_FH_CELL_ORDER=1` (flat-first, the
measured ~35× density prior — non-negotiable), the tested set is NOT a stream prefix. Arms
stream a 500k batch, stable-sort by flatness, complete in sorted order, and die mid-batch.
"Skip the first S streamed candidates" would skip untested non-flat candidates and re-test
flat ones — unsound. The checkpoint must be **positional in the sorted batch sequence**:
`(cell pi, batch b, completed-within-batch k)`.

## Design

### 1. Checkpoint file — one per arm, keyed by CLASS, not job
`$WZ_FH_CKPT_DIR/arm_<shard>.ckpt`, where the driver sets `WZ_FH_CKPT_DIR` to a stable
per-class path: `$SCRATCH/bs45/fh_ckpt/<n>_<A>_<B>_<C>_<D>_<NARMS>_ord<PROF_ORDER>/`.
A new job on the same class auto-resumes; `fh_arms_<jobid>/` stays per-job for logs.
Text format:
```
CFGSIG=<hash>        # invalidates resume if stream order/filters changed
resume_pi=<int>      # first profile cell to re-enter
resume_batch=<int>   # 500k-drain batch within resume_pi
resume_k=<int>       # sorted candidates already completed in that batch
tested_cum=<int64>   # telemetry only
```

### 2. CFGSIG soundness guard
Hash over everything that changes stream order, sort key, or cell skipping:
`n, A,B,C,D, NARMS, shard, PROF_ORDER, M6, CELL_ORDER, AB_PROF, SCORE_MAX/tier,
buffer_cap(500k)`. Mismatch ⇒ warn + start fresh (never silently resume unsound).
`AB_BUDGET` is deliberately EXCLUDED — it changes per-candidate completion depth, not
stream position; waves may vary it freely.

### 3. Resume fast-forward (exact — every step deterministic)
- cells `pi < resume_pi`: skipped entirely (no streaming — they are done).
- `pi == resume_pi`: re-stream via `count_pairs22` (deterministic DFS), re-sort each batch
  (deterministic `stable_sort`); in `drain()` skip completing batches `< resume_batch` and
  the first `resume_k` of batch `resume_batch`; complete the rest.
- `pi > resume_pi`: normal. Re-streaming costs minutes (streaming ≠ the bottleneck —
  measured wave 4); only `fh_complete_ab` work is skipped.

### 4. Write path — periodic + atomic
In `drain()`'s completion loop, every `prog_sec` (reuse the 60s progress cadence) write the
checkpoint via temp-file + `rename()` (atomic, same filesystem). Also write in the
summary/SIGTERM path. `--requeue` preemptions therefore resume cleanly too (Trillium).

### 5. Driver changes
Compute + export the stable `WZ_FH_CKPT_DIR` (mkdir -p). Add `WZ_FH_RESUME=0` kill switch
(ignore checkpoints, fresh start). GATEB line adds min/median/max `resume_pi` and
`tested_cum` across arms — the wave-over-wave depth meter.

### 6. Validation gates (all local, small-n, before any tar-pipe)
a. n=19, resume disabled: bit-identical to HEAD (idx=807/rank=2/nodes=8087).
b. n=29 blind re-find canary: FOUND, NPAF==0.
c. **Resume-equivalence:** n=29 one-shot vs forced mid-drain checkpoint + second run —
   union of tested candidates identical (no gap, no overlap), hit still found.
d. CFGSIG mismatch: flip a flag, arm refuses stale checkpoint, starts fresh, warns.

## Non-goals
No help for enumeration-bound classes ((5,9,0,8)-type — that is the streaming-SIMD lever).
No change to completion math or filters. One validated change; clusters only after 6a–6d.
