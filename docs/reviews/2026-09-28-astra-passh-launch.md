# Pass H launch review — 2026-09-28

Reasoning only. Read the requested plan, entire manifest, Fir submit file, and permitted ordering/CFGSIG source blocks. No execution or implementation. Accept the stated verification results, including the Q v3 canary and deployed decision-identical leaf optimization.

**Verdict:** the ordering and range construction are sound. Tighten checkpoint identity and explicit launch configuration before deployment. Launch H with the incumbent selection; wait for H's first reps before reconsidering the hedge. Shared streaming is sound but should remain a separate optimization.

## 1. Determinism and ownership: two concrete gaps, plus runtime checks

**Claim / proof.** The integer score and string-key comparator `(orbit-min score, orbit ID, cell key)` eliminate the former sort-tie ambiguity. With unique raw cell keys this is a total order. The minimum is taken over listed members, as intended. Refusal compares the full `list:bitmap` digest before search/resume. These are the right mechanisms.

By inspection, the twelve manifest range lists are contiguous, begin at zero, and end at their declared ceiling window counts. Range counts per direction are:

| Class | Ranges |
|---|---:|
| (3,13,0,0) | 6 |
| (9,9,0,4), (3,5,0,12), (1,7,8,8), (5,5,8,8), (5,11,4,4) | 19 each |
| (3,3,4,12) | 35 |
| (7,7,4,8) | 37 |
| (5,7,2,10) | 19 |
| (5,9,6,6), (7,11,2,2), (1,13,2,2) | 10 each |

That is 222 ranges per direction, hence 444 units. The inspected Fir file contains both directions of its six workhorse ranges, ending at window 5837, with the matching digest. This is inspection, not a rerun of the reported ownership simulation. Only Fir's submitted commands were available for review.

**Exact gaps and changes.**

1. **Checkpoint identity binds only the bitmap.** The inspected `.bud%lld.dg%s` append uses `G_ORDER_DIGEST.c_str()+17`, omitting the ordered-list half. Two different ordered cell lists can have the same kept-position bitmap. If the current expected digest is updated to the new list, the run-level refusal passes, yet an old positional checkpoint can still match the bitmap-only CFGSIG. Store and compare the full `list:bitmap` pair in checkpoint identity, or an immutable manifest ID binding both. Check that appending it cannot silently truncate the signature buffer. An immutable current manifest and genuinely fresh namespace protect this specific first launch, but do not repair the general resume contract.
2. **Forward direction is inherited, not explicit.** Every Fir line uses `--export=ALL`; reverse lines set `WZ_FH_STREAM_REV=1`, but forward lines omit it. If the submitting environment contains `WZ_FH_STREAM_REV=1`, both units can search the reverse prefix and the intended forward prefix is absent. Set it explicitly to 0 on forward lines and 1 on reverse lines. Also explicitly set `WZ_FH_DRAIN_BATCHES=1`. Verify the effective buffer size, K, budget, score-gate and normal search mode against the manifest's policy. In particular, **unset** `WZ_FH_LIST_ONLY`: the inspected code exits on its presence, so setting it to `0` still exits. A driver might already sanitize these settings, but it was not read; make the launch contract explicit rather than rely on the submitting shell.

**Additional exact checks.** Assert raw cell-key uniqueness, or explicitly deduplicate identical keys before assigning positions. Require the expected digest for production ORDER=3; currently the source comparison is conditional on the variable being present. On mismatch, the driver must mark the unit failed/uncompleted, never range-exhausted. Each unit must actually launch arms 0…177 with the matching shard count, and distinct units must have distinct checkpoint ownership. Class, direction and range remain identity fields even when two classes share digests. `singleton` job scheduling is not a substitute for that identity.

**Confidence.** High in the comparator, inspected range partition and the two cited configuration gaps. The source/submit evidence does not certify the uninspected driver or other clusters' command files; the reported simulation is evidence for their intended construction.

