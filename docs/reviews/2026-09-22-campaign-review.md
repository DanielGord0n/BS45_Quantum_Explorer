# BS(45,44) campaign review, 22 September 2026

Reviewed local revision `c9b1b22`, the requested solver/documents/champions, Wang–Zhu v3, and Đoković's classification paper. This is a review with temporary experimental source copies, not a deployed fix. No cluster access or large-n search was performed. All solver experiments were at n≤13. The three campaign champions and three Wang–Zhu reference files independently pass `tools/verify_npaf.py`.

The main finding is a **proved interaction bug between zero-sum endpoint pinning and cell canonicalization**. The next largest concern is repeated work between unbounded offset lanes. Neither is a reason to question a verified positive solution; both invalidate stronger coverage/retention claims.

Reproduce the local evidence from the repository root:

```sh
python3 docs/reviews/2026-09-22-evidence/reproduce.py
```

The script compiles temporary copies, checks the pinning counterexample and dispatch overlap, benchmarks the proposed tail check at n=11/13, and exhaustively checks the profile formula on small quads. Results are saved in [the evidence log](2026-09-22-evidence/results.txt). Larger known sequences are only verified or inspected, never searched.

## 1. First change: repair the composition of endpoint pins and cell canonicalization

**Confidence: proved for the counterexample and the lost reference orbit; unknown n=44 loss rate.**

`wz_match.cpp:1163` sets `pinC=(G_SIG_C==0)` and similarly for D. `count_pairs22`, lines 600–601, then requires the corresponding first symbol to be +1. Independently, the canonical-cell construction at lines 1260–1277 selects one profile among independent negations, reversals and swaps.

These symmetry restrictions do not compose safely. Negating a zero-sum sequence preserves its total sum but changes its mod-6 profile. After the cell representative is selected, the negated representative needed to satisfy the endpoint pin can lie in a discarded cell. “There exists an enumerated cell in the orbit” is insufficient: the selected cell must contain a representative surviving every other restriction.

**Actual unmodified solver, n=6, signature (5,1,0,0), all profile filters enabled, no completion cap:**

| Configuration | Streamed candidates | Result |
|---|---:|---|
| Canon off, existing pins | 1 before stopping | FOUND, independent NPAF PASS |
| Canon on, existing pins | 0 after all 7 retained cells | NO HIT |
| Temporary probe: canon on, C/D pins disabled | 1 before stopping | FOUND, independent NPAF PASS |

An enumeration-only comparison also found two distinct C,D orbits in the baseline, zero in the current canon-on path, and exactly those two in the probe. The failure is therefore not a small node budget, ordering, or front-only artifact.

**Deep reference check without running a deep search:** for Wang–Zhu's BS(43,42) pair, the minimum real cell under the implemented 32 C,D transforms has key

```text
-1,1,1,-1,-1,1,|-3,-3,1,-1,3,3,
```

Every transformed pair in that cell starts C[0]=D[0]=−1. The current pins eliminate that whole 32-transform C,D orbit. Our own n=42 pair has a retained representative starting (+1,+1), and survives. This explains why a selected successful canary does not prove the composition sound. It does not establish that every representative under the larger mathematical equivalence group is lost.

**Proposed repair:** in FIRSTHIT with cell canonicalization enabled, disable the independent zero-sum C/D endpoint pins. Retain the current pins when cell canonicalization is off. Later, if desired, derive one joint canonical form with a retention proof. Do not simply add another symmetry test.

**Expected gain:** restored access to valid solution orbits, demonstrated quantitatively as 0→2 candidate orbits in the exhaustive n=6 comparison and removal of the obstruction to the published n=42 C,D orbit. There is no defensible n=44 multiplier yet. The current workhorse (3,13,0,0) activates both problematic pins; the other affected n=44 classes are (3,5,0,12) and (9,9,0,4).

**Pre-registered acceptance:** exact equality of candidate-orbit sets before/after sound canonicalization on small-n exhaustive fixtures; retention of all six deep reference pairs after the complete combination of filters; re-find the three own solutions on clusters under the proposed production configuration; independently verify each. New stream version, new checkpoint namespace, and explicit re-exposure of affected cells are mandatory. Existing checkpoints have already skipped work that the repair makes accessible. No production source was changed in this review.

## 2. Additional mathematical conditions and class eliminations

