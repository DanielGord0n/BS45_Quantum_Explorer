# First-hit phase telemetry — implementation for Claude review

Baseline: `c2a3813`. Branch: `codex/n44-telemetry`. This implements the next
measurement experiment from `2026-09-22-n44-plan-claude-review.md`, not the earlier
shadow-sampling proposal. No solver optimization, search-policy change or deployment.

## What is implemented

`WZ_FH_TELEMETRY=0` (unset default): no diagnostic record.
`WZ_FH_TELEMETRY=1`: full score timing and charged-depth histogram.
`WZ_FH_TELEMETRY=64`: deterministic hash sampling of score calls and whole
completion candidates at 1/64 for the depth histogram. Completion timing, rank
counts/nodes, sort timing and cell/worker totals remain exact in both enabled modes.
Other values fail explicitly. The setting is excluded from CFGSIG and CKDIR.
Sampling uses process-local ordinals and restarts after resume. Samples from
different reps are not asserted to be independent; do not count them as independent
replications for confidence intervals.

`src/solver/wz_match.cpp` emits one terminal `FH_TELEM` JSON record per arm.
DFS has separate compile-time histogram-on/off versions: no histogram sampling
branch is inserted into each DFS node. Both node-charge sites are counted,
including budget-crossing charges and the odd-length middle column. Neither the
charge nor the cap comparison changes. The sampled version counts only selected
candidates and reports that fact; it does not pretend to have an exact full histogram.

Timing is `steady_clock` elapsed nanoseconds, **not CPU time**. On a busy or stalled
node it includes descheduling. Phase definitions:

- `worker_ns`: main's G_T0 through final reporting, including profile setup.
- `cell_ns`: sum of non-profile-dead retained cell visits, including partial visits,
  buffer setup, generation, scoring, sort, completion, and in-cell checkpoint/I/O.
- `complete_ns`: sum of timers around `fh_complete_ab` only; includes its target
  construction and final exact recheck, excludes caller progress/checkpoint logging.
- `sort_ns`: timers around `stable_sort`, including resume-cell re-sorting.
- `score_ns_est`: exact sum of measured scoring intervals in full mode; Horvitz-
  Thompson-style sample sum multiplied by 64 in sampled mode. Clock overhead is
  included in these intervals; no unsupported correction is subtracted.
- Streaming plus callback/bookkeeping residual = cell - sort - completion - score.
  It is **not pure generator CPU**. Sample estimates may make it negative; the
  aggregator flags an invalid decomposition instead of clipping it silently.
- Other/setup = worker - cell. All percentages are ratios of pooled nanoseconds.
- `g = (cell_ns - complete_ns)/worker_ns`: enumeration/scoring/sort/bookkeeping
  share of observed worker time. Completion share is complete/worker. They need
  not sum to one because setup/other work is reported separately.

Cell timers are accumulated at each cell boundary, not printed per candidate.
`cells_live_done` counts completed visits that streamed candidates; a hit or
interruption contributes a partial visit instead. These are per-rep diagnostics,
not an orbit-coverage or cumulative historical-alpha claim.

Ten rank buckets use `floor(10*sorted_index/eligible_drain_length)`, where
eligible length is min(K, buffer size) when top-K applies. Resume uses the original
sorted index, not an index rebased at the resume cursor. Short buffers use their
actual length. Thus these are deciles of the **searched drain**, not whole-cell
percentiles or information about excluded ranks. Counts, charged nodes, elapsed
completion time and aborts are pooled per bucket. Unbuffered completions are
reported separately as unranked. No sort algorithm or tie order was changed.

`tools/aggregate_firsthit_telemetry.py` validates counters and array lengths,
takes the last terminal record once per input arm log, rejects duplicate paths
and mixed n/stride/version, and reports missing arms. Driver emits a separate
`GATEB_TELEM:` JSON line after existing `GATEB:` only when enabled. Aggregation
failure is explicit and leaves the old GATEB output available.

The late-depth statistic uses **remaining quads after the charged quad attempt**:
`max(0, floor((n+1)/2)-d-1)`, with the terminal center counted as zero. At n44,
d=10 has 11 left and is excluded; d=11 has 10 left and is included. Raw depth
arrays remain available if the reviewer prefers entry-depth rather than attempted-
child-depth convention. Sampled late-share uses raw sampled histogram weights.

## Verification and commands

The first test run failed because the opt-in record did not yet exist (RED).
After implementation:

```
python3 tools/test_firsthit_telemetry.py
PASS: 63 baseline/off/full/sampled comparisons + 2 interrupt/resume pairs;
identical verdicts, sequences, counters, streams and checkpoint bytes.

python3 tools/test_aggregate_firsthit_telemetry.py
Ran 8 tests ... OK

bash -n cluster/deploy/cluster_firsthit_probe.sh
git diff --check
```

