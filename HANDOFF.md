# CP493 — BS(45) Solver Project Handoff

**Date**: 2026-09-25 (read TOP OF MIND newest-first; QUICK REFERENCE below has the current
system. Pre-2026-07-24 history — SA era, join saga, firsthit ramp n=32→37 — lives in
`docs/archive/handoff/HANDOFF_ARCHIVE_to_2026-07-23.md`; measured-dead list in `.claude/skills/bs45-campaign/SKILL.md`.)

**⚡ 2026-09-25 (Claude session, evening) — REPO REORGANIZED (nothing deleted).** docs/ now has
plans/ research/ briefs/ reviews/{prompts,evidence}/ archive/ with an index (docs/README.md); 39 paths
git-mv'd, references rewritten in 34 files, 0 dangling live references; HANDOFF trimmed 3,021 -> ~760
lines (entries 07-24..09-14 -> docs/archive/handoff/); dead scripts -> cluster/archive/ (June jobs, SA,
hash-join, GPU spikes, PLACE-V2, old check_all.sh); old dumps -> results/archive/; README status and
layout updated; .agents skill copy synced; CLAUDE.md/AGENTS.md layout rewritten. All test suites and
script syntax checks PASS after the move. Clusters unaffected (they run their own $SCRATCH copies).**

**⚡ 2026-09-25 (Claude session, afternoon) — LOOP STALL GUARDED; CANARY v1 ROOT CAUSE PROVEN; CANARY
v2 SUBMITTED (Fir 61516887).** (1) 1pm stall: the fable CLI sat 13:01:53 -> 13:33:22 BEFORE its
session started (transcript), released 11 s after the display woke (13:33:11); same on 09-24
(13:21:27 -> 13:21:43). Not MCP (strict-mcp 8.9 s, default 7.4 s now), not credit retries (blocked
fable fails in 3.3 s), not reproduced by a 20 s display-off. Mechanism unproven; both cases used the
VS Code-bundled CLI. GUARD in daily_auto.sh: caffeinate -u before launch, watchdog re-wakes every
2 min + phone alert at 4 min until a transcript appears, caffeinate -i around the agent, no IDE
auto-connect, remembered 20 h primary credit block; launchd job now ProcessType=Interactive
(verified 'spawn type = interactive'). (2) Canary v1 INCONCLUSIVE root cause (the loop's raw-vs-flat
diagnosis was wrong): profile-score ties are ordered by std::sort's unspecified tie behaviour, so
macOS libc++ and Fir libstdc++ disagree on indices; libc++ tie randomization moves ours42's cell
582325 -> 549054. v2 LOCATEs in-job on Fir (rehearsed PASS). The macOS-index qprune prediction is
INVALID (renamed); v2 also writes the Fir-order dead list to $SCRATCH/bs45_qcanary/
qprune_dead_3_13_0_0.txt, which the loop now uses for the CELLSIZE check. Production unaffected (one
toolchain per cluster, gcc 12.3 everywhere). CELLSIZE 61315095 RUNNING 10:53 at 14:3x. (3) Astra
Pass H review: docs/reviews/2026-09-25-astra-passh.md (approve with ownership manifest + cell-key
tie-break + transition rules; stream levers: endpoint-quad root filter, exact residual reachability). (4) STREAM LEVERS BUILT, default off: WZ_FH_CD_PRUNE=1|2|3 (root mod-4 filter |
exact mirror-pair residual reachability), stream-IDENTICAL (tools/test_cd_prune.py: 896 runs n=6..13,
dump bytes + per-cell counts + exact verdicts), not in CFGSIG (checkpoint-compatible if it passes).
PAIRED PILOT Fir 61518000 (CDPILOT, $SCRATCH/bs45_cdpilot, 8 cores 8 h, sha 50aec4df..): 4 workhorse
F44g2000 shards x {0,3}, count-only to the 500k prefix, same node. PRE-REGISTERED
(tools/cd_prune_pilot_summary.py): identity must hold else FAIL; PASS = median per-cell time ratio
off/on >= 1.25 over >= 8 paired finished cells; CLOSE < 1.05; else INCONCLUSIVE.**