**No new whole-class elimination or BS(45,44) construction was established.** Independent enumeration reproduces the stated 12 signature classes. The published general existence frontier and NS/NNS exclusions agree with [Wang–Zhu v3](https://arxiv.org/html/2506.20296v3). Those subclass exclusions do not imply that any particular general-BS signature class is empty.

### Exact mirror-quad reachability: the strongest new pruning proposal

**Confidence: proved formula; production gain unmeasured.**

Current `fh_abp_filter` (`wz_match.cpp:760`) checks each residual class sum separately against a capacity box. It ignores the joint constraints imposed by assigning mirror quads from eight legal product-+1 patterns.

For a residue component consisting of two distinct reflected classes, orient every remaining mirror pair toward the same first class. Let

\[
v=(\Delta A_r,\Delta B_r,\Delta A_{r'},\Delta B_{r'})^T,
\qquad
H=\begin{pmatrix}
1&1&1&1\\
1&1&-1&-1\\
1&-1&1&-1\\
1&-1&-1&1
\end{pmatrix}.
\]

The eight legal ordinary quads are exactly the signed rows of H. With q unassigned ordinary quads, the residual is achievable **if and only if**

\[
t=Hv/4\in\mathbb Z^4,\quad \|t\|_1\le q,\quad q-\|t\|_1\equiv0\pmod2.
\]

Proof: each quad becomes a signed coordinate vector under H/4. The minimum number of such steps reaching t is its L1 norm; any additional steps come in cancelling pairs. This is exact for the profile component, not sufficient for the NPAF completion.

Example: q=2 and v=(2,2,2,−2) satisfy the componentwise capacity, entry parity and total mod-4 condition. They fail the exact test because t=(1,1,1,−1) has L1 norm 4.

For a self-reflected residue class, with residual (x,y) from q ordinary quads, the exact condition is x,y even, x+y divisible by 4, and max(|x|,|y|)≤2q. Handle the exceptional A/B root separately by subtracting each allowed root pattern; handle an odd-length central column by subtracting its four possible values.

At n=44, zero-based mod-6 components are:

| Side | Components |
|---|---|
| A,B, length 45 | {0,2}: 8 quads including exceptional root; {1}: 4; {3,5}: 7; {4}: 3 plus central column |
| C,D, length 44 | {0,1}: 8 ordinary quads; {2,5}: 7; {3,4}: 7 |

Apply this first to profile rows and cells, then to residual targets inside `fh_abp_filter` and `count_pairs22`. It is a joint consequence of the already valid quad constraints, not a revival of compression or incremental PSD.

**RAN:** exhaustive equality with explicit quad sums for q=1..6; special-root and self-reflected cases pass; all 42 banked/reference files pass independent NPAF and all 3,028 checks along their known mod-3/mod-6 partial paths. For q=6, the old box+mod-4 relaxation admits 1,201 aggregate vectors, versus 833 reachable vectors. That 30.6% removal is a toy aggregate-space measurement, not a predicted stream rejection rate.

**Acceptance:** preserve all small-n verdicts and all six deep targets; on a frozen production candidate sample, require ≥20% lower CPU time overall, with no >10% class regression. Node reduction alone does not pass, since testing profile rows costs time. Start with static prefiltering and enable residual checks only where measured useful.

### Enforce the C,D first-quad constraint earlier

`count_pairs22:596` uses all 16 patterns for C,D's outermost quad. The correct product is +1 for this quad too: [Đoković, Theorem 2.1](https://arxiv.org/pdf/1002.1414) covers every C,D quad. Wang–Zhu's printed index starts at 2, although the surrounding text says eight patterns per quad.

At n=44/mod-6, the existing reflected-class mod-4 condition already forces this at a completed profile: all other quads in {0,1} contribute 0 mod 4, so the first must too. Enforcing it at the root avoids impossible branches earlier. **Do not claim half the accepted stream disappears.** Confidence in validity: high; runtime gain unknown, at most a twofold reduction of this root's branches before downstream costs.

### Missing symmetry, with a warning demonstrated by the pinning bug

The simultaneous exchange of C,D quad labels 4↔5 preserves their combined autocorrelation. It enlarges the current C,D symmetry subgroup and offers at most another factor two in orbit counts; it is distinct from stabilizers of the existing subgroup. It is described in [Đoković, transformation T4 and §4](https://arxiv.org/pdf/1002.1414).

However, the transformed profile depends on individual quad types. A standalone “first 4/5 must be 4” rule after current cell selection is unsafe. Measure full joint orbits on saved candidates and prove combined retention before considering it. Confidence: high mathematical validity, low confidence of positive net first-hit gain. Global alternation also links some signature classes, so the 12 sum classes are not 12 independent equivalence classes. Neither observation presently eliminates a whole class.

## 3. Completer improvements

### Check the newly resolved outer correlation before placing a quad

**Confidence: proved check and tested small-n gain; n=44 speedup unmeasured.**

At depth d≥1, before placing positions d and n−d, shift s=n−d becomes fully determined. All new terms at that shift are

\[
A_0a_2+B_0b_2+A_na_1+B_nb_1.
\]

Reject the proposed quad unless this equals `FH_CD_target[n-d]-Dab[n-d]`. Put this test immediately before the first `fh_place` in `fh_ab_search:926`. It costs four signed products instead of two placements, two undo operations, and a later bound scan. Under the normalized root the expression is ±2, so an impossible residual can reject the parent outright; otherwise only four of eight ordinary patterns can satisfy it.

Temporary comparison, 2,000 deterministic C,D inputs per size, unlimited completion, AB profile filter off:

| n | Same hits and solution digest | Original nodes | Proposed nodes | Observed wall ratio |
|---|---|---:|---:|---:|
| 11 | 993 hits, identical digest | 833,063 | 419,708 | about 1.4–1.5× |
| 13 | 818 hits, identical digest | 2,132,458 | 1,069,344 | about 1.6× |

These are short laptop microbenchmarks including candidate generation, not cluster forecasts. A provisional target is ≥20% production CPU reduction on a paired, fixed C,D sample. Repeat timings sufficiently for stability on the target x86 nodes.

Crucially, skipping branches before the node increment changes what a “2e6 node budget” buys. For an isolated comparison, either charge the old attempted nodes virtually or use unlimited completion on manageable fixtures; for production, version the completer/budget semantics and measure fixed-time retention. Do not claim bit-identical budget outcomes from identical unlimited solutions.

### Exact A/B swap reduction for equal absolute sums

In (3,3,4,12), (5,5,8,8), (7,7,4,8), and (9,9,0,4), swapping A,B preserves the class and current independent reversal canonicalizations. After A[0]=B[0]=+1, the two root endpoint orientations are equivalent; keep one. This halves the symmetric full no-solution tree in these classes. Real capped first-hit speedup remains workload-dependent. Prove profile-row closure under the swap, exhaust small fixtures, and re-find the known targets before use. Do not impose this on unequal ordered A/B sums without explicitly allowing the swapped signature/profile assignment.

### More expensive bounds, only after the cheap improvements

For a shift, group known-to-unknown contributions by remaining mirror quad, maximize its four-coefficient expression over its eight legal patterns, and bound unknown-to-unknown edges separately. This respects correlations ignored by `Kab`. It can tighten the bound without incremental PSD, but doing it at every node could lose more time than it saves. Test selected depths/near-tight shifts only; require net CPU benefit and exact retention. Confidence: valid upper-bound construction, speculative value.

No mathematical argument makes a smaller completion cap safe. The three successful node counts are selected observations, not a bound on possible completions. Full A/B meet-in-the-middle also requires handling cross-half correlation terms; the existing large-n join measurements offer no reason to fund a generic rewrite.

## 4. Search policy and allocation

### Fix ownership before widening the sweep

**Confidence: repeated-work mechanism proved; production fraction unmeasured.**

`wz_match.cpp:1663` starts at `shard + skip*nshard` and runs toward the end of the whole raw profile list. There is no upper boundary associated with the next lane. Orbit duplicates are skipped at line 1670. The driver gives different skips separate checkpoint directories (`cluster_firsthit_probe.sh:49`). Thus adjacent lanes can complete the same canonical cell's same deterministic front.

**RAN dispatch-only test:** n=11, signature (0,6,1,3), two shards, arm zero, canon on, buffer 20/top 2. Skip 0 selected 51 candidate identities; skip 8 selected 49; **all 49 were shared**, with zero later-only identities. Completion was bypassed solely to observe dispatch. This proves the mechanism, not the duplicate percentage on clusters.

The proposed 730 workhorse F lanes × reported 400–600 cell fronts would produce 292k–438k visits to only 35,925 canonical cells. Conditional on those rates persisting, at least 87.7–91.8% of cell-front visits repeat. The implied 8.1–12.2× duplication is a rollout projection, not a measured historical speedup; the pinning repair will also change front costs. Dead-profile cells, partial fronts and variable cost require an actual identity audit.

Use a versioned manifest of retained cells and disjoint, finite ownership units. Each unit carries stable cell keys, direction, buffer/rank intervals and completer version; it can span multiple 12h resumptions but cannot be owned by another lane. Keep 178 arms and twelve-hour whole-node jobs. Reconstruct prior exposure from available checkpoints/logs where possible. “Assigned”, “front completed”, “candidate rejected exactly”, and “candidate capped” must remain separate ledger states.

Pre-register exact union/disjointness and interrupted-resume equality on small fixtures; then require ≥2× **distinct** candidate decisions per charged node-day in a paired dispatch benchmark, with ≤10% per-candidate cost regression. This is a promotion threshold, not a prediction. No queue cancellation is proposed by this review.

### Optimize marginal detection per charged node-day

For policy p, let U_p be newly exposed candidates, q_y a prior probability of a completion, r_y(b) its detection probability within budget b conditional on existence, and T_p charged node-days. A rare-event planning approximation is

\[
\Delta H_p=\sum_{y\in U_p}q_y r_y(b),\qquad
\text{priority}=\Delta H_p/T_p,\qquad
P(\text{hit})\approx1-e^{-H}.
\]

Repeating the same deterministic search at the same budget contributes zero new detection. Extending a previously capped candidate contributes only its additional detectable portion. Candidate dependencies and class emptiness make this a model, not an established Poisson process. There is no defensible numerical expected time to n=44 from the available selected hits and overlapping negative exposure.

The 2m/5m comparison illustrates the measurement problem:

```text
2m: 600 cells × 50,000 × (1−0.24) ≈ 22.80M resolved candidates
5m: 483 cells × 50,000 × (1−0.05) ≈ 22.94M resolved candidates
```

The apparent 25% advantage in attempts nearly disappears when counting resolved decisions. Neither statistic alone measures hit probability. With comparable costs per cell, 2m beats 5m only if conditional solution retention satisfies r(2m)/r(5m)>483/600≈0.805. This ratio has not been measured. Keep 2m provisionally; do not lower it from three observed successful paths.

### Concrete provisional portfolio, after correctness gates

For the next 120 admitted twelve-hour jobs (60 node-days), use ten jobs per class, distributed across uncovered canonical cells using a frozen manifest/seed. Within each class use five F, three FR, and two **marginal** F2 jobs. Retain M=500k; F/FR K=50k, one buffer; F2 K=175k, two buffers; provisional budget 2m. Omit exact candidate identities already tested at that budget. If a region is already covered, allocate its quota to uncovered work. Reserve 5% of this CPU envelope for a stratified sample of capped candidates extended to 5m and paired performance measurements.

These shares are an exploration prior, not a fitted optimum. They stop the other eleven classes from waiting behind all three workhorse passes without evidence that this class is more likely to contain a solution. Compare distinct work per charged node-day separately from admitted node-days per calendar day. Preserve current twelve-hour jobs and queue age; no repeat of the dead short-job experiments is recommended.

Before promotion, locate the actual surviving representative and exact buffer/sorted rank of all three own targets under the repaired configuration. Re-find all three on clusters in a portfolio that contains them, then verify independently. Use the WZ trio as a retrospective challenge set, not an untouched holdout: their properties already informed the policy. Freeze any genuinely unused fixtures for later choices. A policy intentionally excluding a target cannot use successful direct completion of that target as its end-to-end canary.

## 5. Code-level opportunities

These are independent proposals; measure one at a time after the retention repair. Whole-solver gain depends on the measured time fraction of the affected stage.

| Location | Change | Quantitative rationale and qualification |
|---|---|---|
| `fh_ab_search:926` | Newly resolved tail check before placement | Tested about 1.4–1.6× at small n; see §3. |
| `fh_place:813`, `fh_unplace:837`, `fh_ab_search:939` | Precompute residual `Kab[d][s]`; iterate actual occupied ranges | Fixed outside-in placement makes Kab depend only on depth, not signs. Remove repeated capacity writes; update only shifts with unresolved contributions. Different from dead branchless full-length PLACE-V2. Runtime gain unmeasured. |
| `fh_abp_filter:760` | Only test residue classes changed by the latest placement | Parent survivor rows already passed other classes. At most two classes change per ordinary quad rather than all six. Cache remaining capacities once per call; consider reusable contiguous survivor buffers. Preserve the odd-center leaf check. |
| `flat_score:1718`, `fh_complete_ab:961` | Bit-packed autocorrelations via XOR/popcount | For a length-L binary word b, N_X(s)=L−s−2 popcount((b XOR (b>>s)) AND ((1ULL<<(L−s))−1)). At L≤64 this replaces the nested O(L²) score calculation with O(L) word operations. Guard shifts of 64; n=44 fits. Validate exact equality first. |
| `CellCand:1716`, `drain:1786` | Two packed words per pair; stable top-K selection | Current two heap-allocated int vectors cost at least 408 payload bytes/candidate at n44 on a usual 64-bit ABI, before allocator overhead. Two uint64 words plus score fit about 24 bytes. A 500k buffer's candidate storage can fall from >204MB to ~12MB. Keep original stream ordinal as the tie key; partial selection followed by sorting that total key reproduces the selected stable prefix. |
| `hall_ok:117`, `hall_ok_single:131`, `count_pairs22:570` | Avoid repeated PSD calculations; exploit conjugate angles | For real sequences f(2π−θ)=f(θ); only half the nonzero grid is distinct, and θ=0 follows from the signature. Joint PSD already implies each single PSD bound. Up to fourfold fewer DFT term accumulations for full survivors by one joint half-grid pass, not fourfold whole-solver speed. Early rejects and floating-point boundary behavior require differential testing. |
| Profile `sort:1233`, CFGSIG `:1521`, driver compile `:56` | Explicit profile tie key and stream/build manifest digest | Comparator uses score alone, so equal-score order is not a portable cross-build guarantee. Checkpoints omit a source/order digest. Freeze explicit tie ordering in a new namespace and refuse incompatible resumes. |

For scoring/sorting/PSD changes, preregister exact selected-candidate stream equality on small fixtures and saved real buffers, all six target retentions, exact interrupted-resume equality, and ≥10% stage improvement with no end-to-end regression. Changing tie order, filters or pinning requires a new stream version even when every mathematical solution remains valid.

## 6. Errors and unjustified assumptions

1. **“Orbit canon preserves every solution” is false for the current combined implementation.** The pinning counterexample and WZ-42 orbit inspection prove it. Confidence: certain for the demonstrated cases.
2. **Offset tiling is not disjoint ownership.** Raw-list skips, orbit skipping and unbounded lanes permit repeated identical fronts. “80% assigned” is not 80% uniquely searched. Confidence: certain mechanism; unknown fleet fraction.
3. **`profile_rank=1429` is not candidate rank.** `hit_prof=pi` at `wz_match.cpp:1740` and the print at 1761 make it a cell-list index. Confidence: certain.
4. **The percentiles have been relabelled incorrectly.** `n44_search_narrowing_research.md:246` describes a 1,500-candidate class-background comparison of ours-41, ours-42 and WZ-43. Later lever-20 text treats these as exact in-buffer percentiles of all three own hits. The third own hit did not yet exist when that experiment ran. Confidence: high, direct document comparison.
5. **F catches 2/3 and F2 catches all three are unproved.** Neither background percentiles nor global `idx` establish a per-cell sorted position. Canonical relocation further changes the relevant candidate. The recorded n43 F re-find is genuine evidence for that target. Confidence: high.
6. **The 28.9× cell ratio is not automatically a 28.9× candidate-orbit or time ratio.** Unequal cell sizes, partial traversal, endpoint pins and cross-lane repetition invalidate dividing tested counts by this ratio to obtain distinct exposure. Confidence: high.
7. **FR is not an independent statistical trial.** It is a deterministic alternative front whose intersection and marginal detection must be measured. Confidence: high.
8. **F2's repeated fraction is 14.3%, not 29%, under the stated idealized counts.** It repeats F's 50k among 2×175k=350k completions. The 29% figure applies only to its first buffer. Actual overlap should use candidate identities. Confidence: certain arithmetic.
9. **The successful-node maximum cannot certify a safe budget.** Low-budget capped candidates are unresolved, not failures. Raising a budget in later resumes also does not revisit already skipped capped candidates automatically. Confidence: certain.
10. **“Full 1,024-element equivalence group” is inaccurate.** That subgroup omits alternation and T4; the standard group has order 4,096. The claimed inequivalence nevertheless has a stronger short proof: measured C,D L1 scores are 124/150/130 versus WZ's 140/142/134, and this score is invariant under all five elementary transformations. Thus the incorrect group description need not invalidate the conclusion. Confidence: high.
11. **“Needs new mathematics” is an opinion, not an implication of hitless search.** The pinning and ownership issues alone prevent the current campaign from establishing that conclusion. Confidence: high.

The implementation order supported by this evidence is: restore symmetry retention; establish disjoint, auditable exposure; benchmark the cheap tail check; test exact quad/profile reachability. Deeper budgets, new symmetries and more elaborate pruning follow measured marginal benefit. No measured-dead solver architecture needs to be revived to do this.
