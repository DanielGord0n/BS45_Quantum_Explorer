# Wang-Zhu reconstruction — read from the ACTUAL paper (2026-07-15)

Source: **arXiv:2506.20296, "On Base, Normal and Near-normal Sequences", Xu Wang & Jiayi Zhu**
(HTML: https://arxiv.org/html/2506.20296v1). Read directly this session. Until now the campaign
had reconstructed this method from second-hand notes — the paper was NOT in the repo — and
falsified two guesses (mod-6 class sums; Thm 2.2 alone). This doc replaces the guessing.

## Their algorithm, verbatim structure (Section 3)

- **Step 1:** select `(a,b,c,d)` and `(a*,b*,c*,d*)` satisfying **Thm 2.1**.
- **Step 2:** find all `k₁₃,k₂₃,k₃₃, r₁₃,r₂₃,r₃₃, p₁₃,p₂₃,p₃₃, q₁₃,q₂₃,q₃₃` satisfying **Thm 2.3** at m=3.
- **Step 3:** for every m=3 set, find all `p₁₆…p₆₆, q₁₆…q₆₆` **for which there EXISTS at least one
  group `k₁₆…k₆₆, r₁₆…r₆₆`** such that the *combined* `k,r,p,q` at m=6 **satisfies Thm 2.3**.
  Record those `(p,q)` sets.
- **Step 4:** for every recorded `(p₁₆…q₆₆)`, generate all C,D satisfying **Thm 2.2**; then drop any
  with `f_C(θ)+f_D(θ) > 4n+2` at `θ = jπ/100, j=1..200` (**Thm 2.4**).
- **Step 5:** for each surviving C,D, backtrack for A,B using Def 1.1 + Thm 2.2 **and the isomorphic
  transformations of A,B to truncate branches**. Stop at the FIRST solution.

They note explicitly why they only lift ONE side: *"In our algorithm we only find values of
k…,r… OR p…,q… for a small number m. The benefit is that the overall combinations is relatively
small."*

## 🔴 THE GAP — what we never implemented

**Thm 2.3 / eq 2.11 has TWO parts. We implement one.**

```
(2.11a) N_K(0)+N_R(0)+N_P(0)+N_Q(0) = Σk² + Σr² + Σp² + Σq² = 4n+2      <-- the NORM IDENTITY
(2.11b) N_K(s)+N_R(s)+N_P(s)+N_Q(s)
      + N_K(m-s)+N_R(m-s)+N_P(m-s)+N_Q(m-s) = 0 ,  s = 1..[m/2]        <-- RESIDUE AUTOCORRELATION
        where N_K(s) = Σ_{i=1..m-s} k_{i,m} · k_{i+s,m}   (ditto R,P,Q)
```

- `survive_profiles6()` (src/solver/wz_match.cpp ~L464) enforces **only 2.11a** — its own comment
  says *"norm budget + EXACT complement completion at mod-6 (same norm identity as mod-3)"*.
- **grep for 2.11b across wz_match.cpp returns NOTHING.** `N_K/N_R/N_P/N_Q` do not exist in the code.
- So we keep every profile pair whose **norms** add to 4n+2. WZ *additionally* require the
  **residue-level autocorrelations to cancel** — the residue analogue of NPAF=0 — and require it
  **jointly**, via an existential check over the A,B residue vectors.

This explains every measurement we have:
- **Gate A (mod-6 class sums) = 0.15% reduction.** Of course — it only re-tested 2.11a at a finer
  modulus. The norm identity is nearly parity-generic; it prunes almost nothing extra.
- **Gate A′ (Thm 2.2) = big ratio, wrong level.** Thm 2.2 is real (~10⁵× on the sequence stream) but
  it is a *sequence-level* pair encoding. It does not prune the *profile* space at all.
- **n=29 C,D stream = 1.74e9** — already over the n=36 PASS line, because the profile space feeding
  it was never cut by 2.11b.

## The fix (concrete, and it is a PROFILE-LEVEL change, not a solver rewrite)

In `survive_profiles6`, for each candidate `(px, py)` = `(p₁₆…p₆₆, q₁₆…q₆₆)`:

1. compute `N_P(s), N_Q(s)` for `s = 0..3` from the residue vectors (trivial quadratic forms);
2. the A,B side must supply, for each `s = 1,2,3`:
   `N_K(s)+N_R(s) + N_K(6-s)+N_R(6-s) = −[ N_P(s)+N_Q(s)+N_P(6-s)+N_Q(6-s) ]`
   and `Σk²+Σr² = 4n+2 − (Σp²+Σq²)`;
3. **existential test:** does ANY `(k₁₆…k₆₆, r₁₆…r₆₆)` in `enum_class_sums(L,sigA,6) ×
   enum_class_sums(L,sigB,6)` hit those targets (also obeying 2.10 bounds/parities and 2.12 mod-4)?
4. keep `(px,py)` only if yes.

Precompute the achievable `(N_K(s)+N_R(s)+N_K(6-s)+N_R(6-s), Σk²+Σr²)` tuples for the A,B side ONCE
into a hash set, then step 3 is an O(1) lookup per C,D profile — same shape as the existing
`PairNormSet::feasible()` machinery, just keyed on a richer tuple than a single norm.

## ✅ MEASURED 2026-07-15 (Opus session) — canary PASSED, pruning power measured

Instruments (both in `tools/`, pure python, seconds to run, no cluster needed):
`canary_thm211b.py` (soundness) · `measure_thm211b_prune.py` (pruning power).

**1. Soundness canary: PASS.** 2.11b holds EXACTLY (all autocorrelation sums = 0) on **all six
valid banked champions**, at both m=3 and m=6 (n=7, 11, 29, 29b, 30, 31). The math is real and our
reading of it is correct.

**2. The canary has discriminating power — it caught a bad bank.**
`results/champions/champion_v3_n27.txt` FAILS 2.11b *and* 2.11a. Independent NPAF check confirms
it is **NOT a valid BS(28,27)** — nonzero at shifts 6,7,12,14,16,18,20,23,25. Signature and lengths
are fine, so it looks plausible; it is not. **It has been banked since April in violation of
verify-before-claiming. It should be quarantined** (it also means any "re-find the banked class"
canary keyed on n=27 was chasing a non-solution).

**3. Why WZ lift to m=6 — proven, not guessed.** At **m=3, 2.11b is VACUOUS**: for a length-3
residue vector, `N(v,1)+N(v,2) = ((Σv)² − norm(v))/2`, so the s=1 condition collapses to
`(a²+b²+c²+d²) − (Σ all norms) = 0`, which is exactly **Thm 2.1 minus 2.11a** — implied, never
false. Measured: **1.0× reduction at m=3** for every n tested. **m=6 is the first modulus where
2.11b carries new information.** This retro-explains WZ's Step 2→Step 3 design (m=3 all four sides,
then lift ONE side to m=6) — and explains why our mod-6 work found nothing: we lifted the modulus
but kept testing only the norm identity, which is the part that is already nearly generic.

