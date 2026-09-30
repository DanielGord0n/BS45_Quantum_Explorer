# Completer deep dive — 2026-09-30

Reasoning only. Read Claude's September 26 response and the permitted completer source; accept the September 30 STATE as current. No code edits, execution, builds, tests, or deployment. This file is written incrementally, A then B then C then D.

## A. Exact completion improvements

### Common notation, ranking and acceptance discipline

Use zero-based positions, L=45, h=22 and target T_s=−N_C(s)−N_D(s). At entry to recursion depth d, the assigned positions are `[0,d)` and `[L−d,L)`. Write R_s=T_s−Dab[s]. A trial at depth d places positions d and L−1−d; afterward p=d+1 mirror pairs are assigned. The remaining middle is position h. Except for the already assigned root, every quad `(aL,bL,aR,bR)` has product +1.

Ranked pilot order: **A1 two-lag lookup, then its three-lag extension; A2 exact profile-row reachability; A3 stronger scalar correlation bounds.** A3 is the new idea, but its extra per-node work makes its value less certain. The already built middle-sum check can be timed independently; it is not a new proposal here. None is a forecast of a measured speedup.

With completion fraction 0.85, a fractional completion-time saving f saves **0.85f of worker time**, giving throughput multiplier `1/(1−0.85f)` if the attempted workload stays fixed. Thus 10%, 20%, and 30% completion savings correspond to 8.5%, 17%, and 25.5% worker savings, or approximately 1.093×, 1.205×, and 1.342× throughput. Fixed-cap search can visit additional branches after pruning; measure its actual cost and resolved/aborted outcomes separately rather than infer these gains from node counts.

For all three changes, preserve quad order, middle-sign order, and existing normalization. Their mathematical purpose is to remove branches containing **no accepted completion**, not to select different solutions. Without caps, the first accepted A,B and the complete solution set must be unchanged. With the same quad-trial cap and no new charged pseudo-nodes: an old hit must still be the same hit; an old exhausted-no must remain exhausted-no; an old abort may remain aborted, resolve-no, or become a hit. Node counts may decrease. A reduction in total nodes alone is not a speed measurement. Bind any changed finite-budget attempt semantics to the attempt-policy version.

### A1. Exact two/three-outer-lag reachable tuples

**Claim.** A small table gives every possible tuple of the next k outer correlations, and an eight-bit mask of current quads that could extend to such a tuple, for k=2 or 3. This is exact for those projected equations, not for the full completion problem.

**Derivation.** At entry to depth d, choose k with

    k <= d,       d+k <= floor(L/2).

The next k unknown quads have positions `d+r, L−1−d−r`, r=0,…,k−1. Let their signs be `v_r=(aL_r,bL_r,aR_r,bR_r)`. For j=0,…,k−1, put

    s_j = L−1−d−j,
    R_j = T_(s_j)−Dab[s_j].

The unknown interval has length L−2d, while `s_j >= L−d−k >= L−2d`. Consequently no edge at any selected lag joins two unknown positions. An unknown quad r contributes to lag s_j only when r≤j; its other endpoints are known positions j−r and L−1−(j−r). Therefore its total missing contribution is exactly

    F_j(v_0,...,v_j) = sum_(r=0)^j {
        A[j−r] aR_r + B[j−r] bR_r
      + A[L−1−(j−r)] aL_r + B[L−1−(j−r)] bL_r }.

No later inner quad or middle sign can alter these lags. Enumerate all `8^k` positive-quad choices and retain their F tuples. A target tuple outside this set has no completion. For each tuple, retain the union of first-quad indices v_0 occurring in its enumerations. Removing a first-quad choice outside that mask is also sound.

This adds a genuinely joint condition. For example, with normalized root ends `(A_0,B_0,A_(L−1),B_(L−1))=(1,1,1,−1)` and the next assigned outer quad all +1, R_0=2 forces the current quad's contribution to the following lag to be 0 or +4. The next unknown quad adds ±2. Thus `(R_0,R_1)=(2,−6)` is impossible although the separate unresolved-term intervals permit it. This demonstrates strictness of the local relaxation; it does not assert how often that tuple occurs in production.

