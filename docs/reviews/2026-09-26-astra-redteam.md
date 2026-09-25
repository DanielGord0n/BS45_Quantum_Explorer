# BS(45,44) red-team review — 2026-09-26

Reasoning only, using the three requested documents and targeted source regions. No execution or changes outside this review. Reported controls are accepted as reported. Rankings below combine implementation effort with potential discovery benefit; performance thresholds are prospective.

**Verdict:** I found no new contradiction in the core mathematical filters. I cannot certify the complete loss-free execution path from the permitted evidence. The most actionable risks are truncated profile enumeration, positional resume across policy changes, and interruption handling. On performance, the new stream/completion split materially weakens the argument for K=50k: extra completions amortize substantial generation cost. No class elimination or new symmetry beyond those already discussed was found.

## 1. Highest priority: close the coverage and resume proof obligations

**Claim.** Mathematical rejection, symmetry quotienting, and resource-policy omission must be distinguished. The first two can preserve exhaustive reachability; the third deliberately does not. A successful final verifier proves a reported answer, not that previous rejections were safe.

### Rejection audit

| Point | Mathematical status and combination check |
|---|---|
| Signature classes / `enum_class_sums` | Correct necessary bounds and parity: a residue containing h signs has sum in {-h,-h+2,…,h}. The routine exhausts these vectors with the requested total. Both A,B signed totals must subsequently remain available under root normalization. |
| 2.11a, mod-3 reduction, 2.11b | Necessary: reduce the full polynomial norm identity modulo z^m−1. Its constant and cyclic-correlation coefficients give these constraints. The true complement supplies a common norm/correlation witness. `PairAutoSet` stores their joint key, rather than combining unrelated witnesses. Separate mod-3 and mod-6 witnesses only weaken a test; they do not exclude a true solution. Helper arithmetic/table initialization outside the inspected regions is not independently certified here. |
| `thm212_ok` | At n=44,m=6, the exceptional A,B residue pair is {0,2}, with total 2 mod 4; the other reflected pairs have total 0. C,D reflected pairs have total 0. This follows from the exceptional negative-product A,B endpoint quad and positive interior quads. The implementation excludes the exceptional pair from its ordinary rule. Self-reflected residues give weaker necessary tests, not false exclusions. No conflict with independent signs/reversals found. |
| Hall/PSD leaf tests | In exact arithmetic, each sequence and each pair has spectral energy ≤178 because the four nonnegative energies sum to 178. Conjunction is safe, and Q preserves the C,D pair energy. **The Hall helper implementations, numerical tolerance, and angle initialization were outside the permitted source targets: floating-point rejection is not certified.** Six passing examples do not bound roundoff at a boundary. |
| Mirror encoding / endpoint rules | The stipulated mirror theorem makes A,B's endpoint product negative and interior products positive. C,D emitted candidates at n=44 have positive endpoint product by their total sum modulo four. Exact profile capacity/parity checks are necessary. The new C,D residual reachability runs only after the endpoint assignment, so it correctly applies positive-quad reachability to the remaining quads even if the separate root optimization is off. Actual table contents were not separately read. |
| C,D orbit canonicalization / Q | Sound on the stated domain: keep a real cell in each orbit; transform the entire C,D pair, retaining the same A,B completion. C,D pins are visibly disabled under canonicalization. Q's integer division is exact on raw profiles: C and D have the same parity in every residue, making each numerator even. Q-force mode outside the production guard is not a production certificate. |
| Q-closure prune | Sound with the frozen complete list and the certified reasons. Current reported certificates are all nonrealizability, which proves no quad-positive C,D realization anywhere in that orbit. Unknown absences are visibly retained. Caveat: the code currently prints only a sample certificate per reason, not a certificate for every dead orbit; the requested audit needs a reconstructible per-orbit witness. |
| A[0]=B[0]=+1 and reversal | These act only on A,B, independently of C,D cell selection. Both signed A,B totals are visibly enumerated for profile rows. Comparing X against X[last]·reverse(X) after root normalization is sound. At an odd-length center the implementation may retain an extra reversal representative; that is duplication, not loss. |
| A,B sum / correlation bounds | The remaining-sum interval is necessary for either signed total. `Kab` is an upper bound on the absolute sum of unresolved signed products, so |target−Dab|≤Kab is necessary if its placement bookkeeping is correct. The relevant update/undo code adds each newly fixed pair once and subtracts it on undo; no algebraic inconsistency found. |
| Early outer-correlation check | Exact: after placing depth d, lag L−1−d has no unresolved terms. Its new contribution is the four displayed endpoint products. Charging before this check preserves the old counter behavior. No further longer lag becomes newly determined at that placement. |
| A,B profile rows / leaf check | Both signed targets are generated; rows retain the joint norm/cyclic tuple and 2.12. Capacity filtering preserves the actual completion's row. Leaf membership uses the correct final stack and includes the middle placement. Oversized row lists disable the constraint rather than truncate it: safe. Dropping a cell with an empty list is safe only if row construction is complete for its key. |
| `fh_complete_ab` / final verification | The correlation range prefilter is necessary. Exact final NPAF verification is appropriate. However, an unexpected failure after DFS reports FOUND currently returns ordinary `clean_no`; this should be an internal-error result, not a silently exhausted candidate. |
| Flat score, top-K, first B buffers, score threshold, wall cutoff, node budget | **Not necessary conditions.** Flat score is an exact integer statistic, but its use as a rejection/selection rule is heuristic. A budget-aborted candidate is unresolved. Advancing its checkpoint means “attempted under this policy,” not “proved impossible.” |
| Checkpoint / lane ownership | Safe only with identical candidate generation, cell ordering, policy interpretation, correct interruption state, and exactly-one ownership. The new H manifest and digest requirements address this in design. They are not implemented by the inspected old ordering/resume code, and no driver was inspected. |

