# Lever 19 — stratified window sweep (opened 2026-08-29)

Basis: the known-solution window map (docs/research/n44_search_narrowing_research.md, 08-29):
solutions sit at flat windows 0 / ~8 / ~255-571 / ~499-842 / far reverse end. Cell
streams are effectively bottomless (cells_done ~0), so depth in a few windows is a
bet on a few windows. Sample the whole ordering at spaced offsets instead; each lane
= ONE 12 h rep (the offset's front); passes refine the stride. PROF_SKIP=k starts
each arm k cells into its chain = full-list window k (verified wz_match.cpp:1599).

## Track A — n=44 interior (Fir + Rorqual), flat ORDER=1, ORBIT_CANON=1, 12 h
Pass 1 (submitted 2026-08-29):
- (3,13,0,0) 5,837 windows: k = 100,200,...,5700 (57 lanes). Fir k<=2900 (29, -J F44i$k),
  Rorqual k>=3000 (28, -J R44i$k).
- (3,3,4,12) 5,178 windows: k = 500,...,5000 (10 lanes, Fir, -J F44Bi$k).
- (1,7,8,8) 5,492 windows: k = 500,...,5000 (10 lanes, Rorqual, -J R44Ai$k).
- (5,5,8,8) 5,463 windows: k = 500,...,5000 (10 lanes, Rorqual, -J R44Di$k).
Pass 2 (when pass-1 lanes have run): midpoints — workhorse k = 50,150,...,5750;
other classes k = 250,750,...,4750. Pass 3: quarter-points. Existing front stacks
(w0-8 flat, w1-5 rev) keep their queued reps but are NOT topped up while sweep lanes
are pending.

## Track B — n=43 WZ-43 band control (Trillium + Nibi), (8,-2,5,9) ORDER=1, 12 h
WZ-43's cell lies in the score-26 tie block = windows 255-571 (317 windows; position
within the block is platform-dependent). Phase 1 (submitted 2026-08-29): stride 8,
k = 255,263,...,567 (40 lanes: Trillium k<=439 = 24 (-J T43b$k), Nibi k>=447 = 16
(-J N43b$k, --account=def-ikotsire_cpu)). Phases 2-4: offsets +2, +4, +6 (k = 257,
265,...; 259,...; 261,...). Full band fronted after phase 4 (~160 lane-reps).
PRE-REGISTERED READ: WZ-43 re-found (REPRODUCTION CONFIRMED banner + verify_npaf)
within phases 1-4 => sweep strategy validated at scale (paper: "re-finds the
published n=43"). Not found after full band coverage => the in-cell depth, not the
window, is the bottleneck: measure WZ-43's in-cell rank next. Any FOUND that is not
WZ-43 = a NEW n=43 solution (bank as usual).

## Loop rules (auto_prompt.md, 2026-08-29)
- A sweep lane that finishes hitless is DONE (no singleton restack of the same k).
- Do not top up deep front stacks while sweep lanes are pending on that cluster.
- When a cluster's pending sweep lanes drop below ~8, submit the next pass/phase
  from the tables above (verbatim env; names carry the k), and record it here.
- Controls: Fir F41regr (n=41 re-find) — expect FOUND in one rep; report loudly
  either way. Track B FOUND => verify, bank as replication or new, report.

## RAC cutover (2026-08-31)
DRAC confirmed Daniel is a member of project rrg-ikotsire (ticket 0323126): all submits
now use `--account=rrg-ikotsire` where the association exists (auto-detect via
`sacctmgr -n show assoc user=dangord account=rrg-ikotsire`; Nibi variant likely
`rrg-ikotsire_cpu`); pending jobs moved in place via `scontrol update job <id>
Account=...`. Fallback: def-ikotsire.

## Pass-2 assignment ledger
- Nibi: workhorse k=50..1250 step 100 (13, submitted 08-30)
- Trillium: workhorse k=1350..3650 step 100 (24, submitted 08-30)
- Fir: workhorse k=3750..5750 step 100 (21) + (3,3,4,12) k=250..4750 step 500 (10) +
  F41regr reps 2-3 (submitted 08-31, RAC-detected)
- Rorqual: pass-1 lanes still pending get RAC re-prioritization 08-31; its pass-2 =
  (1,7,8,8)+(5,5,8,8) midpoints k=250..4750 step 500 when pending <8.

## Pass-3 assignment ledger (quarter-points, k ≡ 25/75/125/175 mod 200 for workhorse)
- Rorqual (submitted 08-31, with its pass-2): workhorse k=25..5825 step 200 (30) —
  read 09-01 ALL HITLESS (20007720-749)
- Rorqual (submitted 09-01): workhorse k=125..5725 step 200 (29, R44i$k) + (1,7,8,8)
  k=125..4625 step 500 (10, R44Ai$k) + (5,5,8,8) k=125..4625 step 500 (10, R44Di$k),
  --account=rrg-ikotsire_cpu
- Nibi (submitted 09-01): workhorse k=75..2875 step 200 (15, N44i$k), rrg-ikotsire_cpu
- Rorqual (submitted 09-02 by the loop, queue was empty again): workhorse k=3075..5675
  step 200 (14, 20120262-275) + k=175..5775 step 200 (29, 20120276-304); (1,7,8,8)
  k=375..4875 step 500 (10, 20120305-314); (5,5,8,8) k=375..4875 step 500 (10,
  20120315-324), rrg-ikotsire_cpu. Workhorse quarter-points (25/75/125/175 mod 200)
  now fully assigned; A/D quarter-points (125/375 mod 500) fully assigned.
- Remaining pass-3 inventory: (3,3,4,12) k=125..4625 + 375..4875 step 500 (Fir, after
  its pass-2 lands). After that: pass 4 = eighth-points (workhorse k ≡ 12/37/62/87/...
  mod 100 — define stride table before submitting).

## F41regr control status (2026-08-31)
Rep 1 HITLESS: 38.0M tested (vs 12.9M for the original hit job), arms at
resume_pi 1428-10290 (original hit cell rank 1429; canon keeps a DIFFERENT orbit
cell whose in-cell depth requirement is ~500k on its arm). PRE-REGISTERED RULE:
still hitless after rep 3 (~115M) => CANON REGRESSION FAILURE => disable
WZ_FH_ORBIT_CANON fleet-wide pending investigation.

## Lever 20 control lanes (submitted 2026-09-01, Nibi, RAC)
- N43dt327: (8,-2,5,9) flat window 327 = our n=43 hit's window, WZ_FH_DRAIN_TOP=50000.
  EXPECTED: re-find within one rep (hit was at idx 500000 = first buffer). If not
  re-found: its in-cell rank exceeds 50k => raise K (175k) before any n=44 use.
- N43dt257..567 step 8 (40 lanes): band phase-2 offsets, K=50000 — hunts WZ-43 / new
  n=43 while measuring cells/lane-day under the cap (compare cells_done vs the
  phase-1 uncapped lanes: 0-11 cells/lane).

## Lever-20 / canon verdicts + phase-3 submission (2026-09-03, daily loop)
- LEVER-20 CONTROL PASS: Nibi 21001113 (N43dt, K=50000, canon-on) re-found the banked
  n=43 champion byte-identically (verify_npaf re-PASS); capped cells_done_sum 144-161
  vs 0-26 uncapped ≈ 10x ≥ the pre-registered 3x line. Lever 20 UNGATED for n=44.
- CANON REGRESSION FAILURE (F41regr): rep 2 = 57650091 hitless 43.1M (cum 81.1M),
  rep 3 = 57650092 hitless 35.4M (cum 116.6M ≥ ~115M line) => rule fires: disable
  WZ_FH_ORBIT_CANON fleet-wide pending investigation. COLLIDES with the lever-20 pass
  (canon-on capped config validated same day; canon-off costs 3.8-28.9x dedup).
  n=44 fleet config => NEEDS_HUMAN; discriminator below resolves the mechanism.
- Fir discriminator (submitted 09-03): n=41 (0,2,9,9) flat skip-8, one rep each —
  F41nc (uncapped, canon-OFF: replica of original hit conditions, rank 1429 => expect
  FOUND), F41dt (K=50000, canon-ON: pre-registered lever-20 F41 control, geometry
  predicts no find), F41dtnc (K=50000, canon-OFF: expect FOUND + capped throughput).
  Read rule: F41nc FOUND + F41dt hitless => canon relocation confirmed as the F41regr
  cause AND capped canon-off re-finds shallow hits => recommend capped canon-off for
  shallow-front sweeps, canon-on where dedup dominates — Daniel decides fleet-wide.
- Nibi Track-B PHASE 3 (submitted 09-03): N43dt k=259..571 step 8 (40 lanes),
  K=50000, canon-on, ORDER=1, rrg-ikotsire_cpu — pre-registered +4 offsets, capped
  config = the one validated by today's control pass.

## DECISION 2026-09-03 (Daniel: "do whatever needs to be done") — canon ON + lever 20 fleet-wide
Rule collision resolved: canon soundness is verified independently (LOCATE_CANON: all 5
known solutions keep a cell), so the F41regr failure is relocation DEPTH (the kept
orbit cell's stream puts that solution past its first buffer), not unsoundness. In
expectation canon-on is never worse per lane-day (covers ~29x more orbits on the
workhorse; canon-off = 29 tickets on the same orbit at 29x cost), and the n=43 capped
control re-found ours with canon on. Discriminator lanes (F41nc/F41dt/F41dtnc) keep
running for the record; they do not change the n=44 config.

## PASS F — FRONT TILE (K=50000, ORBIT_CANON=1, flat ORDER=1, one rep each)
A capped arm advances ~8 cells per rep, so a lane at offset k fronts windows k..k+7.
Tile each class with offsets k = 0, 8, 16, ... < windows. Names: <C>44f<k> (workhorse),
<C>44Af<k> (5,9,6,6), <C>44Bf<k> (5,7,2,10), <C>44Cf<k> (1,7,8,8), <C>44Df<k> (5,5,8,8),
<C>44Ef<k> (5,11,4,4), <C>44Gf<k> (9,9,0,4), <C>44Hf<k> (3,5,0,12), <C>44If<k> (3,3,4,12),
<C>44Jf<k> (7,7,4,8), <C>44Kf<k> (7,11,2,2), <C>44Lf<k> (1,13,2,2). Class order = that
list (workhorse 5836 w = 730 lanes; A/B 2788/2718 w = ~345 each; C/D/E/G/H ~5450-5660 w
= ~690 each; I/J ~5180-5550; K/L 2820). LOOP RULE: when a cluster's pending < 8, submit
the next unassigned k-range for the current class (RAC clusters ~40-60 lanes, Trillium
~30), append the range here. Never re-submit an assigned k. Uncapped passes 1-3 are
complete and retired.
### Pass F ledger
- 2026-09-03 tranche 1: Rorqual workhorse k=0..472 s8 (60); Fir workhorse k=480..792 s8
  (40); Nibi workhorse k=800..1112 s8 (40); Trillium (5,9,6,6) k=0..248 s8 (32).
- 2026-09-04 (supplementary loop): Fir k=648..792 s8 RESUBMITTED (19 — tranche-1 jobs
  57972726-744 died header-only/no data with an empty queue; CKDIR resume + singleton
  names, so this is coverage repair, not a re-assignment) + tranche 2: Fir workhorse
  k=1120..1432 s8 (40). Jobs 58093614-632 (resubmits) + 58093633-672 (tranche 2).
  Workhorse tiled k=0..1432; next unassigned workhorse k = 1440.
- 2026-09-05 (supplementary loop): Fir 17 dead lanes RESUBMITTED (58093614-617/619-622/
  626-632/634/672 died header-only again, empty queue = k 648,656,664,672,688,696,704,
  712,744,752,760,768,776,784,792 + 1128 + 1432; 3rd rep for the 648..792 subset — but
  the 4 that completed this round showed tested_cum ≈ 2x = CKDIRs survive the deaths,
  so this is coverage repair) + tranche 3: Fir workhorse k=1440..1752 s8 (40).
  Jobs 58243093-109 (resubmits) + 58243110-149 (tranche 3), 57/57 echoed all PD.
  Workhorse tiled k=0..1752; next unassigned workhorse k = 1760.

- 2026-09-12 (daily loop): Fir queue EMPTY => tranche 5: Fir workhorse k=2080..2392 s8
  (40), verbatim env + --mem=0, rrg-ikotsire_cpu. Header-only tranche-4/resubmit lanes
  NOT resubmitted (09-11 rule: completed reps, CKDIRs intact). Workhorse tiled
  k=0..2392; next unassigned workhorse k = 2400. Jobs 59466759-798, 40/40 echoed, all PD.

- 2026-09-14 (daily loop): Fir 0 PD => tranche 6: Fir workhorse k=2400..2712 s8 (40 @2e6)
  + PRE-REGISTERED 5e6 pair k=2720/2728 (plain names F44f2720/F44f2728,
  WZ_FH_AB_BUDGET=5000000). Jobs 59818926-965 + 59818966/967, 42/42 echoed.
  Rorqual 0 PD => workhorse k=2736..3208 s8 (60 @2e6). Jobs 21071921-980, 60/60 echoed.
  Workhorse tiled k=0..3208; next unassigned workhorse k = 3216.
  Trillium queue EMPTY => (5,9,6,6) k=256..488 s8 (30 @2e6). Jobs 2309932-62, 30/30
  echoed. Class A tiled k=0..488; next unassigned A k = 496.

- 2026-09-15 (daily loop): Rorqual 0 PD (60 R at ~9h) => workhorse k=3216..3688 s8
  (60 @2e6, rrg-ikotsire_cpu, --mem=0). Jobs 21122818-877, 60/60 echoed, QUEUE_COUNT
  120 post-submit. Workhorse tiled k=0..3688 (63% of 5836); next unassigned workhorse
  k = 3696.

- 2026-09-17 (daily loop): Fir 1 PD (12 R) => tranche 7: Fir workhorse k=3696..4008 s8
  (40 @2e6, rrg-ikotsire_cpu, --mem=0). Jobs 60191241-280, 40/40 echoed, QUEUE_COUNT 53
  post-submit. Workhorse tiled k=0..4008 (69% of 5836); next unassigned workhorse
  k = 4016. Trillium 9 PD / Nibi 339 PD => no action; Rorqual Duo missed.

- 2026-09-16 (daily loop): no new k-ranges (all four pending >= 8). Trillium repair:
  9 T44Af lanes (304/312/344/360/368/376/384/392/424, jobs 2309938/939/943/945-949/954)
  were PD-HELD "user env retrieval failed requeued held"; scontrol release cleared all
  9 (reason None, original IDs/CKDIRs kept, no resubmit needed); queue 24 = 15 R + 9 PD.

### 2026-09-10 correction + memory fix
- The 09-09 "tranche 4" (Fir F44f1760..2072) never went in (agent backgrounded the
  submit); SUBMITTED 09-10 by hand with --mem=0, plus Fir's 14 dead windows, Rorqual's
  42, Trillium's 28 (T44Af, killed by the Sept 8-10 outage), Nibi's 2 — all under their
  original names, so surviving arms resume from CKDIRs.
- ROOT CAUSE of "header-only" deaths: Slurm's 48 GB default memory (ReqMem 48G, MaxRSS
  50.3G, 36 oom_kill events/lane). Driver now `#SBATCH --mem=0` (c792cbf); every submit
  also passes `--mem=0`; pending pre-fix jobs get `scontrol update MinMemoryNode=700000`.
  Uncapped lanes had ALSO been losing ~36/178 arms at launch since July.

## AUDIT 2026-09-11 — what Pass F can and cannot catch (against the 3 known solutions)
| hit | stream idx | in-cell flatness pct | buffer | caught by Pass F (buffer 1, top 50k = 10%)? |
|---|---|---|---|---|
| n=41 ours | 500,000 | 0.9% | 1 | yes |
| n=42 ours | 1,000,000 | 31.9% | 2 | NO |
| n=43 ours | 500,000 | 2.7% | 1 | yes |
Pass F is the fastest broad sweep (~10x cells/lane-day) and would have found two of the
three known solutions; it is blind to n=42-like solutions (second buffer, ~32nd
percentile). COMPLEMENT = PASS F2 (deeper front): same offsets, TWO buffers per cell,
top-175k (35%) of each => ~5-6x slower per cell than F, catches all three known cases.
Needs a small solver addition (WZ_FH_DRAIN_BATCHES=2, part of CFGSIG; CKDIR suffix
_dt175000b2). Schedule: F2 starts on the workhorse once its F tile completes (~1 week),
while F continues on the remaining classes; then F2 follows F class by class. The two
passes are complementary, not redundant (F2 re-tests F's top-50k of buffer 1 — ~29%
overlap — accepted for simplicity).

### F2 mechanics (built 2026-09-11, validated locally)
`WZ_FH_DRAIN_BATCHES=B` caps the first B sorted buffers per cell at top-K each (B=1 =
original lever 20; validated B=2 halves cells/rep as expected). Signature gains `.db<B>`
ONLY when B>1, so existing checkpoints stay valid. Driver CKDIR suffix `_dt<K>b<B>`.
KNOWN COST already paid: the unconditional `.dt` signature field (09-01) fresh-started
every lane that resumed after the 09-03 deploy (T43 stack, Fir sA/sB reps) — recorded,
not repeatable under the new rule.

### 2026-09-12: lever 21 applied; Pass FR defined
- Lever 21 PASS on Fir (n=41 re-found 3.1 h at 2e6 vs 4.6 h at 5e7). Applied fleet-wide:
  Fir's 40 pending tranche-5 lanes and Rorqual's pending lanes resubmitted at 2e6; all
  new submits use 2e6. Nibi twin control still queued (informational).
- PASS FR = Pass F offsets with WZ_FH_STREAM_REV=1 (lever 22): each cell's SECOND
  independent front (reversed enumeration), same K=50000, fresh CKDIRs (_sr1). Order per
  class: F -> FR -> F2. Workhorse F tile at k=2392/5836 (41%).

### 2026-09-15: Nibi = the RRG's home => load it. PASS FR tranche 1 on Nibi
Prof. Kotsireas: the RRG is meant for Nibi, several hundred CPU-years, "submit several
hundreds of jobs". Nibi tranche (submitted 09-15): workhorse PASS FR k=0..2392 step 8
(300 lanes, WZ_FH_STREAM_REV=1, K=50000, budget 2e6; lever-23 split: k=0 mod 16 with
WALL_SEC=900, k=8 mod 16 without). Names N44fr<k>. Loop rule: keep Nibi >= 100 pending;
next Nibi tranches = FR k=2400.. then FR on class A/B, then F2 on the workhorse.

### 2026-09-17: fairshare depletion is fleet-wide on RAC clusters
Rorqual LevelFS 0.29/0.33 (0 running, 59 pending), Nibi 0.34: two weeks of RAC-priority
consumption spent the account's fairshare; priority now decays back over ~2 weeks. Fir is
the only cluster still admitting within hours. Responses: lever 24 (3 h backfill lanes)
on Nibi 60/240 and Rorqual 30/29; keep every queue deep (fairshare recovers by itself,
idle queues waste the recovery); Fir carries the F tile meanwhile.

- 2026-09-20 tranche 8 (Daniel session): Fir workhorse k=4016..4328 s8 (40, 2e6). Next unassigned: workhorse k=4336 (74% assigned).

- 2026-09-21 tranche 9 (daily loop): Fir 1 PD (23 R) => workhorse k=4336..4648 s8
  (40 @2e6, rrg-ikotsire_cpu, --mem=0, no wall). Jobs 60795778-817, 40/40 echoed,
  QUEUE_COUNT 63 post-submit. Workhorse tiled k=0..4648 (80% of 5836); next unassigned
  workhorse k = 4656.

- 2026-09-21 Nibi FR tranche 2 (daily loop): Nibi 202 PD < 300 burn floor => FR
  workhorse k=2400..3192 s8 (100 @2e6, STREAM_REV=1, rrg-ikotsire_cpu, --mem=0, NO
  wall). Jobs 22411431..22411533 (100 echoed, IDs non-contiguous), QUEUE_COUNT 312
  post-submit. FR tiled k=0..3192; next FR k = 3200 (F pass complete through 4136).

### 2026-09-21: LEVER 23 (WZ_FH_WALL_SEC=900) VERDICT — FAIL, not adopted
First 60 Nibi FR reads (N44fr720..1192): walled half (k=0 mod 16) 400.0 cells_done/lane
vs unwalled (k=8 mod 16) 399.5 = +0.1%, pre-registered line was >= +15%. Parity-robust
(flipping halves: 399.5 vs 400.0). No wall on any future submit; the _ws900 CKDIRs of
the walled half stay valid (CFGSIG carries .ws only when set — resubmits of those exact
lanes would need the env; NEW lanes get none).

## PASS G (2026-09-22) — SUPERSEDES PASS F/FR/F2 tiling
Two defects found by external review (docs/research/n44_search_narrowing_research.md, levers
26-27): the workhorse was searched at ~25% orbit coverage (endpoint pins under canon)
and Pass F/FR lanes were ~99% redundant (no ownership). PASS G = front-only lanes with
explicit OWNERSHIP ranges, pins fixed: env `WZ_FH_PROF_SKIP=k, WZ_FH_PROF_END=k+S,
WZ_FH_DRAIN_TOP=50000, WZ_FH_AB_BUDGET=2000000, WZ_FH_ORBIT_CANON=1` (+`WZ_FH_STREAM_REV=1`
for the reversed front). A lane is DONE when GATEB shows range_done=178/178; restack it
(singleton) until then. Workhorse (3,13,0,0): S=1000 => G lanes k=0,1000,...,5000 (6)
forward + 6 reversed = 12 lanes. Next classes: S=300 (dedup 4-8x). Then G2 (two
buffers, K=175000) on the same ranges. Names: <C>44g<k> / <C>44gr<k>.
Retired: every F/FR/F2 lane on (3,13,0,0), (9,9,0,4), (3,5,0,12) (their checkpoints
fresh-start under ".np1" anyway) and all stride-8 lanes elsewhere.
### Pass G ledger
| class | dedup | windows | S | fwd lanes | cluster |
|---|---|---|---|---|---|
| (3,13,0,0) | 28.9x | 5836 | 1000 | 6 | Fir |
| (9,9,0,4) | 7.6x | 5443 | 300 | 19 | Rorqual |
| (3,5,0,12) | 7.6x | 5478 | 300 | 19 | Rorqual |
| (1,7,8,8) | 7.9x | 5492 | 300 | 19 | Nibi |
| (5,5,8,8) | 7.9x | 5463 | 300 | 19 | Nibi |
| (5,11,4,4) | 7.9x | 5664 | 300 | 19 | Nibi |
| (3,3,4,12) | 3.9x | 5177 | 150 | 35 | Nibi |
| (7,7,4,8) | 3.9x | 5547 | 150 | 37 | Nibi |
| (5,7,2,10) | 4.0x | 2718 | 150 | 19 | Nibi |
| (5,9,6,6) | 8.0x | 2788 | 300 | 10 | Trillium |
| (7,11,2,2) | 8.0x | 2820 | 300 | 10 | Trillium |
| (1,13,2,2) | 8.0x | 2820 | 300 | 10 | Trillium |
Each class also gets the reversed-front twin (gr) over the same ranges: ~222 forward +
~222 reversed lane-units cover the ENTIRE 12-class front tile at K=50k.
- 2026-09-22 submission: all of the above (see HANDOFF for job IDs).
