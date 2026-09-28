# Policy after the three measurements — 2026-09-27

Reasoning only. Read only Claude's September 26 response and Pass H v3; accept those reports and the user's new measurements without rerunning them. No implementation, builds, tests, or cluster work.

**Recommendation:** retain H's mathematical reductions and budget 2e6. Change the proposed 20% hedge from two buffers at K=175k to **one buffer at K=175k**, initially as a bounded cost pilot. Do not probe whole cells, reopen the closed DFS prunes, or change budgets from abort percentages alone. Finish the already running leaf-test pilot; its result changes the economics of the hedge.

## 1. Representative choice: equal-size cells, different prefixes

**Claim.** There is no smaller full cell to choose within a valid 64-orbit, under the same invariant candidate filters. There is no established representative rule that improves capture. Exhaustive reachability does permit deterministic alternative representatives, but the present deterministic prefix is **not a uniform random sample** of an orbit's candidates.

**Proof/sketch.** If g takes listed cell X to listed cell Y, it gives a bijection between their quad-positive binary C,D realizations. Its inverse supplies the reverse bijection. Each pair NPAF and flatness score is preserved. With pins disabled and the common pair PSD predicate, the filtered candidate sets therefore have the same cardinality and full flatness-score multiset. Single-sequence tests add nothing when implied by that pair test. Thus neither full candidate count nor full-cell score distribution can distinguish these representatives. This is a mathematical equality; unintended noninvariant numerical decisions or additional filters would require separate examination.

What changes is the DFS ordering and the work required to reach a prefix. A symmetry maps a complete set bijectively; it need not map its first 500k elements to the other representative's first 500k. Partial-profile pruning and early spectral failures can also have different costs. Therefore prefix throughput is a possible cost proxy, but neither profile score nor a smaller hypothetical cell is a justified capture proxy.

Choosing the flattest among m fully streamed prefixes costs approximately `mG+C`, rather than `G+C`. At the reported split, that is `0.476m+0.524` times baseline cost before accounting for setup. Even comparing two requires over **1.476×** the success mass to break even. A lower observed score does not establish that improvement. Do not build that selection procedure now.

**Exact specification.** No change to H's launch representative rule merely because of the n=42 relocation. For a future representative-diversity experiment, use the at-most-two old 32-group orbits inside each surviving 64-orbit:

1. In each constituent 32-orbit choose its minimum listed real cell key. This gives one or two alternatives without streaming.
2. Use a fixed, recorded seed and a specified hash of `(seed, signature, 64-orbit ID)` to choose between the two. In a subsequent representative pass, select the other alternative where one exists. Keep the in-cell policy fixed while comparing this choice.
3. Freeze the resulting representative map **before** building the kept-position bitmap and ownership manifest. Bind the map/seed/version and direction to the namespace and ordered-list/bitmap digests. Never select independently from elapsed time inside each worker.

This alternation deliberately varies the newly quotiented Q choice. It is not a claim that these two options exhaust useful prefix diversity; reversal direction and other listed members also change prefixes. Do not claim independent samples or guaranteed new candidates. If future timing data support selecting a faster representative, freeze that measured map centrally and charge its calibration cost.

**Confidence.** High in equal cardinality and exhaustive safety; low in any capture improvement. Under an explicitly randomized representative choice, a witness's capture probability is the fraction of choices whose selected prefix/rank set contains an image of it. That fraction can be zero. It is generally not K/N, and hashing a cell choice does not make every candidate equally likely.

**Verification / preregistered rule / effect.** Every chosen cell must be listed, unpruned, and in the intended orbit. The six controls and all n=6/8/10 solution orbits must retain an actual representative with policy caps removed. Ordered map/digest and exactly-one ownership must agree across toolchains. Separately measure capped witness capture and candidate-orbit overlap; those are policy diagnostics, not correctness proofs. Permit only a small future diversity pilot after these checks and only if matched cost is within 10% of the incumbent representative rule. Promote no capture claim from timing alone. Expected mathematical reduction: none; possible gains are cheaper prefixes or more distinct candidate exposure.

## 2. Update the hedge: widen the paid-for buffer; stop treating buffer two as special

**Claim.** Replace the proposed `80% B=1,K=50k / 20% B=2,K=175k` allocation with a provisional **80% B=1,K=50k / 20% B=1,K=175k**, both at budget 2e6. This is an uncertainty hedge justified by generation amortization, not a proven optimum. The old argument specifically targeting an n=42-like second-buffer hit no longer transfers to the new representative.

**Expected-value argument.** Let M_K be successful-completion probability mass selected from a cell's first buffer at the fixed budget, and assume for costing that the next 125k completions cost the same on average as the first 50k. Normalize baseline time to one:

    T_50 = 0.476+0.524 = 1,
    T_175 = 0.476+3.5·0.524 = 2.310.