**⚡ 2026-09-25 (daily loop 1pm — ALL FOUR reached) — NO HITS; Q CANARY 61331128 = INCONCLUSIVE
(HARNESS ADDRESSING BUG, the Q path was never exercised) => NEEDS_HUMAN; 13 hitless reads, 0 submits.**
NEW FOUND: none on all four. ★ QCANARY 61331128 (read via duo_run, 2 taps; verbatim in
docs/reviews/evidence/qcanary_61331128.txt): COMPLETED in 35 s (not 12 h). Both targets printed
"step1: NO TARGET LINE -> VERDICT: INCONCLUSIVE (image not reached)". Step-1 logs: candidates_streamed=0,
cells_orbit_dup=1, pins_dropped=1, cells_done=0, RANGE EXHAUSTED in 1.7 s on both arms; resume_pi =
582503 (ours42) / 26283 (wz42) = target cell_idx + 178 exactly, so the arm owned position
cell_idx and found an orbit DUPLICATE there. PROBABLE CAUSE (local reproduction on the same fe7485e
source, sha 3c9b9c46..): LOCATE reports cell_idx 582325 canon_kept=YES as a RAW profile index, and
its own flat-order line says "cell_score=30 rank_lo=501588 ties=120016; window_lo=2817 window_hi=3492".
The canary script instead set window=cell_idx div 178, arm=cell_idx mod 178 (3271/87, 146/117), i.e.
used a raw index as a flat-order (PROF_ORDER=1) position. So the job read some other (dup) cell.
The local test harness (tools/test_target_canary.py) uses PROF_SKIP=cell_idx with no NSHARD, the
same assumption; at n=8/10 it passed, so either small-n order coincides with raw order or the
harness shares the bug. My two local n=42 TARGET reruns hit my 90 s bound (not diagnosed further).
PRE-REGISTERED RULE applied literally: INCONCLUSIVE => redesign, NO Q deploy. No FOUND => nothing
to verify. NEEDS_HUMAN: fix the canary addressing (map the kept raw cell to its flat-order
position/arm, e.g. have LOCATE print the kept cell's exact flat position), re-validate, resubmit.
CELLSIZE 61315095 still RUNNING (R 2:35 at check) — not read, not touched.
READS (all hitless, aborts vs ~35% line): FIR 61213834 (F44gr0) cells 431 range_done 0/178 aborts 8.7%;
61213836 (F44gr1000) 807 cells range_done=4/178 aborts 17.7%; 61213837 (F44g2000) 849, 13.3%;
61213838 (F44gr2000) 796, 27.2%; 61213839 (F44g3000) 955 cells, cells_empty=3, 19.8%. All
tested_cum > tested (resumes intact, no CFGSIG/fresh-start). Units began on 3014b95 => rate only.
RORQUAL R44G (9,9,0,4) 21608263-270: cells 634-769 each, range_done 0/178 except 267 (6/178) and
268 (4/178); aborts 26.5/31.9/18.5/28.9/39.9/32.2/27.8/33.5% — 21608267 at 39.9% is OVER the
~35% line (single lane; pooled over the 8 = 89.2M/300.2M = 29.7%). Flagged, no action (budget
lever closed; (9,9,0,4) runs hot). QUEUES: Fir 13 R + 19 PD (incl. CS44g2000), Rorqual 0 R + 48 PD,
Nibi 296 PD, Trillium 60 PD — every floor satisfied (Fir/Rorqual pending >= 8, Nibi >= 100) => no
refill, rung_status not needed (no idle cluster). CHECKER: exclusions +61213834/836-839,
+21608263-270 (regex validated: reads excluded, live 61213835/840, 61305503, 61315095 kept).
ROUND VERDICT: no hits, no verified solutions; canary INCONCLUSIVE by harness bug => NEEDS_HUMAN.**

**⚡ 2026-09-24 (Claude session, late) — ASTRA FOLLOW-UP: Q-CLOSURE PRUNE BUILT (default off),
-5.5% MORE ORBITS, ALL PROVABLY EMPTY.** Astra: docs/reviews/2026-09-25-astra-followup.md (prune
sound under a completeness contract; orbit-min ordering = cleanup for the next fresh ordering; no
further involution). WZ_FH_ORBIT_QPRUNE=1 (needs Q; mod-6; untruncated list; only not_realizable or
eq2.12 certificates, else UNKNOWN = kept; CFGSIG .qp1; WZ_FH_QPRUNE_DUMP lists removed raw cells).
n=44: 44,534/803,724 orbits dead (1.0-10.0% per class), all not_realizable, 0 unknown; Q+prune =
759,190 orbits vs 1,460,098 today (-48%). Six + all n=6/8/10 solutions retained (6,564 LOCATE
runs); order-independent; every pruned cell at n=8/10/12 streams 0 candidates (238). Dead cells are
late in flat order (workhorse window quartiles 4245/4860/5380) => current reps never met them.
PRE-REGISTERED on CELLSIZE 61315095: every streamed pi listed in docs/reviews/evidence/2026-09-24/
qprune_prediction_3_13_0_0_w2000-3000.txt must show cand=0 (a nonzero = STOP, prune unsound);
their sec values = per-cell saving. Pass H plan now = Q + prune + orbit-min ordering, one fresh
namespace, after the canary PASS and Daniel's go.**

**⚡ 2026-09-24 (Claude session, close) — OPEN ITEMS + LOOP NOW READS BOTH JOBS.** The loop
(auto_prompt) reads 61315095 (CELLSIZE) and 61331128 (QCANARY) once they leave squeue, saves raw
output to docs/reviews/evidence/, applies the pre-registered rules and reports; any follow-up is
NEEDS_HUMAN. Astra follow-up prompt: docs/reviews/prompts/astra-followup-prompt.md (3 questions: Q-closure
prune soundness, orbit-min cell ordering, any further involution). OPEN, in order: (1) Q canary
verdict -> (2) Pass H build (Q re-tile, .oq1 lanes; driver CKDIR must gain _oq1) on Daniel's go ->
(3) A,B exchange for the 4 a=b classes -> (4) Q-closure prune if Astra confirms -> (5) whole-cell
top-K only if CELLSIZE says BUILD; cheaper stream if KILL/BETWEEN. Daniel's own to-do: delete the 3
stale wildcard allow rules in ~/.claude/settings.json (auto mode blocked me).**

**⚡ 2026-09-24 (Claude session, night) — Q CLUSTER CANARY SUBMITTED: Fir 61331128 (QCANARY,
2 cores, 12 h, isolated $SCRATCH/bs45_qcanary; sha-checked patch onto Fir's cbe3859: solver
3c9b9c46.. == fe7485e, script b1ac6c95..).** n=42 class (7,11,0,0), Pass G env + ORBIT_Q=1, one kept
cell per target: ours42 -> cell 582325 (window 3271, arm 87; holds ONLY a Q-image = the real test),
wz42 -> cell 26105 (window 146, arm 117; old-group images too = secondary). Step 1 WZ_FH_TARGET
(count-only) gives image idx/batch/rank; step 2 real search with DRAIN_TOP=rank+1, DRAIN_BATCHES=
batch+1. PRE-REGISTERED: PASS = step-2 FOUND (then verify_npaf locally; exact idx if the hit is the
target); FAIL = drained set exhausted without FOUND (=> do NOT deploy Q); INCONCLUSIVE = image not
reached in 12 h or (batch+1)(rank+1) > 300k completions (=> redesign, no deploy). Validated locally:
tools/test_target_canary.py (120 small-n canaries) + full script rehearsal at n=10 PASS.
READ (1 tap): ./cluster/deploy/duo_run.sh fir 'cd $SCRATCH/bs45_qcanary && cat qcanary_61331128.txt'
(then pipe the FOUND block to tools/verify_npaf.py). On PASS: build Pass H (Q re-tile) for Daniel's go.**

**⚡ 2026-09-24 (Claude session, evening) — ASTRA MATH REVIEW VERIFIED; QUAD SWITCH Q = 45%
FEWER n=44 CELL ORBITS (built default-off, NOT deployed).** Review:
docs/reviews/2026-09-24-astra-math-review.md; my verification + questions back:
docs/reviews/2026-09-24-astra-review-claude-response.md; evidence in 2026-09-24-evidence/.
ITEM 1 (Q(C,D)=(U+RV,U-RV)): holds on all six known solutions (binary, sums, pair NPAF,
same A,B completes, formula (1), outside the old 32-group). n=44 audit: 1,460,098 -> 803,724
orbits (-45.0%; per class -39.2% workhorse to -48.1% (9,9,0,4)); A,B key Q-invariant in all
10.8M cells. Retention: six solutions keep a witness under the 64-group; brute force of ALL
BS(n+1,n) at n=6/8/10 (1,094 class-orbits) retained in 4,376 LOCATE runs; small-n exact
verdicts identical Q on/off. Code: WZ_FH_ORBIT_Q=1 (refused unless n even and c+d = 0 mod 4),
CFGSIG .oq1 (new namespace), audit + LOCATE extended; tests tools/test_orbit_q.py,
tools/test_orbit_q_retention.py; default-off identity vs c2a3813 still PASS. ITEM 2 (one-sign
extension): CLOSED, 0 extensions from any seed's 4,096-orbit (criterion == direct NPAF in
393,216 attempts). ITEM 3 (A,B exchange, 4 equal-a,b classes): confirmed, queued after Q.
ITEM 4: waits on CELLSIZE 61315095. ITEM 7: brief corrected. NEXT (needs Daniel's go): a
cluster re-find canary with Q on at n=42, then Pass H = Pass G re-tiled over the 64-group
kept list (new .oq1 lanes; Pass G is only 0-10/178 per lane, so little is lost).**

**⚡ 2026-09-24 (Claude session, later) — CELLSIZE MEASUREMENT BUILT + PRE-REGISTERED
(whole-cell top-K gate; Daniel said go).** New default-off mode `WZ_FH_CELLSIZE=cap`: streams
each live cell in the normal order but only COUNTS candidates (no score/buffer/completion),
stops a cell at cap, prints one `CELLSIZE pi= cand= capped= partial= sec= sec_at_buf= leaves=
hall_ok=` line per streamed cell; never reads/writes checkpoints; cannot report a hit.
VALIDATION PASS: tools/test_firsthit_cellsize.py (8 fixtures n<=13 incl. rev/order2/shards/
END/SKIP, 40 runs: stream bytes == normal full-cell run, per-cell counts sum to
candidates_streamed, every cap = exact per-cell prefix, planted ckpt untouched, none
written); tools/test_firsthit_telemetry.py default-off identity vs c2a3813 (63 + 8 resume
pairs) PASS; n29 canary PASS (idx 26694 rank 588 nodes 81320, telemetry 0/64 identical, NPAF verified); tools/test_cellsize_summary.py 11 checks PASS.
JOB (one Fir node, 12 h, isolated dir $SCRATCH/bs45_cellsize so no production lane, CKDIR
or queued compile is touched): CS44g2000 = workhorse (3,13,0,0), Pass G env of lane
F44g2000 (ORDER=1, canon, PROF_SKIP=2000, PROF_END=3000, 178 arms) + WZ_FH_CELLSIZE=4000000
(8x the 500k prefix). Expected ~1-2 cells/arm (stream ~120 cand/s/arm) => ~180-350 cells.
PRE-REGISTERED RULE (tools/cellsize_summary.py; R = cell size / 500k, capped/partial =
lower bounds), over live cells: NO-OP if P(R<1) > 50% (policy already whole-cell for most
cells: do not build); BUILD if P(R<=1.5) >= 50% and P(R>=4) <= 25% (then known-solution
tests before any lane); KILL if P(R>=3) >= 50% or P(R>=8) >= 50% (prefix policy stands;
cheaper stream enumeration is the lever); BETWEEN otherwise (cheaper stream first, then
re-measure); INSUFFICIENT if < 100 live cells (one repeat at k=5000, then stop). Side
outputs: leaves per candidate (stream-cost anatomy for the cheaper-stream lever) and time
to stream 500k (checks my ~1.4 h estimate).
SUBMITTED via duo_run (1 tap): Fir job 61315095 = CS44g2000. Shipped as a 4 KB patch onto
Fir's own cbe3859 files (sha-checked before: solver c2571d40.. driver d9e8d3f7..; after:
patched solver d7a36528.. == pushed e88ea2e). READ when done (1 tap):
`./cluster/deploy/duo_run.sh fir 'cd $SCRATCH/bs45_cellsize && tail -3 firsthit_output_61315095.txt
&& grep -h "^CELLSIZE" fh_arms_61315095/arm_*.log' > docs/reviews/evidence/cellsize_61315095.txt`
then `python3 tools/cellsize_summary.py docs/reviews/evidence/cellsize_61315095.txt`.**

**⚡ 2026-09-24 (Claude session, afternoon) — LOOP FIXES + NEXT-BUILD GATE.
(1) Phone said "fable blocked ()": the CLI's credit message had no "resets" clause, so
limit_reset_note was empty; the fallback push now always states a reason (block_reason:
out of usage credits / usage limit / model unavailable [+ reset]). Tested on the real 09-23
and 09-24 logs. Fable is out of credits (09-23 note: resets Sep 25 11pm); Opus fallback worked.
(2) 1pm run started 13:21:43 with the Mac AWAKE (sleep prevented; no pmset sleep since
09-23 13:13), 16 s after the display woke at 13:21:27; prior days started 13:00:04-05.
Root cause NOT proven (macOS kept no launchd log; pick_claude measured ~1 s, not it).
daily_auto.sh now stamps results/loop_starts.log as its very first action: next late
start shows whether launchd fired late or the script stalled pre-log.
(3) Loop prompt: telemetry pilot CLOSED (no repeat); raw-evidence rule added (the pilot's
verbatim GATEB_TELEM line was never saved, only paraphrased below).
(4) NEXT-BUILD GATE (my derivation from the pilot, approx): 178 arms x 12 h, stream share
47.6% excl. replay, 742 cells => ~1.4 h of stream per cell for a ~584k prefix. Whole-cell
top-K multiplies that by (full cell size / prefix). So whole-cell top-K must NOT be built
before the review's own priority-3 gate: full-cell candidate counts on sampled workhorse
cells (WZ_COUNT_ONLY, capped). Cheaper stream enumeration (<=1.9x ceiling) helps current
lanes AND is a prerequisite if cells are large. Pending Daniel's go.**