**Exact implementation.**

- At L=45 use k=3 for d=3,…,19 and k=2 for d=2,…,20. Outside these ranges fall back to the existing check. The formula and guards also apply to other L.
- A table depends only on the signs of the first k assigned outer quads, not d, C,D, or the profile row. Under root normalization there are `2·8^(k−1)` boundary patterns. Encode the root by A[last]=±1 and each later boundary quad by its existing positive-table index. If root normalization is disabled, fall back or build the larger root-key family; never reuse a normalized table with an incompatible root.
- F_j is even and `|F_j|<=4(j+1)`. Use an exact integer index for `F_j/2` in `[-2(j+1),2(j+1)]`. Odd or out-of-range residuals have an empty mask. For k=3 a direct grid has `5·9·13=585` bytes per boundary pattern; 128 patterns need **74,880 bytes** of eight-bit masks. k=2 needs `16·5·9=720 bytes`. Populate by enumerating the actual eight positive quads in their existing index order. This is small enough to precompute once per process, outside candidate searches.
- At eligible DFS entry, read k residuals, construct the boundary key, and fetch the mask. An empty mask returns false. Otherwise iterate the existing quad order, skipping bits absent from the mask, then apply the existing reversal rules, charge convention, early check, placement and other bounds. Retain the early check initially as a consistency check.
- Ignore future sum/profile/reversal restrictions while making the table. That produces a safe superset. Applying restrictions belonging to a different DFS state during table construction would not be safe.

**Cost / limits.** Lookup costs O(k) array reads/arithmetic and one byte-mask access; no per-row loop. Table generation enumerates only 128×512 assignments for k=3, each with three short sums, once. It can reject outer-lag-inconsistent nodes or current choices before placement; it cannot see low-lag, profile or deep inner conflicts. The final depths outside its guard receive no benefit. A single-lag ±2 feasibility check is often redundant with existing bounds and parity; do not present that as the gain.

**Confidence and verification.** High, algebraic. Independently enumerate all boundary patterns and next-k quads and compare both reachable tuples and first-quad masks against direct correlation contributions. Compare every allowed completion in the n=6/8/10 corpus and all six known witnesses under combined pruning. **The k=3 depth guard never activates for n≤10**: those controls alone cannot test its implementation. Add finite synthetic L=13,d=3 table-identity fixtures, or equivalent direct contribution tests; no exhaustive n=12 search is needed. Require the common uncapped/capped identity relations above.

**Preregistered rule / expected effect.** Compare k=0,2,3 on the same frozen candidate sample, stratified by class, rank and baseline abort outcome. Record completion CPU, attempts removed before placement, resolved/aborted counts and table overhead. Promote the best variant only if completion CPU falls at least 15% and total worker CPU at least 10%, with no old hit lost and no fall in resolved count at the same cap. Close either extension if its extra lookup does not pay. A 15% completion saving would nominally save 12.75% of worker time; actual fixed-cap savings remain to be measured.

### A2. Exact residual reachability of an A,B profile row

**Claim.** After the exceptional root quad, profile reachability can be decided exactly for the remaining positive quads and free middle, before imposing correlations or reversal normalization. This strictly strengthens separate per-residue capacity bounds. Use an existential test over entire rows, never independent witnesses for different blocks.

**Distinct residue pairs: full derivation.** For modulus six, reflection is `r -> (44−r) mod 6`. The distinct pairs are {0,2} and {3,5}. For one pair {r,s}, let q be the number of remaining mirror quads and

    Delta = (rowA[r]−PA[r], rowB[r]−PB[r],
             rowA[s]−PA[s], rowB[s]−PB[s]).

In this coordinate order the eight positive-product sign vectors are the columns of H and their negatives, where

    H = [ 1  1  1  1
          1 −1  1 −1
          1  1 −1 −1
          1 −1 −1  1 ],        H^T H = 4I.

Thus a remaining quad contributes a signed unit vector to `t=H^T Delta/4`. Necessity is immediate:

    every component of t is integral,
    ||t||_1 <= q,
    q−||t||_1 is even.                           (A2.1)

