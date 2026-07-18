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