**⚡ 2026-09-24 (daily loop 1pm — ALL FOUR reached) — NO HITS, 24 new hitless reads; Fir
restacked 25 (IDs below); TELEMETRY PILOT CLOSED: neither constant-factor build (NEEDS_HUMAN: next build).** NEW FOUND: none on all four.
FIR (5 R + 6 PD): TELEMETRY PILOT 61213832 = F44g1000 READ — 178/178 summarized, 175
interrupted, range_done=3/178, cells 742 (cum_done 742 — the unit began on 3014b95, so
"rate only, not exact alpha"), tested 36.4M, aborts 11.8%. Its GATEB_TELEM line is NOT in
the checker output (the checker grep lacked the token — FIXED today); fetching it via
duo_run, verdict below. 61213833 = F44g0 hitless 318 cells, aborts 5.8%. The 12 Pass-G
SECOND reps (60978552/555/557/559/561/563/565/567/569/571/573/615) all hitless,
range_done=0/178, cells 224-754, aborts 4-29% (all under the 35% line). No CFGSIG/fresh-start
signs: tested_cum > tested and resume_pi > 0 on the cbe3859 reps (resumes intact).
RORQUAL (6 R + 50 PD): 10 R44G (9,9,0,4) reads 21608253-262 hitless; 21608253 range_done=
10/178, 21608254 6/178, rest 0/178; cells 369-597; aborts 19-35% — 21608262 at 35.0% is the
single borderline lane (pooled well under), watch, no action. NIBI 296 PD, TRILLIUM 60 PD:
no reads, floors satisfied, no action. rung_status EXHAUSTED as always (Pass G is the
program; no SA). CHECKER: +GATEB_TELEM/aggregation-FAILED in the firsthit grep; exclusions
+60978552..615 second reps, +61213832/833, +21608253-262 (regex validated).
★ TELEMETRY PILOT VERDICT (61213832, GATEB_TELEM v2 fetched via duo_run): usable read,
178/178 arms, 0 missing/rejected, stats_sufficient=true (full mode), coverage_acceptable,
cells_live_done 742, completions 36.4M, 1/64 histogram 4.4e11 sampled nodes => the pilot is
COMPLETE, NO repeat rep. Phase shares (inclusive / without resume replay): g = 51.7% / 47.6%;
scoring (flat_score) 0.0021% / 0.0019%; buffer sort 0.0004%; completion 48.2% / 52.4%;
replay 7.9%; other 0.07%; late_share 96.8% (charged nodes almost all at the late depths).
Rank deciles: completion cost ~flat across in-drain rank (nodes 3.16e12 -> 2.68e12), aborts
579k in the top decile vs ~390k in the lowest ones. RULES APPLIED: g >= 30% FIRES, but its
score share is ~0.002% => packed flat_score is NOT worth building (g is ~all stream
ENUMERATION, ~7.4 ms of stream work per emitted candidate vs ~0.35 us scoring; 433M streamed to complete
36.4M = 50k top per cell). Completion 48-52% < 70% => joint-reachability benchmark NOT
triggered (despite late_share 97%). => NEITHER constant-factor project. The review's
fallback is selection policy (whole-cell top-K). NEW, not pre-registered: the in-cell
stream enumeration (C,D DFS + 2.11b/2.12 filters) is ~half of worker time, so a cheaper
stream would be a ~1.9x ceiling lever. NEEDS_HUMAN: Daniel picks the next build.
FIR RESTACK (pending 6 < 8; all 12 workhorse G lanes unfinished, range_done 0-3/178): 25
singleton reps, verbatim sacct SubmitLine env (no telemetry), rrg-ikotsire_cpu, --mem=0,
bringing EVERY lane to 3 queued: F44g0 x3 = 61305503/504/505; F44gr0 61305506, 61305517;
then x2 each: F44g1000 507/519, F44gr1000 508/520, F44g2000 509/521, F44gr2000 510/522,
F44g3000 511/523, F44gr3000 512/524, F44g4000 513/525, F44gr4000 514/526, F44g5000 515/527,
F44gr5000 516/528 (all 613055xx; 61305518 is not ours). Fir now 5 R + 31 PD.
ROUND VERDICT: no hits, no verified solutions; telemetry pilot closed (neither constant-
factor build); Fir restacked 25; NEEDS_HUMAN = next-build decision only.

**⚡ 2026-09-23 (Claude session, evening) — BUNDLED REDEPLOY PREPARED, NOT YET DEPLOYED.
(1) Codex's telemetry review fixes committed (664078c) and merged to main (b6340e6):
resume-replay timing separated (FH_TELEM v2), aggregator per-arm rejection + <=3 missing
arms, recovered n29 canary config in the harness; validation PASS (63 identity + 8 resume
pairs, 16 aggregator tests, n29 baseline/0/64 = idx 26694 rank 588 nodes 81320).
(2) Lever 28 flipped DEFAULT ON (controls passed with exact node counts); n=19 fixtures
default == on == off on verdicts, hits, backtracks, aborts and charged nodes.
(3) Loop now uses the newest Claude CLI and aliases fable -> opus (3826a33).
DONE (same evening, 4 taps): fleet redeploy of the pinned main sha (solver + driver + tools/
aggregate_firsthit_telemetry.py) to all four clusters — checkpoint-compatible (CFGSIG
unchanged), brings early check ON + cum accounting; plus refill Fir (pending 5 < 8) and
substitute ONE pending Fir workhorse G rep with WZ_FH_TELEMETRY=64 (cap: that rep + 1 repeat).
RESULT: cbe3859 deployed + grep-verified on Fir, Rorqual, Nibi, Trillium (solver, driver,
aggregator; Fir python3 3.11.4). Fir queue had drained to 5 R / 0 PD => 13 submitted:
TELEMETRY JOB 61213832 = F44g1000 (WZ_FH_TELEMETRY=64, first in its lane) + one restack
rep for each of the 12 workhorse G lanes (F44g/F44gr 0..5000). Queues after: Fir 5 R +
13 PD, Rorqual 9 R + 56 PD, Nibi 296 PD, Trillium 60 PD. Loop: read GATEB_TELEM for
61213832 when it lands; apply the pre-registered rules (review 2026-09-22, section 4).**

**⚡ 2026-09-23 (Codex, Claude telemetry review fixes; NOT DEPLOYED):** Schema-v2
`FH_TELEM` now separates resumed-cell replay, including the saved buffer's re-sort,
and `GATEB_TELEM` reports inclusive and replay-excluded phase shares. Search flags,
CFGSIG and checkpoint format are unchanged. The aggregator preserves good arms,
reports rejected records with reasons, and permits <=3 missing/rejected arms per rep
with an explicit bias warning. Local validation: 63 identity comparisons, eight
resume pairs and 16 aggregator tests PASS; corrected n29 rerun pending below.
**Recovered n29 command (no other WZ_* settings):**
`WZ_FIRSTHIT=1 WZ_FH_M6=1 OMP_NUM_THREADS=1 ./bin 29 0 6 9 1`.
Canon off, order 0, budget 200k; expected idx=26694, profile_rank=588,
nodes_this_cand=81320. The harness now uses this archived stream, separately from
small-n fixtures. Its counter validator reads the terminal summary, not a periodic
progress counter. Claude's original n29 gate already passed all telemetry modes.
**Pilot:** mode 64 only, ONE pending Fir workhorse Pass G rep substituted with the
same name/env/singleton/CKDIR, plus at most one repeat (one node-day total). Keep the
>=500-cell, >=25k-completion, >=1e9 sampled-node gate; two incomplete reads stop.
Same-node overhead gate applies before wider use. No local n44 fallback. Lever-28
controls were read PASS by today's loop below; record the deployed SHA because a
shared-tree Fir deploy changes every subsequently compiled queued job. No remote
deploy, cancellation, submission or early-check default flip done here. Work is
isolated on `codex/n44-telemetry` at `/tmp/bs45-n44-telemetry`; shared checkout stays
on `main`. Full handoff: `docs/reviews/2026-09-23-telemetry-review-fixes.md`.
This entry supersedes the earlier instrumentation entry's open n29/configuration
and pilot-overhead requirements.**

