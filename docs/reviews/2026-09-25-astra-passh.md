# Pass H mathematics and design review — 2026-09-25

Read only the two requested documents and the targeted `count_pairs22` definition. No implementation or execution. Measurements below are accepted as reported. **Pass H is mathematically sound in principle, but the draft needs an explicit ownership manifest and deterministic ordering checks before launch.**

## 1. Pass H: approve the combination, strengthen ownership and transition

**Claim / proof.** Q, certified closure pruning, and orbit-min ordering can share one fresh namespace. Every solution orbit survives pruning, canonicalization selects a representative, and sorting permutes those representatives. This composition is sound. However, a permutation-invariant kept **set** does not prove that independent workers construct the same ordered list or that every retained position has a lane.

Answers to the three open questions:

- **Combine the changes:** yes, provided certificates and retention are checked together and positional checkpoints are fresh. Keeping the per-cell algorithm unchanged does **not** preserve the old sampled candidate set: Q can choose a different representative whose first 500k candidates need not be the Q-images of the previous prefix. Exhaustive orbit reachability survives; front-only hit retention is a separate policy question.
- **Compact versus raw list:** both are sound. Compact indexing makes coverage and load allocation clearer, but is not necessary for H and introduces another index change. Keep raw indexing for this release, with the explicit coverage check below. Orbit-grouped sorting can concentrate representatives unevenly across windows; it does not guarantee that every lane becomes 1.9× faster.
- **Transition gaps:** “submit H lanes in place of canceled G jobs” is unsafe if interpreted literally. Ranges belonging to running or already completed G jobs would then lack H owners. G coverage cannot be transferred by window number into the reordered H list.

**Exact plan changes.**

1. Freeze the full untruncated raw list, certify prune decisions, and select representatives by a deterministic cell-key rule. Replace the final sort's `old position` tie-break with a stable **cell key** (or prove old positions identical in every process). Verify the entire ordered raw-list digest and kept-position bitmap, not just the kept-set size, under input permutations. Every worker must match the manifest digest/configuration before searching or resuming.
2. Generate a complete H manifest for every class and intended direction, independently of G job status. With the stated modulo-178 raw indexing, raw index i belongs to arm `a=i mod 178` and window `w=floor(i/178)`. Partition `[0,ceil(N_raw/178))` into disjoint half-open lane ranges. For **each retained index**, assert exactly one `(class,direction,range,arm)` owner. Use the actual solver mapping if it differs. Never compute this endpoint from 759,190 kept orbits; it comes from each class's raw-list length.
3. Start each H range at its own lower endpoint with a fresh H checkpoint. Running G jobs may finish in their old namespace, but must not suppress corresponding H entries. Disable automatic G resubmission; reconcile the H manifest with actual submitted/queued/running/completed jobs so cancellations or failures leave explicit uncompleted units, not invisible gaps. Before Daniel's deployment approval, make this manifest reviewable.
4. Current prune certificates are all `not_realizable`, so they prove zero quad-positive C,D realizations, not just absence of a BS completion. Audit every removed orbit's certificate. Do not require exhaustive streaming of all removed n=44 cells as a launch gate. For the existing 716-cell prediction, a **finished** stream with cand=0 corroborates the certificate; a timeout, partial zero, or unvisited cell is inconclusive. Any emitted candidate contradicts that prediction and blocks launch. Future certificate types need their own conclusion; “no solution” does not always imply “no emitted candidates.”

**Confidence / retention checks.** High in the mathematical composition; ownership cannot be certified from this draft alone. Require the six actual solution witnesses and the entire available n=6/8/10 solution corpus to retain a searchable representative under the combined configuration. Separately require ordered-list determinism and exactly-one ownership of **every** kept index across all twelve classes. These structural checks cover ownership cases that six examples cannot. Keep the running Q canary as its existing gate.

**Preregistered rule.** Any failed certificate, lost witness, digest mismatch, missing owner, or duplicate owner within a direction is a launch failure. Same-orbit work in opposite directions is intentional, not an ownership error. For performance, retain the proposed ≥90% PASS / <80% rollback floor if desired, but label it an operating policy; specify 80–90% as inconclusive with at most one prespecified follow-up comparison. Compare matched/stratified work and include setup and empty-cell costs. The ratio 1,460,098/759,190≈1.923 is a tile-size ratio, not measured speed: at 90% of G's completion rate, the nominal fractional-tile rate is about 1.73×. Neither ratio proves a discovery-probability gain. Rollback uses untouched G checkpoints, never translated H positions.

## 2. Stream lever, cheapest: eliminate impossible endpoint quads immediately

**Claim / proof.** For even-length C,D, each interior positive-product quad has entry sum divisible by four. Thus the endpoint quad's entry sum must equal `sum(C)+sum(D)` modulo four. At n=44 this forces positive product. The current `count_pairs22` tries all 16 endpoint choices and discovers the incompatibility later through profile constraints.

**Specification.** On the C,D side with even L, before placing a root quad, require

    (quad entry sum) mod 4 = (sum(tx)+sum(ty)) mod 4.

Use normalized residues for negative values. Iterate the existing table in the existing forward/reversed order and skip incompatible entries; do not replace its order with another table. Leave odd lengths and A,B unchanged. This removes half the root choices but has no guaranteed factor-of-two time gain because their subtrees have different costs.

