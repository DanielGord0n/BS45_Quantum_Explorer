# n=44 research improvement plan

**2026-09-23 update:** Claude's independent review supersedes the implementation
priority below. Measurement-first implementation and technical qualifications are
recorded in `docs/reviews/2026-09-23-telemetry-implementation.md`. The shadow instrument
and speculative Stage 5 are deferred; do not execute this original proposal verbatim.

> For agentic workers: this is a proposal for independent review, not authorization to
> implement or deploy. After review, use the executing-plans skill for approved work.
> Speculative experiments require a separate implementation specification after their
> mathematical and measurement design passes review.

**Goal:** select changes that increase useful, distinct n=44 search exposure per allocated
node-day while retaining valid solutions, and measure the blind spots of the current policy.

**Architecture:** retain corrected Pass G as the baseline. Evaluate one change at a time
using deterministic replay, shadow measurement, and bounded cluster experiments. Separate
behavior-preserving acceleration from changes to which candidates or branches are searched.

**Tech stack:** existing C++17 first-hit solver, Python validation, SLURM accounting.

**Baseline:** source commit `cc7d28e`; HANDOFF reports Pass G on `3014b95`, with gated early
check controls on `9b6d117`. These are recorded states, not a fresh cluster inspection.

**Spec/context:** `docs/reviews/2026-09-22-followup.md` and
`docs/reviews/2026-09-22-astra-instrument-spec.md`. This document extends those proposals;
it does not replace their mathematical conditions or instrumentation details.

## Constraints and interpretation

- Review only at this stage. Do not alter production source, defaults, checkpoints,
  queues, allocations, or running jobs as part of reviewing this plan.
- No SSH from the assistant. Daniel runs any later approved, batched cluster commands.
- Local solver validation is restricted to n<=13 and short runs. Larger searches run
  on clusters; independent verification of supplied larger solutions is allowed locally.
- Do not restart measured-dead approaches without identifying a concrete changed mechanism.
- A completed owned range means the configured search pass finished, not mathematical
  exhaustion of every candidate or every completion.
- A capped completion is unresolved. A top-K exclusion is unsearched. Neither is a negative proof.
- Neither candidate throughput nor small-n hit rate is a measured n=44 success probability.
- F and reversed-F exposures overlap. Distinct cell ranges alone do not deduplicate their candidates.
- All numerical acceptance thresholds below are proposed decision rules, not predicted gains.
- No assertion that an idea is globally novel. "Not located in the ledger" requires checking
  source, history, archived experiments, and Claude's campaign context before calling it untried.

## Evidence and prior-work audit

| Proposal | Present evidence | Review obligation |
|---|---|---|
| Early outer-correlation check | Implemented, local identity tests; cluster controls pending in HANDOFF | Read latest F41ec/F43ec results before scheduling anything redundant |
| Joint mirror-quad reachability | Exact condition and toy tests; production rejection unmeasured | Check whether shadow instrumentation has since been implemented |
| Deeper buffers/ranks | F2/G2 support exists; no completed capture verdict found in the reviewed records | Locate all F2/G2 runs and actual outcomes before proposing new ones |
| Extending budget-aborted candidates | Fixed budgets tested; stratified replay was proposed | Check for saved capped candidates and adaptive/replay experiments |
| Packed correlation calculation | Backlog; no measured implementation verdict found | Inspect branches/history for implementations and benchmark results |
| Joint profile-plus-correlation feasibility | Research hypothesis, not an implemented algorithm | Distinguish from old failed per-class reachability and incremental PSD pruning |

Read newest HANDOFF entries before its QUICK REFERENCE: portions of QUICK REFERENCE still
describe pre-Pass-G operation. The ledger also retains superseded claims about ranks,
capture rates, and speedups. In particular, lever 28 preserves charged node counts;
its local improvement is execution time, not fewer charged nodes.

Useful source anchors at the baseline:

- `src/solver/wz_match.cpp`: `fh_abp_filter` (~760), `fh_ab_search` (~862),
  `fh_complete_ab` (~973), configuration/checkpoint handling (~1515 onwards),
  `CellCand`/`flat_score` (~1769), `drain` (~1824).
- `cluster/deploy/cluster_firsthit_probe.sh`: build (~56), GATEB accounting (~218).
- `docs/n44_search_narrowing_research.md`: levers 18–28 and review corrections.
- `docs/lever19_sweep_plan.md`: current range ownership and next-pass policy.
- `HANDOFF_ARCHIVE.md`: older failed bitset feasibility and SA experiments.
- `docs/reviews/2026-09-22-campaign-review.md`: correctness evidence and original proposals.
- `docs/reviews/2026-09-22-evidence/`: mathematical and differential evidence.

## Stage 0 — independent review and frozen experiment manifest

