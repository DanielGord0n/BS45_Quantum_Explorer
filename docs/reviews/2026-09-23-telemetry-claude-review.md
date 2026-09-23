# Claude review of the `codex/n44-telemetry` implementation (2026-09-23)

Scope: uncommitted changes on `codex/n44-telemetry` (base `c2a3813`): `src/solver/wz_match.cpp`,
`cluster/deploy/cluster_firsthit_probe.sh`, `cluster/deploy/auto_prompt.md`, `HANDOFF.md`, and the
untracked `tools/*telemetry*.py`, evidence and report files. Review only; nothing deployed, no queue
changes, no telemetry enabled anywhere. Tags: FACT (run or read here), HYPOTHESIS, UNKNOWN.

## Verdict

The instrumentation is correct and search-invariant. **One timing bias must be fixed before the
Fir rep** (resume-cell replay inside `cell_ns`). Four operational items must also be resolved. The
n=29 gate is now CLOSED (PASS) with the recovered archived configuration.

## 1. n=29 canary — recovered and PASSED (FACT)

The archived canary is **`WZ_FIRSTHIT=1 WZ_FH_M6=1 OMP_NUM_THREADS=1 ./bin 29 0 6 9 1`**, with every
other setting at its default. That means canon off, profile order 0, budget 200k, and the cell-order
and AB_PROF defaults that follow from M6. Evidence chain:
- The historical fingerprint is idx=26694, profile_rank=588, nodes=81320 (HANDOFF ~2336; the 08-05
  PLACE-V2 note in `c09a5c8`: "canon off ... EXACT historical fingerprint (idx=26694, rank 588,
  nodes 81320)").
- rank 588 rules out the mod-3 list, which has only 342 profiles at n=29, so M6 is required.
- Plain defaults (mod-3) stream past idx 26694 without a hit, because the candidate aborts at 200k
  nodes without the mod-6 profile lists.
- Codex's harness (`tools/test_firsthit_telemetry.py` ~78, ~121) additionally forces
  `WZ_FH_ORBIT_CANON=1`, `WZ_FH_PROF_ORDER=1`, `WZ_THM211B=1`, `WZ_THM212=1`. That is a different
  stream, so a 180 s timeout there is expected and says nothing about telemetry.

Results (this Mac, one thread each):

| binary / mode | elapsed | hit (idx, rank, nodes) | summary counters | A,B,C,D |
|---|---|---|---|---|
| baseline `c2a3813` | 158 s | 26694, 588, 81320 | reference | reference |
| telemetry unset | 169 s | identical | identical | identical |
| telemetry 0 | 169 s | identical | identical | identical |
| telemetry 1 | 169 s | identical | identical | identical |
| telemetry 64 | 169 s | identical | identical | identical |

(The four telemetry runs ran concurrently, the baseline alone; the 158 vs 169 s gap is load, and all
four telemetry modes agree to the second.) Mode 1 record: 26,037 completions, total_nodes =
hist_nodes = 1,457,359,631. Mode 64 record: hist_nodes 23,190,901, which is 1.59% of total, close to
the 1/64 = 1.56% expected. Both records pass the aggregator's validation.

**Required:** replace the harness's n29 configuration with the recovered one (keep the 180 s cap per
variant only if the machine reproduces ~160 s; use 300 s otherwise) and record the command in the
harness docstring and HANDOFF so it cannot be lost again.

## 2. Search behaviour, node charging, checkpoints (FACT)

- Depth counting is a compile-time template parameter (`wz_match.cpp` 937, 979, 1010, 1048-1049).
  `fh_ab_search<false>` is the unchanged code path; the only callers are those two sites. It adds
  no node charge, no branch outside the templated increments, and no change to the `> FH_BUDGET`
  comparison.
- Both charge sites (odd-length middle column at 937, main quad at 979) are counted.
  `depth_nodes` has G_N1/2+1 entries (1253); d ranges 0..G_N1/2, so the index stays in bounds for
  odd and even L.
- Nothing is added to CFGSIG, the CKDIR or checkpoint contents. The resume test pairs, together
  with the byte-identical checkpoint claim, are consistent with the diff: no checkpoint code
  changed.