For sufficiency, allocate |t_j| quads to the required sign of column j, then fill the remaining even number with opposite column pairs. This uses exactly q quads and gives Delta. Orientation of a physical pair relative to r,s does not matter because its eight allowed assignments are the same full palette after swapping the two positions.

**Self-reflected residues: full derivation.** Residues 1 and 4 reflect to themselves. One positive quad contributes either `(±2,±2)` or `(0,0)` to the A,B residue sums. If u,v are the residual sums and there is no middle, its q-quad criterion is

    u,v even;
    max(|u/2|,|v/2|) <= q;
    u/2 and v/2 have the same parity.            (A2.2)

Indeed, if a quads are nonzero, the two halved sums are independent sums of a signs: their magnitudes are at most a and both have parity a. Conversely choose `a=max(|u/2|,|v/2|)`. The parity condition makes this a valid sign-count length for both components; a≤q, and the other quads can contribute zero. **There is no requirement that a and q have the same parity.** Zero quads consume a single quad, unlike the opposite-pair padding in (A2.1).

The unassigned middle at position 22 lies in residue 4 and contributes any `(epsilon_A,epsilon_B)` in {−1,+1}². Accept residue 4 iff (A2.2) holds for `(u−epsilon_A,v−epsilon_B)` for at least one of these four choices. At q=0 this requires exactly one allowable middle pair. Do not demand even residuals before subtracting the middle.

Because the four residue blocks involve disjoint remaining quads, their tests jointly are necessary and sufficient for **that row's** unrestricted positive-quad/middle realization. They ignore correlations and remaining reversal comparisons, so passing is not proof of a completion.

**Exact implementation and cost.** Call inside `fh_abp_filter(d)` after positions d,L−1−d have been placed, so p=d+1 and the remaining mirror indices are `p,...,21`. Precompute four counts per p by those indices; never include the negative root quad in (A2.1). Apply only to L=45,m=6 initially, falling back elsewhere unless the generalized residue decomposition is separately implemented. Run current cheap capacity checks first, then (A2.1) for both distinct pairs, (A2.2) at residue 1, and the four-choice middle test at residue 4. Keep a row only when all blocks pass; reject the node only if no row survives.

For reference, before any placement there are 8 mirror quads in {0,2}, 7 in {3,5}, 4 at residue 1 and 3 at residue 4, plus the middle. After root placement {0,2} has 7. These are useful indexing checks, not constants to reuse at every depth.

Each tested row requires two four-coordinate Hadamard transforms, absolute sums/parities, one self-residue test and at most four middle alternatives: O(1) extra work per surviving row, hence O(R) per node for R rows tested. Memory is O(h) counts and existing survivor stacks; no residual-state hash table is needed. Optional precomputed row transforms reduce repeated additions but are not necessary for the first pilot. A per-row mask of permitted middle signs may be carried to the leaf, but do not merge it across incompatible rows and then treat it as a row witness.

**Confidence / verification.** High. Exhaustively enumerate sums of small numbers of positive quads, including q=0 and the middle, and compare with these formulas. Test same-parity versus opposite-parity halved residuals and cases with odd numbers of zero quads. On the six solutions inspect the actual residual row along its normalized completion; at least that row must survive. In n=6/8/10 tests, either implement the appropriate L-dependent reflection/center mapping or retain the old path; do not apply the n=44 residue labels blindly. Independently exercise the production four-block helper on finite synthetic cases, so falling back at small n cannot hide its bugs. Follow the common identity requirements.

**Preregistered rule / expected effect.** Measure rows tested per node, rows additionally removed, **last-row removals**, depth and total completion CPU. Removing many rows but no final witness is not a subtree-saving claim. Promote only for ≥15% lower completion CPU and ≥10% worker CPU on the matched fixed-cap sample, with no loss of old hits or resolved count. Otherwise close or retain only a prespecified late-depth band if a separate bounded comparison passes. The observed late-node concentration warrants a pilot; it does not eliminate the risk that row scanning costs more than it saves.

### A3. Third idea: combine known-neighbor coefficients before bounding unknown quads