- [ ] **Change:** produce `docs/reviews/2026-09-22-n44-plan-claude-review.md` with a
  decision for every proposal: KEEP, MODIFY, ALREADY TRIED, or DROP. Cite exact evidence,
  including previous parameters and failure mechanisms. Documentation-only review.
- **Verify:** every proposal above has a verdict and evidence; missing evidence is marked
  unknown, not filled with an assumed result. Resolve disagreements with this plan explicitly.
- **If verification fails:** finish the missing audit before implementing the affected proposal.

After review, freeze a manifest for the first approved experiment: build hashes, compiler
flags, class, canonical cell, direction, buffer, exact candidate identity, node budget,
feature flags, seed, repetition/accounting boundaries, and allocated CPU envelope.
Use exact packed C,D plus completion context for candidate identity; verify hash collisions.
Record both current stream coordinates and identity so replay is not dependent on a changed order.

## Stage 1 — finish existing controls and establish measurement

- [ ] **Change:** review F41ec/F43ec and existing accounting results. Reuse completed
  controls. Include an even-n42 retention/re-find control under repaired canonicalization
  if an equivalent production-configuration control has not already passed. Directly
  completing a supplied target tests the completer, not its availability in the stream.
- **Verify:** separate stream retention, capped end-to-end re-find, and independent NPAF
  validation. For lever 28, require identical candidate outcomes and charged nodes on
  matched fixtures, including forward/reversed order and interruption/resume.
- **If verification fails:** keep that feature off and diagnose the first differing candidate.

- [ ] **Change:** collect completed Pass G ownership-unit accounting, reusing the cumulative
  counters and exact alpha/eta specification. Use endpoint cumulative snapshots once per
  unit, not a sum of repeated cumulative snapshots. For old builds, reconcile per-rep deltas
  with checkpoints. Report direction-specific rates and cross-direction overlap separately.
- **Verify:** all arms reach their range bound; retained = raw owned - orbit duplicates;
  live fronts = retained - profile dead - empty. Divide by actual allocated node-seconds/86400.
  Report startup/replay/idle time in allocated cost. Failed or interrupted allocations cost time too.
- **If verification fails:** mark that unit's rate incomplete; do not infer full-class alpha or
  speed from it. A nonrepresentative unit cannot establish a class-wide rate.

## Stage 2 — joint-reachability shadow measurement (first new experiment)

- [ ] **Change:** implement the already-written shadow specification at `fh_abp_filter`
  and the corresponding recursive-call boundary. Sample whole candidates deterministically,
  initially p=1/4096. Preserve the original survivor lists and search behavior. Keep a separate
  shadow witness stack; count only disjoint first-cut subtrees. Aggregate depth summaries;
  provide detailed per-call records for sampled candidates without synchronous hot-loop writes.
- **Verify:** old versus shadow-on gives identical candidate verdicts, solutions, charged
  nodes, aborts, and logical resume positions on small fixtures. Test parent-shadow removal,
  original-empty rows, multiple witnesses, nested cuts, odd centers, and capped candidates.
  Exercise exact helper against brute-force legal quad sums. Measure no-op instrumentation overhead.
- **If verification fails:** do not collect production data with that build. Correct shadow
  bookkeeping or the helper before considering a pruning-enabled version.

Use one production lane for screening, then held-out cells from contrasting classes and both
directions before generalizing. Do not call one lane representative of all 12 classes.
Measure `f*(s-h)` using the existing spec. Keep instrumentation/logging cost separate from
the projected cost of a minimal production predicate; remove shadow timing work from baseline
and removed-subtree estimates or document its bias. State uncertainty clustered by cell.
Shadow results describe the observed old search prefix; after a prune, fixed-budget DFS may
reach new branches. Therefore shadow savings are not a complete fixed-budget performance prediction.

**Decision:** advance to pruning-enabled paired cluster replay only if projected total CPU
saving is at least 20% after overhead accounting. This is a screening hurdle. If uncertainty
straddles it, collect more data within a predeclared cap. Fleet promotion requires measured
whole-worker benefit on held-out fixtures, correctness gates, and reviewed budget semantics.
Use a distinct experimental checkpoint namespace when effective search/cap outcomes change.

## Stage 3 — sample what the current policy leaves out

This has two independent subexperiments; never bundle them into one unexplained comparison.

- [ ] **Change A:** use existing G2/F2 machinery to expose deeper ranks and a second buffer.
  Add deterministic samples from ranks >50,000 within an already-generated first buffer,
  stratified by class/cell/direction, so testing the ranking assumption does not require
  enumerating an entire enormous cell. Report overlap with prior F/FR candidates exactly
  within the audit sample. Global enumeration-uniform sampling is not claimed.
