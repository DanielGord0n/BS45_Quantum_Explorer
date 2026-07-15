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

**5. ⛔ RETRACTED — the profile cut does NOT carry to the stream (measured same evening).**
An earlier draft of this doc argued a ~120× *profile* cut would drag the n=36 stream to ~2.3e10 and
un-kill the gate. **That was wrong and is retracted.** Once Step 4 was implemented and the STREAM
(not the profile count) was measured end-to-end:

| n | C,D stream norm-only | +2.11b | **STREAM cut** | profile cut (for contrast) |
|---|---|---|---|---|
| 7 | 91 | 53 | **1.7×** | 2.4× |
| 11 | 809 | 797 | **1.02×** | 9.2× |
| 15 | 55,794 | 23,918 | **2.3×** | 8.5× |

**The profiles 2.11b removes are nearly EMPTY** — they carry almost no sequences. The fat profiles
satisfy 2.11b and survive. Profile count and stream size are different currencies; conflating them
was the error. Projecting the measured ~2× onto n=36: **2.7e12 / 2 ≈ 1.35e12 — still at/above the
1e12 KILL line. The KILL most likely STANDS.**

**Lesson (same shape as the 07-10 "10⁵× reduction" mistake): a ratio on the wrong quantity is not
evidence. Measure the quantity the gate is written against.**

**Honest caveats:** (a) profile-count cut ≠ stream cut 1:1 — the real test is re-measuring the pair22
C,D stream at n=29/31 against the banked baselines (1.74e9 / ~1.4e10); (b) the fit is 5 noisy points
with different signatures; (c) even a PASS only licenses Phase 1 → reproducing WZ's *published* 41-43.
n=44 remains open and needs new mathematics.

## 🔧 IMPLEMENTED 2026-07-15 (Opus) — `WZ_THM211B=1`, and the SECOND gap it exposed

**Shipped, compiling, off by default (A/B-able):** `autocorr_vec` / `pair_auto` / `PairAutoSet`
in `src/solver/wz_match.cpp`; 2.11b wired into BOTH `survive_profiles`' mod-6 tighten and
`survive_profiles6`. Env gate `WZ_THM211B=1`. Only m=6 uses it (m=3 is vacuous — proven above).

**Measured, n=11, banked sig (2,4,-5,1):**

| profile space | norm-only | +2.11b | cut |
|---|---|---|---|
| mod-6 A,B | 8,478 | **2,484** | 3.4× |
| mod-6 C,D | 916 | **234** | 3.9× |
| mod-3 A,B / C,D (what pair22 uses) | 108 / 38 | **108 / 38** | **1.0× — NO CHANGE** |

C,D = 234 matches `tools/measure_thm211b_prune.py`'s independent Python prediction **exactly**,
which is strong evidence the C++ is correct and not merely plausible.

### ⚠️ THE SECOND GAP — 2.11b is INERT in the pair22 path, and here is why

`survive_profiles()` returns **mod-3** profiles; its mod-6 test is only an **existential tighten**
("does SOME mod-6 lift of this mod-3 profile admit a complement?"). Tightening that existential
does not delete a mod-3 profile as long as **one** lift survives — hence 108/38 unchanged.
`count_pairs22` then generates sequences from those **mod-3** targets, so the 3.9× never lands.

**Wang-Zhu's Step 4 is explicit: they generate C,D from the mod-6 profiles recorded in Step 3.**
We generate from mod-3. **That architectural difference — not the filter — is what is left.**

**✅ STEP 4 IS NOW DONE (2026-07-15, same session).** `count_pairs22` was already a generic
class-sum DFS with the modulus hardcoded to 3; it now takes `m` (modulus = profile width), and
`WZ_PAIR22_M6=1` drives generation from `survive_profiles6` output. **Validated:**
- **n=7 ground truth: 66/91 — matches the banked exact figures.**
- **INVARIANT: mod-6 generation with norm-only == mod-3 generation, exactly** (66/91 at n=7,
  1564/809 at n=11). Finer profiles only regroup sequences; nothing is lost. If this ever breaks,
  the mod-6 profile set is incomplete and every number built on it is worthless.
- Switches are independent on purpose: `WZ_PAIR22_M6=1` (Step 4) and `WZ_THM211B=1` (Step 3 filter)
  can be A/B'd separately. That separation is what exposed the retraction in §5.

**What is left is NOT more filtering — it is Gate B.** The stream-size gate (≤1e9 at n=36) asks
"can we enumerate the whole stream". WZ's Step 5 never does: it backtracks A,B per C,D candidate and
**stops at the first solution**. So the deciding quantities are **per-candidate A,B completion cost**
(Gate B) and **solution density × stream ordering** — not stream size. The JOIN22 n=29 canary
(`resolve 64/342 FOUND`, `128/342 FOUND`) is the only density evidence we have and has never been
allowed to finish. **Finish the canary; then measure Gate B.** Do not spend more effort shrinking a
stream that the real architecture never enumerates.