**⚡ 2026-09-23 (daily loop 1pm — ALL FOUR reached) — ★ LEVER 28 CONTROLS PASS →
NEEDS_HUMAN for the pre-declared bundled fleet redeploy. No hits, no records; bookkeeping +
0 submits.** Fir's two lever-28 controls READ and BOTH PASSED the pre-registered correctness
gate: 60980457 = F41ec BS(42,41) FOUND with nodes_this_cand=212872 EXACT (expected 212872) and
60980458 = F43ec BS(44,43) FOUND with 88616 EXACT (expected 88616). Identical node counts =>
the early-outer-correlation check reproduces the search byte-for-byte (correctness-preserving);
both banners are EXPECTED re-finds of banked rungs (sig (0,2,9,9) score 124 / (8,-2,5,9) score
130), NOT news, not banked. Elapsed 8299.7s / 7574.86s are on different nodes => informational
only, establish NO speedup (doctrine: speed needs a paired same-node run). ACTION per the loop's
lever-28 rule: since no sha with `EARLY_CHECK default on` exists yet (Daniel's session flips it),
I set the flag on NOTHING and deployed nothing. This PASS is the exact trigger Daniel pre-declared
for the bundled "one fleet redeploy" (HANDOFF 09-22 night: accounting counters cum_*/cells_empty +
lever-28 default flip) — a code deploy = human step. **NEEDS_HUMAN: flip WZ_FH_EARLY_CHECK default
ON, bundle the cum accounting build, redeploy fleet-wide (checkpoint-compatible; CFGSIG unchanged).**
NEW FOUND: none on all four. PER-CLUSTER: Fir 7 R + 5 PD Pass-G (3,13,0,0) S=1000 — 12 first reps
read ALL HITLESS, range_done=0/178, cells_done_sum 180-700, tested 10-38M/lane, aborts ~15-30%
(second reps 60978552/555/557/559/561/563/565 R + 567/569/571/573/615 PD still live); Rorqual 1 R
(21608253 = R44Gg1500) + 65 PD Pass-G ((9,9,0,4)=R44G, (3,5,0,12)=R44H, S=300) — 10 first reps
21608243-252 HITLESS range_done=0/178 tested 5-27M/lane + 21122874 = R44f3664 drain hitless; Nibi
296 PD (0 R, no reads — ≥100 RRG floor satisfied, ~13d waits expected, no alarm); Trillium 60 PD
Pass-G (classes A/K/L, workhorse-first) 0 R, no reads (2309938-954 = 9 dead T44Af header-only,
cancelled at the Pass-G launch). NO G LANE IS DONE (all range_done=0/178) => nothing to promote to
G2, nothing to "never resubmit"; workhorse S=1000 windows are deep, restack-until-done as designed.
5e6 PAIR VERDICT CLOSED: 59818967 = F44f2728 (5e6) 483 cells vs ~607 for its 2e6 neighbours ≈ 80%,
below the 90% switch line => KEEP 2e6 fleet-wide (aborts 4.5% pass the ≤5% half, but throughput
fails; confirms the ledger's "5e6 FAIL"). NO SUBMITS: no cluster idle (Fir 12 / Rorqual 66 / Nibi
296 / Trillium 60 queued); Fir pending 5<8 but Fir has ~1.5-2d runway (not idle) and Daniel's
imminent bundled redeploy will refresh the fleet binary (checkpoint-compatible, CFGSIG unchanged) —
I HELD the Fir restack so its next reps carry the new cum-accounting build, rather than restacking
verbatim today with a CFGSIG-mismatch risk (did not retrieve the exact prior G submit command);
tomorrow's loop restacks Fir if Daniel hasn't refilled it by then. CHECKER
EXCLUSIONS updated (operative regex + annotation both, validated 34 excl / 0 keeper-false-match):
+60980457/458, +Fir G first reps (551/553/556/558/560/562/564/566/568/570/572/574),
+59818967, +60191241-261, +60671643-666, +60795778-810, +21608243-252, +21122874, +2309938-954.
ROUND VERDICT: no hits, no verified solutions, 0 new lanes queued; lever 28 PASS => NEEDS_HUMAN
for the bundled fleet redeploy (early-check default ON + cum accounting).**

**⚡ 2026-09-23 (Codex, instrumentation for Claude review; NOT DEPLOYED):** Implemented
opt-in `WZ_FH_TELEMETRY=1|64` phase timers, charged-depth histogram and ten rank-cost
buckets, terminal `FH_TELEM` JSON plus validated `GATEB_TELEM` aggregation. No search
policy/default/checkpoint changes. 63 baseline/off/full/sampled n<=13 comparisons and
two interrupt/resume pairs matched streams, verdicts, counts and checkpoint bytes;
8 aggregator tests passed. Local 41-pair timing screen ~0.4% median overhead, NOT a
production claim. **n29 gate OPEN:** unmodified baseline timed out at 180s before the
new binary was compared; no local timeout extension. No deployment until Claude review,
n29 validation and representative same-node overhead gate. Branch `codex/n44-telemetry`;
details/results: `docs/reviews/2026-09-23-telemetry-implementation.md`. Loop now uses last
complete cum_* endpoint per unit (never sums jobs); old units lack historical cells_empty
and cannot yield exact alpha_front. Lever-28 controls remain unread here; corrected the
loop to treat them as correctness gates, with speed judged by paired same-node runs.
No SSH, queued-job changes or n44 local profiling. Measurement-first supersedes the
earlier shadow-instrument priority; direct joint-prune A/B is deferred until profiling.**

**⚡ 2026-09-22 (Daniel session, night) — ASTRA SPEC SAVED (docs/reviews/2026-09-22-astra-
instrument-spec.md) + ACCOUNTING COUNTERS SHIPPED: cells_empty + cumulative cell counters
in the checkpoint (backward compatible) + driver GATEB cum_* sums, validated locally
(python harness — NOTE: two "hung" test rounds today were the zsh `env $E` trap, not the
solver; memory updated). Not yet deployed: bundle with the lever-28 default flip after
F41ec/F43ec read (one fleet redeploy). Shadow instrument for joint reachability = next
build. Pass G lanes running on the 3014b95 build: their per-rep summaries must be summed
by the loop until the cum build is deployed.**

**⚡ 2026-09-22 (Daniel session, late) — ASTRA FOLLOW-UP: LEVER 28 SHIPPED (gated). Early
outer-correlation check in the completer (reject a quad from 4 signed products before
placing; node charging moved before placement => budget semantics identical). Local
differential: 5 fixtures (n=19, budgets 50/300/5e7) verdicts, hit idx, backtracks,
aborts, charged nodes IDENTICAL on/off. Deployed 9b6d117 to Fir with the check DEFAULT
OFF; controls submitted: F41ec (n=41 skip-8 canon-off 2e6, expect nodes_this_cand
212872, < 3.1 h) and F43ec (n=43 w327 K=50k 2e6, expect 88616, < 3.8 h). PASS => flip
default on, redeploy fleet-wide. Also recorded: Astra's expected-time model (alpha/
Lambda unidentified — no calendar ETA), joint-reachability measurement plan (backlog),
full-group equivalence invariant (our 3 solutions inequivalent to WZ's under all 4,096
transformations), Pass G = 444 units (+12 workhorse second reps).**

**⚡ 2026-09-22 (Daniel session) — EXTERNAL REVIEW (Astra) FOUND TWO REAL DEFECTS; BOTH
VERIFIED HERE AND FIXED IN SOURCE (deploy pending local test C). (1) CORRECTNESS: the
zero-sum endpoint pins (C[0]=+1 when sum C=0, D[0]=+1 when sum D=0) conflict with orbit
canonicalization — inside a kept cell the sign of C/D is fixed by the profile, so each
active pin discards ~50% of orbits. Verified: WZ-42's kept-cell reps all start (-1,-1)
(unreachable under canon+pins; ours-42 survived by luck); n=6 (5,1,0,0) canon on: OLD
binary zero candidates, FIXED binary FOUND. IMPACT: (3,13,0,0) both pins active => the
workhorse has been searched at ~25% orbit coverage since 08-05; (9,9,0,4)/(3,5,0,12)
~50%; other classes unaffected. FIX: pins off under canon, CFGSIG ".np1" only when a
pin would have been active (affected lanes fresh-start; n=29 canary byte-identical,
CFGSIG unchanged). (2) EFFICIENCY: lanes had no ownership — an arm ran from its skip to
the END of the list; kept live cells are ~1 per 300-500 raw positions on the workhorse,
so one rep advanced 500-1,500 raw windows (telemetry: Nibi FR skips ~720 -> resume
windows 1,260-2,234) while lanes were placed every 8 => Pass F/FR ~99% redundant.
FIX: WZ_FH_PROF_END (owned range [skip,end), RESULT: RANGE EXHAUSTED, GATEB
range_done=k/178). NEW PLAN = PASS G (docs/plans/lever19_sweep_plan.md): workhorse front =
6 forward + 6 reversed lanes of S=1000 windows (~30 lane-reps total) instead of 730
lanes. Astra's other items (early outer-correlation check 1.4-1.6x, joint
reachability H-transform, bit-packed scoring, extra symmetry) in the backlog with
re-find gates; its corrections to our percentile/rank claims accepted. VALIDATED (tests A-C: n=6
old binary zero candidates vs fixed FOUND; n=29 unaffected class byte-identical, CFGSIG
unchanged; ownership: RANGE EXHAUSTED at the bound, resume = done, neighbour disjoint,
no-END unchanged). DEPLOYED 3014b95 ON ALL FOUR + PASS G LAUNCHED (4 taps): Fir 24 jobs
(workhorse 6 fwd + 6 rev x 2 reps, S=1000; 17 old F44f reps finishing, not restacked),
Rorqual 76 lanes ((9,9,0,4)+(3,5,0,12), S=300, fwd+rev; 31 stride-8 cancelled), Nibi
296 lanes ((1,7,8,8),(5,5,8,8),(5,11,4,4) S=300; (3,3,4,12),(7,7,4,8),(5,7,2,10) S=150;
fwd+rev; 301 workhorse FR lanes cancelled), Trillium 60 lanes ((5,9,6,6),(7,11,2,2),
(1,13,2,2), S=300, fwd+rev; 9 T44Af cancelled). = the ENTIRE 12-class front tile at
K=50k, ~456 lane-units, first time at 100% orbit coverage with no repeated work.
Loop rules: restack a G lane until range_done=178/178, then G2 on finished ranges.**

