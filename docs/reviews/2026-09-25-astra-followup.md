# Follow-up mathematics review — 2026-09-25

Read only `docs/reviews/2026-09-24-astra-review-claude-response.md` for this follow-up. Its measurements are accepted as reported, not independently rerun. No code edits, runs, builds, tests, or deployment work.

Priority: certify and count the closure prune; use an invariant score for the next fresh ordering if desired; no additional involution is proposed. Neither change should be folded into the running Q canary.

## 1. Q-closure prune: sound with a completeness certificate for the cell list

**Claim.** Yes: if a real cell X has QX absent from the **complete raw cell list in the same search domain**, then X contains no solution. Every real cell in its 64-element orbit can also be discarded. However, “every enumerator filter is necessary” is insufficient by itself: absence must certify failure of a necessary condition, not omission by enumeration or search policy.

**Proof.** Write P(C,D) for the pair's mod-6 profile. Suppose a solution (A,B,C,D) has P(C,D)=X. Quad positivity makes Q(C,D) binary; Q preserves each sum and every pair NPAF. Thus (A,B,Q(C,D)) is a solution in the same signature domain, and its cell is exactly QX. A solution-complete raw enumerator must list QX. Its absence is a contradiction.

Now suppose a different listed cell Y in the same 64-orbit contains a solution. A group element taking Y to X takes its actual C,D pair to a binary pair with profile X and the same pair NPAF. Its profile supplies the required signed sums at X. The resulting tuple is a solution at X, contradicting the first paragraph. Quad positivity is preserved by the group, so every transformation in this argument is valid.

This is an **orbit elimination**, not merely the choice of another representative. A missing Q-image is a certificate of infeasibility only under the conditions below.

**Enumerator properties required.**

1. **Solution coverage.** Every solution in the declared signature domain has its C,D profile in the raw list. A sufficient construction is exhaustive generation of all bounded, parity-compatible residue-sum vectors with the allowed signed totals, followed only by necessary filters. A theorem directly establishing solution coverage is also sufficient; enumerating every infeasible profile is unnecessary.
2. **Correct interpretation of filters.** Each rejection is necessary for a solution in this domain, with the appropriate existential A,B witness. A representative-only convention is not a necessary condition on every raw profile. Profile pruning must not silently assume an endpoint pin or a spent sign/reversal freedom.
3. **No policy omissions.** Membership is checked before orbit canonicalization, sharding, lane ownership, window selection, front limits, or scheduling. The list must not have been truncated by a profile-count cap, interrupted generation, timeout, or resource limit. “Not in this lane” and “not generated yet” prove nothing.
4. **Same domain and exact keys.** Use the same length, modulus, signed-sum convention, and filter configuration. Q preserves both signed sums, so its image belongs to the same intended domain. Compute its profile exactly. A fractional Q-image is impossible for a realized quad-positive pair; it must not be rounded into a different cell.
5. **Actual solution transformations.** The mirror-quad precondition must hold for every target solution to which the proof applies. The reported stream property establishes this for emitted n=44 candidates. To claim elimination of mathematical solutions rather than merely existing stream candidates, retain the existing justification that the mirror-pair stream covers every solution. The ordinary 32 transformations and Q must act on this domain of actual pairs as claimed.

Per-residue nonrealizability of QX is an especially direct certificate: a binary Q(C,D) would realize it. Failure of a genuinely necessary equation such as 2.12 is also sufficient. If an absent image's reason is not identified and completeness is not certified, mark it **unknown**, not dead.

**Exact implementation specification.** Freeze the full raw set S for one signature before deduplication or allocation. For every X in S:

    rho(r) = (43-r) mod 6
    QX.C[r] = (p[r]+q[r]+p[rho(r)]-q[rho(r)])/2
    QX.D[r] = (p[r]+q[r]-p[rho(r)]+q[rho(r)])/2.