### Exact changes/checks Claude should make

1. **Fail closed on profile truncation.** `survive_profiles6` warns and returns a prefix at its cap. Disabling Q-prune at the cap does not restore the omitted cells. Production must refuse to advertise a complete tile when the cap is reached; propagate an explicit completeness status and stop or use an explicitly incomplete research mode. No evidence here says the present n=44 lists hit the cap.
2. **Bind checkpoint meaning.** The inspected CFGSIG contains buffer size but not `FH_BUDGET`, and does not identify every completer-policy change. Raising the budget in an existing checkpoint directory leaves earlier low-budget aborts skipped. Add an explicit immutable attempt-policy ID, node budget, ordered-list digest and kept-set digest, or enforce an equivalent immutable manifest. A separately named tail replay may intentionally revisit old aborts; ordinary resume may not pretend they were tried under the new policy.
3. **Test the exhausted-front boundary.** A reachable saved state is `(cell, batch=B, k=0)` after the last selected buffer but before cell advancement. On resume, the inspected `drain` skips old batches, then can process the NEXT buffer in full because `cur_batch < fh_drain_batches` is false. This is a likely extra-work defect, not a demonstrated loss. Before re-streaming, recognize this state as policy-complete and advance the cell. Check stops immediately before/after the final selected completion and batch advancement.
4. **Unify interruption semantics.** `drain` stops on `g_fh_sigterm`, while cell-complete advancement tests `fh_stop`. The real signal-handler/deadline path was not inspected. Prove that interruption cannot leave the former true and the latter false at advancement, or guard advancement with both. Exercise actual SIGTERM during generation, sorting, completion and drain—not only the test hook, which visibly sets both flags. Check checkpoint writes, kills, resumes and manifest completion states.
5. **Clarify node-budget accounting.** Quad trials increment both `fh_cur` and total nodes; middle-element trials increment total nodes but not `fh_cur`. Thus “2e6 charged nodes” is not literally the implemented total-node cap. Record both counters and describe the existing budget accurately. Do not silently change this policy while comparing algorithms.
6. Replace the contradictory old sections of Pass H v2 with its “required changes”; do not leave raw-position tie-breaking and replacement-only submission instructions alongside their corrections. Require every retained index to have one manifest owner per intended direction, independently of G status. A cell-list permutation check alone remains insufficient.

**Confidence.** High in the algebra and the cited source behaviors; conditional on uninspected helpers/runtime paths. These are findings and proof obligations, not a claim that current runs have lost a particular solution.

**Verification / preregistered rule / effect.** Retain an actual representative of all six known solutions and every available n=6/8/10 solution orbit with mathematical filters combined and resource omissions disabled. Separately compare uninterrupted versus interrupted/resumed ordered candidate-attempt identities, allowing replay but no missing eligible attempt. Include the terminal-front checkpoint and config changes above. Any missing witness, unowned index, unrecognized truncation, or incorrect completion state blocks H. Hall arithmetic needs a documented conservative error bound or safe near-boundary fallback, not just fixture passage. This work protects the entire expected discovery rate; its percentage payoff cannot be estimated from the present evidence.