**Claim.** `Kab` treats every unresolved signed product independently. A stronger bound follows by collecting all known-neighbor coefficients of each unknown sign, then maximizing over legal positive quads. It is particularly cheap for a few late-depth lags with no unknown–unknown edges. This is a correlation-domain bound, not incremental PSD.

**Full derivation.** After p≥1 pairs are assigned, let U=[p,L−1−p] be the unknown interval, of length r=L−2p. For a lag s and unknown position i, define

    alpha_A(i,s) = sum of A[j] over known in-range j=i−s or i+s,
    alpha_B(i,s) = the analogous sum for B.

Split the residual at s into known–unknown edges and unknown–unknown edges:

    R_s = sum_(i in U) [alpha_A(i,s) A_i + alpha_B(i,s) B_i]
          + unknown–unknown contribution.                (A3.1)

This counts each crossing edge once. Opposite known neighbors may cancel in alpha, information discarded by Kab. For one remaining mirror quad (i,j), set

    alpha = (alpha_A(i,s), alpha_B(i,s), alpha_A(j,s), alpha_B(j,s)).

Its maximum linear contribution is

    M(alpha) = max_(t=0,...,3) |h_t dot alpha|,            (A3.2)

because its allowed assignments are exactly ±h_t. Its minimum is −M. The middle contributes at most `|alpha_A(h,s)|+|alpha_B(h,s)|`, with negative minimum. There are exactly `E=max(r−s,0)` unknown–unknown position pairs at lag s, each carrying two signed products. Hence every completion satisfies

    |R_s| <= sum_remaining_quads M(alpha)
             + middle_coefficient_bound + 2E.            (A3.3)

This is never weaker than Kab: coefficient cancellation cannot increase the sum of individual edge magnitudes, restricting a quad cannot increase its independent-sign maximum, and 2E is the original bound on the remaining internal edges. It can be strictly stronger: for alpha=(1,1,1,−1), independent signs give maximum 4, whereas positive-product quads give M=2.

If s≥r, E=0. All remaining groups are independent for this one linear equation, so the positive and negative extrema in (A3.3) are **exactly attainable in the unconstrained quad/middle domain**. Values between the extrema may have gaps; interval passage is still only necessary. Ignoring profile, sums and reversal constraints enlarges the domain and preserves soundness. For s<r, the 2E term remains a relaxation; do not claim exact extrema there.

**Exact implementation and cost.** First pilot only when at most four mirror quads remain, and only two unresolved lags with s≥r. During the existing cheap Kab scan, identify up to two such lags with smallest slack `Kab[s]−|R_s|`, excluding already fixed lags `s>=L−p`. Optionally exclude the top lags covered by A1 to focus on independent benefit. Form coefficients directly from the assigned arrays, compute four dot products per remaining quad, add the middle bound, and reject if (A3.3) fails. Otherwise run the existing profile filter. Keep all arithmetic integral; zeros in unassigned positions must not be treated as known signs.

This costs O(q) per selected lag for q≤4 remaining quads, O(1) extra memory, and no candidate-wide precomputation. The coefficients can be ±2, ±1 or 0. Start with the exact high-lag version; do not instrument all 44 lags at every depth. Its purpose is to couple all contributions involving the same unknown variable/quad, something Kab does not do.

**Confidence / verification.** High in necessity, uncertain frequency of useful cuts. At small-n partial assignments, literally enumerate legal remaining quads/middle and compare the extrema at s≥r with (A3.3); verify inclusion for s<r if later enabled. Test cancellation, root orientations, q=0 and middle-only states. Require the six normalized witnesses and exhaustive small-n completions to survive. Keep the production depth restriction separate from helper tests so small-n controls actually exercise the formula.

**Preregistered rule / expected effect.** Pilot only if counters show substantial late nodes surviving the existing early/Kab checks; charged late trials rejected before placement are not its opportunity. Compare against the winning A1/A2 configuration, not just an obsolete baseline. Require ≥10% additional completion CPU saving and ≥7% worker saving, including coefficient construction, with the common identity and resolution gates. Otherwise close it. A 10% completion saving alone nominally saves 8.5% worker time; a new bound with many extra comparisons can instead slow the search.

### A verification is a proof obligation plus tests, not just six successful re-finds