- **Verify A:** a fixed seed reproduces the same eligible sample without repeats; no first-K
  candidate is mislabeled newly exposed. Account for re-streaming and identity-storage cost.
  Compare fresh exposure per allocated node-day and capture on previously unused known examples.
- **If verification A fails:** repair identity/rank accounting before comparing policies.

- [ ] **Change B:** capture a stratified sample of candidates aborted at 2m nodes. Replay the
  same candidates at 5m on clusters, logging additional cost, clean exhaustion, hits, and
  continued aborts. Use actual elapsed replay cost; do not subtract duplicated prefix work
  from resource accounting. Keep class, score band and original depth strata visible.
- **Verify B:** replay reproduces the original 2m prefix under identical build/configuration;
  newly resolved outcomes are separate from merely larger explored prefixes. Resume cursors
  are not assumed to revisit previously skipped candidates automatically.
- **If verification B fails:** investigate candidate/context mismatch before extending budgets.

**Decision:** do not promote a policy merely because it resolves more negatives or visits more
cells. Use distinct exposure, cost, and held-out known-solution capture jointly; the transfer to
n44 remains uncertain. If no hits or discriminating evidence emerge within the fixed envelope,
record the uncertainty and end the pilot rather than declaring a region useless.

Initial proposed allocation for a later agreed batch: 80% corrected production; 10% exposure/
budget experiments; 10% algorithm experiments. These percentages refer to allocated node-days,
including replay and controls, not queued jobs. They are a starting policy, not an optimum.
Do not cancel or re-tile live Pass G merely to enforce them. Fit approved experiments into the
next planned allocation and declare its absolute node-day cap before submission.

## Stage 4 — packed correlation calculation (independent engineering experiment)

- [ ] **Change:** first replace only `flat_score` correlation arithmetic using two packed
  C,D words and exact XOR/popcount identities. Preserve scores, stable tie order and all
  stream decisions. Defer `CellCand` storage redesign and completer target generation to
  separate comparisons so the source of a gain remains measurable.
- **Verify:** exhaustively compare scalar/packed correlations for binary strings of lengths
  1–13; compare deterministic generated strings at lengths 44,45,63,64 without running a
  large solver. Handle zero overlap and 64-bit mask/shift boundaries explicitly. Require
  identical sorted candidate IDs and resume behavior on small full-flow fixtures.
- **If verification fails:** retain scalar arithmetic; investigate sign encoding, masks,
  shifts and stable-order changes before any speed test.

**Decision:** measure the fraction of whole-worker CPU spent scoring before building a wider
rewrite. Proposed promotion threshold: >=5% whole-worker CPU reduction on matched production
fixtures, supported by repeated timings, with unchanged outputs. A fast microbenchmark alone
does not pass. If scoring is too small a fraction, defer this in favor of completion work.

## Stage 5 — joint profile and correlation feasibility (research, not ready to build)

- [ ] **Change:** formulate a necessary condition on the SAME remaining sign assignment
  satisfying both the residual profile row and selected unresolved correlation equations.
  Restrict the first design to a tiny horizon or endgame with explicitly bounded state size.
  Write the state, transitions, boundary contributions and treatment of interactions among
  remaining positions. Independent per-equation feasibility is not the proposed coupling.
- **Verify:** prove every genuine completion induces an accepted state; exhaustively compare
  with direct legal assignments on tiny cases. Then shadow-test incremental rejection and
  cost AFTER the existing filters, including joint profile reachability if adopted.
- **If verification fails:** revise or reject the mathematical proposal. Do not introduce
  heuristic rejection, lossy memoization keys or unproved symmetry reductions.

**Decision:** research-only until there is a precise state representation, memory bound and
measured benefit. If this duplicates the retired bitset/PSD experiments, drop it unless the
review identifies why the new coupling changes their measured failure mechanism. Proposed
CPU hurdle is the same >=20% total saving as Stage 2. No claim of global novelty or speedup.

## Review focus and final acceptance

The reviewer must specifically address: even-n pins plus canonicalization (Stage 1), odd
centers and shared witnesses (Stage 2), capped-prefix censoring (Stages 2/3), duplicate and
checkpoint identity (Stages 1/3/4), and correlated/selected benchmark examples (Stages 2/3).

Known own and WZ solutions already used to tune choices are regression/challenge fixtures,
not untouched holdouts. Freeze unused inequivalent examples before policy comparisons where
available; otherwise explicitly state that independent capture validation is unavailable.

Review acceptance is one evidence-backed verdict per stage, a corrected priority order,
and one concrete next experiment with its stop rule. Implementation acceptance later requires
recorded commands/results, independent verification of every hit, no unexplained behavioral
differences, and an allocated-cost comparison. Do not promise an n44 hit or calendar ETA.

**This planning session changed documentation only. No experiment or deployment was run.**