**Verification / preregistered pass/fail / effect.** Keep the passed six-control and exhaustive n=6/8/10 retention gates. Add a resume regression with two different ordered lists sharing a bitmap: the old checkpoint must be refused under the new list. Check effective launch configuration from a deliberately contaminated submitting environment (`STREAM_REV=1`, `DRAIN_BATCHES=2`, `LIST_ONLY` set). The intended forward/reverse policies must still be distinct and both perform search. Compare actual unit identities against the manifest, and reject missing/duplicate owners within a direction. Any failure blocks launch; no new large search is required. Expected effect is protection against skipped or duplicated intended samples, not additional mathematical pruning.

## 2. G → H: no structural gap if H owns its whole manifest

**Claim / proof.** Canceling pending G jobs and letting running G reps finish creates no ownership gap provided **every H unit** starts at its own lower bound independently of G status. Every surviving H orbit then has an H owner. G need not have reached it previously. The fresh H namespace prevents G's reordered positions from being mistaken for H progress.

This guarantees an assigned opportunity to search, not that every orbit has already been visited at launch or that a front-only capped pass exhausts it. A failed, refused, pending or interrupted H unit remains unfinished.

Some G/H candidate overlap is unavoidable. G and H representatives can expose overlapping or different prefixes; neither raw window numbers nor “G visited this orbit” certify that H's prescribed sample was covered. Forward/reverse H prefixes may also overlap. These repetitions cost time but are not soundness defects. Do not import G coverage to skip H cells without an exact candidate-and-attempt-policy reconciliation, which is not warranted for this launch.

**Exact specification.** Preserve G checkpoints for rollback; disable G resubmission at the transition; instantiate all 444 H units, not merely replacements for canceled jobs. Reconcile the manifest against queued/running/finished/failed units. Require completion of each arm's owned range before a unit is marked exhausted. Give running G jobs their original binaries/configuration or otherwise ensure finishing them does not alter checkpoint semantics.

**Confidence.** High, conditional on the launch/configuration checks in item 1.

**Verification / pass/fail / effect.** Retain the existing six-control, exhaustive-small-n and interruption/resume gates. Audit the unit/arm ledger after submission and after the first reps: every intended identity appears once as an active or completed obligation; refused/canceled/failed units remain obligations. Any disappearing unit fails the transition gate. Report G/H overlap as possible repeated work, not new coverage. No intrinsic loss arises from the stated transition; delaying H until all G completes is unnecessary.

## 3. Shared C,D streaming: sound, modest payoff, separate from launch

**Claim.** Completing the same selected C,D against both (7,11) and (1,13) is sound. Both have A,B norm contribution 170, but their sum constraints and compatible profile-row sets are different. A failure or budget abort for one target says nothing about the other. Equal cell digests do not merge the mathematical targets.

**Proof / specification.** The C,D pair fixes the same target vector `−N_CD` for both searches. If the retained cells, direction, stream filters, ordering and buffer/selection policy coincide, generation and sorting can be shared while both A,B completions remain independent:

1. Generate and select the common buffer once. For each selected pair, invoke each class's completion with its own signed sums, allowed profile rows, normalization rules, stacks, counters and full per-candidate budget. Share the immutable target correlations, not mutable completion state.
2. A class-specific proof that its compatible profile list is empty may skip **that** completion only. Reset all state between targets. Do not replace two budgets by one shared 2e6 cap or feed one class the other's row list.
3. Checkpoint `(cell,batch,rank,target-completed-mask)`, or an equivalent state. Advance past a candidate only after both required attempts finish. A stop after the first target must resume with the second still pending. Log outcomes and coverage separately for each signature.
4. Use a new joint manifest/namespace, including both signatures and attempt policies. Do not coalesce existing class identities just because their digest strings match.

For (1,7,8,8)/(5,5,8,8), both A,B norm contributions are 50, but the listed cells and kept sets differ. Share only the **intersection of exact kept cell keys with identical stream/selection configuration**; run unmatched cells separately. Do not intersect the lists and discard the difference. Matching only a 64-orbit ID is insufficient to preserve present prefix selections if its classes keep different representatives. Adopting a joint representative would be a separate policy change.

