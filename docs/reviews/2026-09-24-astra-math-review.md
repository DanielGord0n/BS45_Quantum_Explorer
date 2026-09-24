# BS(45,44): mathematical review — 2026-09-24

Reasoning only. Read the external brief, the research ledger from Lever 26 onward, Claude's September 22 review, and targeted solver sections for the named functions/canonicalization. No code changes, solver runs, builds, or tests. The user's current state supersedes dated operational statements in those documents.

**Main result: no new condition found that eliminates any of the twelve classes or demonstrably cuts the C,D stream by a large factor.** There is, however, a sound extension of cell canonicalization by the quad switch, with up to another factor of two. There is also a small, exact construction experiment worth distinguishing from another search campaign. Whole-cell flatness selection remains a hypothesis, not a dominance result.

The following items are ranked by expected value per implementation effort. Proposed acceptance thresholds are prospective, not measured results.

## 1. Extend C,D cell canonicalization by the quad switch

**Claim.** The 4↔5 transformation acts on mod-6 profiles themselves. It need not be postponed to candidate-level deduplication. Combine it with the existing 32 transformations before choosing a cell representative; the resulting C,D group has 64 elements and can halve the remaining cell count.

**Proof and formula.** Use zero-based indexing, length L=44, reversal R, and

    U=(C+D)/2, V=(C-D)/2,
    Q(C,D)=(U+RV, U-RV).

For a legal C,D mirror quad, C_i C_(L-1-i) D_i D_(L-1-i)=+1. Consequently the supports of U and V are reversal invariant. Replacing V by RV therefore leaves C',D' binary. It exchanges precisely the two opposite-row, antisymmetric quads

    [ + - ]       [ - + ]
    [ - + ]  <->  [ + - ].

Moreover, on the unit circle,

    |C'|²+|D'|² = 2(|U|²+|RV|²)
                 = 2(|U|²+|V|²) = |C|²+|D|².

Thus every pair autocorrelation is unchanged, the SAME A,B still completes it, and each individual sum c,d is unchanged. This is stronger than merely preserving the signature norm.

For a profile p, define (Rp)_r=p_((L-1-r) mod 6). If p,q are the C,D profiles, then

    p'=(p+q+Rp-Rq)/2,
    q'=(p+q-Rp+Rq)/2.                         (1)

This depends only on the cell. In the symmetric/antisymmetric parts of C,D, Q swaps the antisymmetric parts and leaves the symmetric parts alone. The existing group independently changes signs of the four parts and simultaneously swaps the two symmetric and two antisymmetric parts. Q allows those swaps independently: 16×2×2=64 transformations, versus 32 previously.

The streamer allows 16 endpoint quads before profile matching. This does not invalidate the argument at n=44: every listed signed c,d has c+d divisible by four; every interior positive-product quad has entry sum divisible by four. An emitted candidate with the required totals therefore also has a positive-product endpoint quad. Do not assume this for arbitrary unfinished stream branches.

**Implementation specification.** Inputs: the existing full real-cell list for one signature and its profile keys. Generate the 64 profile transformations, equivalently the existing H and H composed with Q. Use exact arithmetic; never round a half-integer image of a profile that has no binary realization. Choose the minimum key **among real cells in that orbit**, exactly as the present canonicalization does. Keep precisely that cell. Use a new search namespace/checkpoint signature because ordering and ownership change.

Hierarchy:

1. At cell level use the entire 64-element group jointly.
2. Within a retained cell, only its stabilizer is available for an additional candidate canonicalization. If desired, keep the lexicographically least (C,D) bitstring among stabilizer images, with one fixed bit order. This is optional; previous stabilizer gains were small.
3. Do not independently pin C[0], D[0], or the first 4/5 quad. Such a rule is legal only if it is part of that same stabilizer comparison. The cell choice has already used transformations that move cells.
4. Leave A,B normalization unchanged: Q and H preserve the entire C,D target and do not act on A,B.

**Confidence.** High, algebraic. The realized speedup is unknown. Group order gives an upper bound, not a promise that half the cells disappear.

**Verification.** For each of the six known solutions, enumerate its actual C,D orbit and verify (a) binary entries and unchanged individual sums under Q; (b) every pair NPAF unchanged; (c) formula (1) agrees with profiles computed from transformed sequences; (d) at least one actual orbit member lies in the retained real cell and passes any candidate rule. Its original A,B is a completion witness. Check the retained witness, not whether the original bitstring survives. Include fixed points Q(C,D)=(C,D). No full cluster re-find is needed to establish this identity.