**⚡ 2026-09-22 (daily loop 1pm — nibi/rorqual/trillium reached; FIR IN A LISTED POWER
OUTAGE on status.alliancecan.ca, no push sent) — NO HITS, 35 new hitless Nibi FR reads,
NO submits (all floors satisfied), bookkeeping-only round. NIBI READS: the watch-listed
crowded-out 24 surfaced (21997843-866 = N44fr528..712, 21.6-22.9M tested/lane, 178/178,
aborts 11-14%, cells 370-400) + 11 newer (21997929-939 = N44fr1200..1280, 22.9-23.4M,
aborts 16-18%, cells 389-404). Both carry the lever-23 wall split and both CONFIRM the
FAIL: walled-vs-unwalled cells/lane +1.8% (387.8 vs 380.8) and +0.5% (398.8 vs 396.8),
far under the +15% line — verdict stays closed, no wall on any submit. NEW FOUND: none
(the 21707091 = N43b2e6 control banner is correctly excluded). QUEUES: Nibi 0 R + 301 PD
(>= 300 allocation-burn floor by exactly 1 — the next completions drop it below, expect
FR tranche 3 next run; next FR k=3200, extendable to ~4136); Rorqual 1 R (21122874 =
R44f3664, started 12:02 PM, ~1h in — squeue %L is time LEFT, 11:01 shown) + 32 PD;
Trillium 9 PD repair lanes, empty pre-start outputs, no reads. rung_status EXHAUSTED as
always (Pass F is the program). CHECKER FIX: the 09-19-session reads (21085451-487,
21122830-840 even, 21705841/2, 21707091, 21997839-842) were documented in the header
text but NEVER added to the operative grep -vE — 78 stale files re-showed every check
and burned the 250-file cap. Added them + today's 35 to the operative pattern
(regex validated: 23 processed IDs drop, live/future IDs incl. strays 21997872/894
kept). Checker exclusions: +21997843-866, +21997929-939, +the 09-19 set above.
WATCH NEXT: Fir post-outage — tranche-8 tail 60671643-665 reads AND confirm the 09-21
tranche-9 submits 60795778-817 survived the outage (they echoed job IDs pre-outage);
Rorqual R44f3664 (~midnight); Nibi admissions burst; strays 21997872/894.
ROUND VERDICT: no hits, no verified solutions, 0 new lanes queued (nothing needed).**

**⚡ 2026-09-21 (daily loop 1pm — ALL FOUR reached) — NO HITS, 111 new hitless reads;
★ LEVER 23 VERDICT: FAIL. Nibi's FIRST 60 FR reads landed (21997867-928 minus 872/894 =
N44fr720..1192, ~22.3-24.0M tested/lane, 178/178, aborts 17-22%, cells_done_sum 383-427)
and settle the pre-registered WALL_SEC split: walled half (k=0 mod 16) 400.0 cells/lane
vs unwalled (k=8 mod 16) 399.5 = +0.1%, needed >= +15% => WZ_FH_WALL_SEC=900 is NOT
adopted; no wall on any future submit. (Mapping from queue anchor 21997929=N44fr1200 with
two non-FR ID gaps 872/894; verdict is parity-robust — flipping halves gives 399.5 vs
400.0. Caveat: a wall that never triggers is indistinguishable from a neutral wall here;
either way no gain.) The 24 older FR lanes 21997843-866 = N44fr528..712 completed but
were crowded out of the 60-head cap — surface next check now that these 60 are excluded.
Nibi queue 12 R (21997929-939 = N44fr1200..1280) + 202 PD < 300 allocation-burn floor
=> FR tranche 2 submitted this run (see below). Fir: tranche 7 COMPLETE (60191262-280 =
F44f3864..4008 hitless ~30-31.4M, aborts 26-29% — creeping up from ~21-23%, still under
the 35% line, watch it) + 16 tranche-8 heads (60671627-642 = F44f4016..4136 hitless
~30.0-30.7M, aborts 27-29%, cells 597-609); queue 23 R + 1 PD => pending < 8, PASS F
FIRES => tranche 9 submitted this run (see below). Rorqual: 16 new hitless reads
(21122842-872 even = R44f3408..3648 12h group, ~24.4-26.5M, aborts 25-28%, cells
419-490); 32 PD => no action. Trillium: 9 PD repair lanes (empty pre-start outputs), no
reads, no action. rung_status EXHAUSTED as always (Pass F is the program). Checker
exclusions: +60191262-280, +60671627-642, +21997867-928 (minus 872/894), +21122842-872
even. Watch next reads: N44fr528..712 (the crowded-out 24), N44fr1200..1280, Fir
tranche-8 tail 60671643-665, stray IDs 21997872/894 if they ever show output.
FIR SUBMITS: 40/40 echoed — 60795778-817 = F44f4336..4648 s8 @2e6 (rrg-ikotsire_cpu,
--mem=0, singleton, NO wall), QUEUE_COUNT 63 post-submit. Workhorse F tile assigned to
k=4648 (80% of 5836); next unassigned: workhorse k=4656.
NIBI SUBMITS: FR tranche 2, 100/100 echoed — 22411431..22411533 (IDs non-contiguous,
3 foreign gaps) = N44fr2400..3192 s8 @2e6 (STREAM_REV=1, rrg-ikotsire_cpu, --mem=0,
singleton, NO wall per the lever-23 FAIL), QUEUE_COUNT 312 post-submit => >= 300
allocation-burn floor restored. FR tile assigned to k=3192; next FR k=3200 (F pass
completed through 4136, so FR can extend to ~4136 before waiting on the F tile).
ROUND VERDICT: no hits, no verified solutions; 140 new lanes queued (Fir 40 + Nibi
100); lever 23 closed FAIL.**