Identity fixtures use n=6,11,12,13; caps 0/1/30; buffered/unbuffered, short buffers,
top-K, two buffers, finite owned ranges, reversed enumeration, early-check modes,
and interruption followed by telemetry-mode-changing resume. Baseline source is
pinned to c2a3813 so the test remains meaningful after this change is committed.
Every found small-n output is also checked with the independent NPAF verifier.

The explicitly requested n29 canary is available separately:

```
python3 tools/test_firsthit_telemetry.py --n29-canary
```

It has a 180-second timeout per variant and requires a baseline re-find, exact
identity for modes 0/1/64 and independent `verify_npaf.py` success. Its observed
result in this session: **OPEN/FAILED TO COMPLETE**. The pre-change baseline
timed out after 180 seconds, before any instrumented n29 comparison ran. The
archived 94-second canary does not supply a complete reproducible environment;
this harness uses explicit mod6/profile/canonicalization/flat-order flags. Do not
equate its timeout with an instrumentation regression or claim the n29 gate passed.
No timeout extension or further large local search was run. Reproduce the archived
environment or run the paired gate on a cluster before deployment.

Local overhead evidence: `docs/reviews/2026-09-23-evidence/telemetry-overhead.json`.
41 balanced same-host pairs, n13 complete stream with cap1; medians versus new
binary telemetry-off were approximately +0.40% full and +0.42% sampled. The upper
bootstrap bounds for this fixture were below 0.6%. New-binary off versus baseline
median was -0.25%. These are short, timer-heavy local fixtures, not n44 or x86
production measurements. Do not promote those numbers to a production overhead guarantee.

Reproduce after compiling old/new binaries with identical flags:

```
python3 tools/benchmark_firsthit_telemetry.py --baseline /tmp/old --current /tmp/new
```

## Rulings and limits for the review

1. Accept the measurement-first priority and defer the shadow implementation,
   speculative combined filter and new budget-tail capture infrastructure.
2. Paired saved-candidate on/off tests can replace shadow measurement for judging
   a sound prune's actual performance. Soundness does not imply identical outcomes
   under a fixed node cap; report CPU, clean exhaustion, hits and aborts jointly.
3. Neighbouring 2m/5m lanes do not identify the exact fraction of the SAME aborted
   candidates that resolve. The quoted ~79% is an aggregate comparison, not a matched
   replay result. This qualification does not justify building replay infrastructure now.
4. g>=30% is an investigation trigger, not sufficient evidence for packed scoring:
   inspect the separately measured scoring share before choosing that optimization.
   Likewise the late-node threshold is a heuristic prioritization rule, not a theorem
   about joint reachability's benefit.
5. Old 3014b95 summaries never recorded cells_empty. Summing old done/dead/dup fields
   cannot recover exact alpha_front. The loop now says latest cumulative endpoint
   ONCE per unit and labels incomplete historical bases explicitly.
6. Observational instrumentation can change wall-clock stopping/checkpoint WRITE
   times. Identity claims concern fixed-work runs and deterministic test interruption,
   not identical progress before an external walltime signal.
7. Keep the >=500 live cells, >=25k completions and >=1e9 charged histogram nodes
   data-volume gate. In sampled mode the tool reports ACTUAL sampled nodes and does
   not multiply the sample size by 64 to pass the gate. Missing arms or an invalid
   timing decomposition invalidate a whole-lane conclusion even if totals are large.
8. Do not run the proposed ten-minute n44 fallback locally. The standing campaign
   constraint forbids it; use an approved bounded cluster profile or stop with the
   missing measurement. n29 was a specifically requested canary exception.
9. Full-cell top-K remains a selection hypothesis, not a dominance result. Full-cell
   counts and timings alone cannot prove globally flatter candidates are more likely
   to contain a solution. The first buffer is explicitly a DFS prefix.

## Deployment status / next action

Claude reviews this diff first. Production overhead validation remains open; use
same-node representative data and modes 0/1/64 before accepting <2%. No defaults
were flipped, no queued rep was modified, and the lever-28 controls remain unread here.

For a later approved one-rep deployment, ship the solver, driver AND
`tools/aggregate_firsthit_telemetry.py`; preserve the lane's existing CKDIR and
all search flags. Do not overwrite a shared source tree to target just one queued
job: queued jobs can compile whatever source is present when they start. Select
the actual pending rep and use an isolated build snapshot or an audited job-specific
wrapper after inspecting the queue. The current live job ID/state is unknown here,
so this document intentionally contains no guessed scontrol command.

Cap remains one existing rep (0.5 allocated node-day), plus at most one repeat.
If it does not reach the volume gate, report an incomplete pilot. Do not escalate
resource use automatically. A partial instrumented cell still contributes timing;
its visit does not count as a completed live cell.

Independent read-only code review found no important correctness defects. It
specifically noted elapsed-vs-CPU timing, residual bookkeeping overhead, and
process-local sampling; these limits are documented above. No cluster result was read.