**Effect and preregistered rule.** First count old versus new retained cells in all twelve classes, without streaming them. Proceed to a small paired search pilot if the reduction is at least 20% in a class of interest; otherwise defer. Correctness requires all six orbit witnesses and exact identity checks to pass. Promotion requires at least 15% lower CPU for covering corresponding uncapped orbit sets, with setup included. Under front-only caps, also report changes in sampled candidate identities: canonicalization preserves exhaustive reachability, not the contents of a particular 500k prefix.

## 2. A cheap construction test: append one sign to every sequence

**Claim.** There is an exact, small test for extending any given BS(n+1,n) to BS(n+2,n+1) while retaining the old entries. It is not ruled out merely by NS(44)=NNS(44)=empty. I have not proved it succeeds, nor that it escapes every possible classification of special families.

**Proof.** For a known BS(44,43), append signs e_A,e_B,e_C,e_D. The old correlations already cancel. The new ones cancel if and only if

    e_A A_0 + e_B B_0 = 0,
    e_A A_j + e_B B_j + e_C C_(j-1) + e_D D_(j-1) = 0
                                                     for j=1,...,43.       (2)

Equivalently, as a polynomial identity,

    e_A A(z)+e_B B(z)+z[e_C C(z)+e_D D(z)] = 0.       (3)

This follows by subtracting the old NPAF equations from the extended ones at lags 1,...,44. Hence it is necessary AND sufficient for this particular construction. A very cheap preliminary rejection is

    e_A a+e_B b+e_C c+e_D d=0.                       (4)

The unalternated new n=43 solution in the brief fails (4) for every sign choice: its A,B pair gives absolute signed sums 6 or 10, and its C,D pair gives 4 or 14. This rejects that signature's ordinary sign/reversal/swap variants. Global alternation can change the signature, so it must be checked separately.

**Implementation specification.** Take the two verified n=43 seeds. Enumerate their established full equivalence orbits, deduplicate actual tuples, and check the 16 sign vectors against (4), then (2). At most 2×4096×16 checks before deduplication; no A,B search. For every success, append the signs and run the existing independent NPAF verifier. Allowing independent input reversals also covers prepending on any chosen sequences, up to reversal of the output. Do not enumerate arbitrary edits or deletions in this experiment.

**Confidence.** High for the criterion; low prior probability of a construction succeeding. This is a finite extension test, not a general recursive construction theorem.

**Verification, effect, and preregistered rule.** The six controls are seed/control data, not objects that this restricted construction must all extend. Verify their orbit transformations retain their original BS identities. For the n=41 and n=42 controls, compare (2) against direct NPAF evaluation of all attempted extensions; both decisions must agree, whether successful or unsuccessful. Pass only if an n=44 output independently verifies. Otherwise close precisely the tested extension family for these seeds. Do not conclude that all recursive constructions fail. The potential payoff is the target itself, at negligible search effort.

## 3. A,B exchange is available in the four equal-a,b classes; alternation needs a different hierarchy

**Claim.** A,B exchange gives up to a further factor of two in completion for (3,3,4,12), (5,5,8,8), (7,7,4,8), and (9,9,0,4). Global alternation cannot simply be appended to the present fixed-signature C,D cell canonicalization.

**Proof/specification for A,B.** With a=b, swapping A,B preserves the target, accepted signed sums, and the existing identical per-sequence root/reversal rules. Require A≤lex B after those rules. During DFS reject when the earliest lexicographic difference is already determined and has A>B; all preceding indices must be assigned and equal. Do not compare a high-index placed entry while an earlier unassigned entry remains. At a leaf, compare the complete arrays. The compatible profile-row set must be closed under exchanging its two profiles.

**Confidence.** High. Verify closure of the actual row set and retention of at least one normalized A,B orbit witness for each relevant known solution. Controls with a≠b must follow the unchanged path. This is a symmetry quotient, not a necessary property of every raw representative.

**Effect/rule.** At most 2× for the affected completion work, not for the entire fleet. Pass correctness only with witness retention and exact target preservation; promote if a paired frozen-candidate benchmark in an affected class saves at least 15% CPU with no reduction in resolved candidates at the same cap. Its fleet priority is conditional on the pending completion-share telemetry.

**Alternation warning and sound specification.** Let E X_i=(-1)^i X_i. Then N_EX(s)=(-1)^s N_X(s). All FOUR sequences must be alternated. At modulus six the profile map is p_r→(-1)^r p_r, but the new ordinary sums are the old alternating sums. For A,B those are not determined by the C,D cell. They depend on the compatible A,B profile row. Alternation can move a solution between the twelve signature classes.

A sound full-group hierarchy would use an augmented state consisting of **all four mod-6 profiles and its signature**, across all twelve classes. The reversal residue map is 44-r for A,B and 43-r for C,D; signs, within-pair swaps, (1), and alternation have their explicit profile actions. Choose the minimum real augmented state under the full group. Within that state use only its stabilizer to canonicalize actual tuples. Drop a C,D cell only if none of its compatible A,B rows is retained.