- `WZ_FH_TELEMETRY` rejects values other than 0/1/64 with exit 2, before any search (1246-1251).
  The `stride-1` mask in `selected()` (747) requires a power of two, which 64 is.

## 3. Timing decomposition, histogram, rank buckets

**MUST FIX — resume-cell replay biases g upward.** The cell timer starts at 1818 for every retained,
non-dead cell, including the cell an arm RESUMES into. For that cell the stream is regenerated and
the batches before the checkpoint are replayed and discarded (1917-1920), and the resume batch is
re-sorted. All of that time lands in `cell_ns`, and so in g, while completing nothing new. In
Pass G an arm completes only ~2.2-3.4 live cells per rep, and nearly every rep resumes, so up to
~1/3 of an arm's cell visits can carry replay cost. HYPOTHESIS: this materially inflates g exactly
where the g >= 30% rule is decided.
Minimal fix: accumulate the elapsed time of the resumed cell until `fh_resuming` turns false into a
separate `resume_replay_ns` (and `resume_cells`), and have the aggregator report g and the phase
shares both with and without it.

Other items:
- `worker_ns` starts at `G_T0` (2066) and so includes profile enumeration, 2.11/2.12 filtering and
  orbit canonicalization over ~1M workhorse cells. It is correctly separated as `other_ns`, but it
  is a per-process fixed cost that repeats every rep. The report should keep it separate, since it
  is real overhead per lane-rep and not a streaming cost.
- The depth convention (remaining quads after the charged attempt, `4*max(0,half-d-1) < n`,
  aggregator 113) matches the intent of the review's late-node rule. Keep the raw arrays, as Codex
  did.
- Rank buckets (1940) are deciles of the searched drain (`10*ci/stop_at`, absolute index on resume).
  This is correct and honestly labelled. In front-only lanes that means deciles of the 50k completed,
  not of the 500k buffer or the cell. Interpret them as "completion cost vs in-drain rank" only.
- Score sampling (1835) times 1/64 of `flat_score` calls including clock overhead (~20-40 ns per
  sampled call vs roughly 1 µs of n=44 scoring). The bias is a few percent of the scoring share and
  acceptable.

## 4. Missing-arm handling and aggregation conservation (FACT)

- Conservation holds on every path. Every `return`/`break` between the cell-timer start (1818) and
  the accumulation (2027) is inside a lambda (`flat_score`, `drain`, `probe`), so a hit or SIGTERM
  mid-cell still accumulates the partial visit. `rank_ns + unranked_ns == complete_ns` holds
  exactly by construction, and the aggregator's equalities are safe.
- `FH_TELEM` is emitted on the single exit path shared by FOUND, SIGTERM, NO HIT and RANGE EXHAUSTED
  (2066). Arms SIGKILLed by the driver's STOP_GRACE fallback emit nothing and are reported as
  missing: correct, and never counted as zero.
- MODIFY the whole-lane invalidation rule. Requiring zero missing arms turns one straggler into a
  failed pilot. The missing arms are, however, a biased subset (arms stuck in streaming). Rule:
  <= 3 missing arms (<= ~2%) gives a valid read, reported with the count. More than that makes the
  pilot incomplete.
- A single malformed arm record raises and fails the whole aggregation (`last_record` -> ValueError).
  Prefer per-arm rejection into `missing_record_paths` with the reason, so one corrupt log can't hide
  177 good ones.

## 5. Overhead evidence vs the intended deployment

- The n13 benchmark (41 pairs, ~+0.4%) is setup-dominated and says little about n=44 streaming. The
  n=29 canary above is completion-dominated. There, modes 1 and 64 match mode 0 to the second, but
  it does not stress the per-streamed-candidate score timer.
- Deploy **mode 64 only.** Its added per-streamed-candidate cost is one hash plus a branch. Clock
  reads happen per 64th score, per completion (each completion is milliseconds or more) and per
  cell. HYPOTHESIS: overhead well under 1%. Mode 1 adds two clock reads per streamed candidate
  (~500k per cell) and should not be deployed without a same-node test.
