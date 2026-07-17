# Work order for Fable — the first-hit rebuild, aimed at reproducing WZ's n=41

*Written 2026-07-16 (evening) by Opus, after the JOIN22 canary PASSED. Read `HANDOFF.md`
newest TOP OF MIND + `docs/wz_paper_reconstruction.md` + `docs/wz_firsthit_plan.md`
(your own, 2026-07-07) first, then invoke the `bs45-campaign` skill.*

---

## What changed today (two things, both load-bearing)

**1. The complete join WORKS — and is banked.** Canary `16243606` (Rorqual, 192 cores)
COMPLETED, exit 0:0, elapsed 11:42:20, and found **BS(30,29) sig (0,6,9,1)** — a signature
class *distinct* from the banked SA n=29 champions (4,-10,1,1), i.e. a genuinely new solution
obtained by exhaustive search. Independently NPAF-verified; banked at
`results/champions/champion_join22_bs30_29.txt`; `canary_thm211b.py` now 7/7. The Thm 2.2 +
2.3 stack is therefore **validated end-to-end at n=29**, not just at toy sizes.

Measured cost, n=29, 192 cores: STREAM 35,092 s (83%) · RESOLVE ~7,042 s (17%).

**2. Opus made an error that pointed the campaign at "give up". It is retracted here.**
Extrapolating the join's cost + table size gives n=42 ≈ 1,100 years and ≈ 2×10⁶ TB of key
table, and Opus reported that as "n=42 is unreachable." **That is a fact about JOIN22's
architecture, not about the problem.** JOIN22 materializes the C,D stream into a hash table
and probes it. **Wang-Zhu never materialize anything.** Their Step 5, verbatim: *"continue
with the next C,D sequence until a solution is found."* Stream C,D one at a time, backtrack
A,B against the pinned target −CD, stop at the first hit. **Memory O(1).** The OOM wall is
self-inflicted by our design. (Error shape, for the record: a conclusion drawn from the wrong
quantity — the same shape as the 07-10 "10⁵×", the 07-15 "120× profile cut", and the 07-16
"≥24 h stall". Opus's, all four.)

---

## 🔑 THE UNLOCK — why first-hit is not actually dead

`wz_firsthit_plan.md` is **PAUSED at Phase 0** on the strength of **Gate A (07-08: KILL)** and
then **Gate A′** — both of which measure **STREAM SIZE**, read against "≤1e9 at n=36 PASS /
≥1e12 KILL".

**Stream size is the wrong gate for a first-hit architecture, because first-hit never
enumerates the stream.** `docs/wz_paper_reconstruction.md` already flags this (§ "Validation",
point 4): *"the ≤1e9-at-n=36 rule measures stream size, but WZ's Step 5 is FIRST-HIT — it never
enumerates the stream… the right gate may be density × ordering, not size."*

A stream of 10¹⁶ candidates is irrelevant if the first solution sits at depth 10⁷. **The only
gate that bears on first-hit is Gate C (solution density / ordering) — and Gate C has never
been run.** Gate B (per-candidate A,B completion cost) has never been run either.

**So: do not re-litigate Gate A/A′. They are sound measurements of a quantity that does not
gate this architecture.** Your task is the two gates that do.

---

## Task 0 — Table 1 ground truth (do this FIRST; hours, laptop, no cluster)

The WZ paper's **Table 1 contains their actual BS(42,41), BS(43,42), BS(44,43) sequences**
(n=41, 42, 43). We have never transcribed them. This is free, decisive ground truth.

1. Transcribe from the **PDF** (the arXiv HTML table renders badly), arXiv:2506.20296.
2. Run `tools/verify_npaf.py` on each.
3. Run `tools/canary_thm211b.py`-equivalent checks (2.11a/2.11b at m=3 and m=6) on each, and
   `WZ_PROFILE_CHECK=1` (the retention harness — it now asserts 2.11b, added 07-16).

**Pre-registered reads:**
- **All three PASS** → our theorem stack is validated *at the target rungs*. Every filter we
  ship is proven to keep the real n=41/42/43 solutions. Proceed to Task 1 with confidence.
- **Any FAIL on verify_npaf** → either the transcription is wrong (recheck twice) or our
  verifier disagrees with a published, peer-reviewed result at n≥41. Either outcome is
  enormous — **STOP and report**. Do not build on a stack that rejects the target.
- **PASS verify_npaf but FAIL our 2.11b filter** → **CRITICAL**: our filter excludes real
  solutions at the target rungs. That silently invalidates the join too. STOP and report.

Bank whatever passes to `results/reference/` (NOT `champions/` — they are Wang-Zhu's, not
ours; provenance must say so explicitly). They then serve as canaries at n=41+ forever.

## Task 1 — Gate C: solution density × ordering (THE research question)

This is the gate that decides whether n=41 is reachable at all. We hold verified solutions at
n=29 (×3 now, incl. today's join find), n=30, n=31 ⇒ **a hit is guaranteed to exist**, so this
measures depth, not luck.

For each of n=29/30/31, at the banked signature: how deep into the filtered C,D stream is the
first hit, under (i) DFS order, (ii) **PSD-flatness order** (prioritize by Σ|corr_CD| — the
intuition that cracked n=30 as `WZ_PSD_BIAS`), (iii) any ordering you can justify from the
paper?

Report **fractional depth** (hit_index / stream_size) and its trend across n=29→31. The trend
is the whole signal: flat or improving ⇒ extrapolate to n=41; degrading ⇒ first-hit buys
nothing.

*Pre-registered (do NOT move these once you see numbers):*
- **PASS:** first hit within the first ~10⁻³ of the stream under some ordering, and the
  fractional depth does not degrade n=29→31.
- **KILL:** hits uniformly deep AND the trend degrades ⇒ first-hit buys nothing. Say so, stop,
  and we write it up as a real negative result.
- **AMBIGUOUS:** report with the numbers; Daniel decides.

## Task 2 — Gate B: per-candidate A,B completion cost

`wz_generate.cpp` already backtracks A,B per (C,D). Isolate and measure per-candidate cost
(nodes, ms) at n=19/29/31 using banked signatures. Never measured separately from its
enumeration wall.

*Pre-registered:* **PASS** ≤ ~10 ms/candidate at n=31 on one core. **KILL** ≥ 1 s/candidate
with no pruning left.

## Task 3 — only if Gates B and C both PASS

Build Phases 1-3 of `wz_firsthit_plan.md`, with the 07-08 Gate A′ architecture correction
(generate C,D **PAIRS** with comb8 DFS, not independent sequences) and today's fixes carried
over (banner-at-find-time; `count_pairs22` abort hook). **Stream via callback — never
materialize.** Canary ladder n=19 → 29 → 30 → 31, each must re-find blind, before pointing at
n=41 with WZ's own published signature from Table 1.

---

## Known gaps in our stack (relevant to all of the above)

- **eq 2.12 (mod-4) is implemented NOWHERE.** Safe direction (all our counts are upper bounds
  on WZ's), but it is unexploited pruning.
- **Thm 2.4 cascade:** WZ do cheap-then-expensive (l=50, then l=1000 on survivors) for NS/NNS.
  We do a single θ=jπ/100, j=1..200 pass. Not obviously wrong for BS; worth checking.
- **They lift only ONE side (C,D) to m=6**, keeping the other side's combinations small.

## Do NOT rebuild (measured dead — the skill has the numbers)

Exhaustion n≥36 · bias above n=30 · incremental PSD pruning · the n=36 Gate A′ array (a PASS
is arithmetically impossible by monotonicity, and one C,D profile exceeds a 12h walltime) ·
**Gate A/A′ re-litigation** (sound, but wrong gate for this architecture) · >12h walltimes.

## Cluster policy for this work order

**Tasks 0-2 are laptop/local. Do not request cluster time for them.** Daniel must approve a
Duo push per submit, and nothing here needs one. If Task 3 is greenlit, profiles are the
natural shard unit (deterministic, disjoint — no seed logic).

## Honest odds (your own words, 07-07 — still true, do not oversell)

Full success = **replication** of Wang-Zhu's 41-43, **not a record**. *"Most likely Gate C
kills it or n≈36-38 walls it; a real minority chance of 42-43; n=44 only via mathematical
insight this pipeline might enable but cannot brute-force."* n=44 (BS(45,44)) is open for the
entire field — NS(44) and NN(44) are empty and everyone is stopped there.

And: `docs/kotsireas_brief.md` is READY TO SEND and is now **stronger** than it was this
morning — we have a correct, verified, complete solver with a measured frontier and an honest
ceiling. The methods ask is the door to 42+. Compute is not.