Crucially, this redesign spends A,B sign/reversal freedom at the profile level. The present independent A[0]=B[0]=+1 and reversal rules must be replaced by the joint stabilizer rule, not layered on top. All six controls must retain an actual full-tuple witness through BOTH levels. A simpler safe use of full-group canonicalization is after finding a complete solution, which saves no search work.

**Expected effect/rule for alternation.** No cheap extra 2× is established. Do not implement this redesign now. Reconsider only if an offline augmented-state orbit count predicts gains exceeding the loss of current A,B normalization and the engineering cost. The quad extension in item 1 obtains the clean additional factor without this complication.

## 4. Whole-cell top-K: test the selection premise before building a heap

**Claim.** Whole-cell top-K optimizes the flatness objective. It does not, without a statistical assumption and a cost comparison, optimize discoveries per node-day.

**Argument.** Let s(x)=Σ|N_C+N_D| and p(x) be the probability of finding a completion under the chosen cap. If candidates have equal costs and p(x) is a nonincreasing function only of s(x), choosing the K smallest scores maximizes expected successes over a fixed pool. Neither assumption has been demonstrated. Completion difficulty can depend on more than the score. Also a globally low score percentile is not an absolute rank: a genuine solution at the 1% quantile of a 100-million-candidate cell has roughly a million better-scoring candidates and misses K=50k. An arbitrary prefix can accidentally protect it from that competition.

For small success probabilities, the relevant comparison is

    (Σ selected p(x)) / (stream+score+selection+completion time),

with unique candidates, consistent caps, and all costs included. Global top-K can lose this comparison even when its average score is lower.

**Cheap exact specification.** Fix K=50k, B=500k, repaired baseline canonicalization, the score, and a deterministic candidate-identity tie-break before inspecting results. For each of the six known solutions, locate its eligible C,D representatives and compare whole-cell inclusion with forward-prefix, reverse-prefix, and the already specified two-buffer policy. During streaming, count candidates strictly preceding the known representative under the chosen (score, identity) key. No heap and no completion are needed to determine its rank. Once this count reaches K, whole-cell top-K exclusion is proved and that rank measurement can stop. Inclusion requires exhausting the cell; a time-capped unfinished measurement is **unknown**, never a pass. Also record strict-score rank and the size of the score tie, since identity tie-breaking is arbitrary.

Use the same eligible-candidate definition throughout; distinct representatives of one solution are not independent successes. A newly discovered completion sharing the same C,D is useful, but this static test measures only retention of known witnesses. Do not simultaneously change to item 1's canonicalization, which would confound the comparison.

**Confidence.** High for the selection argument and rank test; low for a transfer claim from six selected historical successes to n=44.

**Effect/rule.** This policy intentionally omits solutions, so “never rejects the six” is not a theorem. Preregister a conservative pilot gate: no loss of any known witness captured by the chosen baseline, at least one additional known witness captured, and at least 20% improvement in captured-witness count per measured policy cost. If exact ranks or full-cell costs are unavailable, record inconclusive and retain the current policy. Passing licenses a small n=44 pilot, not replacement of the fleet or an estimated discovery probability. The cost gate is conditional on measurements, including the pending telemetry; do not assume full cells stream cheaply.

## 5. Arithmetic/spectral elimination audit: no new condition found

**Claim.** I cannot justify deleting any of the twelve classes using the requested arithmetic directions. In particular, reducing a known exact identity modulo another integer is not automatically a new obstruction.

**Proof sketches and limits.**

- The sum-of-squares identity is already exact in all twelve classes. Reducing it modulo 2^k, 5, 9, or 11 cannot distinguish them. Stronger constraints would need the actual distribution of entries or correlated residue sums, not just (a,b,c,d).
- Pad C,D to the same length as A,B. At a common cyclic length M at least as large as every sequence, a periodic correlation is the sum of the corresponding aperiodic terms at s and M-s. Thus the full BS identity implies the periodic one. For smaller moduli, the quotient-polynomial identity is precisely a compression/residue identity. It can be a useful relaxation computationally, but this argument does not supply a new condition beyond that family.
- At z=i, write X(i)=x_X+i y_X. For A,B of length 45, x is odd and y even; for C,D of length 44, both are even. The necessary equation is Σ(x_X²+y_X²)=178. It contains an integer representability restriction beyond a sampled inequality, but is a mod-4 residue/compression test, not a new mathematical family. At the relaxed scalar level there is no class obstruction: c,d are both 0 mod 4 or both 2 mod 4, with possible minimal combined i-energy 0 or 8 respectively, and the remaining energies admit 178=13²+3² or 170=13²+1². This is not a claim that these choices jointly realize each mod-6 cell.
- Root-of-unity evaluations of orders 5, 9, and 11 could add bounded residue-profile feasibility checks. Without proving a conflict with the existing mod-6 witnesses or measuring substantial extra rejection, proposing them would just rename the already weak compression direction. Separately feasible witnesses at two moduli also need not be one common binary sequence; coupling them is a larger problem, not a free class theorem.
- Exact nonnegativity at every angle is stronger than checking 200 angles. I found no inexpensive certified between-grid obstruction with demonstrated value here. Passing the grid is not a proof of continuous spectral feasibility; final exact NPAF verification remains decisive.