## 2. Policy: generation cost justifies a breadth-versus-depth hedge, not an optimality claim

**Claim / expected-value argument.** K=50k, B=1, budget 2e6 is a defensible incumbent, but the measurements do not establish that it maximizes P(hit)/node-day. Flat completion cost does not establish flat success probability; high aborts among the flattest candidates do not prove they are bad candidates. Nor does fewer cells/day at 5e6 prove lower discovery probability.

Let M(K,B,b) be expected successful discoveries per visited cell under that selection and budget, counting duplicate solution/candidate orbits only once. For rare discoveries, maximize M/T; under an independent Poisson approximation P(hit by time D)=1−exp(−D·M/T). This is an allocation model, not an identified rate.

Normalize the present one-buffer cost to one. If generation/selection cost stays fixed and marginal completion cost stays constant,

    T(K,1,2e6) ≈ 0.476 + 0.524·K/50,000.

The rank telemetry in this source divides the **eligible top K** into deciles. Constancy within the first 50k therefore does not measure costs at ranks 50k–175k; extrapolation must be checked.

| Policy | Model cost relative to baseline | Break-even success mass |
|---|---:|---:|
| K=25k, one buffer | 0.738 | Must retain >73.8% of baseline mass |
| K=100k, one buffer | 1.524 | Must yield >1.524× baseline mass |
| K=175k, one buffer | 2.310 | Must yield >2.310× baseline mass |
| K=50k, two comparable buffers | 2.000 | Combined mass must exceed 2× baseline |
| K=175k, two comparable buffers | 4.620 | Combined mass must exceed 4.620× baseline |

For example, the next 50k ranks are worth completing if their mean success probability exceeds **52.4%** of that in the first 50k, under this cost model. Streaming has already been paid for. With uniform success density across ranks, K=175k improves discoveries/time by 3.5/2.31≈1.52. With sharply concentrated density, it loses. The six selected historical solutions do not distinguish these models reliably. The n=42 second-buffer case establishes that the excluded region can contain solutions, not its frequency at n=44.

**Exact specification.** Keep budget 2e6. After H correctness gates, provisionally allocate **80% of node-days to the H incumbent and 20% to B=2,K=175k**, on disjoint, reproducibly selected fresh cells/ranges. This is a stated uncertainty hedge, not a fitted optimum. It reaches both the omitted buffer and the moderate-rank region; no replay infrastructure is needed. Start with at most two node-days of broad-policy pilot and measure its actual cost before allocating the full 20%. Do not duplicate the incumbent first 50k on already completed cells unless that replay cost is explicitly charged.

Within each policy, if the aim is robustness to the unknown successful class, allocate class days proportional to estimated tile cost T_i=N_i/r_i. Under the assumption that the useful location is uniform within its class, this equalizes fractional coverage and maximizes the minimum coverage over the twelve possible classes. It is **not** the optimum under every prior: with known class mass q_i, the initial marginal return is proportional to q_i r_i/N_i, which would favor different classes. State the assumption rather than inventing empirical weights. Schedule ranges across low/middle/high cell-score bands; the flat-cell prior is too weak to justify letting a whole class or band starve.

For budget increases, the correct threshold is M_5/M_2 > T_5/T_2, not “more candidates resolve.” A selective replay of aborts also avoids generating entirely new candidates, so the old broad-budget experiment does not rule it out. Conversely, a cold replay costs the full new DFS run plus regeneration/lookup—not just the extra 3e6 nodes. With no success-mass evidence in that tail, do not build replay infrastructure now.

**Confidence.** High in the break-even formulas; low in any particular success-density model. At 13 continuously useful nodes, 540 node-days is roughly 42 days and 1,040 is 80 days of work allocation, not a discovery ETA. Admission and repeated work can lengthen either.

**Verification / preregistered rule / effect.** Policy truncation intentionally does not preserve every solution. First establish the underlying generator/completer's six-control and exhaustive-small-n retention without policy caps. Then report each control's actual H representative, buffer, sorted rank, and capped completion outcome; do not substitute historical ranks. Admit the 20% hedge only if it adds at least one known witness outside the incumbent selection and its measured cost on matched cells is ≤5× baseline; otherwise retain the incumbent and record the test inconclusive/failed. Recalculate any promotion using the measured M/T thresholds, treating six-control capture as a diagnostic, not a calibrated n=44 probability. No-hit results from a short n=44 pilot cannot discriminate these policies. Potential gain is substantial under diffuse rank mass, and negative under a sharply concentrated prior; this uncertainty is why the change is limited.

