# Telemetry review fixes and pilot handoff

This supersedes the deployment gates in the original implementation report.
Claude's review: `docs/reviews/2026-09-23-telemetry-claude-review.md`.
Implementation branch: `codex/n44-telemetry`, isolated at `/tmp/bs45-n44-telemetry`.
The daily-loop checkout remains on `main`; do not switch it to this branch to review.

## Replay accounting

Schema version 2 adds `resume_cells`, `resume_replay_ns`,
`resume_replay_score_ns_est`, and `resume_replay_sort_ns`. Original totals remain
inclusive of replay, so the real allocation cost is not hidden.

Replay begins on entry to a resumed cell with a nonzero batch or in-buffer cursor.
It ends after the saved buffer has been regenerated and re-sorted, before its first
new completion. The timing flag is independent of `fh_resuming`: that search flag
already becomes false BEFORE sorting. No search-flag transition was moved.

Two important boundaries are covered:

- At `batch>0,k=0`, stop replay after discarding the preceding batch; generating and
  sorting the next buffer is fresh work.
- At `batch=0,k=0`, there is no replay. In unbuffered mode, end replay after the
  last skipped candidate, before enumerating the next candidate. If interrupted
  before catching up, include the unfinished replay interval at the cell boundary.

Let W=worker time, C=cell time, A=completion time, S=sort time, F=scoring time,
R=replay time, Rs=replay sort and Rf=replay scoring. The aggregator reports:

```
g = (C-A)/W
g_without_replay = (C-R-A)/(W-R)
completion_share_without_replay = A/(W-R)
sort_share_without_replay = (S-Rs)/(W-R)
scoring_share_without_replay = (F-Rf)/(W-R)
other_share_without_replay = (W-C)/(W-R)
```

Zero denominators produce null. Shares are recomputed from summed durations, not
averaged across arms. Scoring remains estimated in mode 64; both inclusive and
replay-excluded residual decompositions have validity flags. All clocks measure
elapsed worker time, not CPU time. Use adjusted shares for the fresh-search
bottleneck decision and inclusive shares for the actual cost of repeated reps.

## Missing and corrupt arms

Malformed JSON, missing required counters, impossible conservation or an unreadable
arm log rejects that arm only. `rejected_records` includes its path and reason.
Valid records remain visible. Duplicate input paths and mixed valid configurations
still fail the lane-level aggregation because pooling them would be ambiguous.

`coverage_acceptable` permits at most three missing/rejected arms and requires at
least one valid record; `complete_coverage` remains strict. Counts and a bias warning
are printed. Missing work is never imputed as zero or scaled up. For the production
178-arm rep, three losses are approximately 1.7%; the same absolute allowance is
not a general statistical guarantee for small test runs.

The volume gate remains >=500 completed live-cell visits, >=25,000 completions and
>=1e9 ACTUAL sampled histogram nodes. `pilot_read_usable` also requires acceptable
coverage and both timing decompositions valid. It does not assert representative
coverage of all classes or authorize an optimization automatically.

For a repeat rep, aggregate each job's arm logs independently. Pool additive
per-rep timing/counter totals and recompute ratios when assessing the combined
volume; do not average percentages. Assess the <=3 loss rule on each rep. These
FH_TELEM values are process-local and additive; GATEB cum_* values are cumulative
and must NEVER be added across jobs. Sample ordinals restart per process, so the
two reps are not claimed to provide statistically independent samples.

## Recovered n29 regression configuration

Run from an environment with no other WZ_* settings:

```
WZ_FIRSTHIT=1 WZ_FH_M6=1 OMP_NUM_THREADS=1 ./bin 29 0 6 9 1
```

Everything else uses defaults: canon off, profile order 0, budget 200,000, and
mod6-derived cell ordering/profile completion. Expected fingerprint:
`idx=26694 profile_rank=588 nodes_this_cand=81320`.

