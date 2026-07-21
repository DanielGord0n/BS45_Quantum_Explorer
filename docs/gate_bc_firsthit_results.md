# Gate B + Gate C measurements — the first-hit probe (2026-07-16/17)

*Instrument: `WZ_FIRSTHIT=1` mode in `src/solver/wz_match.cpp` (built this session).
Streams the Thm-2.2-constrained C,D PAIR stream in deterministic DFS order
(profiles sequential, single thread ⇒ exact candidate index), completes A,B per
candidate by backtracking under Definition 1.1 + Theorem 2.2 (mirror-pair DFS,
8 combos/pair, NPAF bound pruning, per-candidate node budget), stops at the
first NPAF==0 hit. Work order: `docs/fable_workorder_firsthit_n41.md`.
Pre-registered rules (set before numbers were seen): Gate C PASS = hit within
~10⁻³ of the stream under some ordering and no degradation n=29→31; Gate B
PASS = ≤~10 ms/candidate at n=31 single-core.*

## Task 0 — Wang-Zhu Table 1 ground truth: ALL THREE PASS (banked)

Transcribed from arXiv:2506.20296 PDF (programmatic extraction, zero hand-typing),
banked at `results/reference/wz_table1_bs{42_41,43_42,44_43}.txt`:

| n | sig (a,b,c,d) | verify_npaf | Thm 2.2 encoding | 2.11a+b m=3 | m=6 | retention (all filter levels) |
|---|---|---|---|---|---|---|
| 41 | (−2,0,9,9) | PASS (all s) | PASS | PASS | PASS | KEPT |
| 42 | (7,11,0,0) | PASS (all s) | PASS | PASS | PASS | KEPT |
| 43 | (8,−2,5,9) | PASS (all s) | PASS | PASS | PASS | KEPT |

**The entire theorem stack is validated AT the target rungs.** `canary_thm211b.py`
now includes these as permanent PASS fixtures (10/10). Free intelligence: mod-3
profile spaces at n=41/42/43 are TINY (A,B/C,D survivors: 1308/604, 709/1441,
1369/1398) and mod-6 in the low millions — the profile layer is not a wall.

## Instrument lesson (negative result worth keeping)

The unpaired A,B backtracker (ported verbatim from `wz_generate.cpp`, NPAF
bounds only) **burned a 200k-node budget on 100% of 200,000 candidates at n=19
with zero completions and zero clean exhausts** (~550 s, 40e9 nodes). Adding the
Thm-2.2 mirror-pair encoding to the backtracker — exactly what WZ Step 5 says
they do — made the same 200k candidates tractable: most exhaust in ~43k nodes.
**The encoding is not optional in Step 5; any first-hit build must pair-place.**

## Gate C — first-hit fractional depth (DFS order unless noted)

| n | sig | stream size | hit idx | frac depth | wall (1 core) | notes |
|---|---|---|---|---|---|---|
| 7 | (2,4,3,−1) | 91 | 3 | 3.3e-2 | <1 ms | validation |
| 11 | (2,4,−5,1) | 809 | 4 | 4.9e-3 | <1 ms | validation |
| 19 | (6,4,5,1) | 1,291,990 | 807 | **6.2e-4** | 1.9 s | inside PASS window |
| 19 | ″ flattest-profile-first | ″ | 335 | 2.6e-4 | 0.27 s | ordering helps 2.4× |
| 19 | ″ reverse-profile (control) | ″ | 1022 | 7.9e-4 | 4.4 s | control degrades ✓ |
| 19 | ″ score gate ≤30 | ″ | 842 streamed / **22 completed** | — | 0.098 s | flat candidates ~35× denser in solutions |
| 29 | (0,6,9,1) | 1.737e9 | 17,551* | ~1.2e-5 | 30 s to 1st hit (190-arm node) | 56/190 arms hit; density 1/21,101 |
| 30 | (1,−7,6,6) | ~5e9 est | 77,760* | ~1.1e-5 | 331 s (″) | 16/190 arms hit; density 1/52,967 |
| 31 | (0,−6,9,−3) | ~1.4e10 est | 15,027* | ~2.4e-6 | 421 s (″) | 12/190 arms hit; density 1/33,666 |