The wider policy wins iff `M_175/M_50 > 2.310`. Equivalently, the additional 125k candidates must have mean success probability greater than 52.4% of that in the first 50k. This unknown success probability includes the chance of finding the completion within the cap.

Under uniform success density over these ranks, the wider policy yields `3.5/2.31≈1.515` times the discovery rate. An 80/20 mixture then gives about **1.103×** baseline discovery intensity. If essentially all useful mass lies in the first 50k, the same mixture gives about **0.887×** instead. These calculations assume comparable fresh cells, rare hits and the displayed cost model; they quantify the risk, not an estimated n=44 likelihood.

If two buffers have equal cost and equal rank-conditioned mass, adding buffer two simply doubles both mass and cost. It supplies no special efficiency advantage over visiting another comparable cell. Adjacent DFS buffers are not demonstrated independent samples. The measured large cells and n=42 relocation remove the previous reason to privilege exactly two buffers. They do not prove that every deeper region is worthless.

The 530 capped observations are lower bounds on those sampled cells, not full sizes or a random sample of every class. Even so, they decisively support the stated whole-cell-top-K KILL. In a cell with at least 4M candidates, K=50k covers at most 1.25% of its emitted identities in one front; that fraction is **not** its probability of capturing a solution without an additional distributional assumption.

**Important correction to the n=42 description.** A rank of 257k–301k is excluded by K=50k no matter how many buffers are visited. It is also excluded by K=175k. Thus “14.5M completions deep under the front policy” is not literally true for the incumbent. Assuming zero-based batch 47 and rank r, reaching this witness requires at least 48 buffers and K>r; its attempted position would then be roughly `47K+r+1`. A figure near 14.5M describes a much wider, deeper policy. Do not undertake that expensive re-find merely to price the present hedge.

**Exact specification.** Use at most two node-days to price K=175k,B=1 on a prespecified, stratified set of fresh cells, keeping the current H representative and all other choices fixed. Include stream, scoring, sorting, completion, replay/setup costs and actual time to the same prefix. Freeze policy assignment before seeing results. If admitted, allocate 20% of **node-days**, not 20% of cells: at the model cost, 20% of cells on the wider policy would consume about 36.6% of time. Preserve the previous class-coverage balancing assumption; these workhorse measurements alone do not justify changing all class weights.

The leaf-test pilot can change this decision. If it accelerates generation by v while leaving completion unchanged, use

    T_175/T_50 = (0.476/v+1.834)/(0.476/v+0.524).

For illustration, v=2 raises the break-even mass ratio from 2.31 to about 2.72. Cheaper generation makes breadth more attractive and weakens the amortization argument. Substitute actual measured costs after the pilot; do not assume a speedup.

**Confidence.** High in these thresholds, low in the mass assumptions. The 20% remains a deliberate exploration allocation. No claim that it is the uniquely best fraction.

**Verification / preregistered rule / effect.** The wider one-buffer policy must preserve the exact baseline first 50k attempts within each paired cell and add attempts afterward; verify this on controls and exhaustive small-n streams. All six solution orbits must remain mathematically reachable without caps, but a capped policy need not capture all six. Pass the bounded cost pilot if identities agree and its cost ratio is at most 110% of the updated formula's prediction; otherwise drop the 20% allocation pending a new cost model. An additional historical witness is informative but neither required nor sufficient to infer a gain at n=44. Passing this gate licenses the explicit hedge, not a claim of improved discovery probability.

**Can rank distribution be measured cheaply without a hit? None, model-free.** Rank-specific abort rates, costs, PSD survival and partial-search survival are not positive solution labels. Exhausted candidates supply negative labels; aborted ones remain unlabeled. Small-n and six-control ranks can test transfer hypotheses, and planted solutions can test mechanics, but neither identifies the natural n=44 solution-rank distribution. A no-hit pilot can price a policy and test its implementation, not calibrate its positive success mass.

## 3. Aborts: measurable break-even costs, unmeasured solution enrichment

**Claim.** No fleet budget change is justified by the reported 25–36% abort rates alone. Higher, lower, or selective replay budgets can beat the incumbent under explicit assumptions, but none dominates it without information about success mass in unresolved searches.

**Proof / thresholds.** For a fixed candidate-selection policy, let T_b be cost per attempted candidate, including its allocated generation cost, and p_b the probability of finding a solution within budget b. Budget b' is preferable iff

    p_b'/p_b > T_b'/T_b.                         (1)

The right side is measurable. The left side is not supplied by abort percentages. Resolving more candidates as impossible need not increase p at all.