*(historical — superseded by the line above)* Next change: make the pair22 path consume
mod-6 profiles — i.e. drive `count_pairs22` from `survive_profiles6(...)` output with a mod-6
class-sum target (the modulus-m DFS already exists: `count_seqs_for_profile_m`, used by the
WZ_COUNT_MOD6 probe). This changes `count_pairs22`'s contract (mod-3 target -> mod-6 target),
so it needs the exact small-n ground-truth re-run (n=7: 66/66, 91/91) before it is trusted.

**Regression tests already in place:** `tools/canary_thm211b.py` (2.11b must hold on every valid
banked champion — currently 6/6, and it correctly REJECTS the invalid champion_v3_n27) and
`tools/measure_thm211b_prune.py` (independent Python model of the profile cut; C++ must match it).

**Do NOT queue cluster jobs for this yet.** With generation still driven by mod-3 profiles, a
`WZ_THM211B=1` run on a cluster computes exactly what today's binary computes. The first cluster
job worth submitting is the re-measured pair22 C,D stream at n=29/31 **after** Step 4 lands,
compared against the banked baselines (1.74e9 / ~1.4e10).

## 🚨 THE ACTUAL LEAD (2026-07-15, measured locally) — THE JOIN IS NOT DEAD

While measuring density I ran the **complete JOIN22 join on a 4-core laptop sandbox**. It does not
merely count — it **finds and self-verifies solutions**:

| n | C,D stream | distinct keys | dedup | raw hits | result | wall (4 cores) |
|---|---|---|---|---|---|---|
| 7 | 91 | 16 | 5.7× | 60 | **BS(8,7) FOUND**, NPAF==0 | 0.01 s |
| 11 | 809 | 115 | 7.0× | 404 | **BS(12,11) FOUND**, NPAF==0 | 0.01 s |
| 15 | 55,794 | 9,542 | 5.8× | 1,674 | **FOUND** | ~1 s |
| 19 | 1,291,990 | 236,424 | 5.5× | 10,692 | **BS(20,19) FOUND**, NPAF==0 | **25.9 s** |
| 23 | — | — | — | — | still running at >150 s | — |

**Measured cost growth: ~2.67× per +1 in n** (fit on n=11→19). Single-node 4-core projections:
n=27 ≈ 18.6 h · n=29 ≈ 5.5 d · n=31 ≈ 39.5 d. **On a 192-core Alliance node (~48× more cores):
n=29 ≈ ~3 h, n=31 ≈ ~20 h, n=33 ≈ days (and it shards across an array trivially — each task takes a
slice of the A,B stream).**

**Why this matters — the "join is dead" verdict is STALE.** HANDOFF says the join is dead by TIME
above n≈29 (pair-work 1.58e15 @ n=29, 4.0e16 @ n=31). But those were the **independent-side** counts,
measured BEFORE Thm 2.2. HANDOFF already flags this itself: *"the 2^(L/2) factor shrinks the hash-side
stream too, and the 'join dead by time' verdict was measured on the INFLATED independent-side counts."*
The Thm-2.2-constrained C,D stream at n=29 is **1.74e9**, not 1.58e15 — **six orders of magnitude
smaller.** Nobody re-derived the frontier after that. **The join's real ceiling is UNKNOWN and the
measured curve says it is well above n=29.**

**Memory is not the wall either.** The stream dedups **~5.5-7× into distinct keys** and the v2 table
stores bare 8-byte keys: n=29 ≈ 1.74e9/6 ≈ 2.9e8 keys ≈ **2.4 GB** (HANDOFF's 34 GB figure assumed
`SLOTS_LOG2=32` sized for the un-deduped stream). Size the table from *distinct keys*, not stream.

**Why this beats SA if it holds:** the join is **deterministic and exhaustive per signature** — it
either finds a solution or PROVES none exists for that signature. SA is a lottery capped ~n≈33-35.

**THE EXPERIMENT (this is a real cluster job, unlike everything else tonight):**
1. **n=29 canary `16243606` (Rorqual, 24 h) is already running** — this is exactly this test.
   A PASS = the join re-finds the banked n=29 solution ⇒ the frontier is re-opened.
2. Then walk the join UP the ladder on real nodes: **n=31, 33, 35** — one signature per job,
   `WZ_JOIN22=1`, `WZ_JOIN22_SLOTS_LOG2` sized from distinct-keys (~stream/6), sharded by A,B slice.
3. Every rung it clears above **n=31** beats the banked best, deterministically.

**Caveats (honest):** the 2.67× fit is 3 points with different signatures; per-signature cost varies a
lot (n=23 is already running long vs its 22-min projection); phase-2 hit resolution and the dedup
factor may scale differently at n≥29; and reaching n=41-43 would still need many more rungs than this
curve comfortably projects. **But n=31-33 looks reachable, and that is a real, deterministic result.**

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