**Confidence.** High in these limitations; no claim that stronger arithmetic conditions do not exist.

**Implementation, verification, effect, rule.** None recommended from this item. Do not change a class's status or implement another modular filter on this review's authority. Reopen only with a specific inequality/certificate that is not merely the existing identity in another notation, retains a representative of all six controls, and either proves a class empty or rejects a preregistered ≥20% of currently eligible sampled candidates at <5% added streaming cost. These are research-entry criteria, not reported results.

## 6. Standard product constructions: a useful obstruction, not a classification theorem

**Claim/proof.** Several simple construction routes fail arithmetically:

1. A construction demanding a binary Golay pair of length 44 cannot work. Its sums would satisfy u²+v²=88. Both sums are even, giving (u/2)²+(v/2)²=22. Modulo 11, where -1 is not a square, both halves must be multiples of 11, contradicting 22.
2. For base-sequence polynomials the norm constant is 2(m+n), here 178. An **unscaled** quaternion/four-square product of two nonempty BS objects multiplies two even norm constants and is divisible by four, so cannot give 178.
3. Any construction whose specified parameter rule multiplies normalized orders m+n would require a nontrivial factorization of 89. None exists. Likewise a Golay multiplier with rule (m+n)_out=g(m+n)_seed has no factorization with both factors greater than one.

These conclusions apply to the displayed parameter rules. They do not cover every construction with division, truncation, mixed lengths, addition, or cancellation. The trusted NS/NNS emptiness closes those families only. The supplied evidence does not support “none of the standard constructions apply” as an unrestricted theorem.

**Confidence.** High for the arithmetic; deliberately incomplete as a survey of construction theory.

**Specification/verification/effect/rule.** Before implementing a named product, substitute its exact norm and length formulas into target (45,45,44,44). Reject the route if it hits one of the contradictions above; otherwise require actual admissible seed parameters. This is a check of a proposed construction, not a filter that may reject the six known BS objects. No solver experiment is warranted without those seed parameters. Item 2 is the concrete low-effort construction experiment left by this review.

## 7. Mathematical and evidentiary corrections to the supplied documents

**Claim/evidence.** The external brief should not be treated as fully current mathematical evidence:

- “Full 1,024-element group” is incomplete. The full group under discussion has order 4,096. The ledger's unequal values of Σ|N_C+N_D| are sufficient for inequivalence: ordinary C,D operations and Q preserve the pair NPAF, alternation changes its signs by lag, and A,B operations leave it alone. Equal values would be inconclusive.
- “Five known solutions (ours 41/42/43 + Wang–Zhu 41/42/43)” counts six. Control accounting should use six named identities, not a mixture of five and six.
- The claimed in-buffer percentile evidence and rank 1429 were corrected in the ledger. They do not establish that F captures two of three solutions. They must not be reused as a statistical premise for whole-cell top-K.
- Positive compatible A,B profile lists establish satisfaction of a necessary relaxation, not existence of a binary completion. In particular “the filters PROVE a compatible profile exists” must not be shortened to “prove a completion exists.”
- Three successful completion depths do not make the 2e6 cap mathematically safe. A cap is a resource policy that can discard solutions. Disjoint ownership and complete coverage of twelve class ranges do not make a front-only capped pass exhaustive.
- In Claude's review, short whole-cell streaming time alone does not establish that whole-cell top-K “dominates” FR/F2. Item 4 supplies the missing selection assumption. Reverse and forward prefixes are deterministic views, not statistically independent samples.

**Confidence.** High. The already fixed endpoint and ownership defects, and the now-passed early-check controls, are not reopened.

**Specification, verification, effect, rule.** Claude can update those specific sentences and maintain a six-identity control table. No search behavior changes. Pass when every capture/equivalence/exhaustion claim states what was actually established; do not require new cluster jobs for these wording corrections.

**Recommended next mathematical action:** count the 64-element C,D cell orbits using (1). The construction check (2) is a bounded side experiment. Keep the existing twelve-class search intact; this review supplies no mathematical reason to abandon a class.