The proofs specify safe relaxations. Tests must establish that code computes those relaxations and attaches them to the right depth/state. Use exact helper truth tables, normalized six-solution witness paths, the exhaustive n=6/8/10 solution corpus, and frozen production candidates for cost. Preserve actual C,D identities and first accepted A,B, not merely aggregate hit counts. Exhaustive small-n fallback paths alone do not validate n=44-specific code. Compare each lever separately before combining them; record cuts and costs under the final combined configuration. No solution-preserving prune can certify an abort as a mathematical nonexistence result.

## B. The abort tail: measure work, not supposed solution scarcity

**Claim.** No new cheap certificate of “this candidate certainly aborts” was found. Abort is a property of a particular deterministic search, target signature, row set, ordering, charging rule and cap. It is not a nonexistence property of C,D. High abort rates justify investigating wasted work and censored outcomes; they do not by themselves justify rejecting candidates, raising a class budget, or lowering it.

**Sketch.** A mathematical contradiction can prove no completion, but difficult-to-refute candidates can be either satisfiable or unsatisfiable. An exact predictor that this unchanged implementation will reach its cap before a hit could preserve the hit set of that finite policy, but would still not prove nonexistence. Establishing such a prediction would require a lower bound on actual search work before the first hit; none follows here from score, row count or correlation magnitude. A statistical predictor of abortion can be wrong precisely on a valuable candidate. Using it to omit work is a new selection policy, not a sound prune.

The outer-tuple tables may quickly certify incompatibility, but the baseline would usually reject such shallow contradictions cheaply too. Their main potential is repeated subtree elimination throughout a deep search, not identifying the entire abort tail at the root. Similarly, an empty exact row-reachability set proves impossibility; a large nonempty set does not imply a solution or an inevitable abort.

### Minimal measurement specification

For a fixed reproducible sample, record candidate identity and attempt-policy version, signature, stream/rank band, flat score, the initial compatible-row count, and whether the profile constraint was disabled because of its cap. Initial row count is often cell-level information shared by many candidates; comparisons must not treat them as independent evidence about different cells.

Use sampled counters by depth for: trials charged, trials surviving the early check, placements, survivors of Kab, rows scanned, last-row removals, and survivors entering the next level. Record both the quad-budget counter and total nodes, wall/CPU time, and final outcome `{hit, exhausted-no, budget-abort}`. A few outer-target entries or a target-vector hash permit grouping similar residual patterns without a new expensive feature computation. Do not label the 96.8% late-node share as expensive placement work until the early-check survivor counts establish that.

Aggregate by class and by cell/rank strata. In particular compare the hot (1,7,8,8) class with (5,5,8,8) at matched rank/cell strata, rather than assuming their different abort rates imply different solution densities. Candidate rows disabled by an implementation cap deserve a separate stratum. Require instrumentation overhead below 1% of worker time; otherwise sample more sparsely. Candidate-level timing and counters are preferable to an elaborate shadow-tree instrument.

Cheap lower-budget counterfactuals can be obtained from the same deterministic run: record its termination/hit budget counter, or that it exceeds each selected smaller threshold. A terminated result below a threshold would remain that result; a run needing more charged quad trials would be censored there. Respect the existing middle-trial convention. Higher-budget outcomes cannot be inferred from counters at 2e6: they require a bounded continuation/restart experiment on a prespecified subset of aborts. A cold restart costs the whole new run, not only the budget increment.

### Budget decision rule

For a stratum z, let T_z(b) be measured CPU per selected candidate including allocated generation/replay cost, and p_z(b) its unknown probability of a successful completion within budget b. Prefer b' exactly when

    p_z(b')/p_z(b) > T_z(b')/T_z(b).

For an extra replay of a selected old-abort group, let q be its conditional hit probability and R its incremental reconstruction/restart cost. Prefer replay over fresh work only when `q/R > p_new/T_new`. Neither q nor p is given by the abort rate.

Under the explicit, unverified surrogate that resolved candidates in both policies have equal hit probability, one can replace p ratios by `(1−r_z(b'))/(1−r_z(b))`, where r is the measured abort fraction. This allows a **conditional** resolution-efficiency decision. It is not a measured discovery-efficiency decision. At 85% completion share, a uniform fractional reduction f of completion cost would save about 0.85f of worker time, but lowering budgets may also remove successful mass; that loss must appear in p.