## 3. Cheapest completion change: solve the middle signs before placing them

**Claim / proof.** At n=44, once the 22 mirror quads are assigned, the only unknowns are a=A[22], b=B[22]. Their contribution to every lag is linear. The current leaf loop tries all four pairs, performs placement updates, then checks sums and all correlations. Many of these choices are already impossible from the sums alone.

For each s=1,…,22, define

    alpha_s = A[22−s]+A[22+s],
    beta_s  = B[22−s]+B[22+s],
    R_s = target[s]−Dab[s].

A middle pair works exactly when R_s=alpha_s·a+beta_s·b at these lags, the already determined larger lags match, both absolute sums match, and an allowed final profile row matches. This gives a four-bit feasibility mask. Usually the sum constraints alone leave at most one pair; both signs for one sequence can survive them only when its target magnitude is 1 and its partial sum is zero.

**Specification.** First discard middle pairs failing the existing absolute-sum checks before any placement. If center processing proves costly, intersect a four-bit mask using the displayed correlation equations and the exact final profile-row test; choose the first surviving pair in the current `P22_4` order and materialize it once. Preserve the existing final independent verification. Do not prematurely remove a pair merely because one allowed profile row fails; retain the union over rows. Keep budget-counter semantics explicit per item 1.

**Confidence / effect.** High, exact terminal solving. Maximum local saving is substantial relative to four placement/check/undo cycles, but “96.8% late” does not tell us what fraction is the middle itself.

**Verification / gate.** Six witnesses and all small-n completion verdicts must agree without caps; compare the selected first completion as well. At the same finite budget, no baseline-found candidate may become missed. Build the sum-before-place version first; promote if matched end-to-end worker CPU improves ≥2% with no control discrepancy. Build the mask version only if timing shows center handling ≥10% of completion CPU, and require ≥10% lower completion CPU including overhead. Do not infer timing from charged-node counts alone.

## 4. Stronger exact outer check: a tiny two/three-quad reachable-tuple table

**Claim.** The next k outer correlation equations can be tested jointly before placement, using k=2 or 3. This is exact reachability for a projection of the residual correlation target. It is stronger in principle than separate per-lag intervals and avoids a large full-target meet-in-the-middle structure.

**Proof / exact specification.** At entry to depth d, positions outside [d,44−d] are known. Use k=3 only for 3≤d≤19, and k=2 for 2≤d≤20, so there are k remaining positive quads and no middle complication. Write their proposed entries as `(aL_r,bL_r,aR_r,bR_r)`, r=0,…,k−1. Set

    R_j = target[44−d−j]−Dab[44−d−j],  j=0,…,k−1.

The required new contribution at that lag is exactly

    F_j = sum_(r=0)^j [ A[j−r]·aR_r + B[j−r]·bR_r
                     + A[44−(j−r)]·aL_r + B[44−(j−r)]·bL_r ].

For these ranges, no two unassigned positions can form an edge at these lags, and later inner quads cannot contribute. Therefore a completion requires `(R_0,…,R_(k−1))` to be among the tuples generated by the 8^k positive-quad assignments. Absence proves impossibility. Presence only passes this projection; it does not prove a completion.

The coefficients depend only on the first k **outer** quads, not d or the C,D candidate. Cache tables by those boundary signs. With root normalization there are at most `2·8^(k−1)` boundary patterns. Store, for each residual tuple, an eight-bit mask of extendable first-quad choices. Each F_j is even and |F_j|≤4(j+1), so a direct k=3 grid needs at most 5×9×13 entries per boundary pattern: roughly 75 KB for the masks over all 128 patterns. Build the small tables once, using exact integers.

At a DFS node, look up the residual tuple and intersect its first-quad mask with the existing root/reversal rules. Traverse surviving quads in the old order. Leave the current exact check in place initially. Ignoring sum/profile/reversal restrictions in table generation makes it a safe superset; do not build a table using restrictions that belong to a different current state.

