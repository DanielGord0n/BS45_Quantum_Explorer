# Independent review of `docs/archive/2026-09-22-n44-research-improvement-plan.md`

Reviewer: Claude (Opus 5.5), 2026-09-22. Review only: no code, queue, checkpoint or
deployment changes were made. Baseline inspected: `cc7d28e` (HEAD), HANDOFF entries
through 2026-09-22 night, `docs/archive/handoff/HANDOFF_ARCHIVE_to_2026-07-23.md`, the lever ledger, both Astra reviews and
the instrument spec, and `src/solver/wz_match.cpp`.

Labels used below: **FACT** (measured or read from code/logs), **HYPOTHESIS** (plausible,
unmeasured), **UNKNOWN** (no evidence either way).

## 0. State since the plan was written (checked, not assumed)

- **Lever-28 controls F41ec (60980457) and F43ec (60980458) have NOT been read.** FACT:
  `results/latest_check.txt` is from 2026-09-22 13:00, before their submission; Fir was in a
  listed power outage at that check. No verdict exists. Stage 1's lever-28 gate is still open;
  scheduling any new lever-28 test would be redundant.
- **No F2/G2 lane has ever been submitted.** FACT: the only F2 record is the build and a local
  B=2 validation (HANDOFF line ~478; `docs/plans/lever19_sweep_plan.md` 192-193). There is no capture
  verdict to locate.
- **The shadow instrument has not been built.** FACT: only the spec exists
  (`docs/reviews/2026-09-22-astra-instrument-spec.md`).
- **Cumulative cell counters (cc7d28e) are committed but NOT deployed.** FACT: Pass G runs on
  `3014b95`. Any unit started on 3014b95 and finished on a cum-counter build will have
  cum_* bases of zero for its early reps (old-format checkpoints load as 0). The plan's warning
  about reconciling per-rep deltas for mixed builds is correct and applies to every current unit.
- **No saved n=44 residual-row trace, no saved list of budget-aborted candidate identities,
  and no stream/score/complete time split exist.** FACT (searched repo and docs). Three of the
  plan's stages depend on data that has never been collected.

## 1. Per-proposal verdicts

### Stage 1a — finish lever-28 controls: **KEEP (already scheduled; do not add tests)**
- Prior work: implemented `9b6d117`, default off; local differential on 5 fixtures identical in
  verdicts, hit index, backtracks, aborts and charged nodes (HANDOFF 2026-09-22 late).
  Controls queued on Fir with explicit expected `nodes_this_cand` (212,872 / 88,616).
- Soundness: the check is an exact equality on a fully determined shift; node charging moved
  before placement, so budget semantics are unchanged. Stream order untouched, so no CFGSIG
  or checkpoint effect. Reversed in-cell order changes C,D enumeration only; no interaction.
- Correction to our own ledger: `docs/research/n44_search_narrowing_research.md` lever 28 says
  "1.5-1.6x fewer completer nodes". Under the counter-preserving placement the CHARGED node
  count is identical by construction; any gain is wall-time per charged node. The plan states
  this correctly; the ledger line should be fixed.
- Missing from the plan: the control that actually decides value is elapsed time on the same
  node count, and 2 lanes cannot distinguish a real 1.2x from node-to-node noise (Fir nodes
  vary ~10%). Treat the controls as a CORRECTNESS gate only; take the speed number from a
  paired same-node microbenchmark (below), not from two different jobs.

### Stage 1b — even-n42 retention control: **MODIFY (do statically, not on the cluster)**
- The pin fix is provably monotone: disabling a restriction can only enlarge the searched
  set (FACT from the code path; n=6 end-to-end confirms). The residual risk is not "is WZ-42
  retained" but "does anything else in canonicalization drop it". That is answered statically
  by `WZ_FH_LOCATE_C/_D` + `LOCATE_CANON` (commit 9fc4941), which already reports all five
  known solutions retained, plus the pin-compatibility check done in session (WZ-42 kept-cell
  representatives start (-1,-1); now unrestricted). An end-to-end n=42 re-find costs lane-days
  and tests the completer's in-cell rank luck, not retention. Drop the cluster version.