- The same-node overhead gate Codex asks for is not necessary for a one-rep measurement. The
  overhead only costs part of 0.5 node-day and slightly biases the scoring share, which the
  analysis can bound. It IS necessary before any fleet-wide or long-running use.
- Volume gate: Fir workhorse reps complete ~570-620 cells per lane (GATEB `cells_done_sum` on
  09-19 reads). With the resumed cell excluded, >= 500 `cells_live_done` in one rep is borderline.
  Keep the gate and budget the allowed repeat rep.

## 6. Codex's qualifications against my review

1. Accepted.
2. Accepted. A sound prune changes outcomes under a fixed cap; report CPU, clean, abort and hit
   jointly.
3. Accepted. The ~79% figure is an aggregate comparison, not a matched replay.
4. Accepted. g >= 30% triggers inspecting the separate score share; it is not an automatic build.
5. Accepted. It also means the Fir units can yield rates, not exact alpha, until units begin under
   the cum build.
6. Accepted.
7. Accepted, including not scaling sampled nodes by 64 for the gate. (Moot anyway: a Fir rep has
   ~1e13 charged nodes, so the 1/64 sample is ~2e11, far above 1e9.)
8. Accepted. The local n=44 fallback is withdrawn: CLAUDE.md forbids heavy local solvers.
9. Accepted. Whole-cell top-K remains a hypothesis.

## 7. Operational findings (must resolve before the Fir rep)

1. **The daily loop runs in THIS working tree at 13:00 today.** The tree is on `codex/n44-telemetry`
   with uncommitted changes. The loop's `git add -A && git commit` would sweep Codex's work into an
   "auto:" commit on the Codex branch. The loop then pushes `main`, which would not contain its
   bookkeeping, and its HANDOFF edits collide with Codex's HANDOFF entry. Before 13:00, commit
   Codex's work on its branch and `git switch main`, or move the branch to a separate `git worktree`.
2. **Deploying to Fir's shared tree changes every queued Fir job's binary.** The driver compiles
   `src/solver/wz_match.cpp` at job start (`cluster_firsthit_probe.sh` ~56). Fir runs 9b6d117. This
   branch also carries cc7d28e (cumulative counters). Both are search-invariant and backward
   compatible (old checkpoints load; the old reader ignores the new lines), so a shared-tree deploy
   is acceptable and simpler than a job-specific snapshot. But the queued lever-28 controls
   F41ec/F43ec would then run on the new sha. That doesn't change their node counts, but record it.
   Alternatively deploy after they read.
3. Ship `tools/aggregate_firsthit_telemetry.py` to Fir's `$SCRATCH/bs45/tools/`. The driver calls it
   relative to the submit directory (221-225). Check `python3 --version` on a Fir compute node; any
   3.6+ works.
4. Mechanics for the one rep: pending jobs can't gain an environment variable through `scontrol`.
   Cancel ONE pending Fir workhorse G rep and resubmit it with the same name, `-d singleton`, the
   same env and `WZ_FH_TELEMETRY=64`. The CKDIR is unchanged because telemetry is outside CFGSIG and
   CKDIR, so the rep resumes normally.

## 8. Before substituting into one Fir rep (checklist)

- [ ] Fix the resume-replay bias (section 3) and add `resume_replay_ns` to FH_TELEM and the
      aggregator; rerun the identity suite and the recovered n=29 canary in modes 0/64.
- [ ] Put the recovered n=29 command in the harness and HANDOFF.
- [ ] Missing-arm rule: <= 3 missing = valid with disclosure; per-arm record rejection instead of a
      whole-lane exception.
- [ ] Commit Codex's work on its branch and return the working tree to `main` (or a worktree) before
      13:00.
- [ ] Decide deploy timing relative to F41ec/F43ec (after they read, or record the sha change).
- [ ] Deploy solver + driver + aggregator; resubmit one pending Fir workhorse G rep with
      `WZ_FH_TELEMETRY=64`. Cap: that rep plus at most one repeat. Stop on any identity mismatch
      or two incomplete telemetry reads.