The harness now constructs this configuration separately from its small-n
fixtures. It explicitly checks that fingerprint, compares baseline and modes
0/64, and independently verifies every recovered solution. It permits 300 seconds
per variant as allowed in Claude's review. The earlier timeout was a different
stream caused by forced canonicalization/order/THM flags, not evidence of a
telemetry regression. Claude's pre-fix five-mode gate already passed; this session
reruns baseline/0/64 after the replay fix.

Commands:

```
python3 tools/test_firsthit_telemetry.py
python3 tools/test_firsthit_telemetry.py --n29-canary
python3 tools/test_aggregate_firsthit_telemetry.py
bash -n cluster/deploy/cluster_firsthit_probe.sh
git diff --check
```

Final observed results are recorded in
`docs/reviews/2026-09-23-evidence/telemetry-review-fixes-validation.txt`.

## One-rep pilot, not fleet-wide instrumentation

Mode **64 only** for the bounded pilot. A same-node overhead gate is required before
wider/long-running telemetry use, not before this single measurement rep. Budget
one existing Fir workhorse Pass G rep plus at most one repeat: at most one allocated
node-day total. Keep the >=500-cell gate; the repeat is explicitly reserved for it.
Two incomplete reads end the pilot; the withdrawn local n44 fallback stays withdrawn.

The 13:00 loop has now read F41ec/F43ec: exact charged-node controls PASS. Those
different-node elapsed times still establish no speedup. Deploy after recording
their results (now done in HANDOFF), and record the exact deployed revision.

For Daniel/Claude's cluster step:

1. Read the current Fir queue and select ONE genuinely PENDING workhorse G rep;
   recover its exact original submission environment, name, range and dependency.
   Do not infer environment settings from the job name alone.
2. Ship `src/solver/wz_match.cpp`, `cluster/deploy/cluster_firsthit_probe.sh` (also
   the active top-level driver copy), and `tools/aggregate_firsthit_telemetry.py`.
   Preserve file layout under `$SCRATCH/bs45`. The aggregator uses Python 3.6+
   syntax; confirm Python is available on a compute node.
3. A shared-tree deploy is acceptable per review. It changes the source compiled
   by EVERY subsequently starting queued Fir job, including the cumulative-counter
   build; instrumentation itself remains disabled unless exported. Record that
   scope and revision. Do not imply only the selected rep gets a new binary.
4. Cancel only the selected pending rep and resubmit with the same name, exact env,
   `-d singleton` and `WZ_FH_TELEMETRY=64`. Telemetry does not alter CKDIR/CFGSIG.
   Check the echoed job ID and actual CKDIR before calling the substitution complete.
5. Read GATEB_TELEM and its missing/rejected-arm counts. Use the replay-excluded
   shares to choose the next experiment. g>=30% triggers inspection of scoring
   share; it does not automatically justify packed scoring.

No remote command, source deployment, cancellation, submission or default flip was
performed in this implementation session. There is no guessed pending job ID in
this handoff. The shared checkout is left clean on main; all implementation work
is committed on its isolated branch.

After reviewing the committed branch, this ships the three required files and
records their source revision on Fir (no submission or cancellation):

```sh
cd /tmp/bs45-n44-telemetry
telemetry_revision=$(git rev-parse HEAD)
git archive "$telemetry_revision" src/solver/wz_match.cpp \
  cluster/deploy/cluster_firsthit_probe.sh tools/aggregate_firsthit_telemetry.py |
ssh dangord@fir.alliancecan.ca "cd \"\$SCRATCH/bs45\" && tar -xf - &&
  cp cluster/deploy/cluster_firsthit_probe.sh ./cluster_firsthit_probe.sh &&
  printf '%s\\n' '$telemetry_revision' > telemetry-source-revision.txt &&
  python3 --version &&
  squeue -u dangord -o '%.12i %.24j %.10T %.20E'"
```

The Python check above is on the login node; the pilot's `GATEB_TELEM` confirms
compute-node execution. Exact cancellation/resubmission commands require the
chosen pending job's original submission environment. Preserve it as instructed
above; do not reconstruct it from the displayed name.