**Confidence / expected effect.** High in the formula; rejection rate unknown. It can cut impossible subtrees before placement and scans, with constant-size lookup cost. It targets d up to 20, not the final center. The 96.8% late-node figure includes cheaply rejected trials, so it is not evidence that this table will save 96.8% of work. Also, a proposed single-lag “residual must be ±2” check often adds nothing: mirror parity plus the existing interval already enforces it. The value here is **joint** reachability.

**Verification / preregistered rule.** For every table state encountered in exhaustive n=6/8/10 controls where the generalized depth ranges apply, compare its allowed tuples/masks with literal next-k-quad enumeration. Retain all six full-solution witnesses; preserve uncapped verdicts and baseline finite-budget hits. Benchmark frozen n=44 candidates at the same cap and record wall CPU, resolved/aborted counts, and cuts by depth. Promote k=2 or k=3 only if completion CPU drops ≥20%, worker CPU drops ≥10%, and resolved count does not decrease. Stop after this bounded comparison if neither passes. This ranks ahead of a general residual-target DP.

## 5. Existing symmetry and profile opportunities, now targeted at late depth

**Claim / proof.** A,B exchange in the four equal-|sum| classes remains the cheapest unspent completion symmetry. Require A≤lex B only after the identical per-sequence root/reversal normalization; compare an assigned prefix only when every earlier index is known. Swapping A,B preserves the target and the full signed profile-row set. C,D canonicalization does not consume this symmetry. No additional independent A,B symmetry was found.

Joint A,B profile reachability is also still sound, and the new late-depth concentration removes the previous reason to defer it solely for lack of depth data. After the exceptional root quad, the mod-6 reflection is r→44−r: distinct pairs {0,2},{3,5}, fixed residues {1},{4}, with the middle at residue 4. For each surviving row:

- On distinct residue pairs use the integral Hadamard-transform / L1 / parity criterion from the earlier reviews on the remaining positive quads.
- At a fixed residue, q positive quads contribute pairs `(±2,±2)` or `(0,0)`. For residual `(u,v)`, reachability is exactly: u,v even; `max(|u/2|,|v/2|)≤q`; and `u/2≡v/2 (mod 2)`.
- At residue 4, first subtract one of the four possible middle-sign pairs and accept if **any** choice passes the fixed-residue criterion. Do not apply an even-residual rule before accounting for that middle.

Drop a profile row only if its own residual is unreachable, and prune the DFS only if **all** rows fail. This remains a profile relaxation; it is not full NPAF feasibility.

**Specification / confidence.** High mathematical confidence. Implement A/B exchange on its already planned schedule. For the row criterion, first measure surviving rows per late node and how often all witnesses disappear; 96.8% late nodes alone does not price scanning long row lists. Apply after the cheap correlation cuts, with exact integer arithmetic. Do not replace the entire variable order: outside-in placement is what makes the outer equations exact so early.

**Verification / rule / effect.** For both changes, retain normalized witnesses for the six controls and every exhaustive small-n solution orbit, testing combinations with C,D Q and profile constraints. Benchmark the exchange only on affected classes; require ≥15% lower completion CPU. For reachability require ≥20% lower completion CPU and no reduction in resolved candidates at the same cap. Twofold completion acceleration would improve total throughput only to `1/(0.476+0.524/2)≈1.36×`, not 2×. Fleet gain from exchange is smaller because only four classes benefit. Neither optimization needs a new speculative solution prior.

## 6. Other mathematics and limits of the evidence

**Claim.** **No new class elimination, construction, or additional symmetry found.** A different variable order and a general exact residual-target solver have no demonstrated advantage over the small outer-tuple tables above. Do not rebuild the measured-dead global hash join or incremental PSD backtracker under new terminology.

**Reason / confidence.** The supplied evidence supports invariance and performance measurements; it does not identify where n=44 solutions lie. Equal flatness scores do not identify equivalent candidates, high abort rates do not identify barren candidates, and a verified answer does not certify completeness of a search. I am confident in these limitations, not in a numerical discovery forecast.

**Specification / verification / pass-fail / effect.** Keep all twelve classes. Label counts as emitted, attempted, resolved-no, aborted, or policy-omitted; do not conflate 21–48M attempted completions with that many exhausted candidates. Any future mathematical rejection must carry its domain/precondition proof and retain actual representatives of the six controls plus the exhaustive corpus under the combined configuration. Policy changes instead require explicit capture/cost accounting and must not claim never to omit a solution. No implementation is recommended from this final item.