One testable-cost surrogate requires a strong assumption: the probability of a hit **conditional on resolution** is the same under the budgets being compared. Then `p_b ∝ 1−r_b`, where r_b is the abort rate, and (1) becomes

    (1−r_b')/(1−r_b) > T_b'/T_b.                 (2)

For illustration only, if moving to 5e6 costs 1.25× as much per candidate, (2) requires the following new abort rates:

| Current r at 2e6 | Required r at 5e6 |
|---:|---:|
| 36.4% | <20.5% |
| 27% | <8.75% |
| 25% | <6.25% |

Conversely, if a lower budget cuts total per-candidate cost to 0.8×, the corresponding maximum abort rates under the same surrogate are 49.12%, 41.6%, and 40%. These are **conditional thresholds**, not recommended target abort rates. Measure the cost ratio within each class/stratum; do not import 1.25 or 0.8 from another workload. The large lane-to-lane variation argues for matched comparisons, not class decisions from pooled percentages.

For a selected group of old aborts, let R be full incremental cost of a replay, including reconstruction and a cold DFS restart; q the chance that such a replay finds a solution; and T_new,p_new the corresponding fresh-candidate quantities. Replay wins iff

    q/R > p_new/T_new.                          (3)

If replay resolves a fraction eta of that group and one additionally assumes equal hit probability per resolved candidate in replay and fresh work, this reduces to

    eta/R > (1−r_new)/T_new.

Saved generation cost can make replay competitive, but a cold 5e6 run is not merely 3e6 additional nodes. Any comparison must also price storing/finding candidate identities. The flattest decile's high abort rate establishes neither enrichment nor depletion of q. There is no new evidence for replaying that decile specifically.

**Exact specification.** Keep 2e6 in H. If reconsidering budgets, use a fixed matched candidate sample and frozen DFS policy to measure T and r at the proposed budgets, separately by class and relevant rank bands; include production generation cost when comparing fresh-work policies. For replay, measure the actual extraction/restart path and its R. Budget/selection changes require separate attempt-policy namespaces. Do not build a production replay store merely to collect these statistics.

**Confidence.** High in (1)–(3); low in equal-hit-per-resolution, which may be false precisely because deeper successful searches differ from easy exhausted searches. Report the surrogate explicitly whenever using it.

**Verification / preregistered rule / effect.** Mathematical reachability with caps removed must retain all six controls and all small-n solution orbits. With identical DFS ordering, a larger budget must retain every smaller-budget hit; a lower budget deliberately need not. Replay must match fresh completion of the same identity under the same new budget, and must not advance the original namespace. A measured ≥10% improvement in the ratio in (2), or its replay analogue, permits at most a bounded experimental allocation **under that assumption**; it is not evidence for fleet promotion without validating the success-mass premise. If no such improvement appears, stop. Expected effect is unidentified from current abort rates; “no change” is the warranted production decision.

## 4. Soundness update and corrections to the launch plan

**Claim / proof.** None of the new measurements invalidates Q, closure pruning, the leaf PSD necessity, or the exact DFS profile criteria. The n=42 result demonstrates loss of a convenient **prefix witness**, not loss of its exhaustive orbit. The small DFS time saving is a performance failure, not a mathematical failure. Whole-cell top-K's rejection is an economic decision.

Claude reports the earlier profile-cap, resume-boundary, attempt-policy and internal-error issues fixed or explicitly addressed, plus a conservative Hall margin. Accept these reports as closing those specific review questions; I did not inspect or rerun the fixes. The decision-identical leaf-test pilot remains the most directly evidenced near-term performance work. A pair bound implies the same-bound single tests because their squared energies are nonnegative; reordering a conjunction changes cost, not its logical acceptance set. Numerical decision identity still belongs to that pilot's verification.

**Exact specification.** Update H's policy paragraph to item 2 and its deferred-work list to mark whole-cell top-K KILL and the DFS-prune performance investigation CLOSE. State explicitly that canonicalization preserves full candidate sets up to bijection, but can change which candidate orbits a truncated pass visits. Replace the “14.5M under front policy” shorthand with the actual K/B qualification above. Keep the existing manifest, digest, attempt-policy and deployment gates.

**Confidence / verification / preregistered rule / effect.** High in these distinctions. Require the reported six-control and exhaustive n=6/8/10 retention gates on the combined deployed configuration, and decision-identical emitted streams for the leaf-test optimization. Representative changes require their own map/manifest checks; changed policy capture must be reported separately. Any lost exhaustive witness blocks promotion. No additional large control run is requested here. The immediate expected benefit is cheaper unchanged candidate selection if the leaf pilot passes; no new probability-of-discovery forecast is justified.