**Confidence / exact policy.** High in the distinction and thresholds; no confidence in a class-specific success-density ranking from current abort percentages. Keep 2e6 while pricing A1–A3. Re-measure abort rates after any promoted prune because it changes which candidates reach the cap. Do not combine a new prune and a budget change in the first comparison. No flattest-decile replay preference is established.

**Verification / preregistered pass-fail / effect.** Six-control and exhaustive small-n reachability remain mandatory for mathematical changes. For budget-only changes under identical DFS order, higher caps must retain all lower-cap hits; lower caps deliberately need not. Check candidate identity and first hit, not merely aggregate counts. Permit a bounded budget pilot only when matched measurements predict at least a 10% improvement in resolved candidates per CPU under the explicitly stated surrogate; do not promote it fleet-wide as a P(hit) improvement on that surrogate alone. Freeze the sampled strata before reading outcomes and include cold replay costs. Expected discovery effect remains unidentified; expected computation savings become measurable through these counters and paired runs.

## C. Completer re-audit

**Claim.** No unsound mathematical pruning condition was found in the inspected completer. There is one still-incomplete internal-error path, plus exact bookkeeping simplifications worth separating from new pruning. This audit does not certify caller-side initialization or literal mirror-table contents outside the inspected definitions.

### Deriving the mirror restrictions directly from the BS equations

For completeness, the mirror encoding at n=44 can be derived without assuming a representative convention. Let e_X(i)=(1−X_i)/2 modulo two. Put

    U_i=e_A(i)+e_B(i), i=0,...,44,
    V_i=e_C(i)+e_D(i), i=0,...,43,
    r_i=U_i+U_(44−i), t_i=V_i+V_(43−i),

with these sums in F_2. Expanding each signed product modulo four, the total correlation at lag s has constant term `178−4s ≡ 2 (mod 4)`. Cancellation to zero therefore requires

    sum_(i=0)^(s−1) (r_i+t_i) = 1 in F_2, s=1,...,44.

The endpoint-prefix expression also holds at s=44; the C,D part sums to zero there. Taking successive differences gives `r_0+t_0=1` and `r_i=t_i` for i=1,...,43. Reflection gives `r_i=r_(44−i)` and `t_i=t_(43−i)`. For i=1,...,42 these imply `r_i=r_(i+1)`. Since r_22=0, all r_1,...,r_43 vanish. Then t_1,...,t_43 vanish, t_0=t_43=0, and r_0=1.

Consequently the A,B endpoint quad has product −1, all interior A,B quads have product +1, and every C,D mirror quad has product +1. This is the n=44 mirror restriction used from Wang–Zhu. It is necessary for every solution, before root or reversal normalization. The middle A,B signs are unrestricted by this parity argument.

### Conditions and state bookkeeping