**4. Pruning power at m=6 (C,D profile pairs surviving, norm-only vs +2.11b):**

| n | norm-only (current code) | +2.11b (Wang-Zhu) | cut |
|---|---|---|---|
| 7 | 400 | 169 | 2.4× |
| 11 | 2,155 | 234 | 9.2× |
| 15 | 49,005 | 5,798 | 8.5× |
| 19 | 204,826 | 27,046 | 7.6× |
| 23 | 295,995 | 14,385 | 20.6× |

Noisy and signature-dependent, but trending up: ~**1.145× per +1 in n** → **projected ~120× at n=36**.

**5. Consequence — the 07-15 KILL is NOT safe.** Projected n=36 C,D stream *before* 2.11b = 2.7e12
(KILL). *After* a ~120× profile cut ≈ **2.3e10 → lands IN BETWEEN the 1e9 PASS and 1e12 KILL lines**,
i.e. the rule says **run Gate B (per-candidate A,B completion cost)**, not "abandon". The earlier KILL
judged an under-filtered pipeline, not Wang-Zhu's.

**Honest caveats:** (a) profile-count cut ≠ stream cut 1:1 — the real test is re-measuring the pair22
C,D stream at n=29/31 against the banked baselines (1.74e9 / ~1.4e10); (b) the fit is 5 noisy points
with different signatures; (c) even a PASS only licenses Phase 1 → reproducing WZ's *published* 41-43.
n=44 remains open and needs new mathematics.

## Validation plan (campaign doctrine: measure before building)

1. **Soundness canary FIRST.** Extend `WZ_PROFILE_CHECK` to assert 2.11b on all four banked
   solutions at m=3 and m=6. If a banked solution FAILS, the implementation is wrong — not the math.
   (This is exactly how the mod-6 ≤-invariant bug was caught on 07-07.)
2. **Exact small-n ground truth.** n=7/n=11 profile counts before vs after 2.11b; the filtered set
   must still contain every profile belonging to a known solution.
3. **Then, and only then, re-measure the C,D stream at n=29 and n=31** and compare to the banked
   baselines (1.74e9 / ~1.4e10). The ratio is the answer to "what does the real work?"
4. Re-read the gate rule afterwards. **Note:** the ≤1e9-at-n=36 rule measures *stream size*, but
   WZ's Step 5 is FIRST-HIT — it never enumerates the stream. If 2.11b shrinks the profile space by
   orders of magnitude, the right gate may be *density × ordering*, not size.

## Other facts from the paper worth having

- **Table 1 contains their actual BS(42,41), BS(43,42), BS(44,43) sequences.** These are free ground
  truth: our solver should re-find (or at least verify) them. `tools/verify_npaf.py` can check them
  directly. Transcribe from the paper (the HTML table renders poorly — use the PDF).
- **Their Step 5 confirms first-hit**: *"continue with the next C,D sequence until a solution is found."*
- **Thm 2.4 resolution:** θ = jπ/100, j=1..200 for BS. (For NS/NNS they do l=50 then l=1000 on
  survivors — a cheap-then-expensive cascade we do not do.)
- **They lift only ONE side to m=6** (the C,D side), keeping the other side's combinations small.
- **Scope check on our goal:** they reached n=41,42,43. The paper states plainly that BS(n+1,n) for
  **n > 43 is still open**. So n=44 is the true open frontier — reproducing 41-43 is the achievable
  target; 44 needs new mathematics, exactly as HANDOFF says.
- Bonus: they *disprove* the Yang conjecture (no NNS(42), NNS(44)) and prove no NS(n) for n=8k−2.