**Expected effect.** For two equal-cost standalone searches with generation cost G and completion cost C each,

    separate = 2G+2C; shared = G+2C;
    fraction saved = G/[2(G+C)].

At a 15% streaming share the ideal saving is **7.5% of the paired workload**, or about 1.081× throughput. At the ~13.1% share predicted from the older split and 6.03× stream speedup, it is about 6.55%, or 1.070×. Setup savings may add something; state management and restart overhead subtract. Sharing only an intersection saves less. This is not a 15% fleet-wide gain, and the fleet gain is further limited to time spent on these class pairs.

**Confidence.** High in soundness, conditional on exact class-local state and checkpoint handling; uncertain net performance. Worth retaining as a bounded follow-up, **not worth delaying H or expanding its launch changes**.

**Verification / preregistered pass/fail.** Compare the ordered `(C,D,signature)` attempts and per-target outcomes with the two standalone policies on frozen buffers. Six-control witnesses and every relevant small-n solution orbit must remain; exercise stops between the two targets. Include cells present in only one class for the differing-list case. Pilot the identical-list pair first and promote only if matched total CPU falls at least 5%, including setup/replay, with no lost attempt or altered budget semantics. Otherwise close it. Do not implement the differing-list extension before the simpler case passes.

## 4. Hedge: wait for H's first reps; launch the incumbent only

**Claim / calculation.** The 6.03× stream improvement greatly reduces the amortization advantage of K=175k. Using the earlier split, in old time units:

    G_new = 0.476/6.03 ≈ 0.07894,
    C_50 = 0.524,
    T_175/T_50 = (0.07894+3.5·0.524)/(0.07894+0.524)
                ≈ 3.173.

Thus the wider first buffer wins iff it contains over **3.173×** the successful-completion mass of the first 50k. The additional 125k must have mean success probability above approximately **86.9%** of the first 50k's. If H instead measures exactly 15% generation and 85% completion, the corresponding thresholds are 3.125× and 85%.

These calculations assume equal mean completion cost across the expanded ranks, which still needs measurement. “Successful mass” includes finding the completion within the fixed budget; abort rates alone do not supply it.

Under uniform success density over these ranks, K=175k would improve rate by only about 1.10–1.12×, and an 80/20 mixture by about **2.1–2.4%**. If essentially all useful mass is in the first 50k, that mixture instead yields about **0.863–0.864×** baseline rate. Neither model is established; the new economics no longer support launching the 20% allocation automatically.

**Exact specification.** Launch all H units at B=1,K=50k,budget 2e6. Wait for H's first completed reps to establish actual generation/completion shares, restart costs and class variation. Then reconsider the previously bounded, at-most-two-node-day K=175k,B=1 cost pilot in a separate policy namespace. Do not reserve 20% of the fleet at launch, and do not silently resume incumbent units under the wider policy. Keep the idea available rather than declare it mathematically dominated.

**Confidence / verification.** High in the arithmetic and the decision to separate launch correctness from this weakly supported allocation. Low in the unknown rank-success density. The pilot must preserve the incumbent first 50k attempts per matched cell and add later attempts; retain mathematical six-control/small-n reachability separately from finite-policy capture.

**Preregistered pass/fail / expected effect.** No hedge enters the launch. Reopen its cost pilot only after valid H timing exists; pass the pilot's cost gate only if the measured widened cost is within 110% of the formula using those timings and identity checks pass. That licenses an explicitly exploratory allocation, not an assertion of higher P(hit). Dropping the automatic launch hedge protects the simpler, now much faster breadth policy while retaining a bounded way to investigate rank mass later.

The plan's header and canary references still say “not built” and v2. Update those to the reported built state/v3 result so the launch checklist names the evidence actually used. This is documentation reconciliation, not a request to repeat the passed canary.