- **Root and reversal.** Independently negating A and B preserves their NPAFs and absolute sums, so both initial signs may be +1. For such a sequence X, the normalized reversed image is `X[last]·reverse(X)`. The implementation retains the lexicographically larger image by examining paired positions while the prefix is tied. This composes with C,D cell canonicalization because it acts on different sequences and leaves the entire C,D target unchanged. It requires the full set of both signed A,B profiles; the inspected comments state that contract. At an odd center, an unresolved anti-palindromic comparison is not enforced: that retains duplicates rather than losing a solution. Fixing it would change some first representatives for a tiny potential benefit; it is not recommended as part of the pure-prune pilots.
- **`fh_place` / undo.** A placement adds `A[q]av+B[q]bv` once for every previously assigned q and reduces Kab by two for that newly resolved position pair. The second placement counts the edge between the new endpoints exactly once. Undo clears the position and subtracts the same terms against positions still assigned. The alternative placement implementation expresses the same updates by lag. Whole-pair placement is complete before the recursive bounds are used.
- **Kab and sums.** Each unresolved edge contributes two products of magnitude one, giving the necessary interval. Remaining sequence sums lie in the interval of radius `L−2p`; accepting either signed total is essential. Their parity is already correct for a legal signature and paired placements, so another scalar sum-parity check supplies no new cut.
- **Outer check.** At depth d>0, after choosing positions d,L−1−d, all terms at lag L−1−d are known. Its new terms are exactly the four endpoint products in the code. Longer lags were already determined. This is an equality, not a heuristic bound. At the root, the negative-product quad ensures `A_0 A_44+B_0 B_44=0=T_44`.
- **Profile constraint.** Reduce the full polynomial norm identity modulo `z^6−1`: the coefficient at zero supplies the combined squared-profile norm 178, and other coefficients give cyclic residue-correlation cancellation. Actual A,B profiles provide a joint witness. The mirror restrictions give the additional mod-four tests. At n=44 the exceptional reflected A,B pair is {0,2}, with combined sum 2 mod 4; the other distinct pair has sum 0. `thm212_ok` handles that exception separately. Its self-reflected tests are weaker than A2, but necessary. Given a complete allowed-row set, a real completion's row survives `|row−partial|<=remaining_count` and equals the partial at the final leaf. Different blocks must use the same row.
- **Stack and middle.** `fh_abp_filter(d)` writes stack level d+1 after the current quad. After all h quads, the leaf reads level h; the middle signs have already been added to the partial profiles. The stack is not changed between alternative middle signs, which is correct because it describes the parent possibilities. `MID_SOLVE` rejects only middle pairs that fail the final sum check and preserves their ordering. Middle trials increment total nodes but not the cap counter; this is an existing policy convention, not the literal same cap on total nodes.
- **Candidate initialization and prefilters.** Resetting placed arrays, partial sums and current counters handles FOUND paths that return without undo. The caller must still supply the correct row pointer and root survivor list; that caller region was not reread here. For BS(45,44), the tail-prefilter loop starts at 45 and ends at 44, so is empty. The range prefilter also cannot reject a binary length-44 C,D: its target obeys `|T_s|<=2(44−s)`, already below the A,B capacity `2(45−s)`. Do not count these as substantive candidate rejection at n=44.

### C1. Remaining internal-error gap

The current `fh_complete_ab` prints `FH_INTERNAL_ERROR` if DFS reports FOUND but the final NPAF check fails, **then still returns 2**, its ordinary exhausted-no code. Printing an error is an improvement, but the function itself still conflates the outcomes. An external log reader may catch it; that was not inspected. This is a defensive error-path issue, not evidence that it occurs in valid runs.

**Specification.** Propagate a fatal/internal-error outcome through the caller before it can mark the candidate attempted/completed or advance its checkpoint. Do not merely invent an integer return code without updating the caller's interpretation. Preserve diagnostic candidate/state information and refuse to treat the affected unit as normally complete.

**Confidence / verification / pass-fail / effect.** High from the displayed return path. Inject a final-verifier disagreement in a bounded fixture: it must fail the unit and leave the attempt unadvanced, not emit ordinary clean-no or HIT. Normal six-control and small-n outcomes must remain unchanged. Any conflation fails the gate. This protects correctness reporting; no performance gain is claimed.

### C2. Exact depth-only Kab table and omission of already-certified lags

This is a bookkeeping optimization, not a new necessary condition. After p complete mirror pairs, the unknown interval has length r=L−2p. For lag s, let

    u = max(0, min(r, L−s−p)),
    e = max(r−s,0).

There are u edges whose lower endpoint is unknown, u whose upper endpoint is unknown, and e counted twice because both endpoints are unknown. Therefore

    Kab_p[s] = 2(2u−e).                          (C2.1)

It depends only on L,p,s, not signs or candidate. All bound checks in the inspected recursion occur after a whole quad is placed; the early check reads Dab, not Kab. A precomputed table may replace dynamic Kab additions/subtractions at these decision points. For L=45, p=0,...,22 and s=0,...,44 need about **4.1 KB** of 32-bit integers. The middle leaf checks exact correlations and needs no nonzero bound after its placement.

