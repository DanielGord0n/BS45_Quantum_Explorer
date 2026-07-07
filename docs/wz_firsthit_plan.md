# WZ first-hit pipeline — executable build plan (the one credible route toward n=42–43)

*Written 2026-07-07 (Fable handover). This is escalation-ladder item 3 turned into a plan a
successor session can execute. Decide explicitly with Daniel before starting Phase 1+ — it is
weeks of work with genuinely uncertain odds. Phase 0 is cheap and should happen first regardless.*

## Why this architecture (the verified reasoning, compressed)

Wang-Zhu (arXiv:2506.20296) reached n=41,42,43 with the same theorem set we already use
(Thm 2.3 residues, Thm 2.4 spectral bound) but a structurally different pipeline: they
**generate mod-6-residue-constrained C,D, filter per-sequence DURING construction, then
backtrack A,B against the exact target vector −CD per surviving candidate, stopping at the
FIRST solution** — existence, not exhaustion, and never a materialized product set. Our
measured facts: (a) the flat join is dead by pair-work volume above n≈29 (1.6e15 @ n=29,
4.0e16 @ n=31 — 2026-07-04 count probes); (b) our mod-3-profile stream is ~10⁹/side at n=31;
(c) WZ's effective filtering is ~10³× tighter; (d) targeted A,B completion is exponentially
cheaper than free enumeration because every layer is pinned by AB[s] = −CD[s]. The bet being
made: solutions are dense enough in a well-ordered filtered C,D stream to hit one early.
That bet is MEASURABLE before it is BUILDABLE — hence the gates.

## Phase 0 — measurement gates (days; mostly reuses existing code; do FIRST)

**Gate A — mod-6 stream size. INSTRUMENT BUILT + VALIDATED 2026-07-07 (Fable's last act):**
`WZ_COUNT_MOD6=1` in wz_match.cpp — streaming count under mod-6 profile pairs that must
satisfy the norm identity at BOTH moduli (a real bug was caught here by the ≤-invariant:
without the mod-3-reduction check, mod-6 "survivors" exceeded mod-3 — impossible for a true
refinement; fixed and revalidated at n=11/13). Soundness canary `WZ_PROFILE_CHECK=1`
(feed a banner-format solution + its SIGNED sig): **all four banked solutions PASS at both
moduli.** Honest calibration: at n=11/13 the mod-6 reduction is only ~1-2× (identity has
slack at small n; A,B side at n=11 reduced 0×) — **the gate question is whether the factor
grows to 10²-10³× by n=36, as WZ's results imply it must.** Jobs: count C,D at n=31
(sig 6,4,7,5 — mod-3 baseline measured: 8.2e15 CD pairwork), n=36 (5,11,0,0), n=42 (7,11,0,0).
*PASS: mod-6 stream (pair-work) ≤ ~10⁹ per signature at n=36. KILL: ≥10¹² at n=36.
If n=36 lands between: measure the per-candidate A,B completion cost (Gate B) before judging.*

**Gate B — targeted A,B completion cost.** `wz_generate.cpp` already backtracks A,B per
(C,D) — isolate and measure its per-candidate cost (nodes, ms) at n=19, 29, 31 using banked
solutions' signatures. This was never measured separately from its enumeration throughput wall.
*PASS: ≤ ~10 ms/candidate at n=31 on one core (⇒ 10⁹ candidates ≈ 60 node-hours — feasible).
KILL: ≥ 1 s/candidate with no pruning left.*

**Gate C — solution density / ordering (THE research question).** Run the assembled
prototype at n=29/30/31 (we hold verified solutions ⇒ guaranteed hits exist): how deep into
the C,D stream is the first hit, under (i) DFS order, (ii) PSD-flatness order (sort/prioritize
by Σ|corr_CD| — the exact intuition that cracked n=30 as WZ_PSD_BIAS)?
*PASS: first hit within the first ~10⁻³ of the stream under some ordering. KILL: hits
uniformly deep AND stream ≥10⁹ ⇒ first-hit buys nothing; stop and write up.*

Gates use the campaign doctrine: canary on banked solutions before trusting anything;
pre-registered pass/kill BEFORE running; one validated change per round.

## Phase 1 — mod-6 constrained C,D generator (≈1 week)

New module (or wz_generate extension): enumerate mod-6 class-sum profiles per signature
(reuse `enum_class_sums(L,target,6)` + the mod-6 norm-identity feasibility test already in
`survive_profiles`), DFS-generate sequences constrained to them, apply `hall_ok_single`
during construction (prefix bound where sound), **stream candidates via callback — never
materialize** (the n=29 OOM lesson). Validate: at n=7/11/13 the mod-6 stream must contain
the banked solutions' C,D and be ≤ the mod-3 stream (count both).

## Phase 2 — targeted A,B backtracker (≈1 week)

Adapt wz_generate's A,B fill: place A,B pairwise (comb8 encoding, as `verify_npaf.py`'s
pair-encoding check documents) with running AB[s] forced to −CD[s]; prune the moment any
completed shift mismatches. Add the residue/parity feasibility check on the A,B side before
descending. Validate: feeding a banked (C,D) at n=29/31 must reproduce its A,B (or another
valid one) in measured-Gate-B time.

## Phase 3 — assembled first-hit pipeline + canaries (days)

Stream (Phase 1) → complete (Phase 2) → exact `npaf_at` + `verify_npaf.py` on any hit.
Order stream by PSD flatness. SLURM: shard the profile set across array tasks (profiles are
the natural work units; distinct tasks take disjoint profile subsets — no seed logic needed,
this is deterministic). Canary ladder: n=19 (fast), 29, 30, 31 — each must re-find blind.
Only after all four: point at n=33-36 (sig sweep), then 42.

## Honest odds & framing (do not oversell to Daniel)

Even full success = **replication** of Wang-Zhu's 41-43, not a record. Its record value:
(1) it is the only architecture with measured headroom past the SA ceiling (~n≈33-35);
(2) a working pipeline + the Kotsireas conversation (docs/kotsireas_brief.md — send it!) is
the only credible approach vector to n=44, which otherwise needs new mathematics (NS(44) and
NN(44) empty; the whole field is stopped there). Expected outcome distribution, honestly:
most likely Gate C kills it or n≈36-38 walls it; a real minority chance of 42-43; n=44 only
via mathematical insight this pipeline might enable but cannot brute-force.

## While this runs

The SA blitz continues independently (it needs only refills — see bs45-campaign skill).
Do not cannibalize blitz clusters for gates; Phase 0 jobs are hours, run them as backfill
alongside, exactly like the 2026-07-03/04 probe pattern.
