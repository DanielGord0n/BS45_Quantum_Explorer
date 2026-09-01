# Lever 19 — stratified window sweep (opened 2026-08-29)

Basis: the known-solution window map (docs/n44_search_narrowing_research.md, 08-29):
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
- Remaining pass-3 inventory: workhorse k=3075..5675 step 200 (14) + k=175..5775
  step 200 (29); (1,7,8,8)/(5,5,8,8) k=375..4875 step 500 (10 each); (3,3,4,12)
  k=125..4625 + 375..4875 step 500 (Fir, after its pass-2 lands)

## F41regr control status (2026-08-31)
Rep 1 HITLESS: 38.0M tested (vs 12.9M for the original hit job), arms at
resume_pi 1428-10290 (original hit cell rank 1429; canon keeps a DIFFERENT orbit
cell whose in-cell depth requirement is ~500k on its arm). PRE-REGISTERED RULE:
still hitless after rep 3 (~115M) => CANON REGRESSION FAILURE => disable
WZ_FH_ORBIT_CANON fleet-wide pending investigation.