### Stage 1c — completed-unit alpha/eta accounting: **KEEP, with two corrections**
- Correct that endpoint cumulative snapshots must be used once per unit. The current driver
  sums cum_* across ARMS within one job (correct: each arm's value is its own endpoint), but
  the loop must take the LAST job's values for a unit, never add jobs. This rule is not yet
  written into `auto_prompt.md`; add it before the cum build is deployed.
- Units begun on 3014b95 cannot yield exact D/P/E/O. Either deploy the cum build and restart
  accounting from units that begin after the deploy, or reconcile by summing per-rep
  `cells_done/cells_prof_dead/cells_orbit_dup` over every rep's summary (only valid if every
  rep summary survived; header-only reps break it). Mark those units "rate only, not alpha".

### Stage 2 — joint-reachability shadow sampling: **MODIFY (replace the shadow instrument with a paired benchmark on saved candidates)**
- Prior work: the same FAMILY failed twice in June: `T23Filter::pq_reachable`
  (`src/solver/wz_exact_t23.cpp` 216-230, per-class box + parity against a key set) and the
  per-class residue prune (archive 2315-2347: fired ~0.001% of nodes, ~240x its benefit in
  cost; removed in v4). Failure mechanism (archive 1502-1511, 1718-1722): slack (`rem`) is
  large at mid depth where the tree is widest, so a box bound is vacuous exactly there.
- Does this proposal change the mechanism? Partly. Two differences are real:
  (1) in `wz_match` the same box shape against mod-6 EXACT target rows (`fh_abp_filter`,
  755-777) is the shipped AB_PROF lever that measured 5.2-6.6x fewer nodes, so profile
  pruning demonstrably bites in THIS solver; (2) the H-transform condition is strictly tighter
  than the box (exact lattice + L1 + parity). But its L1 <= q slack also grows with remaining
  quads, so the June mechanism predicts it still bites mainly at deep depth. HYPOTHESIS:
  real gain is modest unless many last-witness removals happen at mid depth. UNKNOWN: the
  depth distribution of completer nodes at n=44 (never measured), which decides this.
- Why the shadow instrument is the wrong first tool: the predicate is SOUND (exact necessary
  condition, validated exhaustively for q<=6 and on 3,028 checks against 42 solutions). A sound
  prune cannot remove a solution, so the question is purely CPU per resolved candidate at a
  fixed cap. That is measured directly and more simply by a paired A/B on a frozen list of
  real candidates: same binary, predicate on/off, same cap, same node. The shadow design
  (shadow survivor stack, first-cut subtree timing, disjoint-union accounting, observer
  calibration) exists to estimate savings WITHOUT enabling the prune; it adds three hooks of
  delicate bookkeeping, has its own documented bias (the saved subtree is measured on the old
  tree, and capped candidates change their visited set once pruning is on), and still ends in
  a paired benchmark before promotion. Skip to the paired benchmark.