**⚡ 2026-09-20 (Daniel session) — BUTTON FLOW WORKED (1pm missed Fir -> "fir unread" note ->
Daniel's tap ran ONLY Fir -> read captured), but the Fir agent hit the new 90-min cap
with ZERO output. ROOT CAUSE FOUND + PROVEN: `claude -p` blocks until stdin EOF (25 s
with an open pipe vs 2 s with </dev/null); the listener launched daily_auto inside its
`curl | while read` stream, so every button-run agent inherited the hour-long stream as
stdin and idled until it ended (explains the 3-6 h supplementary runs on 09-19 too).
FIXES: listener launches with </dev/null + nohup env; daily_auto runs the agent with
</dev/null; rc=124 now says "time cap" (not "API dropped"); results/interim_summary.txt
untracked (its rm by the loop faked "agent acted" => bogus PARTIAL commits); NEW
summarize_check.py = deterministic digest (FOUND count outside the exclusion header,
per-cluster R/PD, new reads with tested/abort/cell ranges, REFILL flag) sent to the
phone right after EVERY check ("BS45 check: numbers") and used as the fallback text when
the agent fails; reminders: the 1pm run's remind_unread.sh was killed by launchd on job
exit => spawn_detached.py (new session) + AbandonProcessGroup=true on bs45check.plist
(verified: detached child survives its parent's group kill). READS: Fir 14 new tranche-7
lanes hitless (60191248-261, 31-33M, 23-26% aborts, cells 601-621); Fir pending 3 => Pass
F tranche 8 SUBMITTED (F44f4016..4328, 40 @2e6; Fir 14 R + 40 PD; F tile 74%). Nibi:
24 FR lanes finally RUNNING (first admissions in ~2 weeks) — first FR reads + lever-23
split verdict tomorrow. Rorqual 16 R + 33 PD, Trillium 9 PD. Both daemons reloaded.**

**⚡ 2026-09-20 (daily loop 1pm — Nibi/Rorqual/Trillium reached, Fir Duo MISSED) — NO HITS;
★ NIBI FINALLY ADMITTED: 24 FR lanes RUNNING (21997843-866 = N44fr528..712, started ~06:00
EDT, 7-8h elapsed at check) — the FIRST FR starts after ~2 weeks of zero admissions,
vindicating the 09-19 revert to 12h whole-node lanes. Header-only outputs so far; first
Nibi FR reads expected tomorrow, and they carry the LEVER-23 split verdict (k=0 mod 16
walled WZ_FH_WALL_SEC=900 vs k=8 mod 16 unwalled — both halves are in the running set;
compare cells_done/lane, >= +15% => wall fleet-wide). Nibi queue ~265 PD (21997867-877 +
21997878-21998080 + 22302012-073 = the reverted 12h stack) => >= 100 rule satisfied, no
action. Rorqual: ONE new read 21122840 = R44f3392 HITLESS (25.4M tested, 178/178, aborted
6.68M = 26% — under the 35% line, cells_done_sum 457); 16 R (21122842-872 even =
R44f3408..3648 12h group, 2.5-4h in) + 33 PD (21434125-155 = the reverted-to-12h
R44f3224..3688 k=8-mod-16 lanes + 21122874/876) => pending >= 8, no submits; the 12h
group keeps admitting (16 starts today) while the age-reset resubmits wait — consistent
with the 09-19 lever-24 FAIL, nothing to decide. Trillium: 9 PD repair lanes
(2309938/939/943/945-949/954, empty pre-start outputs), no reads, pending >= 8, no
action. Fir: Duo missed — tranche-7 head reads (60191247-252 window) roll forward; queue
was 6 R + 28 PD on 09-19 evening so no refill is plausible-needed. rung_status EXHAUSTED
as always (Pass F is the program). Checker exclusions: +21122840. Watch next reads:
N44fr528..712 first Nibi FR data + lever-23 split verdict, Rorqual R44f3408..3648 group,
Fir tranche-7. ROUND VERDICT: no hits, no verified solutions, no new lanes queued.**

**⚡ 2026-09-19 (Daniel session, evening) — WHY TODAY BROKE + FIXED; ALL READS PROCESSED.
Root causes: (1) the 09-13 "button runs only what's left" patch NEVER reached
ntfy_listener.sh (assertion aborted the script before that edit; mtime stayed 09-10) —
so Daniel's 13:26 tap launched a FULL 4-cluster run while the 1pm agent was mid-flight;
(2) no lock between the 1pm run, button runs and the auto-spawned supplementary => 3
agents on one repo, two died after 4-6 h (API timeouts), no reads processed. FIXES
(ff1a939): run_lock.sh = ONE global lock for every daily_auto (cron waits 30 min,
button waits 2 h then re-derives what is still unread; taps during a run are queued
with a phone note); listener rewritten (what's-left for real, BUTTON=1 path); hourly
REMINDERS (remind_unread.sh, no Duo pushes) replace the unattended re-pusher — pushes
now come ONLY at 1pm or on a tap; agent hard cap 90 min (run_with_timeout.py, rc 124);
duo_ssh two-phase deadline (approval budget, then 900 s command budget) so a slow tap
cannot truncate a read; checker RETRY_MAX default 0. Listener restarted, stray runs
killed. FULL CHECK RUN BY HAND (4 taps): 56 reads, 55 hitless + 21707091 = N43b2e6
lever-21 Nibi control RE-FIND of the banked n=43 (3.8 h at 2e6 vs 4.9 h; PASS, not
news, excluded). VERDICTS: 2e6 stays (5e6 lane 59818967 = 483 cells/5% aborts vs ~600/
24% => 80% < 90% line, FAIL); LEVER 24 FAIL on Nibi (0/60 3h + 0/12 split started in
72 h vs 4/234 12h) AND Rorqual (0/30 3h vs 12/29 12h in 48 h) — admission is priority-
ordered and cancel/resubmit lost queue age; ALL reverted to 12 h whole-node (Nibi 296
lanes, Rorqual 48 PD + 1 R, same names => resume). Fir 6 R + 28 PD (tranche 7 first
reads 32M/lane, 24-25% aborts, 574-609 cells), Trillium 9 PD + 9 running. Nibi's 09-03
Pass F set (21085451-487, 37 lanes at 5e7) finally read: 18-24M/lane, 286-423 cells,
hitless. Levers ledger: 25 opened; 12 shipped; 15 dead (24, 25, 5e6 added).**

**⚡ 2026-09-19 (supplementary, Fir re-check ~17:29 — AND the day's only surviving agent
run) — THERE IS NO MAIN 09-19 ENTRY: the 1pm main-run agent died on a 4h API "Request
timed out" (rc=1, commit 16cb5f9 = PARTIAL, only deleted the stale interim file), and the
13:44 supplementary agent for nibi/rorqual/trillium ALSO died silently (no exit logged, no
commit). Today's Nibi/Rorqual/Trillium reads were NEVER processed and their check output
was overwritten — they are NOT excluded, so they re-show as NEW at the next successful
check (self-healing; tomorrow's watch items: lever-24 Rorqual 48h verdict, lever-25 Nibi
split-lane 24h read, any Nibi first start). FIR: reached, but the capture was TRUNCATED
mid-stream right after the ~21.5KB FIRSTHIT header — queue block only, ZERO read
summaries. Queue 34 = 6 R (60191247-252 = F44f3744..3784) + 28 PD (60191253-280 =
F44f3792..4008) => pending >= 8, PASS F does not fire, NO submits. 7 lanes left the queue
since 09-18 and are COMPLETED BUT UNREAD: 60191241-246 = F44f3696..3736 (tranche-7 heads)
+ ★ 59818967 = F44f2728, the SECOND 5e6 pair lane — the pre-registered 5e6-vs-2e6 budget
verdict STILL rolls forward (not excluded, re-shows next check). No exclusions added this
round (nothing read). ROOT CAUSE of the truncation (diagnosed from source):
duo_ssh.py's single deadline spans Duo-approval + remote command, so a late approval
leaves seconds for the checker to run and the pty capture is cut mid-output; AND
check_all_retry.sh's check_one() treats any non-empty body as "reached" (it never checks
duo_ssh's exit status / the END marker), so a truncated capture silently masquerades as a
complete read. Fix (idle-deadline after auth + loud TRUNCATED banner) committed on branch
auto/2026-09-19 for Daniel's review — NOT merged, tomorrow runs on the old driver unless
merged. SUPPLEMENTARY VERDICT: no hits observed (but 7 fir lanes unread), no verified
solutions, no new lanes queued; day's real story = two agent deaths + a checker-capture
bug now diagnosed with a review-ready fix.**

**⚡ 2026-09-18 (daily loop 1pm — ALL FOUR reached) — NO HITS, bookkeeping only, no
submits (every cluster pending >= 8). Fir: 12 NEW hitless reads 59818955-966 —
F44f2632..2712 @2e6 (~32.0-32.7M tested/lane, 178/178, aborted ~6.65-6.98M = ~21%,
cells_done_sum 602-612) + ★ FIRST 5e6-PAIR READ 59818966 = F44f2720: 26.2M tested,
aborted 1.15M = 4.4% (PASSES the <=5% bar) BUT cells_done_sum 491 = 81% of its 2e6
neighbours' ~607 (BELOW the 90% line) => provisional KEEP 2e6; pre-registered pair
verdict is FINAL only when 59818967 = F44f2728 (still PD) reads. Fir queue 41 PD 0 R
(tranche 7 60191241-280 + the 5e6 PD lane). Rorqual: 5 NEW hitless reads 21122820/822/
824/826/828 (12h group, ~25.6-25.9M, 178/178, aborts ~25% — under the 35% line);
LEVER 24 RORQUAL ~24h READ: 12h control 10/29 started (5 read + 21122818 yesterday +
4 R now: 21122830/832/834/836), 3h resubmits 21275347-379 0/30 started => NOT PASS,
trending AGAINST 3h there (confound noted: resubmission reset queue-age priority);
formal 48h verdict tomorrow — if still 0/30 vs 12h progress, lever 24 FAILS on Rorqual
and the 3h lanes should be left to drain, not extended. Queue 4 R + 55 PD. Nibi: NO
reads, 0 R, ~349 PD (29 N44f + ~238 FR 12h + 60 FR 3h 22096724-787 + 12 split-lane
22154359-70 + N43dt/N43b) — lever 24 AND lever 25 both 0 starts in EVERY group at
~24-48h, verdicts roll to the 48h/next read; >= 300 queued rule satisfied, no action.
Trillium: no new reads, 9 PD (released repair lanes, empty pre-start outputs) =>
pending >= 8, no action. rung_status EXHAUSTED as always (Pass F is the program).
Checker exclusions: +598189(5[5-9]|6[0-6]), +2112282[02468]. Watch next reads: 5e6
pair verdict (59818967), Rorqual lever-24 48h fractions, Nibi any first start,
N43b2e6, Rorqual R44f3224..3688 3h lanes. ROUND VERDICT: no hits, no verified
solutions, no new lanes queued.**

**⚡ 2026-09-17 (Daniel session, later) — PROF: fast-track RRG renewal, "run as many jobs
as you can handle, essential to spend/overspend the allocation" => Nibi consumption is
now an objective (renewal evidence). Supply is not the bottleneck (339 lanes queued, 0
admitted in 48 h); ADMISSION is => LEVER 25 TEST LIVE: N44fr480/488 resubmitted as 12
split-lane jobs (30 cores, 3 h, --mem=12G, FH_SHARD_LO/HI ranges, names N44fr<k>s0..5,
partition auto = cpubase_bycore_b1+backfill). Read at 24 h vs the whole-node 3 h (60)
and 12 h (275) groups; PASS => convert Nibi to split-lane small jobs, keep >= 300-1000
queued. Loop rules updated (allocation burn; sum 6 shard outputs per lane). Reply to
the professor drafted (thanks + jobs queued + short-job testing).**

**⚡ 2026-09-17 (Daniel session) — RORQUAL SLOWED TOO (0 R / 59 PD on Priority; cluster
673/678 nodes allocated; account LevelFS 0.29, user 0.33 = our RAC fairshare is depleted
after two weeks of heavy use — same picture as Nibi). LEVER 24 EXTENDED TO RORQUAL for
a faster verdict: driver 4d453d1 (TimeLimit-aware deadline) deployed; alternating
pending R44f lanes (30) cancelled and resubmitted at --time=3:00:00 (same names/CKDIRs,
2e6), 29 stay 12 h. Read rule: started fraction per group (Rorqual admits work in hours,
so this discriminates within a day; Nibi's split showed 0/0 at 24 h). Fir is now the
only fast cluster (tranche 7 queued, F tile 69%); Trillium class-A aborts 24-31% (under
the line; 5e6 pair verdict reads tomorrow). No hits. In test: 5e6 pair (Fir), WS split
(Nibi FR), 3 h backfill (Nibi 60/240 + Rorqual 30/29).**

**⚡ 2026-09-17 (supplementary — Rorqual re-check after the 1pm Duo miss) — NO HITS,
bookkeeping only. 9 NEW hitless reads: 21122818 = R44f3216 FIRST read of the 09-15
tranche (~25.9M tested, 178/178, aborted 6.51M = ~25% of tested — under the 35% line;
cells_done_sum 511) + 20995030-37 = the 2e6-resubmit window lanes' SECOND reps
(~27.7-28.7M this rep, cum ~51.1-51.6M, 178/178, aborted ~2.4-3.1M = ~9-11%, dedup
28.92x headers healthy). Queue 59 PD (21122819-877 = R44f3224..3688, Priority, 0 R)
=> pending >= 8, PASS F rule does NOT fire, no submits. Checker exclusions: +21122818,
+209950(3[0-7]). SUPPLEMENTARY VERDICT: no hits, no verified solutions, no new lanes;
Rorqual restack 21122818-877 now observed and healthy.**

**⚡ 2026-09-17 (daily loop 1pm — Fir/Nibi/Trillium reached, Rorqual Duo MISSED) — NO
HITS; Fir Pass F refill is the round's action. Fir: 25 NEW tranche-6 2e6 reads
59818930-954 = F44f2432..2624 HITLESS (~31.2-35.0M tested/lane, 178/178, aborted
~5.6-7.5M = ~17-23% of tested — expected under the 09-13 rule, under the 35% line;
cells_done_sum 566-657); queue 13 = 12 R (59818955-966 = F44f2632..2720, incl the 5e6
pair lead F44f2720 started overnight) + 1 PD (59818967 = F44f2728 5e6) => pending < 8,
PASS F RULE FIRES: tranche 7 = F44f3696..4008 s8 (40 @2e6, rrg-ikotsire_cpu, --mem=0);
5e6-vs-2e6 pre-registered verdict rolls forward (neither 5e6 lane read yet). Trillium:
15 NEW class-A 2e6 reads 2309940-942/944/950/952/953/955-962 = T44Af320..488 subset
HITLESS (~23.9-27.1M tested, 178/178, aborted ~6.2-8.2M = ~24-31% of tested — the
class-A baseline creeping up but under the 35% line); queue 9 PD = the 9 released
repair lanes (2309938/939/943/945-949/954, healthy Priority, empty output files
pre-start) => pending >= 8, no action. Nibi: NO reads, 0 R + 339 PD healthy; LEVER 24
READ AT ~24h: 3h group 0/60 started, 12h group 0/240 started — no discrimination yet,
verdict rolls to the 48h window. Rorqual: Duo missed (no approval in 180s), no data —
its 60 PD restack 21122818-877 unobserved this round. rung_status EXHAUSTED as always
(Pass F is the program). Checker exclusions: +598189(3[0-9]|4[0-9]|5[0-4]),
+230994[0-24]+23099(5[023]|5[5-9]|6[0-2]). Watch next reads: 5e6 pair F44f2720/2728 vs
2e6 neighbours, lever-24 started fractions at 48h (PASS => convert Nibi 12h lanes to
3h x3 singleton reps), WS split cells_done/lane when Nibi FR reads, N43b2e6, Rorqual
R44f3216..3688 first reads. FIR SUBMITS: 40/40 echoed — 60191241-280 = F44f3696..4008
s8 @2e6 (rrg-ikotsire_cpu, --mem=0, singleton), QUEUE_COUNT 53 post-submit. Workhorse
F tile now assigned to k=4008 (69% of 5836); next unassigned: workhorse k=4016, A
k=496. ROUND VERDICT: no hits, no verified solutions; 40 new lanes queued (Fir only —
Trillium/Nibi pending >= 8, Rorqual unreached).**

**⚡ 2026-09-16 (Daniel session) — NIBI DIAGNOSED + LEVER 24 LIVE. All 339 Nibi lanes
pending on Priority: cluster 680/699 nodes allocated (0 idle), competing jobs at priority
6.0M vs ours 1.35M, user-level LevelFS 0.34; last Nibi lanes waited 13 DAYS (submitted
09-01, started 09-14) even under the RRG => Nibi is the slowest cluster for us, not the
fastest; nothing broken (jobs valid: 192 CPU, mem=0, bynode_b2+backfill partitions). FIX
DIRECTION = backfill: driver now derives its deadline from the job's real TimeLimit
(4d453d1; parser tested on HH:MM:SS / D-HH:MM:SS / MM:SS; end grace 12 min for <= 4 h
jobs); deployed to Nibi. LEVER 24 TEST LIVE: N44fr0..472 (60 lanes, WS split preserved)
cancelled and resubmitted at --time=3:00:00 (same names/CKDIRs; partition auto =
cpubase_bynode_b1+backfill); N44fr480..2392 (240) stay 12 h as control. Pre-registered
read after 24-48 h: started-fraction per group; PASS (>= 3x) => convert Nibi (and any
cluster with multi-day waits) to 3 h lanes with 3 singleton reps. Fir 38 PD, Rorqual 120
PD, Trillium 24 (9 held lanes released by the loop) — no other action. Levers ledger: 24
opened; 12 shipped; 12 dead; in test: 5e6 pair, WS split, 3 h backfill.**

**⚡ 2026-09-16 (daily loop 1pm — ALL FOUR reached) — NO HITS; Trillium held-lane repair is
the round's action. Rorqual: FULL 60-lane read 21071921-980 = R44f2736..3208 HITLESS
(~25.3-28.4M tested/lane, 178/178, aborted 16.9-25.9% of tested — expected under the
09-13 rule, under the 35% line; cells_done_sum 468-539); queue 60 PD (21122818-877 =
R44f3216..3688) => pending >= 8, no action. Trillium: 6 NEW class-A 2e6 reads
2309932-937 = T44Af256..296 HITLESS (~24.8-26.0M tested, 178/178, aborted ~24-27% —
the class-A 2e6 baseline, above the workhorse's ~21% but under the line); 15 R
(started 02:16-03:28 EDT); ⚠️ 9 lanes PD-HELD "user env retrieval failed requeued held"
(2309938/939/943/945-949/954 = T44Af304/312/344/360/368/376/384/392/424) — held jobs
never start on their own => ACTION: duo_run scontrol release, fallback cancel +
verbatim-resubmit by name (outcome appended below). Fir: NO new summaries — 17 R
59818930-946 (7.2-8.4h) + 21 PD incl. the 5e6 pair 59818966/967 still PD
(pre-registered 5e6-vs-2e6 verdict rolls forward); unseen set 59584286/287/288/299
absent a SECOND check => 09-11 header-only rule fires: completed reps with lost
telemetry, CKDIRs intact, one rep counted each, NOT resubmitted, flag CLOSED. Nibi:
0 R + 339 PD healthy (300 FR N44fr0..2392 complete + 36 N44f832..1112 + N43dt315/331 +
N43b2e6 still PD) — nothing has started yet (RRG scheduling wait), >= 100 rule
satisfied, WS split test has no data yet; 21084727 absent a second check => header-only
rule, CLOSED. rung_status EXHAUSTED as always (Pass F is the program). Checker
exclusions: +210719(2[1-9]|[3-7][0-9]|80), +230993[2-7]. Watch next reads: 5e6 pair
F44f2720/2728 vs 2e6 neighbours, first Fir tranche-6 tail + Nibi FR reads (WS split
cells_done/lane comparison when both halves read), N43b2e6. TRILLIUM REPAIR VERDICT:
scontrol release CLEARED all 9 holds (post-release reason None, no cancel/resubmit
needed, fallback loop found nothing held); final queue exactly 24 = 15 R + 9 PD
(released) — the full unread T44Af set intact under original job IDs/CKDIRs. ROUND
VERDICT: no hits, no verified solutions; no new lanes submitted (all four clusters
pending >= 8 after the repair); workhorse F tile unchanged at k=3688 (63%), class A at
k=488; next unassigned: workhorse k=3696, A k=496.**

**⚡ 2026-09-15 (Daniel session) — NIBI LOADED + LEVER 23 SHIPPED. Prof (09-04 email): the RRG
is for NIBI, hundreds of CPU-years, "submit several hundreds of jobs" => Nibi is now the
RRG home (loop rule: keep >= 100 pending). LEVER 23 = stream-wall timeout (WZ_FH_WALL_SEC:
abandon a cell after N s without a candidate, drain its partial buffer, move on; CFGSIG
".ws<N>" only when set; CKDIR _ws<N>) — validated: n=29 byte-identical off/on (no walls),
n=44 workhorse skip-300 arm abandons its silent first cell after 30 s and advances,
(9,9,0,4) (the zero-candidate class) walls 6 cells in 4 min. DEPLOYED 218df6d to Nibi
(grep-verified) and SUBMITTED PASS FR TRANCHE 1: 300 lanes N44fr0..2392 step 8 (workhorse,
WZ_FH_STREAM_REV=1 = reversed fronts, K=50000, budget 2e6, --mem=0, RAC), SPLIT TEST:
k=0 mod 16 with WALL_SEC=900, k=8 mod 16 without — compare cells_done/lane when read
(>= +15% => WALL_SEC=900 fleet-wide). 300/300 echoed, Nibi 339 PD. Other clusters
untouched today (Fir 38 PD incl. the 5e6 pair, Rorqual 120, Trillium 30); they
self-deploy the new source when they need FR/F2/WS lanes (token WZ_FH_WALL_SEC).
Levers ledger: 23 opened; 11 shipped; 12 dead; in test: 5e6 budget pair, WS split.**

**⚡ 2026-09-15 (daily loop 1pm — ALL FOUR reached) — NO HITS; Rorqual restack is the
round's action. Fir: 20 NEW hitless 2e6 reads — tranche-5 tail 59584281-285/289-298/300
(16 lanes, ~31.1-32.9M tested/lane, 178/178, aborted ~6.2-7.1M = ~19-22% of tested,
expected under the 09-13 rule) + FIRST 4 tranche-6 reads 59818926-929 = F44f2400..2424
(~30.9-31.6M, aborts ~22-23%, cells_done_sum 578-588); queue 38 PD 0 R (59818930-965 +
5e6 pair 59818966/967 still PD — pre-registered 5e6-vs-2e6 verdict rolls forward);
UNSEEN, expect next check: 59584286/287/288/299 (not in reads, not in queue — 09-11
header-only rule if absent again). ONE-SHOT unseen-set FOUND grep came back clean on all
four clusters => no banner in the 09-12 header-only set, flag CLOSED, one-shot removed
from checker. Nibi: 23 NEW hitless reads — 19 N43dt 21084716-726/728-735 (~9.7-11.6M
this rep, 178/178, aborted 9-239 = tiny) + 4 N44f800..824 21085447-450 (~20.8-22.6M,
aborted ~0 = pre-lever-21 5e7 lanes, informational); 21084727 unseen, expect next check;
queue ~39 PD healthy (N44f832..1112 + N43dt315/331 resubs); N43b2e6 21707091 STILL PD
(informational only — 2e6 already fleet-wide). Rorqual: NO new summaries — all 60 lanes
21071921-980 (R44f2736..3208) R at ~8.6-9.5h, headers only; queue 0 PD => PASS F RULE
FIRES (pending < 8): submit next workhorse range R44f3216..3688 s8 (60 @2e6,
rrg-ikotsire_cpu, --mem=0). Trillium: NO new summaries — 2309932-937 R (2:22-8:27h,
headers only), 24 PD => pending >= 8, no action. rung_status EXHAUSTED as always (Pass F
is the program). Checker exclusions: +595842(8[1-5]|89|9[0-8])+59584300+5981892[6-9],
+210847(1[6-9]|2[0-6]|2[89]|3[0-5])+2108544[7-9]+21085450. Watch next reads: 5e6 pair
F44f2720/2728 vs 2e6 neighbours (fleet -> 5e6 iff >=90% cells with <=5% aborts), Fir
unseen 59584286/287/288/299 + Nibi 21084727, N43b2e6. RORQUAL SUBMITS: 60/60 echoed —
21122818-877 = R44f3216..3688 s8 @2e6 (Duo attempt 2 after one 180s miss); QUEUE_COUNT
120 post-submit (60 R + 60 PD). ROUND VERDICT: no hits, no verified solutions; 60 new
lanes queued (Rorqual only — Fir/Nibi/Trillium pending >= 8, no action); workhorse F
tile now assigned to k=3688 (63% of 5836); next unassigned: workhorse k=3696, A k=496.**

**Older TOP OF MIND entries (2026-07-24 to 2026-09-14) moved to `docs/archive/handoff/HANDOFF_2026-07-24_to_2026-09-14.md`; pre-2026-07-24 history in `docs/archive/handoff/HANDOFF_ARCHIVE_to_2026-07-23.md`.**

## 🚀 QUICK REFERENCE — the current system (rewritten 2026-07-30)

**CURRENT PROGRAM (2026-09-22): PASS F / FR / F2 front-only tiling, budget 2e6, canon ON —
`docs/plans/lever19_sweep_plan.md` is the operating table; `docs/briefs/external_review_brief.md` is
the one-page state of the art for outsiders.** Workhorse (3,13,0,0) forward tile 80%
assigned (lanes every 8 windows, K=50000), reversed-front tile (FR) running on Nibi, F2
(two buffers, top 35%) next, then the 11 other classes. Every submit: 12 h whole-node,
`--mem=0`, `WZ_FH_ORBIT_CANON=1`, `WZ_FH_DRAIN_TOP=50000`, `WZ_FH_AB_BUDGET=2000000`,
RAC account where it exists (rrg-ikotsire_cpu on Fir/Rorqual/Nibi; Nibi is the RRG home,
keep >= 300 queued). Loop: 1pm cron + one-tap button (only what is unread), one global
lock, hourly reminders (no unattended pushes), numbers digest after every check, agent
cap 90 min. Levers ledger: 25 priced, 12 shipped, 13 dead, 0 open. Older doctrine below
is history.


**Active solver: `src/solver/wz_match.cpp` in `WZ_FIRSTHIT` mode** — streams the
Thm-2.2-constrained C,D candidate stream from mod-6 profile cells (2.11a+2.11b+2.12
forced at n≥36), flat-first cell+in-cell ordering, profile-constrained A,B completion
(WZ_FH_AB_PROF), per-arm candidate-level resume (checkpointed lanes, spec
`docs/research/2026-07-28-per-arm-candidate-resume-design.md`). Deployed via
`cluster/deploy/cluster_firsthit_probe.sh` (178 single-core arms/node, 12h). SA ladder
RETIRED (ceiling ~n=33-35, archive); exhaustive/join RETIRED (archive).

**Ladder record (all NPAF-verified + banked in `results/champions/`):** 29→30→31 (SA) →
32→33→34→35→36→37 (firsthit, 2026-07-17..21) → **41 = BS(42,41) banked 2026-07-30**
(published WZ class (0,2,9,9), NEW inequivalent solution, score 124 vs their 140; first
hit ever at n≥38). n=42/43 = WZ's remaining rungs, under both-ends attack. **n=44 =
BS(45,44) = the OPEN WORLD RECORD** — 12-class frontier enumerated + all stream (2026-07-30);
program: `docs/research/n44_search_narrowing_research.md`.

**Verification rule (hard):** solution exists only after `*** FOUND ***` banner →
`python3 tools/verify_npaf.py` PASS (independent) → champion file with provenance →
HANDOFF entry → commit. bestAB/progress lines are never evidence.

**Checker:** `./cluster/deploy/check_all_retry.sh` (Daniel taps 4 Duo pushes; partial:
`CLUSTERS="fir nibi" ./cluster/deploy/check_all_retry.sh`). Remote command lives in
`cluster/deploy/checker_cmd.txt` — keep its exclusion regex current after every bank.
Daily loop: `daily_auto.sh` at ~1 PM (launchd), ntfy to phone.

**Submit template (checkpointed lane — resubmit the SAME line to auto-resume, zero
re-tread; one job per lane at a time):**
```
ssh dangord@<cluster>.alliancecan.ca 'cd $SCRATCH/bs45 && sbatch --requeue --export=ALL,WZ_N=<n>,WZ_A=<a>,WZ_B=<b>,WZ_C=<c>,WZ_D=<d>,WZ_FH_PROF_ORDER=<1 flat|2 reverse>,WZ_FH_ORBIT_CANON=1,WZ_FH_AB_BUDGET=50000000,FH_NARMS=178,WZ_FH_PROF_SKIP=<k> ./cluster_firsthit_probe.sh'
```
⚠️ `WZ_FH_ORBIT_CANON=1` is MANDATORY on every canonical (wave 13+) lane — the
checkpoint dir is keyed `..._oc${WZ_FH_ORBIT_CANON:-0}`, so omitting it silently forks
the lane to a fresh non-deduplicated `_oc0` checkpoint (no resume, no dedup).
Nibi adds `--account=def-ikotsire_cpu`. Ship source via tar-pipe (scp does NOT expand
$SCRATCH): `tar -cf - src/solver/wz_match.cpp cluster/deploy/cluster_firsthit_probe.sh |
ssh dangord@<c>.alliancecan.ca 'cd $SCRATCH/bs45 && tar -xvf - && cp -f
cluster/deploy/cluster_firsthit_probe.sh ./cluster_firsthit_probe.sh'`.

**Reading GATEB:** `tested=` (backtracks_entered sum) is true depth; `candidates=` is
STREAMED (inflated by the drain buffer); `tested_cum=` is cross-wave cumulative from
checkpoints; `resume_pi_min/max=` is the lane frontier. TIMEOUT@12h = normal completion.
squeue `%L` is time LEFT.

**Window/lane ledger (wave 11, 2026-08-03):** n=41 (0,2,9,9): SOLVED (flat windows 0-8
burned, reverse 0-6). n=42 (7,11,0,0): flat 0-6 burned; ckpt lanes flat 7/8/9 (Fir, cum
~248-268M) + reverse 4/5/6 (Rorqual, cum ~264-272M) + reverse 7/8/9 (Trillium, cum
~354-370M) live — live-window cum ≈2.66B (≈2.9B with burned windows), well past the
1.4-2B expected band; 3 sibling-class hedge lanes (1,5,0,12)/(3,9,4,8)/(1,3,4,12) flat
skip-0 ckpt on Fir (stacked 08-02, IDs unrecorded — read by sig header). n=43
(8,-2,5,9): ckpt lanes flat 0/1/2 (Fir, cum ~98-102M) + reverse 0/1/2 (Rorqual, cum
~89-101M) live. n=44 (all ckpt): Fir (5,9,6,6)/(5,7,2,10)/(1,13,2,2) skip-0 + (3,13,0,0)
windows 1/2/3 (39-53M/window/wave — the workhorse class, cum 105-123M) · Rorqual
(1,7,8,8)/(3,3,4,12)/(3,13,0,0)/(5,5,8,8) skip-0 · Trillium (5,11,4,4)/(7,7,4,8)/
(7,11,2,2) skip-0 — 10 fast classes live; slow (9,9,0,4)/(3,5,0,12) unassigned. Nibi: 9×
n=42 reverse skip-0 (old driver, no ckpt) running/PD. Wave-11 IDs: Fir 52706408-419 ·
Rorqual 18266737-746 · Trillium 2007532-537.

**Repo:** `src/{solver,verifier}/` · `cluster/deploy/` (active scripts) ·
`tools/verify_npaf.py` · `results/champions/` (banked) · `results/reference/` (WZ Table-1
sequences 41/42/43) · `docs/` (n44 program, resume spec, wz_paper_reconstruction,
kotsireas_brief) · `sarukhanian/` (separate sub-project). Local builds:
`clang++ -O3 -std=c++17 -Xpreprocessor -fopenmp -I/opt/homebrew/opt/libomp/include
-L/opt/homebrew/opt/libomp/lib -lomp`. Local runs: small-n only, PROF_ORDER=0 (flat-cell
stall trap at n≥41), no heavy solvers on the laptop.

**Student**: Daniel Gordon (dangord on Alliance clusters) · **Supervisor account**:
def-ikotsire (Nibi: `def-ikotsire_cpu`) · **Goal**: highest-n BS(n+1,n) δ-code. n=41
banked (replication-class result — WZ constructed 41-43; never call ladder rungs
records). **n=44 is the open record and the active research program.** Kotsireas brief:
READY, unsent, now leads with BS(42,41).

---

**Full pre-2026-07-24 history** (SA ladder era, exhaustive/join sagas, firsthit ramp
n=32→37, retired deploys, superseded entries): **`docs/archive/handoff/HANDOFF_ARCHIVE_to_2026-07-23.md`**.