**Confidence / verification.** High. It rejects no profile-matching leaf, hence preserves the entire emitted sequence, not merely solutions. Require byte-identical emitted streams in both directions on exhaustive n=6/8/10 cell controls, plus retention of all six witnesses (odd controls use the unchanged path). Candidate identities and first-buffer membership must agree. Internal DFS counters may decrease.

**Build gate / pass-fail.** The reported ~50% generation share already makes this tiny check worth a paired pilot after H is frozen. Compare complete streams or time to the same emitted prefix; do not use leaves/emitted as the benefit metric, since both counts are unchanged. Promote only with all identity checks and at least 5% lower generation time on a prespecified representative sample. Otherwise defer. Keep this separate from the H launch change.

## 3. Stream lever, strongest candidate: exact remaining-quad profile feasibility

**Claim / proof.** Apply joint mirror-quad reachability to the **C,D generator's residual profiles**. This is the earlier reachability mathematics in a different hot path, not a new theorem. Here it can exactly decide profile reachability before expensive leaf PSD tests.

For even L and modulus six, reversal pairs residue classes into three distinct pairs `{r,s}`, with `s=(L-1-r) mod 6`. A positive quad contributes one of the eight vectors ±h_j, where h_j are the columns of

    H = [ 1  1  1  1
          1 -1  1 -1
          1  1 -1 -1
          1 -1 -1  1 ],    H^T H = 4I.

For a residue pair, let q be the number of unassigned mirror quads and

    Delta = (tx[r]-px[r], ty[r]-py[r],
             tx[s]-px[s], ty[s]-py[s]),
    t = H^T Delta / 4.

Remaining positive quads can realize this residual exactly iff

    t is integral,   ||t||_1 <= q,   q-||t||_1 is even.       (*)

Necessity follows because each future quad changes t by a signed unit vector. Sufficiency follows by using |t_j| required units and filling the remaining even number with opposite pairs. The three residue pairs use disjoint quads. Existing pins, if any, can make this relaxation less than sufficient, but cannot invalidate its necessity.

**Specification.** Guard to even C,D, m=6, with the positive-root precondition established as in item 2; do not generalize to odd-length/m=3 fixed residues without separate handling. Check all three blocks at the root. Thereafter only the block touched by the latest placement changes: update its four integer t-coordinates by the corresponding signed unit, decrement q, test (*), and undo on backtrack. Reject nonintegral initial t exactly. Preserve branch order and all leaf filters. Bounds at the root may already follow from Q-closure and current filters; the intended extra benefit is on **partial residual states**.

**Confidence / verification.** High in the criterion; speedup unknown. This prune eliminates only branches with no profile-matching leaf, so require identical emitted identities and order, including both enumeration directions, across the exhaustive n=6/8/10 controls. Retain all six solution witnesses; odd cases stay on the original path. Check q=0, boundary ||t||_1=q, and parity failures explicitly. A lower internal-node count with unchanged leaf count is expected, not a discrepancy.

**Build gate / pass-fail.** Seconds per 500k together with the generation share quantify the potential saving. Leaves/emitted alone cannot distinguish cheap from expensive profile DFS: branches killed before a leaf are absent from that ratio. If generation remains ≥40% of worker time, run one bounded paired pilot on frozen real cells, timing to the same emitted prefix and separately completing small/empty cells. Record recursive visits and profile leaves. Promote only if generation CPU falls ≥20% including updates/setup, all stream identity checks pass, and end-to-end worker CPU falls ≥10% on the matched workload. Otherwise stop. No new shadow-timing framework is needed.

## 4. Conditional PSD bound: sound, lower priority; no level reordering now

**Claim / proof.** A cheap-form partial spectral bound exists, but it has the same slack risk as the failed A,B PSD idea. At an existing grid angle, let z_C,z_D be the assigned polynomial sums and R_C,R_D the numbers of unassigned entries. Every completion obeys

    |C(theta)|²+|D(theta)|² >=
    max(0,|z_C|-R_C)² + max(0,|z_D|-R_D)².

If this lower bound exceeds 178, reject the branch. This is the triangle inequality; using only a subset of existing grid angles makes the test weaker, not unsound.

**Specification / confidence.** High mathematical confidence, low performance confidence. Only consider a small fixed subset of existing grid angles near the bottom of the tree, with outward-rounded bounds so a near-threshold floating-point error cannot reject a valid branch. Retain the original leaf test. Do not implement a full 200-angle incremental state first.

**Gate / verification / rule.** Consider only if the pending leaves/emitted ratio is ≥10 and Hall work is confirmed to dominate generation; that ratio alone does not prove a useful early bound. A bounded paired pilot must preserve the six witnesses and all n=6/8/10 solution orbits, and preserve emitted streams except for an explicitly diagnosed legacy numerical tolerance discrepancy, which blocks promotion until resolved. Require ≥20% generation-time saving and ≥10% worker-time saving including maintenance; otherwise close it. It ranks behind exact integer reachability.

Reordering mirror levels is mathematically possible only if endpoint rules attach to positions, not recursion depth, and residual counts are rebuilt. It also changes the first 500k candidates. I found no evidence-backed ordering heuristic worth combining with H. **No further sound-and-cheap stream prune is proposed here.**