- Budget semantics: enabling the prune removes charged nodes, so at a fixed cap more
  candidates resolve and fewer abort. That is the intended benefit, not a flaw, but reports
  must show resolved/aborted/CPU jointly (Astra's point). No stream-order or CFGSIG effect.
- Cost/benefit: implementation is small (the helper is ~15 lines plus class {1} and {4}
  handling); the benchmark needs a candidate dump (`WZ_FH_DUMP` exists, n=19 bit-identical)
  and one single-node job. Worth doing, but AFTER the time-split measurement shows completion
  dominates, and gated on the node-depth histogram showing mid-depth mass.

### Stage 3A — deeper buffers and rank sampling: **MODIFY (measure cell sizes and rank-cost first; do not build rank sampling)**
- Prior work: F2/G2 machinery built and validated locally (`WZ_FH_DRAIN_BATCHES`), never run.
  No capture evidence at any depth exists under the repaired stream (FACT). The background-
  sample percentiles (0.9/31.9/2.7%) are not in-buffer ranks and one was WZ-43 (Astra; accepted).
- The plan's rank sampling (>50k within the FIRST buffer) stays inside what I think is the
  real modelling error: **"first buffer" is a DFS-order prefix, not a flatness-selected set.**
  Front-only ranks by flatness only WITHIN an arbitrary enumeration prefix of 500k. Our n=42
  hit sat at stream index 1,000,000 (second buffer), i.e. outside the prefix, which is exactly
  what a prefix selection misses and what FR/F2 try to patch from the other end / one buffer
  deeper. HYPOTHESIS: if the flatness signal is real, the right selection is top-K over the
  WHOLE cell (bounded heap while streaming the cell), not top-K of a prefix.
- What decides between prefix-F2/FR and whole-cell top-K is one number we have never
  measured: the distribution of full-cell candidate counts (and streaming time per candidate)
  on workhorse cells. `WZ_COUNT_ONLY` exists for exactly this. If most live cells stream in a
  few minutes, whole-cell top-K dominates FR and F2 (same completions, globally flattest
  selection, no FR overlap). If cells stream for hours, the prefix policy stands.
- Sampling ranks >50k inside the prefix would produce, without a hit, only a completion-cost-
  by-rank curve. That curve is useful (it is the (1-g)K term of the cost model) but it can be
  logged for free by the existing lanes (nodes per candidate bucketed by in-buffer rank) rather
  than by a new sampling mechanism.
- Duplicate accounting: FR and F overlap on any cell whose total candidate count is below
  ~1M (forward first buffer and reversed first buffer cover the same candidates). The Pass G
  table counts 444 "distinct units"; they are distinct ranges, not distinct candidates. The
  plan says this; the sweep plan and my HANDOFF entries overstate it as "no repeated work".

### Stage 3B — replay of budget-aborted candidates: **ALREADY TRIED (in effect) — DROP as proposed**
- Prior work: the 5e6 middle-point lane (Fir F44f2728 = 59818967) vs its 2e6 neighbours
  (research doc 445; HANDOFF 09-19): aborts 5% at 5e6 vs 24% at 2e6, i.e. roughly 79% of the
  2e6-aborted population resolves clean by 5e6, at a cost of ~20% fewer cells per rep. Astra's
  resolved-candidate accounting (22.8M vs 22.9M) shows the two budgets resolve about the same
  number of candidates per rep. So "what happens to 2e6 aborts at 5e6" is already measured in
  aggregate.
- What replay would add is a hit in the tail. Without a hit it only re-measures cost. The only
  known-solution evidence says completions needed <=213k nodes (FACT, 3 cases), which says
  nothing about the tail (Astra is right that three successes cannot bound depth).
- It also needs infrastructure that does not exist: saved identities of aborted candidates
  (no such file is written today). Cost > value. If the tail matters later, the cheaper form
  is a policy, not a replay: a small fraction of lanes run at 5e6 (they both cover new cells
  and resolve the tail), which is exactly the lane that was already run.

### Stage 4 — packed correlation arithmetic: **MODIFY (gate on a time-split measurement; different hotspot from PLACE-V2)**
- Prior work: PLACE-V2 (`wz_match.cpp` ~799-812, default off) was a branchless rewrite of
  the COMPLETER placement loop: +6% ARM, -4% x86 (HANDOFF 1842-1872). That result does not
  transfer: `flat_score` is O(n^2) per STREAMED candidate, executed on every candidate of the
  500k buffer (and more when streaming continues), while only 50k per cell are completed.
- HYPOTHESIS, and the plan misses this: under front-only mode the streaming share g may now be
  large (500k scored + sorted per 50k completed). If g is large, packed scoring (and a cheaper
  sort, e.g. `nth_element` for the top-K instead of `stable_sort` of 500k) is the biggest
  constant-factor win available. If g is small, it is worthless. UNKNOWN until measured.
- Keep exactness requirements from the plan (identical scores, stable tie order, identical
  checkpoint positions). Note `nth_element` is NOT order-stable; any partial-selection change
  alters which tied candidates enter the top K and therefore requires a CFGSIG field and a new
  lane namespace. Packed scoring alone does not.

### Stage 5 — joint profile + correlation feasibility: **DROP (for now)**
- It is not concretely specified (no state, no memory bound), and its mechanism is the one
  that failed three times in June: incremental partial PSD bound (`spec_lb_prunes`,
  -0.21% nodes for +25% wall), incremental PAF bound, partial-CD spectral bound (archive
  1496-1511, 1718-1722). All failed for the same reason: slack is vacuous at mid depth. The
  current completer already carries the per-shift correlation bound
  (`abs(FH_CD_target[s]-Dab[s]) > Kab[s]`, line ~954) and the profile filter. Coupling them
  in an endgame DP targets the deep layers, which is where the existing bounds already fire.
  It only becomes worth specifying if the node-depth histogram shows the cost is in a band
  where neither current bound fires, and if Stage 2's paired benchmark shows the profile side
  alone has headroom.

## 2. Most important errors or omissions in the plan

1. **It has no stage that measures where the time goes.** Every later decision (joint
   reachability, packed scoring, K/B cost model, Stage 5) depends on three unmeasured
   quantities: the streaming share g, the completer node distribution by depth, and the
   completion cost by in-buffer rank. None requires new search; all can come from one
   instrumented rep of a lane that is already scheduled.
2. **It treats the DFS-prefix buffer as the natural sampling frame.** The flatness ranking
   is applied inside an arbitrary enumeration prefix. Whole-cell top-K selection is the
   obvious alternative and is not considered; full-cell size is the one number that decides it.
3. **It builds a shadow instrument for a SOUND prune.** For a provably sound predicate, a
   paired on/off benchmark on frozen real candidates answers the question directly and more
   simply.
4. **It proposes to re-measure budget tails already measured** (5e6 pair) and requires
   infrastructure (aborted-candidate identities) that does not exist.
5. **It misses the loop-accounting rule** for cumulative counters (take the unit's last job,
   never add jobs) that must be written before the cum build deploys.
6. **Minor:** scheduling a cluster n=42 re-find for retention, which LOCATE_CANON answers
   statically; and the plan's 80/10/10 allocation ignores that allocation is currently
   admission-limited (Nibi ~13-day waits, fairshare depleted), so "10%" of node-days is small
   in absolute terms and experiments must be cheap per rep.

## 3. Corrected priority order

1. Read F41ec/F43ec when they land (correctness gate only; no new lever-28 jobs).
2. **Time-split + depth + rank telemetry on one already-scheduled Pass G rep** (below).
3. Full-cell candidate-count distribution on a sample of workhorse cells (`WZ_COUNT_ONLY`,
   local or one short job) — decides whole-cell top-K vs FR/F2.
4. Depending on (2): if g is large, packed `flat_score` (exact, order-preserving); if
   completion dominates with mid-depth mass, joint-reachability paired benchmark on frozen
   candidates.
5. Write the cum-counter loop rule, then deploy the cum build with the next redeploy.
6. G2 / whole-cell top-K pass only after (2)-(3) choose between them.
Dropped: shadow instrument (replaced by paired benchmark), aborted-candidate replay, Stage 5,
cluster n=42 retention control.

## 4. ONE recommended next experiment

**Instrumented production rep (measurement only, zero behavior change).**
- Change: add wall-clock accumulators (steady_clock, sampled once per candidate and once per
  cell, not per node) for: streaming/enumeration, `flat_score`, buffer sort, completion; a
  per-arm histogram of charged completer nodes by depth d (array of n/2 counters, incremented
  where `fh_nodes_total++` already happens); and completion nodes per candidate bucketed by
  in-buffer rank (10 buckets over K). Print in the arm summary; aggregate in GATEB.
- Validation gate (local, n<=13 and the n=29 canary): identical verdicts, hit indices,
  backtracks, aborts, charged nodes and checkpoint files with instrumentation on vs off; the
  overhead of the timers measured and < 2% (if higher, sample 1 in 64 candidates).
- Deployment: substitute ONE already-queued Pass G workhorse rep on Fir (same range, same
  CKDIR; instrumentation is not in CFGSIG). Marginal search cost: zero. Resource cap: that one
  rep (0.5 node-day), plus at most one repeat rep if <500 live cells were completed.
- Success criterion: the rep reports g (streaming+scoring+sort share of worker time) with at
  least 500 live cells and 25k completed candidates; depth histogram over >= 10^9 charged nodes.
- Decision rules (pre-registered): g >= 30% -> build exact packed `flat_score` next; completion
  >= 70% of time and >= 25% of charged nodes at depths below n/4 remaining -> build the joint-
  reachability paired benchmark next; neither -> neither constant-factor project is worth it,
  and effort goes to selection policy (whole-cell top-K) only.
- Stop rule: if the instrumented binary differs in any identity check, do not deploy; if the
  cluster rep returns incomplete telemetry twice, stop and fall back to a local 10-minute n=44
  single-arm profile (`perf`/`sample` on macOS) for the time split only.

## 5. A better-supported direction the plan overlooks

**Select by flatness over the whole cell, not within a DFS prefix.** The only signal the
campaign has measured (flat-L1 ordering beat L2/PSD/max-shift on the known hits) is being
applied to an arbitrary 500k-candidate enumeration prefix of each cell. The n=42 hit lay
outside the first prefix. If full cells are only a few times larger than the buffer (to be
measured, priority 3), a bounded top-K heap over the entire cell gives each cell's genuinely
flattest K candidates at similar completion cost, removes the FR/F overlap, and replaces both
FR and F2 with one pass. This is a selection-policy change (new CFGSIG field, new lane
namespace), testable against the known solutions' cells with LOCATE plus a single lane each.
It is a hypothesis: it helps only if flatness rank within the whole cell predicts solutions
at least as well as within the prefix, which the three known cases cannot establish.