If QX is nonintegral or absent from S, store a certificate `(X, QX, reason)` and mark the existing 64-orbit ID of X dead. After visiting every raw cell, remove **all listed members** of each marked orbit. Then choose representatives of the surviving orbits. A representative's own Q-image may be present even though another listed member has a missing image; inspect all members, not just the chosen representative.

Do **not** require all 64 formal images to belong to S. Ordinary negations or swaps may move outside its allowed signed-total convention. The valid test is QX membership for each X already in S, because Q preserves those totals. Whole-orbit propagation then uses the proof above.

One pass suffices. Every marked orbit is proved dead; removing it cannot create a new missing partner in a different orbit. Keep the membership test against frozen S rather than making the result depend on deletion order.

**Confidence.** High in the proof, conditional on the stated enumerator contract. The reported 0.3–5.3% counts are promising but are raw-cell percentages, not yet additional orbit or CPU savings.

**Verification that no solution is lost.** Claude should add the closure decision to the existing six-control retention audit: their orbits must remain unmarked and retain an actual binary witness. Apply the same assertion to the already available exhaustive n=6/8/10 control corpus where the precondition holds. For every marked orbit, retain at least one auditable absence certificate. Verify representative choice and input-list permutation do not change the dead-orbit set. These checks support the proof; six examples alone cannot establish enumerator completeness. No fresh large search is needed to audit membership.

**Expected effect and preregistered pass/fail.** Report raw cells with missing images, distinct marked 64-orbits, retained cells removed, and setup CPU separately. Correctness passes only with certified complete lists, zero lost control witnesses, and an explicit valid certificate for every marked orbit. Otherwise the affected elimination is disabled. Count first; promote the optimization if measured end-to-end cost for a fixed corresponding coverage target falls by at least 1%, including setup, with no correctness discrepancy. A zero or negligible gain closes it as a sound but unhelpful optimization. Any changed cell indexing/order requires fresh compatible checkpoints and ownership ranges.

## 2. Order by the orbit minimum: cleaner definition, unproved predictive advantage

**Claim.** I recommend the minimum profile score over the orbit's **listed real members** as the definition of “flattest orbit first” in the next fresh ordering. It is representative independent. There is no mathematical reason that a lexicographically selected representative's score is a better prior for existence of a completion. There is also no proof that the minimum is the statistically best invariant prior.

**Proof/sketch.** For the current score

    s(X) = sum_r (|p[r]|+|q[r]|),
    s_orbit(O) = min { s(X) : X in O intersect S },

the second quantity depends only on the orbit. All 32 old operations preserve s because they only negate, permute, or swap profile entries. Consequently there are at most two distinct scores in a 64-orbit: s(X) and s(QX). After the closure prune has certified that QX is listed for every listed member, one can equivalently use

    s_orbit(O(X)) = min(s(X), s(QX)).

Before that certification, minimize over actual listed members rather than possibly infeasible or out-of-domain images. Grouping the raw list and accumulating its minimum works in either case.

The exact C,D target is identical under Q. A representative's change from one profile score to another therefore does not improve that pair's intrinsic completion problem. The minimum expresses “this orbit has a flat representation,” not evidence that its solutions are more numerous or easier. It also rewards orbits whose two scores differ; that is a modeling choice, not a theorem. The maximum or mean would also be invariant.

There is one practical reason the kept representative's score might predict something useful: the search actually enumerates that representative's cell. Its score could correlate with streaming cost, its first-prefix composition, or discoveries under the particular cap. This would be a cost/selection-policy correlation requiring measurement. It supplies no general reason to prefer it as a solution-existence prior.

**Exact implementation specification.** While assembling the existing orbit map, retain both its current real representative and its minimum real-member score. Sort representatives by

    (minimum real-member score, deterministic orbit ID).

Do not simultaneously change the kept representative or the in-cell enumeration. That would mix an ordering experiment with a different prefix-selection experiment. If closure pruning is enabled, remove certified-dead orbits first. Keep old and new score distributions for comparison. Start a fresh ordering/checkpoint namespace and rebuild disjoint ownership ranges; never interpret old positional checkpoints in the reordered list.