With the early check enabled, lags `s>=L−p` have already been certified and future inner entries cannot affect them. The ordinary Kab scan can stop at `L−p−1`. If the early check is disabled, retain an exact check of the newly determined lag instead of silently skipping it. At the root the mirror endpoint identity supplies the base case. At p=22, lags 1,...,22 remain for the middle; larger lags are fixed.

**Specification / confidence.** High. First replace only the bound source with (C2.1), then remove redundant fixed-lag scans. Avoid mixing this with a new placement algorithm or changed variable order. Keep direct Dab updates and undo unchanged. This may be worth pricing before A3 if update/scan timing shows it is hot; it was already a natural bookkeeping opportunity, not the new third mathematical idea.

**Verification / preregistered rule / effect.** For all small-n partial paired states, compare every table entry with the existing dynamic Kab. Require identical six-control and exhaustive-small-n verdicts, first hits, charged quad nodes and aborts, with early check both on and off. Skipped fixed lags must be equal in a diagnostic build. Promote only if matched completion CPU falls ≥5% and worker CPU falls ≥4%, without identity discrepancies. A 5% completion reduction nominally saves 4.25% worker time. Its actual benefit may be small when branches fail before scanning far enough to reach those lags.

## D. Theory and interpretation of the six controls

**Claim.** No structural obstruction to BS(45,44), new class elimination, or new construction was found. The twin-class phenomenon is useful computationally but supplies neither a completion nor a reason for nonexistence. No additional verified literature result beyond the existing review record is supplied here; this scoped review did not conduct a fresh literature survey.

**Sketch.** For fixed C,D, completion asks for two binary-coefficient length-45 polynomials whose combined norm is `178−|C(z)|²−|D(z)|²`. At z=1, the twin signatures (7,11) and (1,13) both contribute 170; similarly (1,7) and (5,5) both contribute 50. Equality at z=1 and coincident residue relaxations do not identify the full polynomial decompositions. A pair with one prescribed set of sums is not automatically transformable into one with the other. Keeping both completion targets is justified; assigning either a larger existence prior from that equality is not.

The primality of 45+44=89 obstructs constructions with a genuinely multiplicative normalized-order rule and two nontrivial integer factors. It is not an obstruction to existence or to constructions with different parameter rules. The previously supplied emptiness of normal/near-normal families closes those families only. I cannot turn these observations into a classification of all Turyn-type, Golay-like or T-sequence constructions, and recommend no speculative construction implementation from them.

The earlier review exchange recorded the following C,D flatness invariants; these values are inherited from that record, not recomputed in this pass:

| n | Our solution: flatness | Wang–Zhu solution: flatness |
|---|---:|---:|
| 41 | 124 | 140 |
| 42 | 150 | 142 |
| 43 | 130 | 134 |

Both numeric columns give the scalar `sum_s abs(N_C(s)+N_D(s))`. This scalar is invariant under the C,D symmetries including Q; global alternation changes lag signs but not absolute values, and A,B operations do not change it. The unequal paired values therefore certify inequivalence within each n. Equal values would not certify equivalence. Comparisons across different n do not compare members of one equivalence space.

Profiles and DFS positions are less intrinsic. The n=42 relocation to a distant Q-representative prefix, while its flatness stays unchanged, demonstrates why profile location is not a stable measure of solution quality. The six observations do not identify a preferred n=44 signature, profile band, or abort stratum. In particular the scores do not give a consistent “our hits are always flatter” ordering even relative to the three reference examples.

**Confidence / specification / verification.** High in the invariant argument and the limits of the twin-class inference; no numerical discovery forecast. Keep all twelve classes and existing balanced coverage. If using the historical invariant values in a paper or ranking experiment, Claude should confirm them from the six stored tuples; their recomputation is unnecessary for the A1–A3 derivations. New reductions would still require full-domain proofs, six-control witness retention and exhaustive n=6/8/10 checks, with production-specific helpers independently exercised.

**Preregistered rule / expected effect.** No implementation or fleet policy change from D. Reject any proposal to delete or deprioritize a class solely from these six scores, twin-class equality, or the prime normalized order. Expected worker-time gain from this theory item: none established. The concrete next mathematical leverage remains the exact local completion tests in A, priced against the now dominant 85% completion cost.