n=19 independent re-verification: probe find sig (6,−4,5,1) PASS all shifts
(`verify_npaf.py`), distinct C,D from any banked artifact.

### Cluster probe details (Rorqual `16498722`/`16498723`/`16498724`, 2026-07-17 — all COMPLETED)

Setup: 190 single-core arms per node, interleaved profile shards (arm i takes
profiles ≡ i mod 190), 200k-node A,B budget per candidate, driver kills
remaining arms 1800 s after the first FOUND. Wall (launch→grace-kill): 1,830 s /
2,131 s / 2,221 s (sacct job elapsed 30:41 / 35:41 / 37:12).

*The `idx` caveat: `hit_idx` is the WITHIN-ARM candidate index (source:
`wz_match.cpp` firsthit block — "global first hit ≈ min over arms by
(profile_rank, idx)"), so the printed `frac_depth` is idx over the GLOBAL
stream total, an approximation. Strict single-thread DFS depth = Σ(sizes of
profiles before the hit profile) + idx, and per-profile sizes were not
recorded — with average-size profiles that could be ~2-6e-3 of the stream,
though the measured fat-tail (JOIN22: the last 29/541 profiles cost more than
the first 512) means early profiles are far smaller than average. The
ordering-free measurement is HIT DENSITY in the streamed sample (1 hit per
21,101 / 52,967 / 33,666 candidates at n=29/30/31): expected first-hit depth
under any unbiased ordering ≈ 1.2e-5 / 1.1e-5 / 2.4e-6 of the stream — two
orders inside the 1e-3 line, and NOT degrading (n=31 is the shallowest).*

Structure observed: repeated (idx, nodes_this_cand) pairs across different
profile ranks (n=29: nodes=27,599 at 8 distinct ranks with idx 8,214-8,899;
n=31: idx=15,027/nodes=182,739 at ranks 2 and 24) — symmetric-profile copies
of the same solution classes. Hits and stream carry the same multiplicity, so
density-as-expectation is unaffected.

The n=19 "solutions complete cheap" asymmetry NARROWS with n: hit costs at
n=29 span 11.8k-195k nodes (median ~50k, well under the ~198k budget-capped
dead-candidate average), but n=30/31 hits mostly cost 70k-183k nodes.

## Gate B — per-candidate A,B completion cost (from the same runs)

| n | candidates tested | avg nodes/cand | avg ms/cand (1 core) | budget aborts @200k | hit's own cost |
|---|---|---|---|---|---|
| 19 | 807 | ~43k | ~2.3 ms | 57/807 (7%) | 13,017 nodes |
| 29 | 1,181,629 | 197,969 | **294 ms** | 1,156,916 (97.9%) | 131,071 nodes (global-first) |
| 30 | 847,478 | 196,887 | **478 ms** | 823,743 (97.2%) | 182,446 nodes |
| 31 | 403,990 | 199,228 | **1,045 ms** | 400,363 (99.1%) | 182,739 nodes |

ms/cand = wall × arms ÷ candidates (the pre-registered formula; arm-window wall,
190 arms). Slight overestimate (~10-20%): the 56/16/12 hit arms stopped early.
Total AB_nodes: 2.339e11 / 1.669e11 / 8.049e10. Note the cost is
BUDGET-DOMINATED: nodes/cand is pinned at the 200k budget on every rung — what
grew n=29→31 is wall per node (node throughput/core fell 673k → 412k → 191k
nodes/s; whether that is deeper NPAF work per node or C,D stream-DFS overhead
between candidates is not separated by this instrument).

Early structural signal (n=11 and n=19): **solution-bearing candidates complete
in far fewer nodes than dead candidates take to exhaust** — the first-hit bet's
core asymmetry, observed directly. (At n=29-31 the asymmetry narrows; see the
cluster-probe details above.)

## Verdicts (filled 2026-07-17 from the completed n=29/30/31 probes — rules above, pre-registered, unmoved)

- **Gate C: PASS.** First hit at ~1.2e-5 / 1.1e-5 / 2.4e-6 of the stream
  (density-based expectation; probe's interleaved ordering agrees) — two orders
  inside the ~1e-3 PASS line — and the trend n=29→31 does NOT degrade (n=31 is
  the shallowest). Caveat recorded above: strict single-thread DFS depth was not
  directly measured by the sharded probe; the density reading is the operative
  number for any parallel first-hit architecture. Wall-clock corroboration: one
  192-core node re-found n=29/30/31 in 30 s / 331 s / 421 s.
- **Gate B: FAIL as measured** (not a clean KILL). Measured 1,045 ms/cand at
  n=31 vs the pre-registered PASS line of ≤~10 ms — two orders over — and it
  touches the ≥1 s KILL line. But the KILL rider "with no pruning left" is NOT
  satisfied: per-candidate cost is entirely budget-dominated (97-99% of
  candidates burn the full 200k-node budget), and known unexploited levers
  remain — eq 2.12 mod-4 pruning (implemented nowhere), the Thm 2.4
  cheap-then-expensive cascade, budget tuning, and score-gate ordering (35×
  solution-density enrichment measured at n=19).
- **Decision (Daniel's, per the work order):** whether Gate C's PASS + the
  striking wall-clock numbers justify Task 3 despite Gate B's FAIL, or whether
  Gate B redirects effort at the pruning levers first. The Trillium n=41/42
  exploratory probes (`1926730`/`1926731`, PD behind maintenance) will measure
  the target rungs directly on Wang-Zhu's published sigs.

## Second wave — the probe as a SOLVER at n=32/33 (2026-07-18, Rorqual, pre-canon binary)

Task 3 greenlit 07-17 ("do whatever is needed"); 7 probes submitted at n=32/33.
The three Rorqual jobs completed on the first morning (queued ~17h behind SA,
then 42–85 min wall each) and **cleared BOTH rungs**:

| job | n | sig class | arms hit | candidates | aborted | AB_nodes | global first (idx@rank, nodes, s) |
|---|---|---|---|---|---|---|---|
| 16632433 | 32 | (7,9,0,0)  | 6/190 | 248,615 | 246,814 (99.3%) | 4.96e10 | 31,809@20, 5,716, 907.6 |
| 16632434 | 32 | (3,11,0,0) | 5/190 | 684,246 | 676,792 (98.9%) | 1.36e11 | 114,492@30, 85,787, 3,322.3 |
| 16632435 | 33 | (6,4,9,1)  | 1/190 | 85,397  | 83,632 (97.9%)  | 1.69e10 | 85,397@73, 195,591, 2,724.8 |

All 12 hit banners (6+5+1) NPAF-verified independently 07-18
(`tools/verify_npaf.py` PASS on every one; full banners archived in
`results/firsthit_hits_2026-07-18.txt`). Banked: `champion_firsthit_bs33_32_a`
(global first of 16632433), `champion_firsthit_bs33_32_b` (of 16632434),
`champion_firsthit_bs34_33` (**new banked best, n=33**). Context: SA burned 22
twelve-hour 8-node arrays at n=32 with the floor pinned at 8 and never hit;
the probe cleared n=32 twice and n=33 once in under 90 min of wall each on ONE
node, with the OLD binary (no canon cut, no eq 2.12, no score tiers).

**Gate C trend extends and holds:** hit density 1/41k (n=32, sig a) / 1/137k
(n=32, sig b) / 1/85k (n=33, single-hit statistics) candidates — same order as
the n=29–31 reads; still no collapse with rising n.

**Gate B trend also extends (cost keeps growing, still budget-dominated):**
~1.9 / ~1.4 / ~10.1 s/cand at n=32/32/33 by the pre-registered formula
(arm-window wall × 190 ÷ candidates; window = first-hit elapsed + 1800 s
grace, so these are the same slight overestimates as the first wave). Aborts
97.9–99.3% — the 200k budget still dominates, and none of the levers landed
07-17 (canon ×4/20×, eq 2.12, tiers) were in this binary. The n=33 number is
dominated by the single hitting sig's small candidate count, not a cliff.

Structural observation for the levers work: in 16632433 three different arms
(20/116/181) hit the SAME (A,B) with different C,D (nodes_this_cand=5,716
each), and arm_50's hit has C≡D exactly — solution multiplicity across
profiles is real and exploitable by ordering.

**State after this wave:** banked best n=33; next un-cleared rung n=34.
Remaining first-wave probes: Nibi 17871088/90 (n=32) + 17871091/92 (n=33) still
PD — now redundant-as-solvers for their rungs but still useful as Gate B/C
replication data. Trillium 1926730/31 (n=41/42, WZ Table-1 sigs) still PD
behind maintenance — those measure the target rungs directly.

## Third wave — the ENTIRE n=34/35 frontier cleared in one wave (2026-07-19→20, canon+2.12+tiers binary, WZ_THM212=1)

The 07-19 full-rung wave (all 10 admissible sig classes at n=34 on Fir, all 5
at n=35 on Rorqual, 190 arms × 1 core each, budget 200k nodes/cand, first wave
on the NEW binary: canon cut + eq 2.12 + score tiers, WZ_THM212=1) went
**15 for 15 — every sig class at both rungs hit**, 47 banners total, all 47
independently NPAF-verified 2026-07-20 (`tools/verify_npaf.py` PASS on every
one; archive `results/firsthit_hits_2026-07-20.txt`). **New banked best n=35**
(`champion_firsthit_bs35_34_a..j`, `champion_firsthit_bs36_35_a..e`).

| job | n | sig class | arms hit | candidates | aborted% | AB_nodes | density | s/cand | first-hit wall |
|---|---|---|---|---|---|---|---|---|---|
| fir 49628809 | 34 | (1,1,6,10)  | 2/190 | 341,230   | 96.5 | 6.7e10 | 1/171k | 3.2  | 1.1 h |
| fir 49628810 | 34 | (1,3,8,8)   | 1/190 | 167,784   | 94.7 | 3.3e10 | 1/168k | 7.0  | 1.2 h |
| fir 49628811 | 34 | (1,11,0,4)  | 1/190 | 75,408    | 94.9 | 1.5e10 | 1/75k  | 11.4 | 46 min |
| fir 49628812 | 34 | (3,5,2,10)  | 4/190 | 1,412,432 | 97.2 | 2.8e11 | 1/353k | 1.3  | 2.2 h |
| fir 49628813 | 34 | (3,7,4,8)   | 5/190 | 2,995,188 | 96.8 | 5.9e11 | 1/599k | 1.0  | 3.8 h |
| fir 49628814 | 34 | (3,11,2,2)  | 3/190 | 314,725   | 94.2 | 6.2e10 | 1/105k | 2.1  | 29 min |
| fir 49628815 | 34 | (5,7,0,8)   | 1/190 | 86,981    | 97.2 | 1.7e10 | 1/87k  | 11.3 | 56 min |
| fir 49628816 | 34 | (5,9,4,4)   | 8/190 | 290,033   | 93.5 | 5.6e10 | 1/36k  | 1.4  | 5.6 min |
| fir 49628817 | 34 | (7,7,2,6)   | 9/190 | 561,424   | 94.4 | 1.1e11 | 1/62k  | 0.9  | 12.6 min |
| fir 49628818 | 34 | (7,9,2,2)   | 3/190 | 561,185   | 93.9 | 1.1e11 | 1/187k | 2.1  | 1.2 h |
| rorq 16737512 | 35 | (0,6,5,9)  | 2/190 | 401,869   | 96.3 | 7.9e10 | 1/201k | 3.5  | 1.5 h |
| rorq 16737513 | 35 | (2,4,1,11) | 2/190 | 896,528   | 96.3 | 1.8e11 | 1/448k | 3.4  | 3.9 h |
| rorq 16737514 | 35 | (2,8,5,7)  | 4/190 | 1,517,463 | 93.5 | 2.9e11 | 1/379k | 1.7  | 3.3 h |
| rorq 16737515 | 35 | (4,6,3,9)  | 1/190 | 271,509   | 95.2 | 5.3e10 | 1/272k | 7.4  | 2.4 h |
| rorq 16737516 | 35 | (4,10,1,5) | 1/190 | 750,088   | 94.8 | 1.4e11 | 1/750k | 6.5  | 6.6 h |

(density = aggregate candidates ÷ arms-with-hits; s/cand = per-arm wall
(driver first-FOUND delta + 1800 s grace) × 190 ÷ aggregate candidates — same
overestimating formula as the earlier waves. Cross-wave density comparison is
approximate: THM212=1 + the canon cut changed the stream definition vs the
pre-canon waves.)

**Gate C trend: still no collapse.** n=34 densities 1/36k–1/599k, n=35
1/201k–1/750k — the softest classes (median ~1/170k at n=34, ~1/379k at n=35)
have thinned roughly 2–4× per rung since n=32, but every class remains
hittable inside a 200k-node budget on one node in under 7 h.

**Gate B trend: cost per candidate roughly flat (0.9–11.4 s/cand) and still
budget-dominated** — but abort fraction dropped from 98–99% (old binary) to
93.5–97.2% with the new levers in play.

**Bonus replication data (Nibi, n=32, new binary):** 17871088 sig (1,7,4,8) —
9/190 arms, 375,622 cands (1/42k), first hit 1,397 s; 17871090 sig (3,9,2,6)
— 12/190 arms, 347,631 cands (1/29k), first hit 1,616 s. Two MORE n=32 sig
classes hit (four of four tried overall) — not banked (below banked best);
consistent with the n=32 density reads above.

**State after this wave:** banked best **n=35**; next un-cleared rung n=36
(needs sig-class enumeration + validation — Daniel's call, as is Task 3
proper). Still pending: Nibi 17871091/92 (n=33 replication, R), Nibi
18017139/40/41 (n=41/42/43, WZ Table-1 sigs, R), Trillium 1926730/31
(n=41/42, PD behind maintenance) — the target-rung measurements.

## 2026-07-21 — the n=36/37 wide wave lands: n=36 AND n=37 both cleared (7/9 + 4/4-so-far classes) — new banked best n=37

Same probe architecture (canon+eq2.12+score-tier binary, WZ_THM212=1, 190 arms,
200k-node budget), first wave with FH_SCORE_TIERS live. ⚠️ Driver logged both
gated tiers as `<=110` ("arms 0-46 <=110, 47-94 <=110, rest ungated") — the
submitted FH_SCORE_TIERS=110,130 did not reach the second tier; cosmetic-vs-real
to be checked before the n=38 wave. Abort% below is raw GATEB aborted/candidates;
on score-gated waves the gated arms skip candidates without searching them
(score_rejected), which deflates the raw ratio — rows with heavy score_rejected
are marked *.

| job | n | sig class | arms hit | agg cands | abort% | AB_nodes | density | s/cand | first hit |
|---|---|---|---|---|---|---|---|---|---|
| fir 49706278 | 36 | (1,1,0,12)  | 1/190 | 379,582   | 99.0  | 7.6e10 | 1/380k  | 6.6 | 3.1 h |
| fir 49706279 | 36 | (1,3,6,10)  | 1/190 | 582,296   | 22.1* | 2.6e10 | 1/582k  | 1.9 | 1.1 h |
| fir 49706280 | 36 | (1,9,0,8)   | 1/190 | 820,687   | 98.0  | 1.6e11 | 1/821k  | 6.4 | 7.2 h |
| fir 49706283 | 36 | (3,11,0,4)  | 1/190 | 1,153,500 | 97.0  | 2.3e11 | 1/1.15M | 6.4 | 10.3 h |
| fir 49706284 | 36 | (5,7,6,6)   | 1/190 | 1,019,315 | 94.8  | 2.0e11 | 1/1.02M | 6.3 | 8.8 h |
| fir 49706285 | 36 | (5,9,2,6)   | 1/190 | 359,651   | 42.9* | 3.1e10 | 1/360k  | 3.4 | 1.3 h |
| fir 49706288 | 36 | (7,9,0,4)   | 1/190 | 1,519,797 | 35.7* | 1.1e11 | 1/1.52M | 2.4 | 4.8 h |
| rorq 16809931 | 37 | (0,10,5,5) | 3/190 | 1,327,554 | 18.0* | 4.9e10 | 1/443k  | 0.7 | 0.8 h |
| rorq 16809933 | 37 | (2,4,7,9)  | 1/190 | 1,011,238 | 8.4*  | 1.7e10 | 1/1.01M | 2.1 | 2.7 h |
| rorq 16809935 | 37 | (2,12,1,1) | 2/190 | 5,902,894 | 19.2* | 2.3e11 | 1/2.95M | 0.9 | 7.0 h |
| rorq 16809939 | 37 | (6,8,5,5)  | 1/190 | 509,143   | 95.3  | 9.9e10 | 1/509k  | 7.1 | 4.8 h |

(Same formulas as prior waves: density = agg candidates ÷ arms-with-hits;
s/cand = (first-FOUND delta + 1800 s grace) × 190 ÷ agg candidates. All 14
banner instances — 7 fir + 7 rorqual arms — independently verify_npaf PASS
2026-07-21; 11 champions banked, one per sig class, GLOBAL FIRST arm.)

**Gate C trend: still no collapse through n=37.** Densities n=36
1/360k–1/1.52M, n=37 1/443k–1/2.95M — continuing the ~2–3×/rung thinning from
n=34/35 (1/36k–1/750k). Every completed class still hits inside the 200k-node
budget on one node; but the deepest n=36 class needed 10.3 h of a 12 h
walltime — the predicted wall region (36–39) is now visibly eating the clock.
Score tiers cut s/cand on the gated-arm classes (0.7–3.4 vs 6.3–7.1 ungated).

**⚠️ NEW ANOMALY — zero-candidate runs.** Two n=36 classes — fir 49706281
(3,3,8,8) and 49706286 (5,11,0,0) — ran ~11.5 h and streamed **candidates=0**
(0/190 arms, GATEB all zeros). (3,3,8,8) was among the four classes
stream-validated locally at ~470–520 profiles/side, so the class is NOT empty
— the cluster stream produced nothing in a full walltime. Same signature on
**all 9 fir n=41 classes** (49706289–97, ~11.5 h each, candidates=0), and Nibi
18017139/40/41 (n=41/42/43, published sigs) also showed candidates=0 at ~25 h
elapsed. This looks like a stream/enumeration wall (or a tier/order bug) that
switches on somewhere in the C,D pipeline — NOT a searched-and-missed
negative. No coverage claim for these classes. Diagnose before the n=38 wave
and before any more 40s submissions.

**State after this wave:** banked best **n=37** (`champion_firsthit_bs37_36_a–g`,
`champion_firsthit_bs38_37_a–d`). Rung ledger promoted ×2 → n=38, budget 0.
Still live at fetch time: rorqual 16809929/30/32/34/36/37/38 (n=37, 7 classes,
~3 h left) + 16809940–50 (n=42, 11 classes) — next loop reads them; Nibi
18017139/40/41 R; Trillium 1926730/31 still PD. Nibi 17871091/92 (n=33
replication, 1+2 arms, densities 1/76k and 1/77k agg) recorded as density
data, not banked.