**Confidence.** High that this is the appropriate invariant version of the existing score. Low that it improves finite-budget hit probability. If every orbit receives the same completed front tile, ordering alone changes when work happens, not which candidates the tile eventually contains.

**Verification that no solution is lost.** The retained representative set must be exactly the same before and after this ordering change; only its permutation changes. All six control witnesses must remain retained, with unchanged within-cell candidate sequences. Ownership must cover every intended representative exactly once per direction. A finite run can defer a formerly early witness; that is a policy effect, not a soundness failure.

**Expected effect and preregistered pass/fail.** No cell-count reduction and no direct per-candidate speedup are expected. Adopt the invariant definition only after the permutation/retention checks pass and setup overhead is below 1% of the intended tile cost. Call it an ordering cleanup unless an equal-budget comparison establishes benefit. Before observing such a comparison, fix the two orderings, completion cap, candidate budget, and cost accounting. Claim a control-level improvement only if no previously captured known witness is lost and captured witnesses per measured cost improve by at least 20%; otherwise report no demonstrated advantage. Even a pass does not establish a general n=44 discovery prior. Do not interrupt the current Q canary to run this comparison.

## 3. Another involution: none found

**Claim.** **None found.** In particular, reversing U instead of V is already in the 64-element group. The natural class of constant, separate linear transformations of the mirror-symmetric and mirror-antisymmetric channels supplies no further operation.

**Proof/sketch.** Define

    C+ = (C+RC)/2, C- = (C-RC)/2,
    D+ = (D+RD)/2, D- = (D-RD)/2.

At each positive mirror quad, either C+,D+ are both ±1 and C-,D- vanish, or the reverse. On the unit circle, after the common centering phase is removed, symmetric polynomials are real and antisymmetric polynomials are purely imaginary. Therefore

    |C|²+|D|² = |C+|²+|D+|²+|C-|²+|D-|².

Independent signed permutations of the two symmetric channels and the two antisymmetric channels preserve this identity and binarity. There are 8×8=64 such operations: exactly the group already generated. In this description Q swaps C- and D- and fixes C+,D+.

For a constant orthogonal two-channel map to preserve binary entries for every permitted quad, it must map the square's four vertices (±1,±1) to themselves. Its possibilities are precisely the eight signed permutations. Thus, **within this separate-channel linear class**, the existing group is exhaustive. If both individual sums must be preserved for all inputs, the symmetric-channel map must be the identity; the eight antisymmetric signed permutations are already generated by revC, revD, and Q. Special equal/zero sums can allow more stabilizers, but they too lie in the same 64-group.

In the previous notation U=(C+D)/2 and V=(C-D)/2,

    (RU+V, RU-V) = simultaneous_reversal(Q(C,D)),

so reversing the other channel is not a second independent factor of two.

This is not a classification of all nonlinear, position-dependent, or candidate-specific NPAF equivalences. Selectively reversing pieces introduces cross terms; arbitrary shifts can change support or aperiodic correlations; polynomial factor flips need not remain binary or be determined by mod-6 profiles. I found no extra condition guaranteed by this stream that turns those ideas into a sound new profile operation.

**Confidence.** High in the restricted classification and the redundancy of reversing U; no claim of a global maximality theorem for all possible conditional involutions.

**Exact specification and verification.** No new implementation. If a proposed operation reappears, first express it on `(C+,D+)` and `(C-,D-)`. If it is a pair of signed permutations, identify its existing group element and close the proposal as redundant. A genuinely different proposal must prove binary output for every input satisfying its claimed stream precondition, preservation of both sums and every pair NPAF, and a well-defined profile action. It must then retain an actual witness for each of the six controls under joint canonicalization; separate extra pins are not permitted.

**Expected effect and preregistered pass/fail.** No additional gain is predicted. Reject redundant transformations without a search experiment. A new proposal reaches an implementation pilot only after the preceding proof obligations and control retention pass, and an exact cell-orbit audit predicts at least 20% further reduction beyond the current 64-group. That is a gate for future ideas, not a result of this review.
