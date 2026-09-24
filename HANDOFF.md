# CP493 — BS(45) Solver Project Handoff

**Date**: 2026-09-24 (read TOP OF MIND newest-first; QUICK REFERENCE below has the current
system. Pre-2026-07-24 history — SA era, join saga, firsthit ramp n=32→37 — lives in
`HANDOFF_ARCHIVE.md`; measured-dead list in `.claude/skills/bs45-campaign/SKILL.md`.)

**⚡ 2026-09-24 (Claude session, late) — ASTRA FOLLOW-UP: Q-CLOSURE PRUNE BUILT (default off),
-5.5% MORE ORBITS, ALL PROVABLY EMPTY.** Astra: docs/reviews/2026-09-25-astra-followup.md (prune
sound under a completeness contract; orbit-min ordering = cleanup for the next fresh ordering; no
further involution). WZ_FH_ORBIT_QPRUNE=1 (needs Q; mod-6; untruncated list; only not_realizable or
eq2.12 certificates, else UNKNOWN = kept; CFGSIG .qp1; WZ_FH_QPRUNE_DUMP lists removed raw cells).
n=44: 44,534/803,724 orbits dead (1.0-10.0% per class), all not_realizable, 0 unknown; Q+prune =
759,190 orbits vs 1,460,098 today (-48%). Six + all n=6/8/10 solutions retained (6,564 LOCATE
runs); order-independent; every pruned cell at n=8/10/12 streams 0 candidates (238). Dead cells are
late in flat order (workhorse window quartiles 4245/4860/5380) => current reps never met them.
PRE-REGISTERED on CELLSIZE 61315095: every streamed pi listed in docs/reviews/2026-09-24-evidence/
qprune_prediction_3_13_0_0_w2000-3000.txt must show cand=0 (a nonzero = STOP, prune unsound);
their sec values = per-cell saving. Pass H plan now = Q + prune + orbit-min ordering, one fresh
namespace, after the canary PASS and Daniel's go.**

**⚡ 2026-09-24 (Claude session, close) — OPEN ITEMS + LOOP NOW READS BOTH JOBS.** The loop
(auto_prompt) reads 61315095 (CELLSIZE) and 61331128 (QCANARY) once they leave squeue, saves raw
output to docs/reviews/evidence/, applies the pre-registered rules and reports; any follow-up is
NEEDS_HUMAN. Astra follow-up prompt: docs/reviews/astra-followup-prompt.md (3 questions: Q-closure
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
range_done=k/178). NEW PLAN = PASS G (docs/lever19_sweep_plan.md): workhorse front =
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

**⚡ 2026-09-14 (daily loop 1pm — ALL FOUR reached) — NO HITS; Fir + Rorqual + Trillium
restacked per Pass F ledger. Fir: 21 NEW tranche-5 2e6 reads 59584260-280 HITLESS (~31.3-32.1M
tested/lane, 178/178, aborted ~6.3-6.7M = ~20-21% of tested — expected under the 09-13 rule,
well below the 35% escalation line; cells_done_sum 569-590 = the +30% cells/rep confirmed
across the full tranche); 16 lanes 59584281-300 R (header-only = current rep in progress);
0 PD => tranche 6 F44f2400..2712 s8 (40 @2e6) + PRE-REGISTERED 5e6 pair F44f2720+F44f2728
(plain names, WZ_FH_AB_BUDGET=5000000, compare cells_done+aborted vs 2e6 neighbours; if 5e6
gives >=90% of cells with <=5% aborts, fleet switches to 5e6). Rorqual: 15 2e6-resubmit reads
20995014-26/28/29 HITLESS (~27.6-28.8M this rep, cum ~50-54M, aborted 1.8-3.2M = ~6-11% —
lower than Fir's because these windows carry CKDIR coverage; 178/178, cells_done_sum 547-573);
20995027 finished-unread (expect next check); 8 R 20995030-37, 0 PD => next workhorse range
R44f2736..3208 s8 (60 @2e6); ⚠️ 20887514-523 absent a THIRD check — sacct asked in this
round's duo_run. Trillium: FULL T44Af (5,9,6,6) read — 2285217-44 (28 lanes, 10.6-16.7M this
rep, cum ~18-35M, aborted 1-105 = 5e7-budget lanes, informational) + 2252668-71 first reps
(17.6-18.8M) + 2222596 T43sC (26.7M, cum 46.6M) ALL HITLESS; queue EMPTY => next A range
T44Af256..488 s8 (30 @2e6, def-ikotsire). Nibi: NO new reads (21084716-35 N43dt427..571 +
21085447-50 N44f800-824 all started ~09:23 today, headers only), 24 R + ~39 PD healthy,
N43b2e6 21707091 STILL PD (informational only — 2e6 already fleet-wide) => no action.
rung_status EXHAUSTED as always (Pass F is the program). Fir unseen set 59191099/100/118/137/
141-145/147 absent again (3rd check) — grep asked in Fir's duo_run. Checker exclusions:
+595842(5[7-9]|6[0-9]|7[0-9]|80), +209950(1[4-9]|2[0-6]|2[89]), +2285217-44, +2252668-71,
+2222596. LIVE ROUND job IDs appended below after each cluster's submits echo. FIR SUBMITS: 42/42 echoed — 59818926-965 = F44f2400..2712 s8 @2e6, 59818966/967 = F44f2720/2728 @5e6 (pre-registered pair); Fir queue 57 lines post-submit. Fir unseen set VERDICT: all 10 files have NO arms_with_hits/GATEB lines = header-only lost-telemetry reps (09-11 rule: completed, CKDIRs intact, one rep counted, not resubmitted); one-shot FOUND grep added to checker to close the banner gap. RORQUAL SUBMITS: 60/60 echoed — 21071921-980 = R44f2736..3208 s8 @2e6; queue 66 lines post-submit. sacct VERDICT on 20887514-523: NO accounting rows under those IDs (sacct echoed only unrelated lc14 array tasks) => they were cancelled while PD in the 09-12 lever-21 resubmit sweep and are SUPERSEDED by 20995014-37 (same windows by name/CKDIR) — flag CLOSED. 20995027: no summary lines = header-only completed rep (09-11 rule), CKDIR intact, no action. TRILLIUM SUBMITS: 30/30 echoed — 2309932-962 (no 2309951, scheduler skip) = T44Af256..488 s8 @2e6, queue exactly 30 post-submit. ROUND VERDICT: no hits, no verified solutions; 132 new lanes queued fleet-wide (Fir 42, Rorqual 60, Trillium 30); workhorse F tile now assigned to k=3208 (55% of 5836), class A to k=488; next unassigned: workhorse k=3216, A k=496. Watch next reads: the 5e6 pair F44f2720/2728 vs 2e6 neighbours (fleet switches to 5e6 iff >=90% cells with <=5% aborts), Nibi N43b2e6 21707091 (informational), one-shot FOUND grep on the Fir unseen set.**

**⚡ 2026-09-13 (Daniel session) — FIR READ (Daniel missed the Duo twice; I ran it): NO HITS.
First three budget-2e6 lanes 59584257-259 (F44f tranche-5): 31.5-31.8M tested, 178/178
arms, cells_done 571-574/lane (vs 382-454 at 5e7 = +30% cells/rep) BUT budget_aborted
~6.6M = ~21% of tested (at 5e7 aborts were ~0). Interpretation: the cap drops the deep
failing tail (known solutions need <=213k nodes); net coverage of "cheap" candidates up
~30%/rep; recorded, kept at 2e6, a 5e6 middle-point measurement pre-registered for the
next Fir restack. Fir queue 11 R + 26 PD (2e6) — no restack needed. BUTTON UPGRADE
(Daniel's ask): the checker now keeps results/reached_<date>.txt; the one-tap button runs
ONLY the clusters not yet read today (all four if none missing); the hourly re-pusher drops
a cluster once another run has read it. Old re-pusher killed for today.**

**⚡ 2026-09-13 (supplementary re-check ~13:10 — only Trillium answered) — NO HITS, nothing
new. Fir Duo missed AGAIN (checker auto-retry armed, round 1/10 hourly — the 40 tranche-5
lanes 59466759-798 + the 09-12 unseen reads still roll forward); Rorqual missed this
attempt too (harmless — fully read in today's main run). Trillium: same board as main —
2252668-71 (T44Af224/232/240/248) R with header-only outputs stamped 13:01-13:02 = just
started, orbitcanon dedup 7.97872x live on all four; 29 PD (2285217-44 + T43sC 2222596)
=> pending >= 8, no action. Nibi absent from this check (main run covered it). No FOUND
banners, no submits, no exclusion changes, ledger unchanged.**

**⚡ 2026-09-13 (daily loop 1pm — Nibi/Rorqual/Trillium reached, Fir Duo missed) — NO HITS,
bookkeeping only, no submits. Nibi: ONE new read 21084715 = N43dt419 HITLESS (10.4M tested,
178/178 arms_summarized, aborted=160 = 0.0015% of tested — well under the 5% budget_aborted
watch line, cells_done_sum 140); queue ~63 PD healthy (N43dt lanes + N44f800..1112 +
N43dt315/331 resubmits); N43b2e6 21707091 STILL PD — no lever-21 twin verdict (informational
only; 2e6 already applied fleet-wide 09-12). Rorqual: 8 more 700G-resubmit window lanes read
HITLESS — 20887529-536, sig (3,13,0,0), ~24-25M tested this rep, tested_cum ~46-47M (CKDIR
coverage carried), 178/178, aborted=0, cells_done_sum 464-490, dedup 28.92x; ⚠️ 20887514-523
STILL unseen (flagged "expect next check" on 09-12 — if absent again next check, ask sacct);
queue = the 24 2e6 resubmits 20995014-37 all PD => pending >= 8, no action. Trillium:
4 T44Af lanes (2252668-71 = T44Af224/232/240/248) went R ~13:01, headers only = just
started, NOT dead; 29 PD (2285217-44 resubmits + T43sC 2222596) => no action. Fir: Duo
missed — 40 tranche-5 lanes 59466759-798 + the 09-12 "unseen" reads
(59191099/100/118/137/141-145/147) roll to next check. rung_status EXHAUSTED as always
(Pass F is the program). Checker exclusions: +21084715 (210847 group -> 1[0-5]),
+20887529-536 (208875 group -> 2[4-9]|3[0-6]). No FOUND banners, ledger unchanged.**

**⚡ 2026-09-12 (Daniel session) — LEVER 21 APPLIED + LEVER 22 BUILT. Lever 21: Fir control
F41b2e6 PASSED (n=41 re-found in 3.1 h at budget 2e6 vs 4.6 h at 5e7, verify PASS; the
deepest known hit needs 213k nodes = 10x under the cap) => applied fleet-wide without
waiting for Nibi's queued twin: Fir 40 pending tranche-5 lanes + Rorqual 23 pending lanes
cancelled and resubmitted by name at 2e6 (same CKDIRs), loop rule updated (all submits
2e6; watch budget_aborted). Lever 22 (WZ_FH_STREAM_REV=1: reversed DFS branch order in
count_pairs22 = a second independent first-buffer per cell; CFGSIG ".sr1" only when set;
CKDIR _sr1): VALIDATED at n=19 on 3 classes — forward and reversed both FOUND with
identical stream totals at the cell boundary (same candidate set, different order).
Defined PASS FR (F offsets with SR=1) for after each class's F tile; order F -> FR -> F2.
Not deployed yet (loop self-deploys when FR starts). Board: Fir 40 PD (2e6), Rorqual 8 R
+ 23 PD (2e6), Trillium 33 PD, Nibi 63 PD. Workhorse F tile 41% (k<=2392). Levers
ledger: 22 opened; 10 shipped; 12 dead.**

**⚡ 2026-09-12 (daily loop 1pm — ALL FOUR reached) — ★ LEVER-21 FIR CONTROL PASS, no new
results. 59193560 = F41b2e6 (n=41, canon-off, uncapped, WZ_FH_AB_BUDGET=2e6) *** BS(42,41)
FOUND *** — same fingerprint as the banked champion (idx=500000 profile_rank=1429 score=124
= the known solution re-found), elapsed 11023.6s ≈ 3.1 h vs 4.6 h baseline at 5e7, tested
9.17M, aborted=1,575,142 (budget-abort tail cut as designed), verify_npaf.py PASS locally.
NOT banked, NOT news (pre-registered control). Nibi twin N43b2e6 21707091 still PD =>
BUDGET STAYS 5e7 FLEET-WIDE until it reads; if it also re-finds, switch every subsequent
submit to WZ_FH_AB_BUDGET=2000000 (same CKDIRs, budget outside CFGSIG). Fir reads: 25
tranche-4/resubmit F44f lanes summarized HITLESS (~23-26M tested/lane, 178/178, aborted=0,
cells_done_sum 408-422, dedup 28.92x, no CFGSIG-mismatch lines); 26 lanes HEADER-ONLY =
completed reps with lost telemetry per the 09-11 rule (11 = the F44f704..1504 resubmit set
again — pre-fix source, ran full walltime, CKDIRs intact — + 15 tranche-4: 59191101/104/
105/107/112-115/119/120/122/124-126/128), counted one rep each, NOT resubmitted (sweep =
one rep); unseen (expect next check): 59191099/100/118/137/141-145/147. FIR QUEUE EMPTY =>
ACTION: Pass F tranche 5 = F44f2080..2392 s8 (40 new workhorse lanes), verbatim env,
rrg-ikotsire_cpu, --mem=0. SUBMITS: 40/40 echoed, 59466759-798 = F44f2080..2392 all PD
(squeue confirmed queue = exactly these 40). Rorqual: 14 summarized
HITLESS — 20887508-513/524-528 = 700G resubmits (tested ~24-25M this rep, tested_cum
~46-47M = CKDIR coverage carried, 178/178 FIRST TIME on these windows, cells_done_sum
466-493) + 20161638/640/641 = R44f448/456/464 (25.4-26.1M, 178/178); 3 HEADER-ONLY
completed-rep 20161636/637/639 bookkept + excluded; 20887514-523 unseen, expect next
check; queue ~30 PD => no action. Nibi: N43dt419 R (7:21h), ~63 PD healthy => no action;
N43b2e6 still PD. Trillium: 33 PD (T44Af re-arms + T43sC), no new outputs => no action.
rung_status EXHAUSTED as always (Pass F is the program). Checker exclusions: +26 Fir reads
+ 59193560 + 26 Fir header-only + Rorqual 14 reads + 3 header-only.**

**⚡ 2026-09-11 (Daniel session, "double check your work") — AUDIT RESULTS. Soundness: SIGTERM
handler wired (g_fh_stop_ptr=&fh_stop), interrupted cell NOT counted done (resume_pi stays
on it), buffered-path interrupt writes its ckpt (resume_k=30) and resume re-engages it —
verified locally on the M6 path the clusters use. STRATEGY: Pass F (buffer 1, top 10%)
would have caught n=41 (0.9%, buf 1) and n=43 (2.7%, buf 1) but NOT n=42 (buf 2, 31.9%)
=> built PASS F2 (WZ_FH_DRAIN_BATCHES=2, K=175k; CKDIR _dt175000b2; validated B=2 halves
cells/rep) as the deeper complement, scheduled after the workhorse F tile. FOUND A PAST
COST: the unconditional ".dt" CFGSIG field (09-01) invalidated every pre-09-03 checkpoint
=> lanes that resumed after the 09-03 deploy fresh-started (T43 stack cum 149-327M, Fir
sA/sB reps) — bounded (sweep lanes are 1-rep) but real; HANDOFF cum figures for those
lanes are stale from 09-03. NEW RULE: new CFGSIG fields append only when non-default
(".db" only if B>1) — verified B=1 signature byte-identical to deployed lanes. Not yet
deployed (F2 needs it; loop self-deploys, grep token WZ_FH_DRAIN_BATCHES). CONFIRMED ON
TRILLIUM (arm logs): "[firsthit ckpt] STALE/MISMATCHED checkpoint IGNORED (...oc1 !=
...oc1.dt0) — starting fresh" on T43f6 2222591 and T43r6 2222592 (09-04): the n=43
published deep lanes RESTARTED at position 0 (per-arm tested_cum ~200k vs prior lane cum
149-327M); old ckpts overwritten, positions unrecoverable. Every lane with a pre-09-03
CKDIR that resumed after the deploy did the same (Fir F44sA/sB reps incl.). Sweep/Pass-F
lanes (fresh CKDIRs) unaffected. All cum figures for pre-09-03 lanes are now "since
09-04". Rule in force: never change CFGSIG for existing lanes (conditional fields).**

**⚡ 2026-09-11 (Daniel session) — THE "NON-MEMORY DEATH MODE" SOLVED: NOT A DEATH. The 11
Fir resubmits ran the FULL 12 h (sacct TIMEOUT 12:00:xx, 178 arms, arm logs show 1M
streamed / 50k-capped completions / checkpoints written 30 min before the end); only the
SUMMARY was lost: "[driver] deadline — stopping 178 arms" then Slurm's walltime kill.
Cause: a lever-20 regression — count_pairs22 was given the per-CELL stop flag instead of
the arm flag that the SIGTERM handler sets (g_fh_stop_ptr), so an arm inside a stream
wall (silent for hours: no candidates => no progress lines => no probe() => no stop
check) ignored SIGTERM; 1-4 such arms per lane hung the driver's bare `wait` past
walltime. Uncapped arms never left their first cell so it never showed. FIXES: (1)
solver: g_fh_sigterm defined above count_pairs22 and checked in rec() alongside `stop`
(local test: exit 0.0 s after SIGTERM with summary); (2) driver: bounded post-SIGTERM
wait (FH_STOP_GRACE=300 s) then SIGKILL stragglers and aggregate anyway; (3) loop rule:
header-only-at-walltime = a completed rep with lost telemetry, checkpoints intact,
resubmit-by-name resumes — never "dead". The 11 windows' work is banked in CKDIRs; the
ledger's "dead windows" language is retired. One-tap button confirmed working (Daniel's
13:11 and 14:07 checks). Mem fix confirmed 4x at 178/178. Lever-21 controls still PD. FIX DEPLOYED 5de899c on ALL FOUR (4 taps, grep-verified
solver + driver): Fir 24 R + 14 PD, Rorqual 1 R + 42 PD, Trillium 33 PD, Nibi 63 PD.
Pending lanes compile the fixed source at start (SIGTERM honored); their captured
driver copy still has the bare wait, which now returns promptly because arms exit.**

**⚡ 2026-09-11 (supplementary #2, Fir-only re-check ~14:07 — Duo retry) — NO HITS,
bookkeeping only. ONE new read since 13:11: 59191102 = F44f1768 (tranche-4 fresh
window) hitless, 178/178 arms_summarized (4th mem-fix confirmation), tested 24.4M =
tested_cum (fresh CKDIR), cells_done_sum 416, aborted=0. NOT added to checker
exclusions — squeue still lists 59191102 with 53 min walltime left (summary landed
early), so it stays visible; tomorrow's loop should treat its re-show as already
bookkept here. 59191092/093/098 summaries re-showed despite the 13:11 exclusions
(no new info). The 11 header-only resubmits (59191085-091/094-097) unchanged — only
~1h since 13:11, so this does NOT count as the "re-show next check" trigger; the
non-memory-death-mode investigation waits for the next daily check. Queue = 27 R
(F44f1760..1968) + 13 PD (F44f1976..2072) + F41b2e6 59193560 PD — pending >= 8, no
restack. LEVER-21: F41b2e6 still PD, no verdict, budget stays 5e7. No submits,
ledger unchanged.**

**⚡ 2026-09-11 (supplementary re-check ~13:11 — all four reached, Fir's missed Duo
recovered) — NO HITS, bookkeeping only. Fir (the news): 3 of the 14 resubmitted F44f
window lanes read HITLESS with **178/178 arms_summarized — first confirmation the
--mem=0/750G fix works on Fir** — 59191092/59191093/59191098, tested 25.0-26.3M this
rep, tested_cum 93.6-102.9M (CKDIR coverage carried), cells_done_sum 506-533, aborted=0,
dedup 28.92x. ⚠️ The other 11 resubmits (59191085-091/094-097) ended HEADER-ONLY again
despite 750G (headers Sep 10 19:05-21:22 PDT, past walltime, gone from queue) — left
visible, NOT resubmitted (Fir pending 19 >= 8); if they re-show header-only next check
that is a non-memory death mode => investigate. Fir queue = 22 R + 18 PD tranche-4
(F44f1936..2072) + F41b2e6 PD. LEVER-21 CONTROLS: Fir F41b2e6 59193560 PD and Nibi
N43b2e6 21707091 PD — still no verdict, budget stays 5e7 fleet-wide. Rorqual: no new
summaries (20161636/637 = R44f432/440 finished header-only minutes-to-hours before the
check — summaries may land next read, left visible); queue 3 R + 44 PD => no action.
Nibi: no new reads, 65 PD healthy => no action. Trillium: no new outputs, 34 PD
(33 T44Af + T43sC) => no action. No FOUND banners, no submits, ledger unchanged.
Checker exclusions: +3 Fir reads (5919109[238]), +13 Fir dead originals
(58243096/099/102/103/105-107/109-112/116-118, superseded by the 59191085-098
resubmits), +36 Rorqual dead originals (20161589-624/626-632/634 = R44f56..416,
superseded by the 20887506-557 700G resubmits; 636/637 kept visible).**

**⚡ 2026-09-11 (daily loop 1pm — nibi+rorqual+trillium reached; Fir Duo MISSED) — NO
HITS, bookkeeping-only cycle. Trillium big read: 2225668-91 = ALL 24 T44i (3,13,0,0)
quarter-point lanes hitless, tested 39-64M/lane, 178/178 arms (full-node memory as
always), cells_done_sum 0-26, aborted 0-10 telemetry only; T43 stack COMPLETE hitless —
2222591/92/94 (8,-2,5,9) 22.2/27.4/22.2M + 2222593 (6,8,5,7) 23.8M + 2222595 (8,10,1,3)
37.2M + 2192781 (0,2,1,13) 19.9M, all 178/178; queue = 33 T44Af PD (2285217-44 re-arm
resubmits + 2252668-71) + T43sC dep => pending >= 8, no action; 2252640-67 = outage-dead
T44Af originals now excluded as superseded by the 2285217-44 resubmits. Rorqual: 3 new
R44f reads hitless — 20161625/633/635 = R44f344/408/424 tested 21.6/22.0/21.7M,
cells_done_sum 393/393/381, arms_summarized 142/178 = the pre-fix 48G OOM signature
(these reps ran before the --mem=0 re-arm; coverage banked in CKDIRs, windows re-covered
by the 700G resubmits); queue = 3 R (20161638-640 = R44f448/456/464, headers Sep 11) +
~44 PD (20161641 + 20887506-557 at 700G) => pending >= 8, no action. Nibi: NO new reads
(21084702/704 old dead headers re-showed => excluded as superseded by 21705841/842);
queue = 72 PD healthy (20 N43dt 419..571 + 50 N44f 800..1112 + N43dt315/331 resubmits +
21707091 N43b2e6) => no action. LEVER-21 CONTROLS: Nibi N43b2e6 21707091 still PD (no
verdict); Fir F41b2e6 59193560 unknown (Fir unreached) — budget stays 5e7 fleet-wide
until BOTH controls read. No FOUND banners, no submits, ledger unchanged. Checker
exclusions +33 reads, +30 superseded.**

**⚡ 2026-09-10 (Daniel session, late: "out of the box") — LEVER 21 = BUDGET CUT: the
per-candidate AB node budget is 5e7 while the three hits needed 213k/87k/89k nodes and
lanes average ~450k nodes/candidate (heavy tail of deep failures eats arm time). Budget
is outside CFGSIG => changeable per rep on existing CKDIRs, no code. CONTROLS SUBMITTED
(2 taps): Fir 59193560 F41b2e6 (n=41 skip-8, canon-off, uncapped, budget 2e6, RESUME=0;
baseline F41nc found in 4.6 h at 5e7) and Nibi 21707091 N43b2e6 (n=43 window 327,
canon-on, K=50000, budget 2e6, RESUME=0; baseline 4.9 h). PASS = both re-find, not
slower => fleet-wide WZ_FH_AB_BUDGET=2000000 at next restack (loop rule added). LEVER 22
designed (reversed in-cell DFS order = a second independent "front" per cell, part of
CFGSIG; test = F41dt-rev re-finding n=41 where F41dt did not) — build after lever 21.
Levers ledger: 22 priced/opened; 8 shipped; 12 dead; 2 in test.**

**⚡ 2026-09-10 (Daniel session, back after a week) — 🔥 ROOT CAUSE OF THE "HEADER-ONLY"
DEATHS = OUT OF MEMORY FROM SLURM'S 48 GB DEFAULT, AND IT HAS BEEN TAXING EVERY LANE
SINCE JULY. Rorqual sacct: R44f lanes = OUT_OF_MEMORY (ReqMem 48G, batch MaxRSS 50.3G)
or TIMEOUT with "36 oom_kill events"; arm processes "Killed"; arms_summarized 142/178.
Local measurement: an arm peaks ~257 MB during startup (cell enumeration + canon + sort)
then runs at ~55-60 MB; 178 x 257 MB = ~46 GB = the cap => ~36 arms/lane OOM-killed at
launch on Fir/Rorqual/Nibi ON EVERY LANE (the long-standing 142-171/178 "interrupted"
counts were OOM, not walltime). Trillium schedules whole nodes with all memory =>
178/178 always (the tell). Capped (lever-20) lanes cycle cells and died outright.
FIX: driver `#SBATCH --mem=0` (commit c792cbf); deploy via GitHub curl per cluster;
pending pre-fix jobs get `scontrol update MinMemoryNode=700000`; dead lanes resubmitted
under the SAME names (surviving arms resume from CKDIRs). Lever 20 stays: the one capped
lane that survived (R44f344, 20161625) covered 393 cells in one rep. Also today: full
4-cluster read NO HITS; Fir queue was EMPTY (09-09 PARTIAL run backgrounded its submit a
3rd time => new PreToolUse hook .claude/hooks/no_background_duo.py DENIES backgrounded
duo_run/sbatch/ssh); one-tap "Run check now" shipped (ntfy action button on every
notification -> private control topic -> launchd listener com.dangord.bs45listener runs
a supplementary check immediately; tested dry-run; installed). Discriminator verdict
(loop, 09-04): F41nc + F41dtnc FOUND, F41dt not => canon relocation depth confirmed.
RE-ARM EXECUTED (4 taps): driver c792cbf deployed on ALL FOUR via GitHub curl; Fir 14
dead lanes resubmitted + tranche 4 F44f1760..2072 (40) => 54 PD on RAC, new ReqMem 750G;
Rorqual 42 dead resubmitted + 5 queued raised to 700G => 47 PD + 4 R; Trillium 28 dead
(T44Af, outage-killed) resubmitted => 33 PD; Nibi 2 dead resubmitted + 60 queued raised
to 700G => 62 PD (ReqMem 700000M confirmed). Every lane now runs with full node memory
=> 178/178 arms for the first time on Fir/Rorqual/Nibi.**

**⚡ 2026-09-09 (daily loop 1pm — fir+nibi+rorqual reached; Trillium in its listed
Sept 8-10 outage, no push sent) — NO HITS; FIR QUEUE EMPTY => Pass F restack (only
action). Fir: 38 of the remaining 51 Pass F lanes read HITLESS — 58243100/101/104/108
= F44f712/744/768/1128 (tested ~21-24M this rep; 712 tested_cum 71.4M ≈ 3x and 1128
cum 48.6M ≈ 2x => dead-rep CKDIR coverage survived again) + 58243113-115/119-149 =
F44f1464..1752 subset first reps (tested ~20.7-25.5M/lane, cells_done_sum 357-481,
dedup 28.92x live). ⚠️ 13 lanes DIED HEADER-ONLY (headers Sep 6 00:38-02:36, empty
queue past walltime): 58243099/102/103/105/106/107/109-112/116-118 = F44f704/752*/
760*/776/784/792/1432/1440/1448/1456/1488/1496/1504 (*752/760 note: 101=744 and
104=768 read, so the dead set is 704,752,760,776,784,792 in the resubmit block +
1432 + 5 tranche-3 windows) — 4th death for k=672 (58243096 re-showed, still dead).
Fir pending 0 < 8 => ACTION: resubmit the 14 dead windows (672,704,752,760,776,784,
792,1432,1440,1448,1456,1488,1496,1504 — verbatim env, CKDIR resume, singleton
names) + Pass F tranche 4 = F44f1760..2072 s8 (40 new workhorse lanes), RAC
(rrg-ikotsire_cpu). Rorqual: 6 Pass F reads HITLESS — 20161586-588/593/596/608 =
R44f32/40/48/88/112/208 (tested 22.9-25.5M/lane, cells_done_sum 422-460); ⚠️ 23
lanes died HEADER-ONLY (headers Sep 7 20:55 - Sep 8 19:35, none in queue, past
walltime): 20161589-592/594/595/597-607/609-614 = R44f56/64/72/80/96/104/120..256
step 8 (minus the 6 read) — left visible, NOT resubmitted (Rorqual pending 27 >= 8,
same call as prior header-only deaths; windows tracked here as unsearched-this-rep).
Queue = 27 R44f PD (20161615-641 = R44f264..472) => no other action. Nibi: 13 N43dt
phase-3 reads HITLESS — 21084700/701/703/705-714 = N43dt299/307/323/339..411 (minus
315/331; tested 9.1-11.7M/lane, aborted 16-179 telemetry only, 162-163/178
summarized); ⚠️ 21084702/704 = N43dt315/331 died HEADER-ONLY (headers Sep 7 06:26,
gone from queue) — left visible, NOT resubmitted (Nibi pending 60 >= 8); 21084697 =
N43dt275 re-showed => NOW EXCLUDED (windows 275/315/331 remain unsearched — tracked
here). Nibi queue = 20 N43dt PD (419..571, note 21084727 absent from queue) + 40
N44f PD (800..1112) => no action. No FOUND banners anywhere. rung_status EXHAUSTED
as always (SA rail closed; Pass F is the program). Checker exclusions +57 reads +
21084697 + 58093614-672 superseded-block cleanup. SUBMITS: see next paragraph
(appended after Duo echo).**

**⚡ 2026-09-09 (supplementary re-check ~05:18 EDT — the Duo retry chain for the missed
fir+nibi from the cycle above; Nibi reached on retry round 1, Fir NEVER reached — gave up
after 10 hourly rounds) — NO HITS, bookkeeping only. Nibi: first 5 of the 40 N43dt phase-3
lanes finished — 21084695/696/698/699 = N43dt259/267/283/291 read HITLESS, tested
10.1-11.5M/lane, cells_done_sum 128-165, aborted 81-250 telemetry only, 162/178 arms
summarized (walltime rep, normal), dedup 3.81x live. ⚠️ 21084697 = N43dt275 died
HEADER-ONLY (header Sep 7 02:11, gone from queue, no summary) — left visible in checker,
NOT resubmitted (Nibi pending 61 >= 8, same call as R44i5575/5775 + F44f672; window 275
tracked here as unsearched-this-rep). Queue healthy: 14 N43dt R (299..403, 1-3h elapsed
at check) + 21 N43dt PD (411..571) + 40 N44f PD (800..1112) => no action. Fir: 57 Pass F
lanes (58243093-149) still unread, queue presumed intact — expect next check. No FOUND
banners, no submits, ledger unchanged. Checker exclusions +4.**

**⚡ 2026-09-07 (daily loop 1pm) — NO HITS, bookkeeping-only cycle. Fir + Nibi Duo MISSED
(no approval in 180s) => not reached; Fir's queue (was 30 R + 21 PD Pass F on 09-06, incl.
the 57 Pass F lanes 58243093-149 in flight) and Nibi's 80 PD presumed intact — expect
their reads next check. Rorqual reached: 14 new hitless reads — 20161582-585 = FIRST
Rorqual Pass F workhorse reads (R44f0/8/16/24, sig (3,13,0,0), tested 20.9-24.6M/lane,
cells_done_sum 382-454, dedup 28.92x live) + 20120315-324 = R44Di quarter-point family
COMPLETE (sig (5,5,8,8) s500 = R44Di375..4875, tested 13.3-29.8M/lane, cells_done_sum
0-15, aborts 65-1308 telemetry only); queue = 56 R44f PD (20161586-641) => pending >= 8,
no action. Trillium reached: T43 stack (2192781, 2222591-95) + 24 T44i (2225668-91) +
T44Af0..72 (2252640-49) all RUNNING with in-progress headers (header-only + R in queue =
normal, leave visible for their finished reads) + 22 T44Af PD (2252650-71) + T43sC dep
=> no action. No FOUND banners anywhere. No submits, ledger unchanged (workhorse tiled
to k=1752). Checker exclusions +14.**

**⚡ 2026-09-06 (supplementary re-check ~14:10 — Fir only, reached on Duo retry round 1;
main run above handled the rest) — NO HITS, bookkeeping only. First 6 of the 09-05
dead-lane resubmits finished: 58243093-095/097/098 = F44f648/656/664/688/696 read HITLESS,
tested ~22-24M this rep, tested_cum 72-77M ≈ 3x tested => the two prior header-only reps'
CKDIR coverage SURVIVED and resumed (deaths cost reps, not coverage). ⚠️ 58243096 =
F44f672 died HEADER-ONLY AGAIN (3rd rep death for that window; header Sep 5 19:00, gone
from queue, no summary) — left visible in checker, NOT resubmitted (Fir pending 51 >= 8,
same call as Rorqual 20120303/304; window k=672 tracked here as unsearched-this-rep).
Queue healthy: 30 R (58243099-128 = F44f704..792 subset + 1128/1432 + tranche-3 through
k=1584) + 21 PD (58243129-149 = F44f1592..1752). No submits, ledger unchanged. Checker
exclusions +5.**

**⚡ 2026-09-06 (daily loop 1pm) — NO HITS, bookkeeping-only cycle. Fir Duo MISSED (no
approval in 180s) => not reached; yesterday's 57 Pass F submits (58243093-149) unread,
expect them next check. Nibi reached: 80 PD healthy (40 N43dt259..571 + 40 N44f800..1112,
all Priority) => no action. Rorqual reached: 20120315 = R44Di375 RUNNING (header 02:58 EDT,
~1:55 elapsed at check — leave visible for its read), 60 R44f Pass F PD (20161582-641) +
R44Di875 PD + 8 R44Di maintenance-held => pending >=8, no action; 20120303/304 =
R44i5575/5775 known header-only dead re-showed => NOW EXCLUDED (regex 201203(0[0-2]|0[5-9]|
1[0-4]) widened to 201203(0[0-9]|1[0-4]); 09-05 decision stands: not resubmitted, windows
k=5575/5775 remain unsearched — tracked here). Trillium reached: 63 PD (T43 stack 6 + 24
T44i + 32 T44Af0..248), deep through the Sept 8-10 outage => no action. No new FOUND
banners, no new completed reads on any reached cluster. No submits, ledger unchanged
(workhorse still tiled to k=1752). Checker exclusions +2.**

**⚡ 2026-09-05 (supplementary re-check ~16:30 — all four clusters reached: Fir+Trillium
on the first push, Rorqual retry round 1, Nibi retry round 3; NOTE no 09-05 main-run
HANDOFF entry exists and last_summary.txt was empty at start, so this is the day's only
written record) — NO HITS. Fir: 42 of the 09-04's 59 Pass F lanes read hitless —
58093633+58093635-671 = F44f1120..1424 (minus 1128) tranche 2 tested ~24-28M/lane,
cells_done_sum ~430-525/lane, dedup 28.92x live; 58093618/623/624/625 = F44f680/720/728/
736 resubmits read hitless with tested_cum 48-53M ≈ 2x tested => the first (header-only)
runs' CKDIR progress SURVIVED and resumed — header-only deaths cost the rep, not the
coverage. ⚠️ 17 lanes DIED HEADER-ONLY AGAIN (empty queue past walltime): 58093614-617/
619-622/626-632 = 15 of the 19 F44f648..792 resubmits (2nd death for those windows) +
58093634 = F44f1128 + 58093672 = F44f1432; left visible in checker, resubmitted below
(3rd rep for the 15; CKDIR+singleton collision-proof). 57972743 re-show => 57972726-744
now excluded (superseded twice over). Rorqual: 20120300-302 = R44i4975/5175/5375 hitless
30-32M + 20120305-314 = R44Ai375..4875 s500 hitless 13-27M (quarter-point family now
fully read except:) 20120303/304 = R44i5575/5775 HEADER-ONLY dead (not requeued — noted,
not resubmitted; sweep lanes, pending ≥8); queue = 60 R44f Pass F PD (20161582-641) + 10
R44Di maintenance-held => no action. Nibi: 80 PD healthy (40 N43dt + 40 N44f) => no
action. Trillium: 63 PD deep through the Sept 8-10 outage => no action. rung_status
EXHAUSTED as always (Pass F is the program). Checker exclusions +74 (42 Fir reads + 19
superseded + 13 Rorqual reads). ACTION: Fir pending 0 < 8 => resubmit the 17 dead lanes
(verbatim env, CKDIR resume, singleton names) + Pass F tranche 3 = F44f1440..1752 s8
(40 new workhorse lanes), RAC. SUBMITS: Fir 57/57 echoed, SUBMIT_COUNT=57 — 58243093-109
= the 17 dead-lane resubmits (F44f648..792 subset + F44f1128 + F44f1432), 58243110-149 =
F44f1440..1752; all PD at submit.**

**⚡ 2026-09-04 (daily loop, supplementary re-check — the 1pm main run left no entry/summary
and made no commits, so this covers the whole fleet; fir+nibi Duo missed at 1pm, all four
reached on the hourly retry ~14:21) — ★ CANON/CAP DISCRIMINATOR RESOLVED, canon-relocation
CONFIRMED as the F41regr root cause. Fir 57971034 = F41nc (uncapped canon-OFF, replica of
original hit conditions): *** BS(42,41) FOUND *** in ONE rep — byte-identical quad to banked
champion_firsthit_bs42_41.txt, same fingerprint (idx=500000 profile_rank=1429 score=124),
elapsed 4.6h, tested 12.8M. Fir 57971038 = F41dtnc (K=50000 canon-OFF): ALSO FOUND, same
quad/fingerprint, elapsed 4.6h, tested 9.4M, cells_done_sum=136 => the cap does NOT lose
the n=41 hit and capped throughput re-finds it. Fir 57971036 = F41dt (K=50000 canon-ON):
hitless 178/178 tested 21.2M — exactly as geometry predicted (canon keeps a different orbit
cell). All three verdicts match the relocation hypothesis; canon soundness NOT in question;
the 09-03 canon-ON + lever-20 decision stands validated. Re-finds NOT banked (expected
re-find of a banked champion, quad verified identical locally). REST ALL HITLESS: Fir Pass F
tranche-1 F44f480..640 = 57972705-725 (21 lanes) read hitless, tested 27.6-29.4M/lane,
cells_done_sum ~500-535/lane (front tile advancing as designed); ⚠️ 57972726-744 = F44f648..792
(19 lanes, started 15:06-17:05 PDT 09-03) are HEADER-ONLY with an EMPTY Fir queue past
walltime = died without summaries (precedent: 18724004-009 rebalance-scancel "header-only,
no data") — their windows were never searched; left visible in the checker (not excluded)
in case any is a zombie, resubmitted below (CKDIR resume + singleton names = collision-proof
either way). Nibi: 20990490 re-show only (exclusion regex slip `2099048[90]` doesn't match
20990490 — fixed), 80 PD healthy (40 N43dt phase-3 + 40 N44f800..1112 = 21085447-480+482-487),
no action. Rorqual: 5 R (R44i4975..5775 quarter-point tail) + ~80 PD (60 R44f Pass F =
20161582-641 now visible + 20 A/D maintenance-held), no action. Trillium: ~63 PD (32 T44Af
Pass F = 2252640-671 + 24 T44i + T43 stack), deep through the Sept 8-10 outage, no action.
rung_status EXHAUSTED as always (SA rail closed; Pass F is the program). Checker exclusions
+25 (discriminator 3 + Fir tranche-1 reads 21 + 20990490 fix). ACTION: Fir pending 0 < 8 =>
Pass F ledger advance: resubmit F44f648..792 s8 (19 dead lanes, verbatim) + tranche 2
F44f1120..1432 s8 (40 new lanes), workhorse class, RAC. SUBMITS: Fir 59/59 echoed,
SUBMIT_COUNT=59 — 58093614-632 = F44f648..792 resubmits, 58093633-672 = F44f1120..1432.**

**⚡ 2026-09-03 (Daniel session: "do whatever needs to be done to find bs45") — DECISION
+ PASS F LAUNCHED FLEET-WIDE. Rule collision resolved: CANON STAYS ON (soundness verified
by LOCATE_CANON; F41regr failure = relocation depth of that one solution's kept cell; in
expectation canon-on is never worse per lane-day and the n=43 capped control re-found
ours with canon on) and LEVER 20 (K=50000) is applied to all n=44 lanes. Canon-disable
rule RETIRED; F41nc/F41dt/F41dtnc discriminator stays running (informational). PASS F =
front tile: capped lanes every 8 windows (an arm advances ~8 cells/rep), class order
workhorse -> A(5,9,6,6) -> B(5,7,2,10) -> C/D/E/G/H -> I/J -> K/L, ledger + loop rule in
docs/lever19_sweep_plan.md (commit 048109b). TRANCHE 1 via duo_run (4 taps, all echoed):
Rorqual lever-20 self-deployed + R44f0..472 s8 (60, RAC; 85 PD total); Fir F44f480..792
s8 (40, RAC; discriminator 3 PD); Nibi N44f800..1112 s8 (40, RAC; 80 PD incl. band
phase 3); Trillium lever-20 self-deployed + T44Af0..248 s8 (32, def; 63 PD, deep enough
for the Sept 8-10 outage). Workhorse tiled k=0..1112 tonight (~19% of its 5836 windows);
the loop advances the ledger as queues drain.**

**⚡ 2026-09-03 (daily loop) — TWO PRE-REGISTERED VERDICTS FIRED, THEY COLLIDE =>
NEEDS_HUMAN on n=44 fleet config. (1) ★ LEVER-20 GATE PASS: Nibi capped control lane
21001113 (N43dt set, K=50000, ORBIT_CANON=1) RE-FOUND the banked n=43 champion —
byte-identical quad to champion_firsthit_bs44_43.txt, same GLOBAL FIRST fingerprint
(idx=500000 rank=58505 score=130), tested 5.26M, ~4.9h, verify_npaf.py re-PASS this
run; throughput prong also met: capped lanes cells_done_sum 144-161/lane vs 0-26
uncapped = ~10x >= the 3x line. NOT banked (re-find of an already-banked solution,
not news). Lever 20 is now UNGATED for n=44 per the loop rule. (2) CANON REGRESSION
FAILURE fired: F41regr rep 2 (57650091, 43.1M, cum 81.1M) + rep 3 (57650092, 35.4M,
cum 116.6M >= ~115M line) BOTH hitless => pre-registered rule says disable
WZ_FH_ORBIT_CANON fleet-wide pending investigation. COLLISION: the disable rule fired
off n=41 geometry (canon keeps a different orbit cell, hit depth ~500k), yet the
config it disables (canon-on) is exactly what found AND re-found n=43 today, and
canon-off costs 3.8-28.9x dedup (28.9x on the n=44 workhorse — canon-off sweep would
be a throughput catastrophe). Rule-vs-rule = Daniel's call; discriminator submitted
(below). REST OF READS ALL HITLESS: Nibi N44i pass-3 read complete (20990489/490 +
20990501-513, 15 lanes); N43dt band set 21001114-1153 hitless (21001134 unread);
Rorqual 20120262-299 = 38 of the 63 quarter-point lanes already consumed overnight
post-maintenance, 20120300-324 (25) still PD => pending >=8, no action; Trillium 31
PD healthy, no action (compute offline Sept 8-10, queue deep enough through it); Fir
08-31 batch fully read (57650093-123 = F44i3750-5750 + F44Bi250-4750) + crowd-out
stragglers 57086727-32/57238027-33/57273915-19/57437183; 19838963 STILL unread.
FIR+NIBI QUEUES EMPTY => submitted: Fir = 3-lane canon/cap DISCRIMINATOR at n=41
skip-8 (F41nc = uncapped canon-OFF, replica of original hit conditions, in-cell rank
1429 => should re-find in ~1 rep if canon relocation is the cause; F41dt = K=50000
canon-ON, the pre-registered lever-20 F41 control, geometry predicts NO find; F41dtnc
= K=50000 canon-OFF => should re-find AND measures capped n=41 throughput) after
self-deploy of pinned b4547cd lever-20 source via GitHub curl; Nibi = Track-B band
PHASE 3 (pre-registered offsets +4: N43dt k=259..571 step 8, 40 lanes, K=50000,
canon-on = the exact config that passed its control today), rrg-ikotsire_cpu both.
rung_status EXHAUSTED as always (SA rail closed). SUBMITS: Fir 3/3 echoed —
57971034 = F41nc, 57971036 = F41dt, 57971038 = F41dtnc (lever-20 source b4547cd
self-deployed via GitHub curl, DEPLOYED_LEVER20 + grep-verified); Nibi 40/40 echoed,
SUBMIT_COUNT=40 — 21084695-726 + 21084728-735 = N43dt259..571 step 8, all PD
Priority at submit.**

**⚡ 2026-09-02 (daily loop) — NO HITS; FIR BACK from Slurm-upgrade outage (reached, ~35
lanes read), Rorqual reached + queue EMPTY again, Trillium reached (31 PD healthy, no
action), Nibi in listed outage (AIaaS Brine — checker skipped push as designed; lever-20
N43dt control lanes UNREAD, lever 20 stays GATED off n=44). READS ALL HITLESS: Rorqual
20052847-895 = the full 09-01 pass-3 (49 lanes: R44i125..5725 s200 quarter-points 26-53M/
lane + R44Ai/R44Di 125..4625 s500 13.8-34M) consumed OVERNIGHT under RAC + 19921447-454 =
R44Ai pass-1 tail finally read (13.8-25.1M, crowd-out resolved); Fir 57238023-26/28 =
front-stack reads ((3,13,0,0) deep windows cum 415-428M, (5,9,6,6) cum 215.6M); Fir
57454076 + 57456358-396 = F41regr rep 1 + the 39-lane pass-1 sweep — RE-SHOWS of Daniel's
08-31 session reads (their exclusions were never written to checker_cmd.txt; now in the
live regex, +96 total). 19838963 STILL unread. F41regr control: rep 2 RUNNING (57650091,
~6.5h in), rep 3 PD dependency (57650092) — canon-failure verdict fires only after rep 3
reads hitless. Fir queue healthy (9 R + 24 PD RAC sweep lanes). rung_status EXHAUSTED as
always (SA rail closed; lever-19 sweep is the escalation). ACTION: Rorqual pending <8
(=0) => pass-3/4 advance per lever19 rules = remaining quarter-point inventory, 63 lanes:
R44i k=3075..5675 s200 (14) + k=175..5775 s200 (29) + R44Ai/R44Di k=375..4875 s500 (10+10),
verbatim env, ORBIT_CANON=1, one rep each, rrg-ikotsire_cpu. (Rorqual Slurm maintenance
today 12:00 — submits queue and start after; that is the point.) SUBMITS: Rorqual 63/63
echoed, QUEUE_COUNT=63 — 20120262-275 = R44i3075..5675 s200 (14), 20120276-304 =
R44i175..5775 s200 (29), 20120305-314 = R44Ai375..4875 s500 (10), 20120315-324 =
R44Di375..4875 s500 (10). Workhorse quarter-point family now COMPLETE (25/75/125/175 mod
200 all assigned); A/D quarter-points complete (125+375 mod 500).**

**⚡ 2026-09-01 (Daniel session, late) — DUO-NAG BUG FIXED: the hourly re-pusher kept
sending "tap to approve" for Fir all afternoon while Fir was in a listed Slurm-upgrade
outage (login shows the Duo menu but no push reaches the phone). check_all_retry.sh
now consults status.alliancecan.ca before EVERY push (cluster_outage(): "cloud_off"
marker => skip the push, one quiet low-priority note when first seen, silent hourly
re-checks, an "is back" note + real push when it clears); after 3 undelivered pushes
on an up cluster the nudge downgrades to low priority with "ignore these". bash-3.2
safe (macOS: no associative arrays — first version crashed on declare -A, caught by
the stub e2e). Tested: outage-skip, outage-clears, normal path, live parse (Fir =
outage, Rorqual = up). STATUS_SRC=<script|file> overrides the fetch for tests.
Today's old-code re-pusher was killed; tomorrow's 1 PM loop reads Fir on the new code.
Loop guidance stands: Fir F41regr reps 2-3 + 40 RAC lanes wait on the upgrade.**

**⚡ 2026-09-01 (Daniel session, evening) — LEVER 20 BUILT + VALIDATED: FRONT-ONLY TOP-K
DRAIN (`WZ_FH_DRAIN_TOP=K`, wz_match.cpp; driver CKDIR gains `_dt<K>`; CFGSIG gains
`.dt`). Evidence: all 3 deep hits surfaced inside the FIRST 500k sorted buffer of
their arm's cell (idx 500000/1000000/500000; in-cell percentiles 0.9/31.9/2.7%) while
completing a full buffer costs ~14 h/arm => today an arm spends a whole rep on one
buffer of one cell. K caps completions per cell at the K flattest of buffer 0, then
the arm advances: cells/lane-day x(500k/K). VALIDATION (local, n=29 with WZ_FH_M6=1 to
force the buffered path, 500 s bounded): off => byte-identical stream/cells vs the
pre-edit binary (442,763 streamed, same cells/dups/resume_pi); on (K=200) => 15 cells
advanced vs 1, cells_capped=4 at exactly 200 each; CFGSIG dt2000 confirmed in the
ckpt file; interrupt+resume under the lever works. DEPLOYED TO NIBI via duo_run curl from GitHub (repo is
public; pinned b4547cd; grep-verified before mv) and the CONTROL IS LIVE: N43dt327
(our n=43 hit's window, K=50000 — must re-find) + N43dt257..567 step 8 (band phase 2,
K=50000), ~40 lanes on rrg-ikotsire_cpu. Other clusters self-deploy via the new loop
rule when needed; lever 20 stays GATED off n=44 until a control re-finds a known n=43. Pre-registered test in docs/n44_search_narrowing_research.md:
n=43 controls (our window-327 hit re-find + band phase-2 offsets) at K=50000 on Nibi
(RAC), F41regr-style K=50000 at n=41 on Fir; success = known solution re-found by a
capped lane at >=3x cells/lane-day. FULL n=44 CLASS INVENTORY recorded (all 12 stream;
windows 2,718-5,836 each). Fir: Duo flow reaches the Duo menu (push generated hourly);
delivery to phone is the failing step during the Slurm upgrade — tip: open Duo Mobile
directly after the nudge. Zsh trap: unquoted $var does not word-split (broke a loop).**

**⚡ 2026-09-01 (Daniel session) — MAINTENANCE WEEK MAP (status.alliancecan.ca, fetched
this session): FIR in OUTAGE TODAY (planned Slurm upgrade) — that is why no Fir Duo
push arrived (ssh unreachable = no push generated); hourly re-pusher will catch it on
return; F41regr reps 2-3 (canon verdict) blocked until then, rule stays armed.
RORQUAL: Slurm maintenance 09-02 12:00 (running jobs requeue; checkpoints lossless).
TRILLIUM: compute offline Sept 8-10 (STACK ITS QUEUE BEFORE THE 8th so it resumes
instantly). NIBI: planned outage listed, date unclear — treat unreachable as
maintenance this week, not failure. Loop guidance: missed/unreachable clusters this
week are EXPECTED; do not escalate; requeued jobs resume their CKDIRs. Sweep status
per 09-01 loop: Rorqual ate its entire 50-lane pass-2/3 overnight under RAC + pass-3
submitted by the loop itself (49 lanes); Nibi pass-2 done, pass-3 in (15); Trillium
31 pending; all hitless so far.**

**⚡ 2026-09-01 (daily loop) — NO HITS; RAC THROUGHPUT CONFIRMED: Rorqual consumed its
ENTIRE 50-lane 08-31 pass-2/3 submission OVERNIGHT (20007700-749 all read hitless:
(1,7,8,8) midpoints 250-4750 = 20007700-709, (5,5,8,8) midpoints = 20007710-719,
workhorse pass-3 k=25..5825 step 200 = 20007720-749, tested 11.1-52.1M/lane) plus the
R44Di pass-1 tail 19921455-464 ((5,5,8,8) k=500-5000, 13.4-27.1M) — 60 files shown =
EXACTLY the head cap; 19921447-454 (R44Ai pass-1 k=1500-5000) + 19838963 finished but
crowded out, telemetry next check (the NEW FOUND grep scans ALL files uncapped =
confirmed no hit hiding there). Nibi N44i pass-2 set COMPLETE hitless: 20835991-95 +
20835997-99 + 20836000-004 = 13 lanes k=50..1250, tested 21.7-55.2M. Rorqual + Nibi
queues EMPTY => pass-3 advance per lever19 loop rules (pending <8): Rorqual =
workhorse quarter-points k=125..5725 step 200 (29, R44i$k) + (1,7,8,8) k=125..4625
step 500 (10, R44Ai$k) + (5,5,8,8) same ks (10, R44Di$k); Nibi = workhorse k=75..2875
step 200 (15, N44i$k); verbatim env, ORBIT_CANON=1, one rep each, --account=
rrg-ikotsire_cpu (verified live on both 08-31). Trillium healthy: 31 PD all Priority
(24 T44i pass-2 lanes now queued as 2225668-91 + wave-21 2222591-96 + 2192781),
pending >=8, no action. Fir Duo missed — F41regr reps 2-3 unread (canon-failure rule
still armed, fires only after rep 3 is read hitless). rung_status EXHAUSTED as always
(SA rail closed; firsthit work is the escalation). Checker exclusions +73. SUBMITS:
Rorqual 49/49 echoed = 20052847-875 (R44i125..5725 step 200) + 20052876-885
(R44Ai125..4625 step 500) + 20052886-895 (R44Di125..4625), QUEUE_COUNT=49. Nibi
15/15 echoed = 20990489/490 + 20990501-513 (N44i75..2875 step 200), QUEUE_COUNT=15.**

**⚡ 2026-08-31 (Daniel session) — RAC CUTOVER EXECUTED + blocked day recovered (loop
was usage-limit blocked at 1 PM; this session did the cycle). DRAC ticket 0323126:
Daniel is a member of rrg-ikotsire; the working association is rrg-ikotsire_cpu on
FIR, RORQUAL, NIBI (Trillium: none — stays def-ikotsire). Done via duo_run (6 taps):
Fir — 33 new lanes (F41regr reps 2-3 + workhorse pass-2 k=3750..5750 + (3,3,4,12)
k=250..4750), 57238027 released from held, ALL 40 pending moved to RAC; Rorqual —
queue was EMPTY (pass 1 fully consumed overnight, hitless), 50 pass-2/3 lanes
submitted under RAC ((1,7,8,8)+(5,5,8,8) k=250..4750 + workhorse pass-3 k=25..5825
step 200), 2 RUNNING WITHIN SECONDS of submit (RAC priority live); Trillium — no rrg
assoc, 31 lanes stay def; Nibi — 9 pending moved to RAC, 4 running. READS: Fir 39
pass-1 sweep lanes + F41regr all HITLESS (F41regr rep 1: 38.0M tested vs original
hit job's 12.9M, arms at resume_pi 1428-10290; canon-failure rule pre-registered in
docs/lever19_sweep_plan.md: hitless after rep 3 => disable canon fleet-wide).
Rorqual/Trillium data from the blocked 1 PM cycle was overwritten locally by the
supplementary checker — no agent processed it; nothing lost on-cluster; tomorrow's
loop reads current state. Docs: RAC doctrine + pass-2 ledger committed (152c06f).**

**⚡ 2026-08-31 (daily loop) — NO HITS; Rorqual+Trillium reached, Fir/Nibi Duo missed.
Rorqual pass-1 sweep: first 30 lanes ALL HITLESS (R44i3000..5700 workhorse (3,13,0,0)
tested 24-38M/lane, cells_done_sum 0-26, plus R44Ai500/1000 (1,7,8,8) 21-22M), 18 lanes
still R (R44Ai1500-5000 + R44Di500-5000, IDs 19921447-464), pending 0 => PASS-2 TRIGGER
FIRED: submitted Rorqual pass-2 via duo_run = R44i3750..5750 step 100 (21 workhorse
midpoints, completing pass-2 coverage 50..5750 with Nibi 50-1250 / Trillium 1350-3650)
+ R44Ai/R44Di 250..4750 step 500 (10+10 class midpoints), verbatim env, one rep each.
Also read: Rorqual 14th-rep front-stack tails 19838962/964-971 hitless ((3,13,0,0)
tested_cum to 445M, (1,7,8,8) 275-282M, (5,5,8,8) 230-234M), Trillium wave-20 BOTH reps
2192770-780 hitless ((8,-2,5,9) f6/r6/f7 cum 245-388M, (6,8,5,7) 203-223M, (8,10,1,3)
141-148M). Trillium board healthy: 24 T44i pass-2 lanes + wave-21 (2222591-96, 2192781)
all PD, pending >=8, no action. CHECKER BUG FIXED: 08-30's "+24 exclusions" landed only
in the section-header comment, not the live grep -vE — the 24 T43b lanes re-showed
today; band range 2222998-2223021 now in the real regex (2223007 stays the banked hit).
F41regr control still unread (Fir missed). Job IDs appended below after submit echoes.**

**⚡ 2026-08-30 (Daniel session, afternoon) — TRILLIUM BAND LANES READ (Daniel saw the
COMPLETED emails; RunTime 11:30 = driver's 30-min pre-walltime stop, NOT a hit signal):
21 of 23 hitless T43b lanes read, 0/178 arms each, 14.5-19.9M tested per lane,
cells_done 0-11; NEW FOUND none — 2223007 (T43b327) remains the only n=43 hit.
Track B phase 1 fronted ~68 of the 317 band windows. Trillium queue emptied =>
RE-AIMED to n=44 (consistent with the morning decision): 24 workhorse pass-2 midpoint
lanes T44i1350..3650 step 100 submitted via duo_run (24 echoed). Pass-2 workhorse
coverage now: Nibi k=50..1250, Trillium k=1350..3650; Fir/Rorqual pass 2 (k=3750..5750
+ other classes) due when their pass-1 pending <8. Checker exclusions +24.**

**⚡ 2026-08-30 (Daniel session, "go") — POST-n=43 RE-AIM EXECUTED via duo_run (3 taps):
(1) Nibi: 13 pending Track-B control lanes (N43b471..567) scancelled; 13 n=44 workhorse
pass-2 midpoint lanes submitted (N44i50..1250 step 100, --account=def-ikotsire_cpu).
Trillium's 23 running T43b lanes left to finish (second n=43 = bonus). (2) Fir: 11
pending 12th-rep front top-ups (57437171-182) scancelled; ALL 39 Fir sweep lanes
(F44i/F44Bi) are RUNNING and F41regr (n=41 re-find control) is RUNNING. (3) Rorqual:
9 pending 15th-rep top-ups (19917514-522) scancelled; sweep lanes 10 running + 38
pending. Track A pass 2 on Fir/Rorqual per plan when pending <8. Professor email
(trio complete + sequences) drafted; Daniel to send.**

**⚡ 2026-08-30 (Daniel session) — n=43 RE-VERIFIED INDEPENDENTLY (Daniel: "please
verify"): pure-python check outside project code — lengths 44/44/43/43, sums
(8,-2,5,9), sum-of-squares 174 = 4n+2, all entries ±1, NPAF sum = 0 at every shift
1..43; NOT any of Wang-Zhu's 1024 variants (A<->B x C<->D x negations x reversals) =
NEW solution in the published class. Official gate tools/verify_npaf.py re-run: PASS
(s=1..44, pair encoding OK). Champion file provenance reviewed (Trillium 2223007,
T43b327, first rep, 5.82M tested, window 327 inside the predicted 255-571 band).
VERDICT CONFIRMED. Trio complete: 41 (07-30) / 42 (08-04) / 43 (08-30), all new
solutions. Decisions pending Daniel: re-aim remaining Track B lanes to n=44; Track A
pass 2; professor update (draft below in session).**

**⚡ 2026-08-30 (daily loop) — ★★★★★ n=43 HIT: BS(44,43) FOUND, VERIFIED, BANKED —
THE WANG-ZHU TRIO (41/42/43) IS COMPLETE, AND THE WINDOW-MAP BAND PRIOR IS CONFIRMED.**
Trillium job 2223007 = lever-19 Track B lane T43b327 (sig (8,-2,5,9), flat window 327,
ONE rep, fresh CKDIR, orbit canon on) hit at elapsed 14441s (~4.0h into the lane's FIRST
slot), tested only 5.8M candidates. GLOBAL FIRST: idx=500000 profile_rank=58505
nodes_this_cand=88616 score=130, arms_with_hits=1/178. Independent
tools/verify_npaf.py PASS (NPAF[s]=0 all s=1..44, sumsq 174); NOT Wang-Zhu's published
quad — no match under the full 1024-form variant group vs wz_table1_bs44_43.txt = a NEW
solution in the published class. BANKED: results/champions/champion_firsthit_bs44_43.txt.
STRATEGIC READ: window 327 is INSIDE the WZ-43 band (w255-571) that the 08-29 LOCATE
window map flagged — the w0-11 lanes ground ~330M+ hitless for weeks while the band lane
hit in ONE rep at 5.8M. The band prior is real; this is exactly the Track B control
lever 19 was designed to fire. Other reads ALL HITLESS: Fir 10th rep sA2/A3+sB1-3
57086733-737 (cum 190-207M), Rorqual 13th rep 19742337-345 ((3,13,0,0) rev w1-5 cum
415-423M, A 260-265M, D 214-217M), Nibi tail tC/tD/tE FIRST telemetry 20506644-646 =
10.4/9.5/18.7M all survive the 0.5M line, Nibi Track B first 3 reps 20792775-777
(b447/455/463, ~11-11.8M each), Trillium T43sC 1st rep 2192775 (18.8M, cum 181.8M).
Board: all 4 clusters saturated with lever-19 pass-1 lanes (Fir 39 R incl F41regr +
F44tAr, Rorqual 10 R + 37 PD, Trillium 23 Track B R + wave-20/21 PD, Nibi 14 Track B
PD) — no refills due (pending ≥8 everywhere), sweep-pending = no front top-ups. NOTE:
Fir 57086727-732 + Trillium 2192770-774 finished but were crowded out of the checker's
head -30 cap by the 24+ new sweep headers — cap raised to 60 in checker_cmd.txt, their
reads land next check. ACTIONS: banked + excluded 2223007 and the 20 read lanes,
no submits. NEEDS_HUMAN: WZ trio done — decide (a) whether remaining T43b/N43b control
lanes keep running or get scancelled and re-aimed at Track A n=44 band windows, (b)
whether the professor update / allocation ask ships now with the trio complete, (c)
Track A pass-2 window placement given the band prior is confirmed at n=43.**

**⚡ 2026-08-29 (Daniel session, late) — LEVER 19 PASS 1 SUBMITTED via duo_run (Daniel
tapped 4): Track A n=44 interior — Fir 39 lanes (F44i100..2900 workhorse step 100 +
F44Bi500..5000 (3,3,4,12) step 500), Rorqual 48 (R44i3000..5700 workhorse + R44Ai/R44Di
500..5000 for (1,7,8,8)/(5,5,8,8)); Track B n=43 WZ-43 band control — Trillium 24
(T43b255..439 step 8), Nibi 16 (N43b447..567 step 8). All -J named, -d singleton,
ORBIT_CANON=1, one rep each, fresh CKDIRs (skip-keyed). Plan + pass tables + loop rules:
docs/lever19_sweep_plan.md (prompt updated: no front top-ups while sweep pending; advance
passes when pending <8). Queue order: sweep lanes sit behind the already-queued front
stacks (age priority) — expect first sweep reads in ~2-3 days on Fir/Rorqual. Controls
outstanding: F41regr 57454076 (n=41 re-find), Track B (WZ-43 re-find).**

**⚡ 2026-08-29 (Daniel session, evening) — SOLVER AUDIT PASSED + WINDOW MAP FINDING.
Daniel asked whether the post-08-04 build (orbit canon) broke the search. Verified
locally with the LOCATE instrument (extended: LOCATE_CANON retained=YES/NO, committed):
all 5 known solutions (ours 41/42, WZ 41/42/43) keep >=1 canonical cell -> canon is
sound at the campaign lengths incl. even n. End-to-end regression queued: Fir 57454076
F41regr (our n=41 hit's exact lane, oc1) — expect FOUND in one rep. NO BUG. FINDING:
known solutions' cells sit at flat windows 0 / ~8-15 / ~255-571 / ~499-842 / and the
far reverse end (ours-42 = rev w4) — our n=43 published-class lanes (flat w0-11, rev
w0-11) have NEVER reached WZ-43's band (w255-571); the "overdue" verdicts assumed a
front-loaded prior the data refutes (56k-cell tie blocks). NEXT: lever 19 = broad
shallow window sweep (docs/n44_search_narrowing_research.md, design pending: walltime,
CKDIR keying, queue limits). Professor update drafted for Daniel.**

**⚡ 2026-08-29 (Daniel session, after the PARTIAL) — ROOT CAUSE of the supplementary
PARTIAL: reads + bookkeeping succeeded and were committed (a7c37ed, commit-as-you-go
worked), but the agent ran the Fir duo_run submit IN THE BACKGROUND and ended its turn
"to wait" — headless -p sessions are not re-invoked, so the run ended rc=0 with 0
submits and no summary. FIXES: auto_prompt.md hard rule (duo_run FOREGROUND only;
ending the turn ends the run); daily_auto.sh SUBMITS count "0\n0" bug fixed and rc=0
no-summary case worded correctly. THE THREE PLANNED SUBMITS still due -> PASTES
ISSUED to Daniel: Fir 12th-rep top-up x11 + F44tAr ((9,9,0,4) REVERSE try per the
lever-18 rule, ord2 skip0 fresh CKDIR); Rorqual 15th-rep top-up x9. SUBMITTED BY CLAUDE via duo_run (Daniel tapped):
Fir 57437171-183 = 12th rep x11 (F44w4-8/sA1-3/sB1-3) + F44tAr (57437183) rev try
=> 38 F44 jobs queued; Rorqual 19917514-522 = 15th rep x9 => 26 R44 jobs (depth 3
both). Trillium 2222591-596 = wave-21 x6 (T43f6/r6/sA/f7/sB/sC, configs parsed from each
running lane's CKDIR name: (8,-2,5,9) ord1 skip6 / ord2 skip6 / ord1 skip7; siblings
(6,8,5,7)/(8,10,1,3)/(0,2,1,13) ord1 skip0) => 18 T43 jobs, depth 3. Nibi left as-is
(tails tC-E PD backup, n=43 lanes empty by design). FLEET 4/4 COVERED, depth 3 on
F/R/T. Trillium's push was missed once and approved on duo_run's retry — the
retry path works live.**

**⚡ 2026-08-29 (supplementary) — NO HITS; Fir/Nibi/Rorqual ALL REACHED on the hourly
re-push (Fir on retry round 1); 23 lanes read ALL HITLESS, incl. the FIRST lever-18
tail-class telemetry.** Reads: Fir 9th window-front rep 56917602-612 — (3,13,0,0)
flat w4-8 tested 21.2-23.3M (cum 380.5-387.7M), (5,9,6,6) w1-3 14.3-15.7M (cum
187.6-191.5M), (5,7,2,10) w1-3 14.6-17.1M (cum 175.8-191.2M); Rorqual 12th rep
19666521-529 — (3,13,0,0) rev w1-5 tested 27.7-30.0M (cum 385.4-395.5M,
cells_done_sum 1-4/lane continuing), (1,7,8,8) w1-2 19.5/18.7M (cum 243.9/239.4M),
(5,5,8,8) w1-2 15.9/16.0M (cum 202.8/199.2M), A/D aborts 25-89 telemetry only.
LEVER-18 TAIL TELEMETRY: (9,9,0,4) produced ZERO candidates over a full window on
BOTH clusters (Nibi 20506642 + Fir mirror 57273914: candidates=0 tested=0
AB_nodes=0, arms ran the whole slot, resume_pi advanced) ⇒ the pre-registered
<0.5M-after-1-rep rule FIRES: stream-walled at flat ⇒ ONE rev try (F44tAr, ord2
skip0, fresh auto-keyed CKDIR) — if the rev rep also lands <0.5M the class is
DROPPED per the rule. (3,5,0,12) Nibi 20506643 tested 1.87M (aborted=43, AB_nodes
5.0e12) — ABOVE the 0.5M line, survives; its Fir mirror F44tB is R. Board at check:
Fir 10th rep w4-8 57086727-731 R (~9.5h) + sA/sB 57086732-737 PD Priority + 11th
rep 57238023-033 PD = depth 2 ⇒ 12th-rep top-up due; Fir tail mirrors F44tB-E
(57273915/917/918/919) R, 1-rep-at-a-time by lever-18 design (read-then-decide,
NOT depth-3 stock); Rorqual 13th rep 19742337-345 R (~3.4-5.2h) + 14th rep
19838962-971 PD = depth 2 ⇒ 15th-rep top-up due; Nibi tail copies 20506644-646
still PD (accepted backup), n=43 lanes stay EMPTY (lever-18 still Nibi's queued
work — same rationale as 08-28). Workhorse (3,13,0,0) accounting: deep w0-3 ~1.28B
+ flat fronts w4-8 ~1.92B + rev fronts w1-5 ~1.95B ≈ 5.16B tested ≈ 1.94x n=42's
~2.66B fall depth — grinding, no tilt trigger (no new n=43 reads this pass).
rung_status check run for the idle rail: EXHAUSTED = SA-refill forbidden as always
(firsthit ckpt work, not SA). ACTIONS: Fir 12th-rep top-up (11 lanes x1 singleton)
+ F44tAr rev try + Rorqual 15th-rep top-up (9 lanes x1 singleton) via duo_run,
verbatim configs, same CKDIRs (F44tAr fresh by ord-key). Job IDs appended below
after submit. Checker exclusions +23 (56917602-612, 57273914, 19666521-529,
20506642/643).**

**⚡ 2026-08-29 (daily loop) — NO HITS; TRILLIUM ONLY (Fir/Nibi/Rorqual Duo pushes
unanswered in 180s — no reads, no board visibility there this pass); Trillium wave-19
2nd rep 2178717-722 read ALL HITLESS (6 n=43 lanes, arms 178/178 summarized each,
arms_with_hits=0).** Reads: (8,-2,5,9) f6 (2178717) tested 18.7M (cum 229.3M) / r6
(2178718) 30.6M (cum 326.6M, deepest n=43 lane fleet-wide) / f7 (2178719) 16.5M (cum
227.8M); siblings FLAT (6,8,5,7) (2178720) 22.9M (cum 181.6M), (8,10,1,3) (2178721)
8.3M (cum 132.9M), (0,2,1,13) (2178722) 20.3M (cum 163.1M); aborted 0-57/lane,
telemetry only; all cums arithmetic-consistent with the 08-24 1st-rep reads. Board at
check: Trillium wave-20 1st rep 2192770-775 R (~6.4-6.6h in, lands ~19:00 EDT) + 2nd
rep 2192776-781 PD Dependency = depth 2, NOT at the restack trigger (last-rep-with-
nothing-behind) — wave-21 due at the next natural read, no action today. Tilt-criterion
bookkeeping: NEW n=43 reads this cycle — deepest sibling (6,8,5,7) 181.6M vs the
~450M/class line, no tilt, grind continues. rung_status check run for the idle rail:
EXHAUSTED = SA-refill forbidden as always (this is firsthit ckpt work, not SA).
ACTIONS: bookkeeping only — NO submits this pass; the daily_auto SUPPLEMENTARY child
(hourly Duo re-push, up to 10 rounds) takes over fir/nibi/rorqual after this run: it
will read Fir 9th rep + Rorqual 12th rep + any lever-18 F44tA-E telemetry (57273914/
915/917/918/919) and top up Fir (11 lanes) / Rorqual (9 lanes) to depth 3 with real
queue state; blind top-ups from the main pass would only duplicate that. Depth math:
Fir and Rorqual each held 3 reps at the 08-28 check, so even with today's rep landed
they sit at depth 2 — covered through tomorrow's loop even if every re-push is missed.
Checker exclusions +6 (2178717-722).**

**⚡ 2026-08-28 (Daniel session) — LEVER 18 STUCK: Nibi tail-class lanes 20506642-646
still PD with ZERO runtime after 3 days (Nibi priority), so the breadth hedge has not
run at all while the workhorse climbed to ~4.92B (~1.85x n=42 raw depth, ~50x its
distinct cost). FIX: mirror the 5 tail-class lanes onto FIR (the only cluster
admitting our jobs daily): -J F44tA..E -d singleton, same env, fresh Fir CKDIRs.
Nibi copies left PD as backup (if both run it is ~5 lane-days of duplicate work,
accepted; whichever reports first is read under the pre-registered stream-wall rule).
CONFIRMED IN: Fir 57273914/915/917/918/919 = F44tA..E, PD (None) — Fir admits daily, first tail-class telemetry expected within 1-2 reads.**

**⚡ 2026-08-28 (daily loop) — NO HITS; 21 lanes read ALL HITLESS (Fir 8th
window-front rep 56749324/325/386/390/395-401 + Rorqual 11th rep 19530229-238 +
Nibi 20249828 = last n=43 2nd-rep lane, set COMPLETE); all 4 clusters reached.**
Reads: Fir (3,13,0,0) flat w4-8 (56749324/325/386/390/395) tested 23.6-26.1M
(cum 358.6-368.7M, aborted=0, arms 142-143/178, cells_done_sum=0), (5,9,6,6)
w1-3 (56749396-398) 15.3-16.2M (cum 173.0-175.5M), (5,7,2,10) w1-3
(56749399-401) 15.8-17.7M (cum 160.8-175.0M); Rorqual (3,13,0,0) rev w1-5
(19530229-233) tested 28.8-30.9M (cum 361.3-365.3M, aborted=0) — cell
completions continue on rev fronts (19530230 cells_done_sum=2 orbit_dup=22;
third consecutive cycle), (1,7,8,8) w1-2 (19530234/235) 20.4/19.8M (cum
226.7/219.9M), (5,5,8,8) w1-2 (19530237/238) 15.9/15.6M (cum 184.1/184.1M) —
A/D budget-aborts still elevated (13-97/lane, telemetry only, consistent since
08-20); Nibi n=43 (8,-2,5,9) w11 2nd rep (20249828) tested 22.4M (cum 85.0M,
arms 158/178, aborted=2) — Nibi 2nd-rep set 20249823-828 COMPLETE all hitless.
Workhorse (3,13,0,0) accounting: deep w0-3 ~1.28B + flat fronts w4-8 ~1.82B +
rev fronts w1-5 ~1.82B ≈ 4.92B tested ≈ 1.85x n=42's ~2.66B comparable fall
depth — grinding, no tilt trigger. Board at check: Fir 9th rep w4-8 56917602-606
R (~7.5h in) + 9th rep sA/sB 56917607-612 PD Priority + 10th rep 57086727-737 PD
= depth 2 (⇒ 11th-rep top-up due); Rorqual 12th rep 19666521-529 R (~1.8-3.9h
in) + 13th rep 19742337-345 PD = depth 2 (⇒ 14th-rep top-up due); Trillium
wave-19 2nd rep 2178717-722 now R (~4-4.4h in, first runtime after days PD) +
wave-20 2192770-781 PD Dependency = 3 reps, no action; Nibi lever-18 tail-class
lanes 20506642-646 PD Priority (still zero runtime — queue latency), n=43 lanes
now EMPTY — no n=43 restack this cycle: lever-18 is Nibi's queued work and 6
more n=43 singletons in the same account queue would contend with the
pre-registered hedge; revisit once lever-18 gets runtime. Tilt-criterion
bookkeeping: NEW n=43 read this cycle (Nibi w11 cum 85M) — far below the
~450M/class line, no tilt, grind continues. rung_status check run for the idle
rail: EXHAUSTED = SA-refill forbidden as always (this is firsthit ckpt work, not
SA). ACTIONS: Fir 11th-rep top-up (11 lanes x1 singleton) + Rorqual 14th-rep
top-up (9 lanes x1 singleton) via duo_run, verbatim configs, same CKDIRs.
FIR 11TH REP IN, all 11 echoed `Submitted batch job`: 57238023-027 (F44w4-8),
57238028-030 (F44sA1-3), 57238031-033 (F44sB1-3) — Fir back to depth 3 (9th
R/PD + 10th PD + 11th PD). RORQUAL 14TH REP IN, all 9 echoed `Submitted batch
job`: 19838962/964-967 (R44rC1-5), 19838968/969 (R44rA1-2), 19838970/971
(R44rD1-2; no 19838963) — Rorqual back to depth 3 (12th R + 13th PD + 14th PD);
fleet 4/4 covered.
Checker exclusions +21 (56749324/325/386/390/395-401, 19530229-238, 20249828).**

**⚡ 2026-08-27 (daily loop) — NO HITS; 25 lanes read ALL HITLESS (Fir 7th
window-front rep 56601161-171 + Rorqual 10th rep 19526686-694 + Nibi n=43 2nd rep
20249823-827); all 4 clusters reached.** Reads: Fir (3,13,0,0) flat w4-8
(56601161-165) tested 24.7-28.5M (cum 334.2-341.8M, aborted=0, arms 142-143/178),
(5,9,6,6) w1-3 (56601166-168) 16.8-17.5M (cum 157.6-159.4M), (5,7,2,10) w1-3
(56601169-171) 16.7-17.6M (cum 144.8-157.2M), cells_done_sum=0 all Fir lanes;
Rorqual (3,13,0,0) rev w1-5 (19526686-690) tested 29.4-31.1M (cum 327.1-336.9M,
aborted=0) — cell completions CONTINUE on rev fronts (cells_done_sum 2/0/1/2/0,
orbit_dup 55-60 on three lanes; second consecutive cycle), (1,7,8,8) w1-2
(19526691/692) 20.5/19.2M (cum 205.3/200.3M), (5,5,8,8) w1-2 (19526693/694)
16.5/17.4M (cum 169.5/168.5M) — A/D budget-aborts still elevated (23-144/lane,
telemetry only, consistent since 08-20); Nibi n=43 (8,-2,5,9) w9-11 2nd rep
(20249823-827, 5 of 6 lanes) tested 18.3-21.5M (cum 70.5-85.5M, arms 158/178).
Workhorse (3,13,0,0) accounting: deep w0-3 ~1.28B + flat fronts w4-8 ~1.69B + rev
fronts w1-5 ~1.66B ≈ 4.63B tested ≈ 1.74x n=42's ~2.66B comparable fall depth —
grinding, no tilt trigger. Board at check: Fir 8th rep 56749324-401 R (~2h in) +
9th rep 56917602-612 PD = depth 2 (⇒ 10th-rep top-up due); Rorqual 11th rep
19530229-238 R (~6.7h in, 9 lanes — no 19530236) + 12th rep 19666521-529 PD =
depth 2 (⇒ 13th-rep top-up due); Trillium wave-19 2nd rep 2178717-722 PD Priority
+ wave-20 2192770-781 PD Dependency = 3 reps queued, no action; Nibi 20249828
(N43r11, last 2nd-rep lane) PD + lever-18 tail-class lanes 20506642-646 PD
Priority (still zero runtime — queue latency), no action. Tilt-criterion
bookkeeping: NEW n=43 reads this cycle (Nibi w9-11 cum 70-86M) — far below the
~450M/class line, no tilt, grind continues. rung_status check run for the idle
rail: EXHAUSTED = SA-refill forbidden as always (this is firsthit ckpt work, not
SA). ACTIONS: Fir 10th-rep top-up (11 lanes x1 singleton) + Rorqual 13th-rep
top-up (9 lanes x1 singleton) via duo_run, verbatim configs, same CKDIRs.
FIR 10TH REP IN, all 11 echoed `Submitted batch job`: 57086727-731 (F44w4-8),
57086732-734 (F44sA1-3), 57086735-737 (F44sB1-3) — Fir back to depth 3 (8th R +
9th PD + 10th PD). RORQUAL 13TH REP IN, all 9 echoed `Submitted batch job`:
19742337-341 (R44rC1-5), 19742342/343 (R44rA1-2), 19742344/345 (R44rD1-2) —
Rorqual back to depth 3 (11th R + 12th PD + 13th PD); fleet 4/4 covered.
Checker exclusions +25 (56601161-171, 19526686-694, 20249823-827).**

**⚡ 2026-08-26 (daily loop) — NO HITS; 20 lanes read ALL HITLESS (Fir 6th
window-front rep 56591139-143/148/157-161 + Rorqual 9th rep 19526677-685); all 4
clusters reached.** Reads: Fir (3,13,0,0) flat w4-8 (56591139-143) tested
29.4-32.8M (cum 310.0-314.2M), (5,9,6,6) w1-3 (56591148/157/158) 18.6-18.8M (cum
140.5-141.5M), (5,7,2,10) w1-3 (56591159-161) 16.8-18.2M (cum 128.1-139.6M),
aborted=0, arms 141-168/178; Rorqual (3,13,0,0) rev w1-5 (19526677-681) tested
29.1-31.4M (cum 302.0-308.0M, aborted=0), (1,7,8,8) w1-2 (19526682/683) 19.5/19.7M
(cum 185.8/180.7M), (5,5,8,8) w1-2 (19526684/685) 17.3/16.9M (cum 154.0/150.0M) —
A/D budget-aborts still elevated (27-131/lane, telemetry only, consistent since
08-20). NOTE: first nonzero cells_done on workhorse rev lanes (19526677
cells_done_sum=4 orbit_dup=47; 19526680 sum=1 orbit_dup=13) — deepest rev arms are
now COMPLETING whole cells. Workhorse (3,13,0,0) accounting: deep w0-3 ~1.28B +
flat fronts w4-8 ~1.56B + rev fronts w1-5 ~1.52B ≈ 4.36B tested ≈ 1.64x n=42's
~2.66B comparable fall depth — grinding past the 08-25 overdue-analysis line, no
tilt trigger. Board at check: Fir 7th rep 56601161-171 R (~5.5h in) + 8th rep
56749324-401 PD = depth 2 (⇒ 9th-rep top-up due); Rorqual 10th rep 19526686-694 R
+ 11th rep 19530229-238 PD = depth 2 (⇒ 12th-rep top-up due); Trillium wave-19 2nd
rep 2178717-722 PD Priority + wave-20 2192770-781 PD Dependency = 3 reps queued,
no action; Nibi n=43 2nd rep 20249823-827 R (first Nibi runtime since the 08-21
submit — queue latency cleared) + 828 PD, lever-18 lanes 20506642-646 PD Priority,
no action. ACTIONS: Fir 9th-rep top-up (11 lanes x1 singleton) + Rorqual 12th-rep
top-up (9 lanes x1 singleton) via duo_run, verbatim configs, same CKDIRs.
FIR 9TH REP IN, all 11 echoed `Submitted batch job`: 56917602-606 (F44w4-8),
56917607-609 (F44sA1-3), 56917610-612 (F44sB1-3) — Fir back to depth 3 (7th R +
8th PD + 9th PD). RORQUAL 12TH REP IN, all 9 echoed `Submitted batch job`: 19666521-525 (R44rC1-5),
19666526/527 (R44rA1-2), 19666528/529 (R44rD1-2) — Rorqual back to depth 3 (10th
R + 11th PD + 12th PD). rung_status check run
for the idle rail: EXHAUSTED = SA-refill forbidden as always (this is firsthit
ckpt work, not SA). Tilt-criterion bookkeeping: NO new n=43 reads this cycle
(Nibi lanes R, Trillium all PD) — sibling cums unchanged vs the ~450M/class line,
no tilt, grind continues. Checker exclusions +20 (56591139-143/148/157-161,
19526677-685).**

**⚡ 2026-08-25 (Daniel session) — OVERDUE ANALYSIS + LEVER 18: workhorse (3,13,0,0)
~4.06B tested, almost all post-dedup => ~40x n=42's DISTINCT cost (~91M), hitless;
n=43 published ~10x overdue. Verdict: n>=43 is >=10x harder per distinct candidate
than the shallow 41->42 trend — uniformly sparser OR concentrated classes empty
(indistinguishable without breadth). Five n=44 classes essentially untested
((9,9,0,4) 0, (3,5,0,12) 0, (1,13,2,2) ~9M, (7,11,2,2) ~8M, (5,11,4,4) ~20M; slow
streamers deprioritized 07-31). LEVER 18 = exploration hedge: 1 canonical lane each,
flat skip-0, Nibi (--account=def-ikotsire_cpu, -J N44tA..E -d singleton), ~10% of
fleet; pre-registered: <0.5M after 1 rep => stream-walled -> one rev try -> drop.
CONFIRMED IN: Nibi 20506642-646 = N44tA (9,9,0,4) / tB (3,5,0,12) / tC (1,13,2,2) / tD (7,11,2,2) / tE (5,11,4,4), all PD; Nibi's n=43 2nd rep 20249823-828 PD Priority behind. Percentage-of-space answer for Daniel:
<0.1% of cells exhausted, ~1e-4..1e-7 of the candidate stream — correct but the
wrong yardstick (ordered search); depth-vs-hit-history is the yardstick, above.
Fleet otherwise 4/4 stacked 2-3 deep (loop's first clean-sweep Duo day; nudges +
commit-as-you-go confirmed working: 3 commits).**

**⚡ 2026-08-25 (daily loop) — NO HITS; 20 lanes read ALL HITLESS (Fir window-front
5th rep 56591125-138 + Rorqual 8th rep 19479909-917); all 4 clusters reached, first
zero-missed-Duo day since the hourly re-push shipped.** Reads: Fir (3,13,0,0) flat
w4-8 (56591125/27/28/30/32) tested 36.1-37.4M (cum 277.7-283.8M), (5,9,6,6) w1-3
(56591133-135) 19.7-20.2M (cum 121.2-122.3M), (5,7,2,10) w1-3 (56591136-138)
17.2-18.7M (cum 110.8-120.9M), aborted 0-2, arms 142-168/178; Rorqual (3,13,0,0)
rev w1-5 (19479909-913) 29.9-33.1M (cum 271.9-276.9M, aborted=0), (1,7,8,8) w1-2
(19479914/915) 20.1/19.7M (cum 165.1/162.8M), (5,5,8,8) w1-2 (19479916/917)
16.6/17.1M (cum 136.7/132.9M) — A/D budget-aborts still elevated (45-102/lane,
telemetry only, consistent since 08-20). Workhorse (3,13,0,0) accounting: deep w0-3
~1.28B + flat fronts w4-8 ~1.40B + rev fronts w1-5 ~1.38B ≈ 4.06B tested ≈ 1.5x
n=42's ~2.66B comparable fall depth — variance territory still, but the class is now
half again past the n=42 precedent. Board at check: Fir 6th rep R + 7th rep
56601161-171 PD = 2 reps/lane (depth-3 rule ⇒ top-up due); Rorqual 9th rep
19526677-685 R + 10th 19526686-694 PD + 11th 19530229-238 PD = 3 deep, no action;
Trillium wave-19 2nd rep 2178717-722 PD with NOTHING behind (⇒ wave-20 restack due,
same trigger as 08-22); Nibi 2nd rep 20249823-828 PD Priority (queue latency days —
1 rep queued is normal cover there, no action). ACTIONS: Fir 8th-rep top-up (11
lanes x1 singleton) + Trillium wave-20 (6 lanes x2 singleton) via duo_run. FIR
TOP-UP IN, all 11 echoed `Submitted batch job`: 56749324/325 (F44w4/w5),
56749386 (F44w6), 56749390 (F44w7), 56749395-401 (F44w8, sA1-3, sB1-3) — Fir
back to depth 3 (6th R + 7th PD + 8th PD). TRILLIUM WAVE-20 IN, all 12 echoed:
2192770-775 (1st rep T43f6/r6/f7/sA/sB/sC) + 2192776-781 (2nd rep, singleton) —
Trillium 3 reps queued behind wave-19's 2178717-722. rung_status check run for the idle rail:
EXHAUSTED = SA-refill forbidden as always (this is firsthit ckpt work, not SA).
Tilt-criterion bookkeeping: NO new n=43 reads this cycle (Trillium/Nibi lanes all
PD) — sibling cums unchanged vs the ~450M/class line, no tilt, grind continues.
Checker exclusions +20 (56591125/27/28/30/32-38, 19479909-917).**

**⚡ 2026-08-24 (later) — HOURLY DUO RE-PUSH shipped (Daniel: "if I miss a push, ask
again every hour until accepted"). Three files, all bash -n clean + stub-tested
(missed-then-approved, never-approved cap, duo_run retry ok/give-up, missed-list
parsing, no false NEW-HIT on the new output shape, real FOUND still alerts):
(1) check_all_retry.sh — per-cluster attempt is now check_one(); after the first
pass, every missed cluster gets a HIGH-priority phone nudge then a fresh push each
RETRY_INTERVAL (3600s) for up to RETRY_MAX (10) rounds; tunables RETRY_MAX /
RETRY_INTERVAL / RETRY_NUDGE(45s) / DUO (stub-able); "check starting" count is
dynamic; NEW FOUND parser also stops at cluster banners / retry / missed lines.
(2) duo_run.sh — nudge before every push + ONE bounded retry (RUN_RETRIES=1,
RUN_RETRY_WAIT=300s) because it runs inside the agent session (hours of blocking
would drop the API connection); sources notify.conf. (3) daily_auto.sh — main pass
runs the checker with RETRY_MAX=0 (today's read is never delayed), parses MISSED
from the Summary line, and AFTER the main agent pass spawns a SUPPLEMENTARY=1 child
(own log auto_<date>_supp.log) for the missed clusters: hourly re-push, then a
prompt-prefixed agent run that reads/restacks ONLY those clusters; zero taps on the
first pass => exec straight into the supplementary for all four; supplementary
never spawns another; give-up notification names the manual re-check command.
Manual use unchanged: CLUSTERS="fir" ./cluster/deploy/check_all_retry.sh now
re-pushes hourly until approved (Ctrl-C to stop).**

**⚡ 2026-08-24 (Daniel session) — LOOP HARDENING after the 08-23 failure: root cause =
"API Error: Connection closed mid-response" after a 3.5 h headless run (log
results/auto_2026-08-23.log); the loop correctly refused to retry (agent had already
restacked Rorqual 19479900-917 -> double-submit risk) BUT nothing was committed, so
the cycle's reads were lost until 08-24 reconstructed them, and a missed Fir Duo the
same day left Fir IDLE until the 08-24 restack. FIXES SHIPPED: (1) daily_auto.sh —
any post-action death (not just usage limits) now preserves the agent's uncommitted
edits as an "auto: PARTIAL run" commit + bounded push, counts "Submitted batch job"
echoes in the run log, and the phone text says exactly that (bash -n OK); (2)
auto_prompt.md — COMMIT-AS-YOU-GO hard rule (commit reads before any submit; commit
IDs after each cluster's submits) and STACK DEPTH rule (maintain THREE singleton reps
per lane on Fir/Rorqual so a missed Duo + a failed loop cannot idle a cluster).
DEPTH-3 CONFIRMED IN (Daniel's pastes, all echoed): Fir 7th rep 56601161-171 (11
lanes, Dependency behind 56591125-161 = 33 window-front jobs 3 deep); Rorqual 11th
rep 19530229-238 (9 lanes, Dependency; 8th rep 19479915-917 R, 9th+10th 19526677-694
PD) — both clusters now survive a missed Duo + a failed loop without idling. Workhorse (3,13,0,0) total
~3.7B tested (~1.4x n=42's fall depth) — variance territory, not yet anomalous.**

**⚡ 2026-08-24 (daily loop) — NO HITS; 47 lanes read ALL HITLESS (Fir 3rd+4th
window-front reps 55911371-392 — first Fir data since 08-21 — + Nibi 1st rep
20249817-822 + Rorqual 6th-rep tail 19379861-864 + full 7th rep 19479900-908 +
Trillium wave-19 1st rep 2178711-716); Fir IDLE + Rorqual 8th rep R with nothing
behind ⇒ DOUBLE RESTACK via duo_run, all 40 echoed `Submitted batch job`: Fir
56591125-161 = 11 flat lanes x2 singleton (5th+6th window-front rep), Rorqual
19526677-694 = 9 rev lanes x2 singleton (9th+10th rep) — verbatim configs, same
CKDIRs resume.** ⚠️ BOOKKEEPING NOTE: the 08-23 loop read Rorqual 5th rep
19379847-855 + 6th-rep C lanes 19379856-860 and Trillium wave-18 2nd rep
2126198-203 (all hitless) and RESTACKED Rorqual 19479900-917, updated the checker
exclusions, but never wrote HANDOFF or committed — that cycle is recorded here and
folded into today's commit. Reads: Fir (3,13,0,0) flat w4-8 tested 42.8-58.3M/rep
(cum 243.9-248.1M), (5,9,6,6) w1-3 (cum 100.8-101.9M), (5,7,2,10) w1-3 (cum
93.0-101.6M); Rorqual (3,13,0,0) rev w1-5 7th rep 32.4-33.3M (cum 241.0-247.0M),
(1,7,8,8) w1-2 cum 145.3/142.5M, (5,5,8,8) w1-2 cum 120.0/117.0M (aborts still
elevated on A/D, 23-107/lane, telemetry only); Nibi n=43 (8,-2,5,9) f9-11/r9-11
1st rep 18.5-24.5M (cum 52.2-64.0M); Trillium (8,-2,5,9) f6 19.8M (cum 210.6M) /
r6 30.6M (cum 296.0M, deepest lane fleet-wide) / f7 19.8M (cum 211.2M), siblings
(6,8,5,7) 23.7M (cum 158.7M) / (8,10,1,3) 9.4M (cum 124.6M) / (0,2,1,13) 22.6M
(cum 142.7M), all 178/178. Workhorse accounting: (3,13,0,0) deep w0-3 ~1.28B +
flat fronts w4-8 ~1.23B + rev fronts w1-5 ~1.22B ≈ 3.7B tested — ~1.4x past
n=42's ~2.66B comparable fall depth. Board after action: Fir 22 PD (5th rep
Priority + 6th Dependency), Rorqual 19479909-917 9 R (8th rep, ~3.3h left) +
19526677-694 18 PD, Trillium 2178717-722 6 PD (wave-19 2nd rep), Nibi
20249823-828 6 PD — fleet 4/4 double-stacked. rung_status check run for the idle
rail: EXHAUSTED = SA-refill forbidden as always (this is firsthit ckpt work, not
SA). Tilt-criterion bookkeeping: Trillium sibling cums now 124.6-158.7M flat-end
vs the ~450M/class line — no tilt, background grind continues. Checker exclusions
+47 (55911371-392, 20249817-822, 19379861-864, 19479900-908, 2178711-716).**

**⚡ 2026-08-22 (daily loop) — NO HITS; 13 lanes read ALL HITLESS (Rorqual 4th-rep tail
19215949-953 + Nibi 19683932/933 + Trillium wave-18 FIRST rep 2126192-197 — first
Trillium data since 08-15); Trillium's last rep was R with nothing behind it ⇒ WAVE-19
RESTACKED via duo_run: 2178711-722 = 6 verbatim lanes x2 singleton (T43f6/T43r6/T43f7/
T43sA-C), all 12 echoed `Submitted batch job`.** Reads: Rorqual (3,13,0,0) rev w5 36.3M
(cum 151.3M — matches w1-4's 144.7-149.2M band), (1,7,8,8) w1-2 22.1/21.5M (cum
82.5/80.8M, aborted 2/19), (5,5,8,8) w1-2 18.6/17.7M (cum 67.5/64.7M, aborted 32/65 —
the elevated A/D aborts persist, telemetry only); Nibi n=43 (8,-2,5,9) rev lanes
22.1/22.4M (cum 38.8/39.3M, 158/178); Trillium n=43 178/178 all six — published
(8,-2,5,9) f6 21.1M (cum 170.0M) / r6 32.5M (cum 234.3M, deepest lane fleet-wide) /
f7 21.4M (cum 170.6M), siblings (6,8,5,7) 27.5M (cum 109.8M) / (8,10,1,3) 12.1M (cum
104.0M) / (0,2,1,13) 22.0M (cum 98.1M). Board after action: Fir UNREACHED (Duo 180s
missed; per 08-21 ledger 55911371-392 = 22 booked), Rorqual 19379847-855 9 R (5th rep,
headers only) + 19379856-864 PD, Trillium 2126198-203 6 R (2nd rep, ~7.5h left,
finish ~20:40 EDT) + wave-19 2178711-722 12 PD singleton, Nibi 20249817-828 12 PD —
fleet 4/4 double-stacked. rung_status check run for the idle rail: EXHAUSTED =
SA-refill forbidden as always (this is firsthit ckpt work, not SA). Tilt-criterion
bookkeeping: n=43 sibling cums now 98-110M flat-end (T) vs the ~450M/class line —
no tilt, background grind continues per the 08-20 foreground/background decision.
Checker exclusions +13 (19215949-953, 19683932/933, 2126192-197).**

**⚡ 2026-08-21 (daily loop) — NO HITS; Fir window-front SECOND rep (55203769-780, all
11 lanes) + Rorqual 4th-rep C1-4 (19215945-948) ALL HITLESS; Fir idle + Rorqual/Nibi
queues emptying ⇒ TRIPLE RESTACK via duo_run, all 52 echoed `Submitted batch job`:
Fir 55911371-392 = 11 flat lanes x2 singleton (3rd+4th window-front rep), Rorqual
19379847-864 = 9 rev lanes x2 singleton (5th+6th rep), Nibi 20249817-828 = 6 n=43
lanes x2 singleton (NEW -J names N43f9-11/N43r9-11 — Nibi now in the singleton
discipline; its running unnamed 19683932/933 had ~30 min left at submit and Nibi
queue latency is days, so no CKDIR-collision window).** Reads: Fir 2nd rep —
(3,13,0,0) flat w4-8 tested 69.2-72.1M (cum 146.5-148.9M), (5,9,6,6) w1-3 24.3-25.0M
(cum 53.0-53.5M), (5,7,2,10) w1-3 17.0-19.0M (cum 53.9-61.2M), cells_done_sum=0;
Rorqual 4th rep (3,13,0,0) rev w1-4 tested 35.7-36.7M (cum 144.7-149.2M, aborted=0 —
the elevated aborts were the A/D lanes, still R). WATCH ITEM 19215948 RESOLVED: it
finished with tested_cum=148.7M >> tested=35.8M and resume_pi_max=31,193 (the known
CKDIR high-water) ⇒ it RESUMED the canonical CKDIR, no _oc0 fork; the missing
[orbitcanon] header on 08-20 was just the arm log mid-start; no scancel needed.
Workhorse accounting: (3,13,0,0) total ≈ deep w0-3 ~1.28B + flat fronts w4-8 ~0.74B +
rev fronts w1-5 ~0.70B ≈ 2.7B tested — now PAST the ~2.66B comparable accounting at
which n=42 fell. Board after action: Fir 22 PD (first rep starting), Rorqual 5 R
(19215949-953, ~6h left) + 18 PD, Trillium wave-18 2126192-197 finally R (~5h in,
first data tomorrow) + 198-203 PD singleton, Nibi 19683932/933 finishing ~13:35 (read
tomorrow) + 12 PD — fleet 4/4 double-stacked. rung_status check run for the idle
rail: EXHAUSTED = SA-refill forbidden as always (this is firsthit ckpt work, not SA).
Tilt-criterion bookkeeping: NO new n=43 reads this cycle (Nibi lanes end after the
check; Trillium mid-run) — sibling cums unchanged vs the ~450M line, no tilt, grind
continues. Checker exclusions +15 (55203769-780, 19215945-948).**

**⚡ 2026-08-20 (Daniel session) — TILT MADE EXPLICIT: since lever 17 (08-16) the loop's
verbatim restacks put Fir + Rorqual 100% on n=44 window fronts; Trillium (n=43
published f6/r6/f7 + siblings sA-C, 2126192-203) has been PD Priority since 08-15;
Nibi is the only n=43 compute running => siblings frozen at ~76-184M, the 450M tilt
line is NOT being approached. DECISION (recorded, not drifted): n=44 = FOREGROUND
(Fir+Rorqual: window fronts w1-8 both ends + deep w0-3), n=43 = BACKGROUND
(Trillium+Nibi only; siblings advance at T+N pace; the 450M line becomes a watch, not
a fleet gate). Rationale: record target is 44; fronts run ~3x/lane-day; published
n=43 class is the measured anomaly (~10x overdue); n=43 is optional for the record.
Progress framing: n=44 workhorse (3,13,0,0) total tested across w0-8 both ends is
now roughly 2.2B+ (deep w0-3 ~320M each + rev fronts 110-114M x5 + flat fronts ~80M
x5) — comparable accounting to the ~2.66B at which n=42 fell. NO estimate for n=43
is defensible (published class anomalous, siblings at background pace). Watch item
from loop: 19215948 arm log lacked [orbitcanon] header at read time (likely not yet
printed at job start) — loop rechecks 08-21; if it is a fresh-start _oc0 CKDIR,
scancel + resubmit with ORBIT_CANON=1. Fleet 4/4 booked, nothing to queue.**

**⚡ 2026-08-20 (daily loop) — NO HITS; Rorqual window-front THIRD rep read (19215936-944,
all 9 rev lanes) ALL HITLESS + Nibi 19683931 (n=43) HITLESS; fleet fully booked ⇒
bookkeeping only, no submits.** Reads (3rd rep on window-front CKDIRs): (3,13,0,0) rev
w1-5 (19215936-940) tested 37.7/37.0/38.5/38.0/37.7M (cum 110.1/113.9/114.2/112.5/112.8M
— w1-5 now ~3 reps deep, aborted=0, arms 142/178); (1,7,8,8) w1-2 (19215941/942)
22.3/22.4M (cum 61.1/59.8M, aborted 8/15); (5,5,8,8) w1-2 (19215943/944) 18.5/18.6M (cum
48.9/46.8M, aborted 57/69 — budget-aborts elevated on the deep-resume A/D lanes,
telemetry only). cells_done_sum=0 this rep (resume_pi_max holds at 31,193 high-water).
Nibi 19683931 n=43 (8,-2,5,9) rev lane HITLESS tested 21.9M (cum 39.4M, 158/178
summarized). Board: Fir 2nd rep 55203769-780 all 11 R (~5.5-6.6h in, headers only);
Rorqual 4th rep 19215945-948 R (~8h, headers only) + 949-953 PD Priority; Trillium
wave-18 2126192-203 all PD; Nibi 19683932/933 PD Priority — fleet 4/4 booked, no idle
capacity. WATCH: fh_arms_19215948 shows no-orbitcanon-header (fresh arm log mid-start;
sibling lanes 945-947 canonical 28.92x — recheck next read). rung_status check run for
the idle rail: EXHAUSTED = SA-refill forbidden as always (this is firsthit ckpt work,
not SA). Tilt-criterion bookkeeping: one n=43 read (Nibi, published class, cum 39.4M);
sibling cums unchanged vs the ~450M line — no tilt, grind continues. Checker exclusions
+10 (19215936-944, 19683931).**

**⚡ 2026-08-19 (daily loop) — NO HITS; Fir window-front FIRST rep read (54990112-122,
all 11 flat lanes) ALL HITLESS; fleet fully booked ⇒ bookkeeping only, no submits.**
Reads (first read on fresh window-front CKDIRs): (3,13,0,0) flat w4-8 (54990112-116)
tested 78.8/80.3/80.5/80.7/79.3M (arms 142/178 summarized, aborted=0, dedup 4x on flat);
(5,9,6,6) w1-3 (54990117/119/121) 27.8/27.8/28.0M (dedup 7.98x, aborted=1 each);
(5,7,2,10) w1-3 (54990118/120/122) 36.5/41.8/38.2M (dedup 4x). cells_done_sum=0 on all
11 — fresh fronts, no cells consumed yet (contrast Rorqual's 2nd-rep cells completing).
Board: Fir 2nd rep 55203769-780 now PD PRIORITY (dependency cleared, 11 lanes next up);
Rorqual 3rd rep 19215936-944 R (~6h in, headers only) + 4th rep 19215945-953 PD
Dependency; Trillium wave-18 2126192-203 all PD; Nibi 19683931 R (~5.6h, header only) +
932/933 PD — fleet 4/4 booked, no idle capacity. rung_status check run for the idle
rail: EXHAUSTED = SA-refill forbidden as always (this is firsthit ckpt work, not SA).
Tilt-criterion bookkeeping: NO n=43 reads this cycle (Nibi lane mid-run) — sibling cums
unchanged vs the ~450M line, no tilt, grind continues. Checker exclusions +11
(54990112-122).**

**⚡ 2026-08-18 (daily loop) — NO HITS; Rorqual window-front SECOND rep read
(19123948-956, all 9 lanes) ALL HITLESS; Rorqual queue EMPTY ⇒ 3rd+4th rep RESTACKED
via duo_run: 19215936-944 (R44rC1-5/A1-2/D1-2, PD starting) + 19215945-953
(singleton-Dependency) — all 18 echoed `Submitted batch job`, verbatim QUICK-REFERENCE
config (rev PROF_ORDER=2, ORBIT_CANON=1, budget 5e7, NARMS=178, skips 1-5/1-2/1-2),
same CKDIRs resume.** Reads (2nd rep): (3,13,0,0) rev w1-5 tested 37.0/39.6/39.2/38.4/
39.5M (cum 73.8/77.8/76.1/76.2/77.1M — the fresh windows now ~2 reps deep); (1,7,8,8)
w1-2 22.0/21.2M (cum 38.9/37.8M); (5,5,8,8) w1-2 17.2/15.7M (cum 30.4/28.5M). NOTE:
first cells COMPLETING on the (3,13,0,0) window lanes (cells_done_sum=1-2,
resume_pi_max to 31,193) — window fronts are being consumed, not just nibbled. Board:
Fir window-front 1st rep 54990112-122 R (~6-7h in) + 2nd rep 55203769-780 PD
Dependency; Trillium wave-18 2126192-203 all PD; Nibi 19683931-933 PD Priority — fleet
4/4 booked. rung_status check run for the idle rail: EXHAUSTED = SA-refill forbidden as
always (this is firsthit ckpt resume, not SA). Tilt-criterion bookkeeping: NO n=43
reads this cycle — sibling cums unchanged vs the ~450M line, no tilt, grind continues.
Checker exclusions +21 (19123948-956 this cycle's reads + 54681559-570, read 08-17
Daniel session but never excluded — they were reprinting in the FIRSTHIT section).**

**⚡ 2026-08-17 (later) — CODE-CURRENCY AUDIT (Daniel asked "any clusters on outdated
code?"): NO — every active lane fleet-wide shows [orbitcanon] headers = canonical
checkpointed solver; the "old" checker sections (SA tail, GATE PROBES) are July
archive files, not running jobs; only staleness = Nibi driver missing the cosmetic
lscpu line (solver current; refresh piggybacks on next Nibi touch). CANCEL NOTHING.
WZ-v3 edge summary recorded: WZ proved NNS(44)/NS(44) EMPTY (no classical shortcut
exists at 44); their v3 describes no orbit canon / no checkpointing / no validated
ordering => our search holds 3 measured edges over published SOTA. PASTE ISSUED: Fir
window-front SECOND rep x11 (-J F44w4-8/F44sA1-3/F44sB1-3, -d singleton = collision-
proof vs loop restacks). CONFIRMED IN: 55203769-780 (11 echoed, all PD Dependency
behind the Priority first reps 54990112-122 — Fir window fronts double-stacked).**

**⚡ 2026-08-17 (Daniel session) — Fir recheck + LITERATURE SWEEP: Fir 5th rep
54681559-570 read ALL HITLESS (n=43 flat cum 162-170M/lane; n=44 (3,13,0,0) w0-3 cum
309-338M; (5,9,6,6) 146M; (5,7,2,10) 143M; NOTE resume_pi_max 27,586/35,925 = deepest
arms have ENTERED ~77% of workhorse cell lists — flat w0-3 marginal value falling,
exactly what lever 17 addresses); Fir window-front lanes 54990112-122 STILL PD
Priority (no flat-side lever-17 data yet). LITERATURE: Wang-Zhu has a v3 (2026-02-05)
— still constructs ONLY 41-43, "n>43 open" stands, NS nonexistence extended to n=46,
NNS(42)/(44) counterexamples unchanged; NO other 2026 base-sequence papers found =
nobody has scooped n=44. Their described C,D method (Hall polynomial test at 200
angles, Thm 2.4) VERIFIED PRESENT in our solver (hall_ok/hall_ok_single,
wz_match.cpp:103-140, per-seq + per-pair) — stream-filter parity with the published
state of the art confirmed at source level. No new lever; no action; fleet booked.**

**⚡ 2026-08-17 (daily loop) — NO HITS; 9 lanes read (Rorqual LEVER-17 WINDOW-FRONT
FIRST rep 19123939-947, the first-ever reads of the fresh n=44 windows) ALL HITLESS;
no idle capacity ⇒ bookkeeping only, no submits.** Reads: (3,13,0,0) rev w1-5
(R44rC1-5 = 19123939-943) tested 38.6/39.9/39.3/40.5/39.9M (arms 142/178
summarized, aborted=0); (1,7,8,8) rev w1-2 (19123944/946) 16.9/16.7M; (5,5,8,8)
rev w1-2 (19123945/947) 13.2/12.8M (A/D lanes 138/178, orbit dedup 7.88x on the
fresh windows vs 3.81x on skip-0). These are first-read depths on fresh CKDIRs —
no prior cum. Board: Rorqual second rep 19123948-956 all R (40min-3h in at check);
Trillium wave-18 2126192-203 all PD; Nibi 19683931-933 PD (Priority); Fir Duo push
MISSED (180s) — unreached, its 5th rep 54681559-570 + 11 window-front flat lanes
54990112-122 unread this cycle. rung_status check run for the idle rail: EXHAUSTED
= SA-refill forbidden as always (this is firsthit ckpt work, not SA). Tilt-criterion
bookkeeping: NO n=43 reads this cycle — sibling cums unchanged vs the ~450M line,
no tilt, grind continues. Checker exclusions +9 (19123939-947).**

**⚡ 2026-08-16 (Daniel session) — DUPLICATE-SUBMIT INCIDENT + FIX: the 08-15
window-front pastes were run by Daniel TODAY, but the loop had already submitted
lever 17 itself on 08-16 (Fir 54990112-122 = 11 flat lanes; Rorqual 19123939-956 =
9 rev lanes x2 singleton). Daniel's copies (Fir 54994789-799, Rorqual 19130381-389,
all PD name=FIRSTHIT) are same-CKDIR duplicates -> concurrent same-lane jobs would
clobber per-arm checkpoints; SCANCEL PASTES ISSUED for the duplicate sets (keep the
loop's earlier, named, stacked copies); CONFIRMED CANCELLED from Daniel's squeue
output same session — both queues clean, only the loop's named lanes remain. Lesson for future pastes: any block older
than the latest loop notification must be re-validated against what the loop already
submitted — the loop can now execute pre-registered submits itself via duo_run.**

**⚡ 2026-08-16 (daily loop) — NO HITS; 32 lanes read (Fir wave-18 4th rep 12 +
Rorqual wave-18 BOTH reps 20, ALL HITLESS); Rorqual queue EMPTY ⇒ LEVER-17
WINDOW-FRONT SWEEP SUBMITTED BY THE LOOP (Daniel's 08-15 pastes never went in):
Rorqual 19123939-956 = 9 rev lanes ×2 singleton, Fir 54990112-122 = 11 flat lanes.
⚠️ DANIEL: do NOT paste the 08-15 window-front blocks — they are IN (all 29 echoed
`Submitted batch job`).** Reads: **Fir 4th rep 54681547-558 ALL HITLESS** — n=43
(8,-2,5,9) flat 0-5 tested 21.1-22.4M (cum 142-149M), n=44 (3,13,0,0) w0-3
28.6-30.3M (cum 293-306M!), (5,9,6,6) 17.0M (cum 130.4M), (5,7,2,10) 15.7M (cum
126.0M). **Rorqual wave-18 first rep 19023731/33-41 + second rep 19023742-45/71-76
ALL HITLESS** — n=43 sibling rev cum: (0,2,1,13) 131.7M, (0,2,7,11) 132.0M,
(0,10,5,7) 184.2M, (2,12,1,5) 169.0M, (4,6,1,11) 143.5M, (4,10,3,7) 138.5M; n=44
rev cum: (1,7,8,8) 187.9M, (3,3,4,12) 127.7M, (3,13,0,0) 306.3M, (5,5,8,8) 153.0M.
Window-front lanes as designed: Rorqual rev (3,13,0,0) w1-5 (-J R44rC1-5) +
(1,7,8,8) w1-2 (R44rA1-2) + (5,5,8,8) w1-2 (R44rD1-2), first rep starting + second
rep singleton-Dependency; Fir flat (3,13,0,0) w4-8 (F44w4-8) + (5,9,6,6) w1-3
(F44sA1-3) + (5,7,2,10) w1-3 (F44sB1-3), PD behind the running 5th rep (~9h in,
lands tonight ⇒ no idle gap). Fresh CKDIRs auto-keyed by skip, ORBIT_CANON=1,
budget 5e7, NARMS=178 — same config as all wave-13+ lanes, no code change. Board:
Fir 12 R + 11 PD, Rorqual 18 PD, Trillium wave-18 2126192-203 all PD, Nibi
19683931-933 PD (Priority) — fleet 4/4 covered. rung_status check run for the idle
rail: EXHAUSTED = SA-refill forbidden as always (this is firsthit ckpt resume, not
SA). Tilt-criterion bookkeeping: deepest sibling (0,10,5,7) rev 184M vs the ~450M
line — no tilt, grind continues (lever 17 already pre-empts it partially, +20 n=44
lanes). Checker exclusions +32 (54681547-558, 19023731-45, 19023771-76).**

**⚡ 2026-08-15 (Daniel session) — n=44 WINDOW-FRONT SWEEP designed + issued (lever 17,
docs/n44_search_narrowing_research.md): board audit against hit-location evidence
found the real gap — both banked hits came from MID-BAND windows (n=41 = rank 1429
of skip-8; n=42 = rev w4) but the n=44 board (shaped 07-31, never revisited) only
covers flat w0-3 + rev skip-0. Fresh-window FRONTS (flattest candidates of each
window) are the highest-EV untested space in the program. PASTES ISSUED: Fir 11 flat
lanes ((3,13,0,0) skips 4-8; (5,9,6,6)+(5,7,2,10) skips 1-3), Rorqual 9 rev lanes
((3,13,0,0) skips 1-5; (1,7,8,8)+(5,5,8,8) skips 1-2) — all fresh CKDIRs, no
collisions, ORBIT_CANON=1, additive (n=43 program untouched; tilt criterion stands).
Job IDs pending Daniel's Duo pastes.**

**⚡ 2026-08-15 (daily loop) — NO HITS; 9 lanes read (Trillium wave-17 SECOND rep all 6 +
Nibi restack first 3), ALL HITLESS; Trillium queue EMPTY ⇒ WAVE-18 double-stack RESTACKED
(2126192-203, all 12 echoed).** Reads: **Trillium wave-17 second rep 2089861/63/65/67/69/71
(6 lanes) ALL HITLESS** — n=43 published (8,-2,5,9) f6 22.4M (cum 149.0M) / r6 32.9M (cum
201.8M, deepest Trillium lane) / f7 22.7M (cum 149.2M); siblings FLAT (6,8,5,7) 28.6M (cum
82.2M), (8,10,1,3) 18.7M (cum 91.9M), (0,2,1,13) 27.6M (cum 76.1M). **Nibi 19683928/29/30
(n=43 (8,-2,5,9) resumed lanes) ALL HITLESS** — tested 17.9-18.7M each (cum ~33-35M),
158-159/178 summarized; 19683931-933 still PD (Priority). Board: **Trillium RESTACK (wave
18): 2126192-197 first rep (PD Resources) + 2126198-203 second rep (-J T43f6/T43r6/T43f7/
T43sA-C, singleton, all `Submitted batch job` echoed)**; Fir wave-18 fourth rep 54681547-558
all R (~6.3h in, fifth rep PD Dependency); Rorqual wave-18 first rep 19023731-741 all R
(~2.5h in, second rep PD); fleet 4/4 busy. rung_status check run for the idle rail:
EXHAUSTED = SA-refill forbidden as always (this is firsthit ckpt resume, not SA). Checker
exclusions +9 (2089861-871 odd, 19683928-930). Tilt-criterion bookkeeping: sibling cums
still 48-110M/class-end vs the ~450M line — no tilt, grind continues. No code change, no
new lever (program CLOSED — this is the grind).**

**⚡ 2026-08-14 (Daniel session) — PRE-REGISTERED TILT CRITERION (answering "is it
working"): the n=43 published-class anomaly (~10x past both rungs' distinct cost) was
caught 08-08 and answered by the 9-class diversification; the sibling bet is only
~3-4% evaluated (siblings 77-110M cum each vs ~2.66B-equivalent at which n=42
yielded). CRITERION, registered now: when EACH sibling class reaches ~450M cumulative
tested (~= one n=42-rung of distinct depth at 3.81x dedup) with all 9 n=43 classes
still hitless, n=43 is declared empirically sparse and HALF the n=43 fleet share
tilts to n=44 window breadth at the next natural restack. ETA at current throughput
~10-14 days; an allocation bump shortens it proportionally. No board change today
(wave-18 restacked by loop, fleet 4/4, dry-streak length still within both prior
rungs' pre-hit history).**

**⚡ 2026-08-14 (daily loop) — NO HITS; 28 lanes read (Fir 3rd rep + Rorqual wave-17
2nd rep + Trillium wave-17 1st rep, ALL HITLESS); Fir + Rorqual queues EMPTY ⇒ WAVE-18
double-stack RESTACKED on both, all 44 echoed.** Reads: **Fir third rep 53995221-234
(12 lanes) ALL HITLESS** — n=43 (8,-2,5,9) flat 0-5 tested 21.9-24.2M (cum 120-126M),
n=44 (3,13,0,0) w0-3 32.4-35.7M (cum 264-277M!), (5,9,6,6) 18.1M (cum 112.6M),
(5,7,2,10) 16.7M (cum 109.6M). **Rorqual wave-17 SECOND rep 18780489-507 odd (10
lanes) ALL HITLESS** — n=43 sibling REV (0,2,1,13) 28.3M (cum 76.7M), (0,2,7,11)
29.7M (77.8M), (0,10,5,7) 37.2M (110.4M), (2,12,1,5) 34.9M (99.3M), (4,6,1,11) 28.8M
(84.9M), (4,10,3,7) 29.0M (82.5M); n=44 REV (1,7,8,8) 21.0M (cum 145.8M), (3,3,4,12)
14.1M (98.7M), (3,13,0,0) 32.5M (cum 242.3M rev-side), (5,5,8,8) 17.5M (121.0M).
**Trillium wave-17 FIRST rep 2089860-870 even (6 lanes) ALL HITLESS, 178/178 arms**
— n=43 (8,-2,5,9) ×3 23.5-31.8M (cum 126.5-168.9M), (8,10,1,3) 35.4M (73.2M),
(0,2,1,13) 27.8M (48.5M), (6,8,5,7) 29.6M (53.6M). Board: **RESTACK (wave 18):
Rorqual 19023731-741 first rep + 19023742-776 second rep (10 verbatim rev lanes ×2,
-J R43sA-F/R44rA-D, singleton, all `Submitted batch job` echoed; first rep PD
Priority)** — first Fir Duo push MISSED (180s), retry seconds after the Rorqual
approval SUCCEEDED: **Fir 54681547-558 fourth rep + 54681559-570 fifth rep (12
verbatim flat lanes ×2, -J F43f0-5/F44w0-3/F44sA-B, singleton, all echoed; first rep
PD None = starting)**. Trillium wave-17 second rep 2089861-871 odd all R (~6.7h in,
lands tomorrow), Nibi restack 19683928-933: 3 R + 3 PD (Priority). rung_status check
run for the idle rail: EXHAUSTED = SA-refill forbidden as always (this is firsthit
ckpt resume, not SA). Checker exclusions +28 (53995221-234, 18780489-507 odd,
2089860-870 even). No code change, no new lever (program CLOSED — this is the
grind). n=44 workhorse (3,13,0,0) now ~264-277M flat + 242M rev.**

**⚡ 2026-08-13 (daily loop) — NO HITS; Rorqual WAVE-17 FIRST REP READ (10 lanes, all
hitless), fleet 4/4 busy, bookkeeping only.** New read: Rorqual 18780488/90/92/94/96/
98/500/02/04/06 ALL HITLESS (arms_with_hits=0/178 each) — first sibling SECOND-rep
data: n=43 sibling REV (0,2,1,13) 27.9M (cum 48.3M), (0,2,7,11) 27.4M (48.0M),
(0,10,5,7) 40.4M (73.2M), (2,12,1,5) 36.1M (64.3M), (4,6,1,11) 31.1M (55.9M),
(4,10,3,7) 30.2M (53.5M); n=44 REV (1,7,8,8) 20.4M (cum 125.2M), (3,3,4,12) 14.9M
(86.7M), (3,13,0,0) 33.3M (cum 210.2M rev-side), (5,5,8,8) 17.3M (102.6M). Board:
Rorqual second rep 18780489-507 (odd) all R ~3h (double-stack rolling as designed),
Fir third rep 53995221-234 RUNNING 12 lanes ~3h in (started 00:45-01:37 PDT), Trillium
wave-17 first rep 2089860-871: 6 R ~7:49 in + 6 PD (Dependency), Nibi restack
19683928-933 all PD (Priority). No cluster idle, no queue empty ⇒ NO submits, no code
change, no new lever (program CLOSED — this is the grind). Checker exclusions +10
(wave-17 Rorqual first rep). Next read: Trillium wave-17 first-rep data + Fir third
rep land tomorrow; Rorqual second rep too.**

**⚡ 2026-08-12 (paper data COMPLETE) — Fir chunked sacct: 839,449 ch => campaign
TOTAL 2,120,846 core-hours (~242 core-years) across all 4 clusters since 06-20,
measured. Nibi compute nodes = dual Intel Xeon 6 GRANITE RAPIDS 192c/766GB
(scontrol c148: feature "granite") — login's 8480+ was NOT the compute model (third
login-node trap caught). paper_methods_record.md hardware + compute sections now
COMPLETE; only open insert = exact Nibi SKU (minor) + n=43 provenance if found.
Day closed: fleet 4/4 covered, docs current, no builds (ledger 16 priced / 0 open).**

**⚡ 2026-08-12 (paper data landed) — sacct totals (since 06-20): Rorqual 804,037 ch,
Trillium 319,711 ch, Nibi 157,649 ch = 1.28M core-hours (~146 core-years) EXCLUDING
Fir (its slurmdbd rejects wide ranges; chunked query outstanding). Old "10-15
core-years" estimate superseded — off by ~10x. Fir compute nodes CONFIRMED dual EPYC
9655 (24 job lscpu outputs; login's 9135 is not the compute model). Nibi model still
pending (its deployed probe script predates lscpu logging) — scontrol query
outstanding. paper_methods_record.md §5 updated with all measured figures.**

**⚡ 2026-08-12 (later) — Paper-support day (solver blocked, campaign not): (1) coverage
accounting added to docs/paper_methods_record.md §7 — 12h lanes exhaust only tens of
cells vs 10^5-10^6 per class, cumulative <0.1%, so hitless waves bound nothing
(budget-bound not coverage-bound); (2) OUTSTANDING PASTE: paper-data collection
(Fir/Nibi compute-node CPU models via Model-name grep on firsthit outputs + sacct
core-hour totals on all 4 clusters) — closes the two "insert when available" items
in §5/§7 before Kotsireas's OverLeaf lands; (3) allocation-ask paragraph drafted for
Daniel's next email (fairshare healthy, queues competitive — framed as priority ->
search-days). No cluster action needed beyond the data-collection pastes.**

**⚡ 2026-08-12 (Daniel session, Rorqual recheck after missed Duo) — WAVE 17 FIRST REP
IS RUNNING on Rorqual: all 10 first-rep lanes 18780488/90/92/94/96/98/500/02/04/06
started 04:18-05:16 EDT today (~8-9h elapsed, ~3-4h left at 1:20 PM check; finish
~4-5 PM), second reps 18780489...507 PD (Dependency) behind them — double-stack
working as designed. Headers confirm the rebalance board: 6 n=43 sibling REV lanes
(0,2,1,13)/(0,2,7,11)/(0,10,5,7)/(2,12,1,5)/(4,6,1,11)/(4,10,3,7) + 4 n=44 REV
(1,7,8,8)/(3,3,4,12)/(3,13,0,0)/(5,5,8,8). Orbit canon live on n=44 lanes (7.88x/
3.95x/7.88x/28.92x). Outputs header-only (mid-run), NEW FOUND? none. NO ACTION:
Rorqual busy + fully stacked; fleet 4/4 covered today (Nibi restacked 19683928-933
this morning, Fir 3rd rep + Trillium wave 17 PD). Next read = tomorrow's loop, first
sibling SECOND-rep data lands there.**

**⚡ 2026-08-12 (daily loop) — NO HITS; Nibi re-entry set COMPLETE (all 6 hitless),
Nibi queue was EMPTY ⇒ verbatim RESTACK submitted, all echoed.** New read: **Nibi
19217376 HITLESS** — n=43 (8,-2,5,9), tested 16.9M (orbit_dup=557, 158/178
summarized, dedup 3.81x live), closing the 08-06 re-entry set 19217371-376 (6 lanes,
w9-11 both ends, ~95-100M total, all hitless). Nibi had NOTHING queued after it ⇒
anti-idle restack per 08-08/08-10 precedent: **19683928-933 = 6 verbatim resumed
CKDIR lanes (flat+rev, skips 9/10/11, oc1, all `Submitted batch job` echoed, all PD
12h)** — one Duo push, approved. Rest of fleet unchanged: **Fir third rep
53995221-234 all 12 PD (Priority)**, **Trillium wave 17 2089860-871 all 12 PD**,
**Rorqual Duo missed** (18780488-507 presumed PD per 08-11). rung_status check run
for the idle-cluster rail: EXHAUSTED = SA-refill forbidden as always (this was a
firsthit ckpt resume, not SA; its listed escalations were completed weeks ago).
Checker exclusions +1 (19217376). No code change, no new lever (program CLOSED —
this is the grind).**

**⚡ 2026-08-11 (Daniel session) — Nibi recheck: 19217376 RUNNING (~4h in, window 11);
19217375 read 08-10 (15.4M). PRIORITY DIAGNOSIS (Fir sprio): wave-17 jobs carry
priority ~1.526M, almost entirely FAIRSHARE component (1.5257M) + AGE 260 — the
fairshare standing is NOT collapsed; the queues are simply BUSY with higher-priority
competition. sshare returned headers only (association row not visible) — the earlier
"allocation ceiling" inference stands as queue-competition, softened from "fairshare
exhausted." Kotsireas allocation/RAC ask remains the one capacity lever, framed as
"long queues everywhere, a priority allocation converts to search-days." NO new
solver work opened: research ledger stays 16 priced / 6 shipped / 0 open. The board
is fully stacked and grinding as scheduled.**

**⚡ 2026-08-11 (daily loop) — NO HITS; Fir SECOND singleton rep read, fleet fully
stacked, bookkeeping only.** New read: **Fir 53882207-218 (12 lanes, second singleton
rep) ALL HITLESS** — n=43 (8,-2,5,9) flat 0-5 tested 23.2-25.2M/lane (cum 97-102M),
n=44 (3,13,0,0) w0-3 38.4-42.9M/lane (cum 233-244M!), (5,9,6,6) 21.2M (cum 94.4M),
(5,7,2,10) 19.0M (cum 92.7M). Queues: **Fir wave-17 third rep 53995221-234 all 12 PD
(Priority)** — singleton dependency satisfied, so Fir is momentarily idle waiting on
priority; per 08-07 allocation-ceiling verdict more queued jobs ≠ more throughput, so
no action. **Rorqual 18780488-507 all 20 PD** (10 Priority + 10 Dependency pairs),
**Trillium 2089860-871 all 12 PD** — wave 17 not yet started on either. **Nibi Duo
unapproved (skipped)**; 19217376 presumed still PD per 08-09/08-10 precedent. No
refill possible or needed (every reached cluster has a full queue), no code change,
no new lever (program CLOSED — this is the grind). Checker exclusions +12
(53882207-218).**

**⚡ 2026-08-10 (daily loop) — NO HITS (all 4 clusters reached); FULL READ of the
08-09 restack+rebalance; WAVE 17 DOUBLE-STACK submitted, all echoed, singleton-
serialized (2 jobs/lane => ~24h autonomous coverage per cluster).** Reads: **Fir
first singleton rep 53882195-206 ALL HITLESS** — n=43 flat 0-5 tested 24.6-26.1M
(cum 75-77M), n=44 (3,13,0,0) w0-3 52-56M/lane (cum 195-203M!), (5,9,6,6) 22.3M
(cum 72.8M), (5,7,2,10) 18.8M (cum 73.4M); second rep 53882207-218 R (~7h left at
check). **Rorqual: FIRST sibling-class read 18725943-948 ALL HITLESS** (rev skip-0,
20.4-32.8M each across (0,2,1,13)/(0,2,7,11)/(0,10,5,7)/(2,12,1,5)/(4,6,1,11)/
(4,10,3,7)); n=44 REV 18724010-013 hitless, cum (1,7,8,8) 106.0M / (3,3,4,12)
73.8M / (3,13,0,0) 177.8M / (5,5,8,8) 86.1M; 18724004-009 output files are
header-only = the rebalance-scancelled published rev lanes (no data, NOT an
anomaly). **Trillium: waves 15/16 (2061447-452, 2069143-148) + kept published
f6/r6/f7 (2077225-227, cum 102.8-137.2M) + FIRST sibling FLAT read (2077295-297:
(6,8,5,7) 24.1M / (8,10,1,3) 37.8M / (0,2,1,13) 20.7M) ALL HITLESS.** Nibi:
19217375 read hitless 15.4M (slow nodes), 19217376 still PD (Priority) — left
alone per 08-09 precedent. Rorqual+Trillium queues were EMPTY at check ⇒ **WAVE 17
submitted via duo_run, all 44 echoed: Rorqual `18780488-507` = 10 lanes × 2
(6 sibling rev + 4 n=44 rev, verbatim configs, -J names R43sA-F/R44rA-D +
--dependency=singleton so reps serialize); Trillium `2089860-871` = 6 lanes × 2
(T43f6/T43r6/T43f7 published + T43sA-C siblings); Fir `53995221-234` = third
singleton rep on all 12 lanes (queued Dependency behind the running rep, covers
tonight 20:10 EDT → tomorrow morning).** Checker exclusions +50 IDs. No code
change, no new lever (program CLOSED — this is the grind).**

**⚡ 2026-08-09 (late) — ORDERING EXPERIMENT VERDICT: FLAT-L1 CONFIRMED (0.9%/31.9%/
2.7% percentiles on the three known deep solutions — only consistently strong score;
PSD-peak wins a striking single case at n=43 (WZ solution = rank #1 of 1500) but fails
n=41 at 65% and the cell-rank dominates anyway — recorded, not actionable). The last
open research thread is CLOSED: 16 ideas priced this month, 6 shipped, 10 dead or
confirmed-baseline, 0 open. Board overnight: all 9 n=43 classes hunting both ends,
n=44 on 10+ lanes, Fir singleton-stacked 2 days deep, ordering formally optimal.**

**⚡ 2026-08-09 (session, cont.) — n=43 DIVERSIFICATION: published class is ~10x past
the distinct-orbit cost of BOTH previous rungs (a real statistical anomaly — sparse
class or bad luck, either way the EV now favors spreading); ALL 8 sibling classes
enumerated + triaged (norm 174: (0,2,1,13) (0,2,7,11) (0,10,5,7) (2,12,1,5) (4,6,1,11)
(4,10,3,7) (6,8,5,7) (8,10,1,3)) — 7/8 confirmed streaming with uniform ~3.81x orbit
dedup (8th completing). Ladder precedent: n=36 7/9 classes bore solutions, n=37 4/4.
REBALANCE EXECUTED, all echoed: Rorqual sibling REV lanes `18725943-948`
((0,2,1,13)/(0,2,7,11)/(0,10,5,7)/(2,12,1,5)/(4,6,1,11)/(4,10,3,7)) replacing the
scancelled fresh-PD published rev 0-5; Trillium sibling FLAT lanes `2077295-297`
((6,8,5,7)/(8,10,1,3)/(0,2,1,13)) replacing the 3 PD published. 8th sibling audit
completed: ALL 8 uniform ~3.81x, all streaming. Published class keeps Fir flat 0-5
(x2 singleton) + Trillium f6/r6/f7 R + Nibi 9-11 = largest single share. **ALL 9 n=43
CLASSES NOW UNDER SEARCH** — the n=36/37 multi-class pattern finally applied to n=43. Ordering-prior experiment (flat-L1 vs L2/PSD-peak/max-shift on
the 3 known deep hits) running in bg — any switch is a future validated change, not
tonight's lanes.**

**⚡ 2026-08-09 (Daniel manual check, full 4-cluster) — NO HITS; ~20+ NODE-DAYS LOST
TO IDLE (Fir empty since 08-08 morning after 2 missed Duo windows; Rorqual empty since
~midnight after burning waves 15 AND 16); RESTACK ISSUED with singleton double-stack
on Fir.** Reads: **Fir wave-14 (12 lanes, ran overnight 08-07→08) ALL HITLESS** — n=43
flat 0-5 tested 25.7-27.3M (cum 48-51M, aborts 2-300), n=44 (3,13,0,0) w0-3
66.7-70.0M/lane (cum 142-149M!), (5,9,6,6) 23.1M (cum 49.8M), (5,7,2,10) 18.5M (cum
53.9M). **Rorqual wave 15 + wave 16 BOTH complete hitless**: n=43 rev 0-5 cum 96-104M/
lane, n=44 rev (1,7,8,8) cum 81.5M / (3,3,4,12) 57.5M / (3,13,0,0) cum 143.2M /
(5,5,8,8) 67.9M. **Nibi first canonical reads (19217371-374) HITLESS**: n=43 w9-11
15.3-17.5M/lane (slower nodes), orbit_dup 307-557 = dedup skipping live, 375 R /
376 PD. n=44 workhorse (3,13,0,0) now ~293M combined both ends on a 35,925-orbit
class. Checker exclusions +36 IDs (53407677-89, 18622442-51, 18662519-28,
19217371-74). **RESTACK CONFIRMED, all echoed: Fir `53882195-218` (24 = 12 lanes × 2, singleton-
serialized per lane = 2 autonomous days) · Rorqual `18724004-013` (10 verbatim) ·
Trillium `2077225-230` (6; 3 started immediately). Fleet whole: 40 jobs queued/running
across 4 clusters, windows 0-11 both ends at n=43, workhorse + 5 classes at n=44.** Fir Duo misses are now the campaign's
main leak (3 of last 4 checks) — the singleton double-stack is the mitigation.**

**⚡ 2026-08-08 (daily loop) — NO HITS; wave-14 Rorqual tail + Trillium 2054696-701
ALL read HITLESS; WAVE 16 verbatim anti-idle STACKED on Trillium (2069143-148) +
Rorqual (18662519-528), all echoed, PD behind wave 15.** (1) Wave-14 Rorqual tail
18533304-310 completed HITLESS: n=43 rev (8,-2,5,9) tested 25.4-27.5M/lane
(tested_cum 45-48M), n=44 REV (1,7,8,8) 22.3M / (3,3,4,12) 14.6M / (3,13,0,0) 38.8M
/ (5,5,8,8) 16.8M, arms 134-158/178 summarized. (2) Trillium 2054696-701 (Daniel's
08-06 anti-idle stack) completed HITLESS: n=43 tested 27.2-36.8M/lane, arms 178/178,
dedup 3.81x. (3) Wave 15 mid-flight: Rorqual 18622442-451 all 10 R (~1h in, headers
02:00 EDT = likely requeued restart), Trillium 2061447-452 all 6 R (~11h in, finish
~14:00), Nibi 19217371-374 R (7-8.5h) + 375/376 PD. Fir UNREACHED this check (Duo
push not approved in 180s) — its 12 wave-14 lanes unread, presumed still PD
(priority-limited per 08-07 verdict); squeue %r confirmation still pending. (4) WAVE
16 stacked via duo_run, both approved, all 16 echoed: Trillium 2069143-148 = 6
verbatim n=43 flat+rev 6/7/8 (PD Resources — start as wave 15 drains); Rorqual
18662519-528 = 10 verbatim n=43 rev 0-5 + n=44 REV 4-sig board (PD). Same CKDIRs +
WZ_FH_ORBIT_CANON=1 ⇒ lanes resume, zero re-tread. Checker: 18533304-310 +
2054696-701 excluded as processed, narrative updated. No code change, no new lever
(program CLOSED — this is the grind).**

**⚡ 2026-08-07 (session close) — QUEUE-TIME LEVER PRICED DEAD (whole-node schedules
FASTEST: 192c 11:14:50 vs 32c 11:50:25 vs 16c 11:15:50 on Fir `--test-only`) ⇒ no
split-lane migration, META-Farm answer is settled, split-lane driver stays dormant.
COROLLARY: new whole-node job would start within the hour while 12 Fir lanes sit PD ⇒
those are PRIORITY-limited, not resource-limited ⇒ the fleet is at its allocation
ceiling; more queued jobs ≠ more throughput (confirm PD reason with squeue %r next
check). WAVE 15 SUBMITTED: Rorqual `18622442-451` (10: n=43 rev 0-5 + n=44 rev ×4),
Trillium `2061447-452` (6: n=43 flat+rev 6/7/8) — both stacked behind running lanes,
new driver (orbit_dup= GATEB + per-job lscpu) live on all four clusters. Board: Rorqual
7 R + 10 PD, Trillium 6 R + 6 PD, Fir 12 PD, Nibi 6 PD = 47 lanes. Levers now priced:
9 dead, 6 shipped, 0 open — the solver research program is CLOSED; remaining work is
the grind, the paper, and (if wanted) an allocation conversation.**

**⚡ 2026-08-07 (Daniel session) — GPU CLOSED (loop verdict confirmed: warp 24.0x,
sorted 2.8x, <60x rule => the research program's every lever is now PRICED); Trillium
`2054696-701` CONFIRMED = Daniel's 08-06 evening anti-idle stack (6 verbatim n=43
lanes, flat+rev 6/7/8 — ledger gap was mine, closed); SPLIT-LANE DRIVER built
(FH_SHARD_LO/HI: small jobs cover disjoint arm ranges of the same ckpt lane —
sbatch --cpus-per-task=32 backfill without losing exact resume; defaults byte-identical
= whole lane). Today: queue-time experiment (the Kotsireas META-Farm follow-up) via
sbatch --test-only estimates on Fir (192 vs 32 vs 16 cpus); wave-15 anti-idle stacks
on Rorqual (10 verbatim) + Trillium (6 verbatim) — first wave with orbit_dup= GATEB +
per-job lscpu on all clusters.**

**⚡ 2026-08-07 (daily loop) — GPU SPIKE V2 VERDICT: 24.0x ⇒ <60x PRE-REGISTERED RULE
⇒ GPU CLOSED PERMANENTLY (last unpriced lever now priced dead — every lever in the
n=44 narrowing table has a number); NO HITS; wave-13 Trillium read complete, wave-14
Rorqual 3/10 read, everything else R/PD — no refill, no code change.** (1) Spike2
53498573 (Fir h100, production budget 5e7, 6k cands): warp-cooperative kernel V2B
79.88 cands/s = **24.0x** vs 1 core (verdicts_nodes_match=YES, hist 0/0/5999/1),
V2A host-sorted 9.31/s = 2.8x, naive 8.20/s = 2.5x. Rule was >=200x build / 60-200x
marginal / <60x closed ⇒ CLOSED, no fix round-trip needed (kernel compiled and
cross-checked clean). CPU waves remain the engine. (2) Wave-13 Trillium 2012042-047
n=43 (8,-2,5,9) reverse: ALL HITLESS, tested 22.8-28.7M/lane, arms 178/178, dedup
3.81x live — first Trillium canonical read. (3) Wave-14 Rorqual partial: 18533301-303
n=43 rev COMPLETED HITLESS tested 27.3-28.6M (tested_cum 48-50M, arms 158/178
summarized); 18533304-310 still R (~2.6-5.3h in; n=44 REV lanes incl (3,13,0,0)
28.92x + (1,7,8,8)/(3,3,4,12)/(5,5,8,8) headers live). (4) Board otherwise: Fir
wave-14 53407677-687+689 all 12 still PD (Priority — day 1 in queue); Nibi re-entry
19217371-376 all 6 still PD; Trillium NEW 2054696-701 (6× n=43 (8,-2,5,9), resumed
CKDIRs, dedup 3.81x) R since ~08:00 EDT 08-07 — **these IDs are not in the ledger;
presumably Daniel's morning submit — Daniel: confirm/annotate**. NEW FOUND: none
anywhere. Checker updated: spike2 section VERDICT CLOSED, 2012042-047 + 18533301-303
excluded as processed. No idle capacity, nothing to submit.**

**⚡ 2026-08-06 (session close) — ALL FOUR PASTES LANDED: spike-v2 pricing QUEUED
(Fir `53498573`, h100, production budget, rule 200x/60x); NIBI RE-ENTERED with 6
canonical n=43 lanes (`19217371-376`, windows 9/10/11 both ends, PD); drivers with
GATEB orbit_dup= + per-job lscpu echo staged on ALL FOUR clusters (wave 15 telemetry
complete + paper hardware self-documenting). Login-node hardware recorded (Rorqual
EPYC 9654 / Trillium EPYC 9655 = 192-core compute-class; Fir/Nibi compute models
confirm via next wave's job headers). Board: wave 14 running (F+R), Trillium 6 lanes
running, Nibi 6 PD, spike2 PD. DONE FOR THE DAY — tomorrow's loop reads wave-14
verdicts + first Nibi lanes + the spike-v2 GPU verdict (checker sections armed for
all three).**

**⚡ 2026-08-06 (session, cont.) — PAPER RECORD COMPILED + GPU WARP-V2 BUILT (last
unpriced lever, ready to queue).** (1) `docs/paper_methods_record.md` = self-contained
paper input (both solutions + provenance job IDs/timings, method description, measured
cost table with redundancy correction, negative-results section, hardware TBD pending
lscpu fetch) — Daniel pastes it into Claude Desktop to draft the paper. (2) GPU spike
v2: warp-cooperative kernel (one candidate/warp, lane 0 drives DFS, 32 lanes
parallelize the O(L) place/prune loops via fused shift-partitioned updates + ballots)
+ V2A variant (naive kernel with host-side flatness sort). Job
`cluster/deploy/gpu_spike2.sh` (30min, h100, production budget 5e7, 6k cands, exact
CPU cross-check built in). PRE-REGISTERED: >=200x build / 60-200x marginal / <60x GPU
CLOSED permanently. Checker section added. CAVEAT: warp kernel is untestable locally
(no nvcc) — a compile error on Fir is possible; if nvcc fails the output says so and
one fix round-trip is expected. (3) Queue blocks issued: Nibi re-entry (6 canonical
n=43 lanes), combined driver-tar-pipe+lscpu per cluster (wave-15 GATEB orbit_dup= +
paper hardware specs), spike2 on Fir.**

**⚡ 2026-08-06 (Daniel session) — KOTSIREAS VERIFIED BOTH SOLUTIONS, PAPER IS ON
(his reply: "definitely correct... record all search details... I will take care of the
combinatorial-objects part of the paper"); timeline reply sent with the CORRECTED cost
model.** COST-MODEL CORRECTION (Daniel's instinct caught it): the n=41→n=42 "×4.7 cost
step" was a REDUNDANCY ARTIFACT — solved classes' orbit factors differ (n=41 (0,2,9,9)
7.53× vs n=42 (7,11,0,0) 29.18×); in DISTINCT ORBITS the rungs cost ~93M vs ~113M
(ratio 1.2, not 4.7). n=43 (3.81×) has already absorbed ~525M distinct orbits = ~4.6×
n=42's real cost — the rung is statistically OVERDUE, not weeks away (honest caveat:
per-orbit density varies by class; n=42 overran too). Paper deliverable opened:
compile `docs/paper_methods_record.md` (job IDs/timings/configs from HANDOFF+archive)
+ hardware-spec fetch block pending. STAB LEVER (lever 7) PRICED DEAD same session:
avg stabilizer 1.015-1.11 ⇒ ≤5-10% ceiling, not worth building. NIBI RE-ENTRY block
issued (tar-pipe canonical solver + 6 n=43 lanes windows 9/10/11 both ends,
--account=def-ikotsire_cpu). Remaining unpriced: GPU warp-v2 only.**

**⚡ 2026-08-06 (daily loop) — WAVE 13 FULL READ (FIR+RORQUAL): FIRST CANONICAL WAVE
ALL HITLESS, DEDUP CONFIRMED AT PRODUCTION SCALE (workhorse ~2-3x real throughput);
PLACE-V2 = DROP (x86 bench slower); WAVE 14 SUBMITTED, ALL ECHOED.** (1) Wave-13 Fir
`52894730-741` + Rorqual `18334381-390` completed 12h, arms_with_hits=0 everywhere.
Telemetry: Fir n=43 flat 0-5 tested 22.3-23.7M/lane (cum ~143-148M; per-wave rate
unchanged vs wave 11 — n=43's 3.8x dedup shows as distinct-orbit coverage, not
tested/day); n=44 (3,13,0,0) w0-3 76.5-81.8M/lane vs 28-44M in wave 11 = ~2-3x real
throughput (the 28.9x class; w0 first canonical read 76.5M, w1-3 cum ~216-250M);
(5,9,6,6) 25.9M + (5,7,2,10) 29.9M (~2x wave-11). Rorqual n=43 rev 0-5 19.4-21.7M
(cum ~132-148M); n=44 FIRST REVERSE reads (1,7,8,8) 16.0M / (3,3,4,12) 14.1M /
(3,13,0,0) 37.6M / (5,5,8,8) 13.2M. [orbitcanon] headers live on all three clusters,
dedup 3.81-28.92x, cells_orbit_dup 0-92. (2) PLACE-V2 VERDICT (53188641,
pre-registered rule): V2_wall=82s vs V1_wall=79s on Fir x86 = V2 SLOWER (~-4%);
correctness cross-check identical. Rule <5% => DROP — V1 stays production,
WZ_FH_PLACE_V2 stays dormant opt-in, no wave-15 default-on (Mac-ARM +6% did not
transfer to x86/AVX-512). (3) WAVE 14 SUBMITTED via duo_run, same board + same
configs incl WZ_FH_ORBIT_CANON=1 (same CKDIRs => lanes resume), all echoed, all PD
12h at submit: Fir `53407677-687`+`53407689` (12: n=43 flat 0-5 + n=44 (3,13,0,0)
w0-3 + (5,9,6,6) + (5,7,2,10)) · Rorqual `18533301-310` (10: n=43 rev 0-5 + n=44 REV
(1,7,8,8)/(3,3,4,12)/(3,13,0,0)/(5,5,8,8)). Wave 14 runs the on-cluster source — the
pending driver upgrade (GATEB orbit_dup= aggregate) still needs Daniel's tar-pipe
before wave 15; the checker's arm-log grep covers that telemetry meanwhile. Trillium
`2012042-047` (6 n=43 lanes) still R (~5h left at check) — first read tomorrow. Nibi
`18545822-24` stale PD, leave to lapse. Checker: wave-13 F+R IDs excluded as
processed, PLACE-V2 section marked VERDICT CLOSED; QUICK REFERENCE submit template
gained WZ_FH_ORBIT_CANON=1 (stale-template trap: without it the CKDIR forks to
_oc0).**

**⚡ 2026-08-05 (Daniel session) — CRY-WOLF NOTIFICATION FIXED + ORBIT VISIBILITY
CLOSED + PLACE-V2 LEVER BUILT (pricing on cluster).** (1) The 1:02 PM trophy title was
FALSE (no hit): daily_auto matched substring 'verified' inside "no verified solutions" —
fixed with a sentinel contract: trophy fires ONLY on a summary line starting
`RESULT_BANKED:` which auto_prompt now instructs the agent to write ONLY after a
verify_npaf-passed bank (tested both directions). (2) Checker audit found tomorrow's
canonical read would be BLIND to dedup telemetry (cells_orbit_dup aggregated nowhere):
checker now greps [orbitcanon] + cells_orbit_dup from newest arm logs (works tomorrow,
no cluster change); driver GATEB adds orbit_dup= (ships with wave-14 tar-pipe). (3)
n=44 REDUNDANCY TABLE completed: every class 3.9-8.0x ((1,7,8,8) 7.88 · (1,13,2,2) 7.98
· (3,3,4,12) 3.95 · (3,5,0,12) 7.59 · (5,5,8,8) 7.88 · (5,7,2,10) 4.0 · (5,9,6,6) 7.98
· (5,11,4,4) 7.88 · (7,7,4,8) 3.95 · (3,13,0,0) 28.92 measured 08-04; last two
computing). (4) NEW LEVER: WZ_FH_PLACE_V2 — branchless split-loop fh_place (the
completer's hottest O(L)-per-node loop; zero-entries contribute zero so the branch only
skipped vectorizable work). VALIDATED bit-identical at n=19 (V1 vs V2, incl. hit path;
per-candidate FH_PLACED_AT reset covers the FOUND-no-unwind case); Mac-ARM hint +6%;
the DECIDING x86/AVX-512 number = `cluster/deploy/place_v2_bench.sh` (30min, 1 core,
pre-registered: >=15% => default-on wave 15+, bit-identical so NOT in CFGSIG and lanes
resume unaffected; 5-15% keep opt-in; <5% drop). Wave 13 mid-flight, healthy, first
canonical verdicts tomorrow. NOT blocked.**

**⚡ 2026-08-05 (daily loop) — WAVE 13 FIRST READ: ALL 28 CANONICAL LANES HEALTHY, NO
HITS, INSUFFICIENT RUNTIME — NO ACTION.** Fir `52894730-741` all 12 R (~4.2-4.9h into
12h, started ~02:15 PDT; headers confirm canonical source: n=43 (8,-2,5,9) ×6 + n=44
(3,13,0,0) ×4 + (5,9,6,6) + (5,7,2,10)); Rorqual `18334381-390` all 10 R (~3.2h in,
headers confirm n=43 ×6 + n=44 REVERSE (1,7,8,8)/(3,3,4,12)/(3,13,0,0)/(5,5,8,8));
Trillium `2012042-047` all 6 PD (Priority — normal queue wait); Nibi `18545822-24`
still PD behind maintenance (stale old-driver, inert — leave to lapse). NEW FOUND:
none. Outputs are day-0 headers only, so no telemetry to record; first real
depth/dedup read (incl. cells_orbit_dup at n=43/44 production scale) comes at the
next check after the 12h walltime. Board full on all three productive clusters —
nothing idle, nothing to refill, no code change.**

**⚡ 2026-08-04 (late session) — 🔥 ORBIT CANONICALIZATION: 3.8-29x REDUNDANCY FOUND IN
THE CELL STREAM, LEVER BUILT + FULLY VALIDATED (the biggest solver gain since flat
ordering; Daniel's "think outside the box" session delivered).** The C,D cell list
never deduped its equivalence orbits (negC/negD/revC/revD/swap — all completion-
invariant): measured redundancy n=29 3.74x · n=42 29.2x · n=43 3.8x · n=44 (3,13,0,0)
28.9x / (1,7,8,8) 7.9x. The n=42 three-lane same-quad convergence WAS this waste, live.
Built WZ_FH_ORBIT_CANON (keep lex-min real cell per orbit; retention structural; CFGSIG
+oc; driver lanes _oc1; cells_orbit_dup telemetry). VALIDATED: canon-off bit-identical
(n=19); canon-on finds n=19 hit; **n=29 canary: SAME solution, idx 15,850 vs 26,694,
94s vs 149s — the dedup compresses the stream ahead of hits exactly as predicted**;
n=44 dedup 28.92x with flowing stream. EFFECT ON THE LADDER: n=43 ~month -> ~8-10 days
effective; n=44 effective cost deflates 8-29x per class — THE RECORD IS BACK IN RANGE
of the CPU fleet. CUTOVER EXECUTED SAME NIGHT — WAVE 13 SUBMITTED, all echoed:
Fir `52894730-741` (12: n=43 flat 0-5 + n=44 (3,13,0,0) w0-3 + (5,9,6,6)/(5,7,2,10)
skip-0, ALL oc1) · Rorqual `18334381-390` (10: n=43 REV 0-5 + n=44 REVERSE skip-0
(1,7,8,8)/(3,3,4,12)/(3,13,0,0)/(5,5,8,8) — n=44's FIRST reverse lanes, the n=42
lesson applied) · Trillium `2012042-047` (6: n=43 flat+rev 6/7/8). Tar-pipes landed
all 3 BEFORE submits (jobs compile at start = all wave-13 runs canonical).
WAVE-12 SCANCEL EXECUTED (Daniel, same night): 52885658-69 + 18333391-400 +
2011886-91 all cancelled while still PD — zero completed work lost, canon lanes
promoted a full day. FINAL BOARD: 28 canonical (_oc1) lanes ONLY — Fir `52894730-741`
(n=43 flat 0-5 + n=44 flat: (3,13,0,0) w0-3, (5,9,6,6), (5,7,2,10)) · Rorqual
`18334381-390` (n=43 rev 0-5 + n=44 REVERSE ×4) · Trillium `2012042-047` (n=43
flat+rev 6/7/8) · plus Nibi's 3 stale old-driver PD (18545822-24, inert). Every
node-hour from tonight runs deduplicated territory. Checker: wave-13 IDs un-excluded
until verdicts; wave-12 IDs never produced outputs (cancelled PD) — nothing to
exclude. Loose end: 9-class n=44 batch audit printed empty (single-class
runs fine) — re-run when tabulating. Kotsireas email: OFF per Daniel's decision —
progress updates on his own cadence; research continues in-house.**

**⚡ 2026-08-04 (Daniel session) — BS(43,42) RE-VERIFIED IN-SESSION (bank stands:
NPAF[s]=0 all s=1..43, norm 170, WZ encoding OK); WAVE 12 = FULL REALLOCATION to
n=43 + n=44.** Notable reads: the window-0 locator lanes (18288317-319, 104-122M) did
NOT produce the hit — the long-grinding REVERSE lanes did (reverse window 4, three-lane
convergence): classes hold MULTIPLE solutions and deep compounding lanes win; the
locator aim was sound but another solution sat closer to the reverse frontier. Sibling
(3,9,4,8) measured 152.8M/day (fastest n=42 class) — moot post-bank. **MEASURED COST
CURVE (replaces all band estimates): n=41 ≈ 0.7B, n=42 ≈ 3.3B (×4.7 step) ⇒ n=43
naive ≈ 10-15B ⇒ at ~24M/lane/day needs ~18 lanes × ~30 days. n=44 extrapolates
50-70B+ = CPU-infeasible without triage/methods — KOTSIREAS (brief now leads with BOTH
new solutions) is no longer optional for the record.** WAVE 12 (28 jobs): retire ALL
n=42 lanes + siblings + n=44 slow tail ((1,13,2,2)/(5,11,4,4)/(7,11,2,2) at 1-3M/day);
Fir 12 = n=43 flat 0-5 (0-2 resume cum ~124M, 3-5 fresh) + n=44 (3,13,0,0) w1-4 +
(5,9,6,6)/(5,7,2,10) skip0 resume · Rorqual 10 = n=43 rev 0-5 (0-2 resume cum ~126M,
3-5 fresh) + n=44 (1,7,8,8)/(3,3,4,12)/(3,13,0,0)/(5,5,8,8) skip0 resume · Trillium 6 =
n=43 flat 6/7/8 + rev 6/7/8 fresh. n=43 total = 18 lanes both ends. Nibi 18545822-24
PD untouched. **WAVE 12 CONFIRMED, all echoed: Fir `52885658-669` · Rorqual
`18333391-400` · Trillium `2011886-891`. LEVER 5 (symmetry lanes) killed by prior same
session: 0 of 31 banked solutions show any palindromic/anti-palindromic structure (best
0.77, typical 0.5-0.65 = random) — the research program's every lever is now MEASURED;
n=44 = lanes + obstruction triage + Kotsireas, by measurement not opinion.**

**⚡ 2026-08-04 (daily loop) — ★★★ n=42 FALLS: BS(43,42) FOUND, VERIFIED, BANKED — THE
SECOND RUNG OF THE WZ TRIO, BY THREE INDEPENDENT LANES.** (1) THE HIT: wave-11 lanes
Trillium `2007533` (rev ckpt, GLOBAL FIRST chronologically — hit at elapsed 2,280.7s,
~20:24 EDT 08-03), Trillium `2007532` (rev, elapsed 17,179s), and Rorqual `18266737`
(rev w4, elapsed 36,114s, ~00:09 08-04) ALL found the SAME quad (Rorqual's C,D =
Trillium's D,C — one solution up to C<->D swap), sig (-7,11,0,0) = the published class.
R2 COMPLETE: `verify_npaf.py` PASS on BOTH forms (NPAF[s]=0 all s=1..43, norm 170, WZ
comb8 encoding OK); NOT Wang-Zhu's sequences — checked against
`results/reference/wz_table1_bs43_42.txt` under the FULL 128-variant group (A<->B x
C<->D x negations x reversal), no match; C,D flat score 150 vs WZ's 142 = a NEW
inequivalent solution in the published class. BANKED:
`results/champions/champion_firsthit_bs43_42.txt` (Trillium 2007533 form + full
provenance). Claim discipline (same as n=41): independent replication + NEW inequivalent
solution; NO runtime comparison vs WZ; not a record. (2) IRONY FOR THE LOCATOR DOCTRINE:
the 08-03 window-0 skip lanes `18288317-319` read HITLESS at 104-122M tested each — the
hit came from the LONG-GRINDING REVERSE lanes (profile_rank 851/1572), not window 0; the
locator's "WZ-42 cell sits in window 0" remains true but OUR solution lived elsewhere
(same lesson as n=41: our solutions surface where cumulative depth is, not where WZ's
cell is). n=42 total effort at hit ≈3.4B tested across all lanes/windows/siblings —
~1.7x past the 1.4-2B band; the density model needs re-pricing before it prices n=43.
(3) EVERYTHING ELSE HITLESS (wave 11 + siblings first read + skip-0): n=43 cum flat
121-124M / rev 112-126M; n=44 (3,13,0,0) cum 135-169M (w1/2/3) + 113M (R skip-0), other
classes 7-85M cum; siblings (1,5,0,12)/(3,9,4,8)/(1,3,4,12) = 27.8/152.8/67.9M first
read — (3,9,4,8) is FAST (153M/day). Checker exclusions +31 (whole wave-11 board + ★x3).
(4) **NO WAVE 12 SUBMITTED — NEEDS_HUMAN: the n=42 bank retires 12 of 28 verbatim lanes
+ 3 siblings; reallocation is Daniel's call.** RECOMMENDED single move: paste the
continuation block below (19 uncontroversial lanes: n=43 + n=44 verbatim, n=42 dropped),
then decide the freed ~15-lane capacity next session (leading candidate: locator-guided
n=43 windows — WZ-43 measured deep at ranks ~255-571 on 08-03 — vs widening n=44).
Kotsireas brief: NOW STRONGER — two independent inequivalent WZ-class solutions (n=41 +
n=42); worth a one-line update before sending. STILL UNSENT. Continuation block
(3 pastes, one per cluster; `;`-chains, ckpt lanes resume automatically):
```
ssh dangord@fir.alliancecan.ca 'cd $SCRATCH/bs45 && for k in 0 1 2; do sbatch --requeue --export=ALL,WZ_N=43,WZ_A=8,WZ_B=-2,WZ_C=5,WZ_D=9,WZ_FH_PROF_ORDER=1,WZ_FH_AB_BUDGET=50000000,FH_NARMS=178,WZ_FH_PROF_SKIP=$k ./cluster_firsthit_probe.sh; done; sbatch --requeue --export=ALL,WZ_N=44,WZ_A=5,WZ_B=9,WZ_C=6,WZ_D=6,WZ_FH_PROF_ORDER=1,WZ_FH_AB_BUDGET=50000000,FH_NARMS=178,WZ_FH_PROF_SKIP=0 ./cluster_firsthit_probe.sh; sbatch --requeue --export=ALL,WZ_N=44,WZ_A=5,WZ_B=7,WZ_C=2,WZ_D=10,WZ_FH_PROF_ORDER=1,WZ_FH_AB_BUDGET=50000000,FH_NARMS=178,WZ_FH_PROF_SKIP=0 ./cluster_firsthit_probe.sh; sbatch --requeue --export=ALL,WZ_N=44,WZ_A=1,WZ_B=13,WZ_C=2,WZ_D=2,WZ_FH_PROF_ORDER=1,WZ_FH_AB_BUDGET=50000000,FH_NARMS=178,WZ_FH_PROF_SKIP=0 ./cluster_firsthit_probe.sh; for k in 1 2 3; do sbatch --requeue --export=ALL,WZ_N=44,WZ_A=3,WZ_B=13,WZ_C=0,WZ_D=0,WZ_FH_PROF_ORDER=1,WZ_FH_AB_BUDGET=50000000,FH_NARMS=178,WZ_FH_PROF_SKIP=$k ./cluster_firsthit_probe.sh; done; squeue -u dangord -h -o "%.14i %.10j %.2t %.11L %R"'
ssh dangord@rorqual.alliancecan.ca 'cd $SCRATCH/bs45 && for k in 0 1 2; do sbatch --requeue --export=ALL,WZ_N=43,WZ_A=8,WZ_B=-2,WZ_C=5,WZ_D=9,WZ_FH_PROF_ORDER=2,WZ_FH_AB_BUDGET=50000000,FH_NARMS=178,WZ_FH_PROF_SKIP=$k ./cluster_firsthit_probe.sh; done; sbatch --requeue --export=ALL,WZ_N=44,WZ_A=1,WZ_B=7,WZ_C=8,WZ_D=8,WZ_FH_PROF_ORDER=1,WZ_FH_AB_BUDGET=50000000,FH_NARMS=178,WZ_FH_PROF_SKIP=0 ./cluster_firsthit_probe.sh; sbatch --requeue --export=ALL,WZ_N=44,WZ_A=3,WZ_B=3,WZ_C=4,WZ_D=12,WZ_FH_PROF_ORDER=1,WZ_FH_AB_BUDGET=50000000,FH_NARMS=178,WZ_FH_PROF_SKIP=0 ./cluster_firsthit_probe.sh; sbatch --requeue --export=ALL,WZ_N=44,WZ_A=3,WZ_B=13,WZ_C=0,WZ_D=0,WZ_FH_PROF_ORDER=1,WZ_FH_AB_BUDGET=50000000,FH_NARMS=178,WZ_FH_PROF_SKIP=0 ./cluster_firsthit_probe.sh; sbatch --requeue --export=ALL,WZ_N=44,WZ_A=5,WZ_B=5,WZ_C=8,WZ_D=8,WZ_FH_PROF_ORDER=1,WZ_FH_AB_BUDGET=50000000,FH_NARMS=178,WZ_FH_PROF_SKIP=0 ./cluster_firsthit_probe.sh; squeue -u dangord -h -o "%.14i %.10j %.2t %.11L %R"'
ssh dangord@trillium.alliancecan.ca 'cd $SCRATCH/bs45 && sbatch --requeue --export=ALL,WZ_N=44,WZ_A=5,WZ_B=11,WZ_C=4,WZ_D=4,WZ_FH_PROF_ORDER=1,WZ_FH_AB_BUDGET=50000000,FH_NARMS=178,WZ_FH_PROF_SKIP=0 ./cluster_firsthit_probe.sh; sbatch --requeue --export=ALL,WZ_N=44,WZ_A=7,WZ_B=7,WZ_C=4,WZ_D=8,WZ_FH_PROF_ORDER=1,WZ_FH_AB_BUDGET=50000000,FH_NARMS=178,WZ_FH_PROF_SKIP=0 ./cluster_firsthit_probe.sh; sbatch --requeue --export=ALL,WZ_N=44,WZ_A=7,WZ_B=11,WZ_C=2,WZ_D=2,WZ_FH_PROF_ORDER=1,WZ_FH_AB_BUDGET=50000000,FH_NARMS=178,WZ_FH_PROF_SKIP=0 ./cluster_firsthit_probe.sh; squeue -u dangord -h -o "%.14i %.10j %.2t %.11L %R"'
```
**

**⚡ 2026-08-03 (Daniel session) — 🎯 THE LOCATOR MEASUREMENT: WZ's n=42 SOLUTION SITS IN
WINDOW 0 — THE CAMPAIGN'S SHALLOWEST-TESTED TERRITORY. Skip-0 ckpt lanes queued = the
most targeted aim of the campaign.** New instrument `WZ_FH_LOCATE_C/_D` (in solver,
default off): locates a known solution's C,D profile cell in the flat ordering, reports
rank bracket + window (=rank/178); checks all 64 swap/neg/rev variants. VALIDATED: our
banked n=41 hit brackets to windows 4-26 containing the cluster-true rank 1429/window 8
(4,043-cell tie block explains platform spread). RESULTS: WZ-41 = windows ~499-842 (deep
— why OUR shallower solution surfaced first); **WZ-42 (7,11,0,0) = cell_score 12, rank
bracket [0,111] = ENTIRELY WINDOW 0**; WZ-43 = windows ~255-571 (deep). THE HOLE: window
0 got ONE un-checkpointed day (wave 4, ~370k in-cell cands/arm) and was never revisited
— the window-selector doctrine (skips 1-6 one day each, ckpt lanes 7-9 now 250M+ each)
assumed wave-4 "covered" it; it didn't. ~2.66B went to windows 4-9 while a known
solution's cell sat in barely-touched window 0. **QUEUED+CONFIRMED (Rorqual, ckpt, flat): n=42 (7,11,0,0) skips 0/1/2 = `18288317-319`
(submitted ~17:40 after the SSH-key detour: digicopy overwrote ~/.ssh/id_ed25519 at
14:09 — unrelated project; resolved via NEW dedicated key ~/.ssh/alliance_ed25519 +
IdentitiesOnly config pin + CCDB registration; old key unrecoverable, old CCDB entry
stale-safe). Lane skip-0 grinds a window CONTAINING A PUBLISHED SOLUTION'S CELL,
compounding daily.** BOOKKEEPING CORRECTIONS from Daniel's cross-chat pastes:
(1) Fir sibling lanes WERE submitted 08-03 = `52718619-621` ((1,5,0,12)/(3,9,4,8)/
(1,3,4,12) flat skip-0 ckpt); (2) **Nibi 18545816-24 = the n=42 SIBLING sweep in
REVERSE order, NOT the published class** (sig headers: 816=(1,13,0,0) 135M streamed ·
817=(3,5,6,10) 100.7M · 818=(3,9,4,8) 96.2M · 819=(3,11,2,6) 102M · 820=(5,9,0,8)
97.3M · 821=(7,7,6,6) 101.8M — all hitless, old driver, STREAMED not tested, true depth
unknown; 822-24 still PD); (3) **TRAP REVISED: the "enumeration-bound" verdicts on
(5,9,0,8)/(1,13,0,0)/(3,5,6,10) are ORDER-DEPENDENT stream walls, not class properties**
— natural/flat-end stalls, reverse streams ~100M/day (misdiagnosed twice: wave-3/4 and
the 08-02 local triage). Checker exclusions +18545816-21. Density re-fit note: n=41 fell
INSIDE its band at ~700M; n=42 has overrun 1.4-2B by ≥1.3× — but the window-0 hole means
the band was measured against the WRONG coverage (flattest territory undertested);
re-price only after skip-0 lanes report. Kotsireas brief STILL unsent.**

**⚡ 2026-08-03 (daily loop) — WAVE 10 READ ALL HITLESS (28 lanes, F/R/T); WAVE 11
SUBMITTED = 28 verbatim ckpt resubmits; n=42 live-window cum now ~2.66B, well past the
band.** (1) Wave-10 numbers (arms_with_hits=0 everywhere, checkpoints advancing): n=42
(7,11,0,0) — Fir flat 7/8/9 tested 46-48M (cum 248/268/248M), Rorqual rev 4/5/6 60-63M
(cum 264-272M), Trillium rev 7/8/9 78-81M (cum 354-370M) → live-window cum sum ≈2.66B
(≈2.9B with burned flat 0-6), the 1.4-2B band estimate is now clearly optimistic — the
08-02 diversification hedge (3 sibling lanes on Fir) is the standing response; sibling
outputs were NOT in this check (they started ~01:30 EDT after wave 10 drained, finish
~13:30 — read them tomorrow by sig header, their job IDs were never recorded). n=43
(8,-2,5,9) cum: flat 98-102M, rev 89-101M (aborted 69-223/lane Fir, 0-9 Rorqual). n=44:
(3,13,0,0) confirmed workhorse again — Fir windows 1/2/3 tested 39/43/53M (cum 105-123M),
Rorqual skip-0 20.9M (cum 96.9M); other 9 classes 1.3-20.5M/wave. (2) **WAVE 11 submitted
~13:1x-13:2x EDT 08-03, all echoed** (Fir + Rorqual each needed retries — unapproved Duo
pushes; Rorqual took 3 attempts): Fir `52706408-419` (12 verbatim: n=42 flat 7/8/9, n=43
flat 0/1/2, n=44 (5,9,6,6)/(5,7,2,10)/(1,13,2,2) skip-0 + (3,13,0,0) windows 1/2/3) ·
Rorqual `18266737-746` (10 verbatim: n=42 rev 4/5/6, n=43 rev 0/1/2, n=44 ×4 skip-0) ·
Trillium `2007532-537` (6 verbatim: n=42 rev 7/8/9, n=44 ×3 skip-0). Nibi missed its Duo
window in the check — untouched (18545816-24, old driver). Board = 28 wave-11 + 3 Fir
siblings + 9 Nibi. Checker: exclusions +28 wave-10 IDs, pending → wave 11 + siblings.
rung_status = EXHAUSTED (SA retired, deliberate). No banks, no code changes. Kotsireas
brief: SEND-READY, still unsent — the methods ask is the door to 42+.**

**⚡ 2026-08-02 (Daniel session) — n=42 CLASS DIVERSIFICATION: the band-overrun response.**
n=42 (7,11,0,0) is ~2.3B tested, PAST its 1.4-2B band, hitless (P(no hit|band) ≈ 25% — not
an exclusion, but a signal). Our own ladder data (n=36: 7/9 classes bore solutions; n=37:
4/4) says solutions live in MANY classes per rung — concentration on the published class
was right until the band ran out; now we hedge. Sibling triage (local, 300 cands/class,
flat-score prior): **(1,5,0,12) median 150 · (3,9,4,8) 150 · (1,3,4,12) 154 — all with
min score 122, flatter than the n=41 hit (124) — LANES QUEUED on these 3** (flat, skip 0,
checkpointed, stacked on Fir behind wave 10). (1,13,0,0) + (3,5,6,10) = locally
enumeration-bound (240s, 0 candidates, PROF_ORDER=0) — deprioritized alongside (5,9,0,8).
Remaining 4 siblings ((3,11,2,6),(7,7,6,6),(7,9,2,6),(9,9,2,2)) still triaging — ranking
lands in scratchpad; wave-11 can swap if one ranks flatter. GPU verdict CLOSED by the loop:
secondary 5.9× at production budget = KILL (primary 69.3× was light-budget flattery);
all three throughput levers now measured dead — the program is AIM (lanes + triage +
Kotsireas), and the engine that took n=41 keeps grinding.**

**⚡ 2026-08-02 (daily loop) — GPU LEVER CLOSED: SECONDARY = 5.9× AT PRODUCTION BUDGET
= KILL; WAVE 9 READ ALL HITLESS (28 lanes); WAVE 10 SUBMITTED = 28 verbatim ckpt
resubmits on Fir/Rorqual/Trillium.** (1) GPU spike `52348541` secondary (budget 5e7 =
the budget production lanes actually run): cpu 3.31 / gpu 19.47 cand/s = **5.9× vs 1
core** (verdicts_nodes_match=YES) — deep-budget divergence collapsed the primary's
69.3× exactly as predicted; per the pre-registered rule (<30× ⇒ kill) the verdict is
CLOSED: **no production GPU build**; 1 H100 ≈ 0.03 CPU-node at production budget.
Recorded in `docs/n44_search_narrowing_research.md` lever 3; all three throughput
levers now priced dead (compression ×2, SAT-direct, GPU) ⇒ **the record program is CPU
lanes + class-triage theory + the Kotsireas brief (SEND-READY, still unsent — the
methods ask is the door)**. (2) Waves read 08-02, ALL HITLESS, checkpoints healthy:
Fir wave-8 `52169750-58` + Fir combined wave-9 `52348542-553` + Rorqual wave-9
`18099250-59` + Trillium wave-9 `1995240-45`. n=42 (7,11,0,0) cum per window: Fir flat
7/8/9 = 201-219M, Rorqual rev 4/5/6 = 204-209M, Trillium rev 7/8/9 = 257-270M —
**total n=42 cumulative ≈2.3B tested, now PAST the 1.4-2B expected band, still
hitless** (Poisson tail, no rule breached, but the band estimate is starting to look
optimistic — flag for the next density conversation). n=43 (8,-2,5,9) cum: flat 0/1/2
= 75-79M, rev 0/1/2 = 66-76M (aborts trending: 138-497/lane on Fir wave-8, 0-5 on
Rorqual wave-9). n=44: (3,13,0,0) confirmed the workhorse — windows 1/2/3 tested
64-70M each in ONE wave on Fir + skip-0 cum ~74M on Rorqual; other 9 fast classes
1.8-21M/wave. (3) **WAVE 10 submitted ~13:1x EDT, all echoed** (first Fir Duo push
timed out unapproved — retry succeeded; `;`-chains per the 08-01 lesson): Fir
`52480094-099` + `52480101-106` (12 verbatim: n=42 flat 7/8/9, n=43 flat 0/1/2, n=44
(5,9,6,6)/(5,7,2,10)/(1,13,2,2) skip-0 + (3,13,0,0) windows 1/2/3) · Rorqual
`18152036-045` (10 verbatim: n=42 rev 4/5/6, n=43 rev 0/1/2, n=44 ×4 skip-0) ·
Trillium `2000138-143` (6 verbatim: n=42 rev 7/8/9, n=44 ×3 skip-0). Nibi `18545816-24`
untouched (6 R staggered + 3 PD, old driver, no ckpt). Board = 37 jobs. Checker:
exclusions +28, GPU SPIKE header now carries the closed verdict, pending list → wave
10. rung_status = EXHAUSTED (SA retired, deliberate). No banks, no code changes.**

**⚡ 2026-08-01 (late) — GPU SPIKE PRIMARY RESULT: 69.3× vs 1 core = the MARGINAL band;
the naive port does NOT transform n=44.** Daniel's live peek mid-run (job `52348541`, H100
confirmed): `GPU_SPIKE: cpu_cands_per_s=5.74 gpu_cands_per_s=398.28 speedup_vs_1core=69.3
verdicts_nodes_match=YES` (budget 1e6, 20k real n=44 candidates; exact CPU/GPU cross-check
PASS — the port is correct). NODE MATH: 1 H100 ≈ 69 cores ≈ 0.36 of a 192-core CPU node —
node-for-node WORSE than the CPU fleet. Per the pre-registered rule (≥300 build / 30-300
marginal / <30 kill): NO production GPU build on this evidence; divergence-tolerant
restructuring (warp-per-candidate / persistent threads) is the only path to ≥300 and is
UNPRICED — do not start it without a new spike-level measurement. Secondary line (budget
5e7) still running; deep-budget divergence usually reads LOWER — tomorrow's loop reads it
via the new checker GPU SPIKE section and should close the verdict. Record program after
this week's measurements: throughput levers all priced (compression dead, SAT dead, GPU
marginal) ⇒ n=44 rides on CPU lanes + obstruction-theory triage + the Kotsireas
collaboration (brief SEND-READY). n=42 remains the near-term result engine.**

**⚡ 2026-08-01 (session close) — CLASS-KILLER TRIAGE: ALL 12 n=44 CLASSES SURVIVE
(negative, recorded).** Compression-as-existence-test (class dead iff no valid compressed
quadruple): n=41 control FEASIBLE at d=2/3/6/7 ✓ method sound; all 12 n=44 classes
feasible at d=3+d=5 → no free eliminations; deeper class triage = obstruction theory =
Kotsireas brief question 1. Day closes: 46 CPU jobs + GPU spike `52348541` queued; brief
SEND-READY; compression dead ×2 ways, SAT dead, GPU = the live decision. Tomorrow's loop
reads Fir wave-8 + the spike's GPU_SPIKE line (rule: ≥300× build / <30× kill).**

**⚡ 2026-08-01 (Daniel session, cont.) — COMBINED FIR PUSH LANDED: GPU spike `52348541`
(PD, gres=gpu:h100:1 after Fir's new GPU-model rule bounced the first attempt — note the
&&-chain lesson: one failed sbatch killed everything after it; blocks now use `;`) + 12
CPU lanes `52348542-553` (9 verbatim wave-9: n=42 flat 7/8/9, n=43 flat 0/1/2, n=44
first-3 skip-0 · 3 NEW n=44 (3,13,0,0) windows 1/2/3 — the 43.4M/day class widened).
Fir rolls straight through the night. Board = 46 jobs + the spike.**

**⚡ 2026-08-01 (Daniel session) — GPU SPIKE QUEUED TODAY (was the forgotten item);
Fir anti-idle stack + fast-class widening; KOTSIREAS BRIEF REWRITTEN (send-ready).**
(1) The combined Fir paste (ONE Duo push) ships+queues the GPU spike AND stacks Fir's 9
verbatim wave-9 resubmits behind its running wave-8 lanes (they finish ~19:30; without
the stack Fir idles ~17h until the 1 PM loop) AND widens n=44 (3,13,0,0) — the fastest
n=44 class at 43.4M/lane-day, 27× the slow tail — with fresh flat windows skip 1/2/3.
(2) `docs/kotsireas_brief.md` REWRITTEN from the stale 07-07 SA-era version: now leads
with the new inequivalent BS(42,41), the measured n=41-44 frontier, and three targeted
methods questions (class triage via his NS/NNS(44) obstructions; stronger aperiodic
compression; MathCheck-grade SAT+CAS viability) — SEND-READY, the send itself remains
Daniel's. GPU spike rule stands: ≥300× build / 30-300× marginal / <30× kill.**

**⚡ 2026-08-01 (daily loop) — WAVE 8 HALF-READ: RORQUAL (10) + TRILLIUM (6) ALL HITLESS;
WAVE 9 SUBMITTED = 16 verbatim ckpt resubmits on those two; Fir's 9 wave-8 lanes still
RUNNING (~5h in), Nibi's 9 still PD.** Wave-8 R+T numbers (arms_with_hits=0 everywhere,
checkpoints healthy): n=42 (7,11,0,0) reverse — Rorqual 4/5/6 tested 68.5/71.5/69.9M
(cum 137.7/141.7/139.1M), Trillium 7/8/9 tested 88.6/89.6/91.8M (cum 175.4/181.0/184.5M)
→ the n=42 attack is deep into its 1.4-2B expected band; n=43 (8,-2,5,9) reverse 0/1/2
(Rorqual) tested 26.6/25.4/23.5M (cum 47.1/45.4/41.4M, aborted 0/0/7); **7 NEW n=44
classes' first data (all flat skip-0, hitless): (1,7,8,8) 17.7M · (3,3,4,12) 17.6M ·
(3,13,0,0) 43.4M · (5,5,8,8) 15.0M · (5,11,4,4) 6.4M · (7,7,4,8) 19.7M · (7,11,2,2)
1.6M** — full 10-class n=44 per-lane cost table is now complete ((3,13,0,0) is the
fastest n=44 class measured so far; (7,11,2,2) and (1,13,2,2) are the slow tail).
Wave-9 submits ~13:1x EDT, all echoed: Rorqual `18099250-259` (10 verbatim: n=42 rev
4/5/6, n=43 rev 0/1/2, n=44 ×4) · Trillium `1995240-245` (6 verbatim: n=42 rev 7/8/9,
n=44 ×3). Board = 34 jobs. Checker exclusions +16 (18008165-74, 1989519-24); Fir
52169750-58 stay un-excluded until read. rung_status = EXHAUSTED (SA retired,
deliberate). No banks, no code changes. **GPU spike still UNQUEUED — needs Daniel's
tar-pipe (block below); Kotsireas brief still unsent.** Paste-ready GPU spike ship+queue
(Fir; 30min, 1 GPU):
```
cd ~/Projects/BS45_Quantum_Explorer && tar -cf - src/solver/gpu/fh_gpu_spike.cu cluster/deploy/gpu_spike.sh results/n44_cands_20k.txt | ssh dangord@fir.alliancecan.ca 'cd $SCRATCH/bs45 && tar -xvf - && cp -f results/n44_cands_20k.txt ./n44_cands_20k.txt && sbatch cluster/deploy/gpu_spike.sh'
```

**⚡ 2026-07-31 (daily loop) — WAVE 7 READ: ALL 15 LANES HITLESS (first n=44 cost data in
hand); WAVE 8 SUBMITTED = verbatim ckpt resubmits + the remaining 7 fast n=44 classes →
the FULL 10-class n=44 frontier is now live.** Wave-7 numbers (all arms_with_hits=0,
checkpoints healthy — tested_cum/resume_pi present everywhere): n=42 (7,11,0,0) +~753M
fresh tested this wave (Fir flat lanes 7/8/9 = 87.6/98.5/88.4M · Rorqual reverse 4/5/6 =
69.3/70.1/68.8M · Trillium reverse 7/8/9 = 86.8/91.4/92.7M); n=43 (8,-2,5,9) +~130M
(Fir flat 0/1/2 = 23.0/25.7/22.9M with aborted=254-846/lane — n=43 arms abort where
n=42's don't, watch the trend; Rorqual reverse 0/1/2 = 20.4/19.9/17.9M). **FIRST n=44
DATA: (5,9,6,6) 11.9M / (5,7,2,10) 17.9M / (1,13,2,2) 1.9M tested per 12h lane** —
hitless as expected; this is the per-class cost measurement the GPU/SAT sizing wanted,
and (1,13,2,2) tests 6-9× slower than its streaming rank suggested (streaming speed ≠
completion speed). Wave-8 submits ~13:25 EDT, all echoed job IDs: Fir `52169750-758`
(9 verbatim: n=42 flat 7/8/9, n=43 flat 0/1/2, n=44 first-3) · Rorqual `18008165-174`
(6 verbatim: n=42 rev 4/5/6, n=43 rev 0/1/2, +4 NEW n=44 flat skip-0 (1,7,8,8)
(3,3,4,12) (3,13,0,0) (5,5,8,8)) · Trillium `1989519-524` (3 verbatim n=42 rev 7/8/9,
+3 NEW n=44 (5,11,4,4) (7,7,4,8) (7,11,2,2)) · Nibi `18545816-24` still PD, untouched.
Board = 34 jobs. Slow n=44 classes (9,9,0,4)/(3,5,0,12) stay unassigned per 07-30
triage. rung_status = EXHAUSTED (SA retired, deliberate, no refill). Checker exclusions
+15 wave-7 IDs; wave-8 stays un-excluded until verdicts. No banks, no code changes.
Kotsireas brief: STILL unsent; rewrite (lead with BS(42,41)) still queued.**

**⚡ 2026-07-31 (Daniel session, cont.) — GPU SPIKE BUILT + VALIDATED LOCALLY; READY TO
QUEUE (the "something to queue" Daniel asked for).** `src/solver/gpu/fh_gpu_spike.cu`:
the fh_complete_ab mirror-pair DFS (root canon + reversal-tie canon + sum bounds +
Dab/Kab pruning + budget) ported to an ITERATIVE form that compiles BOTH as plain C++
(host validation) and CUDA (one candidate per thread). VALIDATION (local, host build):
verdict histograms EXACTLY match production at n=19 (807 cands: 1 hit/806 clean, and
canon-off 749/57) AND n=41 no-ABP (300 cands: 103 clean/197 abort); node counts within
0.005-0.13%, residue explained = production over-counts during abort unwinding (each
stack level re-places one branch before re-hitting the budget) + one tie-canon
order-difference on the n=19 hit tree — verdict-neutral, fine for a RATIO instrument.
ABP profile constraint deliberately NOT ported (spike measures architecture speedup;
production levers multiply). Driver `cluster/deploy/gpu_spike.sh` (30min, --gres=gpu:1,
compiles on-node, runs 20k real n=44 (1,7,8,8) candidates at budgets 1e6 + 5e7, prints
GPU_SPIKE: cpu_cands_per_s / gpu_cands_per_s / speedup + an exact CPU-vs-GPU
verdict/node cross-check). Candidate file shipped at `results/n44_cands_20k.txt`.
**PRE-REGISTERED RULE (before the number exists): speedup ≥300× ⇒ build the production
GPU completer (n=44 ≈ weeks-scale); 30-300× ⇒ marginal, decide vs GPU availability;
<30× ⇒ killed, record path = CPU grind + triage + Kotsireas.** SAT+CAS was killed at
canary earlier today — this is the last throughput lever; its number decides the
program's shape.**

**⚡ 2026-07-31 (Daniel session) — SAT+CAS LEVER: KILLED AT CANARY STAGE (one afternoon,
not the weeks a full build would cost).** Built `tools/sat_bs_encoder.py` (pysat/CaDiCaL;
XNOR product vars, exact-cardinality NPAF, nonneg-sum WLOG). Soundness gate PASS (banked
n=29 satisfies the encoding, instant); blind n=11 SAT in 0.9s with NPAF re-check 0 (the
encoding finds REAL solutions); **blind n=19: >120s (seqcounter) and >600s (totalizer +
A0 unit) vs firsthit's 0.2s = ≥3,000× deficit** — far beyond the pre-registered 100× kill
line, at a rung ~10 orders easier than n=44. Negative recorded in
`docs/n44_search_narrowing_research.md` (measured-dead block); direct encoding: do not
rebuild. The "would a REAL MathCheck-style PB+CAS system work?" question is delegated to
the Kotsireas email (he co-leads MathCheck). **Record program status after two kill
tests: compression DEAD (07-30), SAT-direct DEAD (07-31) — the GPU completer spike is
the sole surviving throughput lever and is the NEXT BUILD; class-triage theory + the
email are the aim levers. Fleet meanwhile: n=42 at ~1.55B cumulative = inside its
1.4-2B expected band — the next hit is likeliest there, any wave now.**

**⚡ 2026-07-30 (session close) — FIRST n=44 JOBS IN CAMPAIGN HISTORY QUEUED: Fir
`51834907-909` (classes (5,9,6,6), (5,7,2,10), (1,13,2,2) — the 3 fastest-streaming of
the 12; flat, skip 0, CHECKPOINTED lanes; PD behind Fir's 6 n=42/43 jobs, start as nodes
free). Board: 27 queued/running jobs — Fir 6× n=42/43 + 3× n=44, Rorqual 6× n=42/43
reverse, Trillium 3× n=42 reverse, Nibi 9× n=42 reverse. HANDOFF SPLIT this session:
pre-07-24 history → HANDOFF_ARCHIVE.md (~3,100 lines out of the per-session context
budget); QUICK REFERENCE rewritten for the firsthit/checkpoint era. Next builds queued
(none blocked on cluster results): SAT+CAS n=29 canary → GPU kernel spike → class-triage
theory → Kotsireas brief rewrite (leads with BS(42,41)).**

**⚡ 2026-07-30 (Daniel session, research) — n=44 RECORD PROGRAM WRITTEN + FIRST LEVER
VALIDATED ON REAL DATA.** Full ranked program: `docs/n44_search_narrowing_research.md`
(committed). Headlines: (1) **all 12 n=44 admissible classes STREAM** (50 cands each,
local; no enumeration-bound classes at the record rung; slow: (9,9,0,4) 242s,
(3,5,0,12) 113s — deprioritize). (2) **Đoković–Kotsireas COMPRESSION (arXiv:1302.0571)
route MATHEMATICALLY VALIDATED on our banked BS(42,41)**: pad C,D with one zero →
quadruple is PERIODIC-complementary at length n+1 (max |PAF sum| = 0 at all shifts,
verified numerically), and ALL six compressions (d=2,3,6,7,14,21 at L=42) pass exactly.
This is a filter axis WZ never used (their paper slices only mod 2/3/6; at n=44,
L=45=9·5 exposes mod-5/9 structure). KILL TEST RUN SAME SESSION — **VERDICT: KILLED at
the profile level.** Rejection on real streamed candidates: n=41 d=7 = 0.0% (0/1000),
n=44 d=5 = 0.1% (2/2000), n=44 d=9 = 0.6% (12/2000) — all far under the pre-registered
10% line (2.11a/b+2.12 survivors already satisfy compression almost surely). Negative
result RECORDED in the research doc's measured-dead block — do not rebuild; the
completion-level variant is deprioritized below SAT+CAS and GPU. Cost of the answer: ~1
laptop-hour. New permanent instrument: `WZ_FH_DUMP` candidate-dump env (default-off,
n=19 bit-identical re-verified with it off). BONUS MEASUREMENT: n=44 (1,7,8,8)
streamed-candidate flat-score distribution (N=2000): min=124 (= our n=41 hit's exact
score — solution-grade flatness EXISTS at the record rung) / p25=152 / median=160 /
max=200. Flat ORDERING (cell_order) suffices; no score-gate tiers needed at n=44.
OPTIONAL TONIGHT (Daniel's call): stack the FIRST 3 n=44 lanes on Fir behind its 6 PD
n=42/43 jobs (start as nodes free; checkpointed from minute one; classes = the 3
fastest-streaming: (5,9,6,6), (5,7,2,10), (1,13,2,2), flat, skip 0):
```
ssh dangord@fir.alliancecan.ca 'cd $SCRATCH/bs45 && sbatch --requeue --export=ALL,WZ_N=44,WZ_A=5,WZ_B=9,WZ_C=6,WZ_D=6,WZ_FH_PROF_ORDER=1,WZ_FH_AB_BUDGET=50000000,FH_NARMS=178,WZ_FH_PROF_SKIP=0 ./cluster_firsthit_probe.sh && sbatch --requeue --export=ALL,WZ_N=44,WZ_A=5,WZ_B=7,WZ_C=2,WZ_D=10,WZ_FH_PROF_ORDER=1,WZ_FH_AB_BUDGET=50000000,FH_NARMS=178,WZ_FH_PROF_SKIP=0 ./cluster_firsthit_probe.sh && sbatch --requeue --export=ALL,WZ_N=44,WZ_A=1,WZ_B=13,WZ_C=2,WZ_D=2,WZ_FH_PROF_ORDER=1,WZ_FH_AB_BUDGET=50000000,FH_NARMS=178,WZ_FH_PROF_SKIP=0 ./cluster_firsthit_probe.sh; squeue -u dangord -h -o "%.14i %.10j %.2t %.11L %R"'
```
First n=44 search jobs of the campaign: they measure per-class candidate cost + density
at the record rung (the experiment that grounds the whole GPU/SAT sizing) while hunting. (3) SAT+CAS
(MathCheck, Bright–Kotsireas–Ganesh — Williamson ≤70 record in the sibling class) = the
second-engine lever; canary = re-find n=29, then blind n=38. (4) GPU spike + class-triage
theory + symmetry minority lanes ranked behind. Cluster results are NOT a blocker for any
of this — solver R&D is local; lanes just get re-aimed as verdicts land. n=44 first lanes
ship with tomorrow's loop (10 fast classes, flat windows, checkpointed).**

**⚡ 2026-07-30 (Daniel session, cont.) — REDEPLOYMENT SUBMITTED + n=44 CAMPAIGN OPENED.**
Submits all landed ~14:40 EDT: Fir `51808294-296` (n=42 flat ckpt lanes 7/8/9) +
`51808297-299` (n=43 (8,-2,5,9) flat ckpt lanes 0/1/2) · Rorqual `17952942-944` (n=42
reverse 4/5/6) + `17952945-947` (n=43 reverse 0/1/2) · Trillium: n=41 lanes
`1970159/160/162` scancelled (class solved, were PD) → `1982605-607` (n=42 reverse ckpt
lanes 7/8/9). Nibi untouched. Fleet = 24 jobs, all on n=42/43 published classes + n=42
class sweep. **n=44 FIRST CONCRETE STEP: admissible frontier ENUMERATED = 12 signature
classes** (a,b odd / c,d even / norm 178; canonical nonneg a≤b,c≤d): (1,7,8,8) (1,13,2,2)
(3,3,4,12) (3,5,0,12) (3,13,0,0) (5,5,8,8) (5,7,2,10) (5,9,6,6) (5,11,4,4) (7,7,4,8)
(7,11,2,2) (9,9,0,4). Local stream-validation of all 12 launched (bounded 50-cand/300s
per class, PROF_ORDER=0 per the local-stall trap) → identifies non-empty vs
enumeration-bound classes = the first n=44 triage data. CLAIM DISCIPLINE for the n=41
result (Daniel asked): we may NOT claim faster-than-WZ (they published no runtime — no
comparison exists); the honest claims are (i) first independent BS(42,41) since WZ, (ii)
NEW inequivalent solution (score 124 vs 140), (iii) reproducible deterministic
architecture with a measured cost curve (~700M tested/class cumulative to the hit — the
4.9h was window 8's clock, not the search's total cost; the days of window-burning WERE
the search). Deterministic = same lane re-run finds the same solution at the same
position; the cost model (hit inside the 500M-1B band) validated on its first test.**

**⚡ 2026-07-30 (Daniel session) — BS(42,41) INDEPENDENTLY RE-VERIFIED in-session
(verify_npaf.py: NPAF[s]=0 all s=1..42, norm 166, WZ encoding OK — the bank stands).
FLEET REDEPLOYED to n=42 + n=43 (Daniel's call): the n=41 recipe (published-class flat
windows + checkpointed lanes + both ends), applied up-ladder.** Allocation: **Fir** (idle)
= n=42 (7,11,0,0) flat ckpt lanes skip 7/8/9 + n=43 (8,-2,5,9) flat ckpt lanes skip 0/1/2
(n=43 published sig from `results/reference/wz_table1_bs44_43.txt`; fresh rung, no burned
windows) · **Rorqual** (idle) = n=42 reverse ckpt lanes 4/5/6 + n=43 reverse ckpt lanes
0/1/2 · **Trillium** = scancel the now-redundant n=41 reverse lanes `1970159/160/162`
(PD, class solved, nothing lost) → n=42 reverse ckpt lanes 7/8/9 · **Nibi** = untouched
(9 PD n=42 reverse skip-0, still valid coverage tickets). Window ledger: n=42 flat burned
0-6, reverse burned 1-3 (+0 pending Nibi) — all new lanes fresh by construction. Cost
model from the n=41 hit (fell in window 8, ~700M cumulative on-class): ×2-3/rung thinning
⇒ n=42 ≈ 1.4-2B/class expected, n=43 ≈ 3-6B ⇒ wave-8+ = verbatim lane resubmits
(auto-resume) + the GPU spike is THE n=43/44-scale lever. **n=44 RESEARCH PROGRAM (next
session, Daniel greenlit out-of-box work): (1) GPU completer feasibility spike (measure
first: one cell's completion throughput, two numbers); (2) class-triage theory — mine WZ's
NS(44)/NNS(44) non-existence proofs for what they do NOT forbid at BS(45,44), rank the
n=44 admissible classes by obstruction invariants before spending compute; (3) symmetry-
restricted lanes (skew/palindromic subspaces) as a reach multiplier; (4) flat-score prior
modeling (4 deep-n data points now: ours 124, WZ 140/142/134). Honest frame stands: n=44
needs ~50-200B tested candidates across unknown-viability classes on CPU — the record
attempt is real only if GPU × triage both land.** KOTSIREAS: brief must be REWRITTEN to
lead with the new inequivalent BS(42,41) — strongest possible opener for the methods ask.**

**⚡ 2026-07-30 (daily loop) — 🎯 BS(42,41) SOLVED AND BANKED: THE CAMPAIGN'S TARGET EVENT.
Fir wave-6 job `51517707` (n=41 published WZ class (0,2,9,9), flat, window skip 8, FIRST
CHECKPOINTED lane) hit at elapsed 17708.9s (~4.9 h): GLOBAL FIRST idx=500000
profile_rank=1429 nodes_this_cand=212872 score=124. Full R2 done THIS RUN: Daniel approved
a duo_run.sh fetch of the banner; `tools/verify_npaf.py` PASS (NPAF[s]=0 all s=1..42, norm
166 exact, WZ pair encoding OK); banked → `results/champions/champion_firsthit_bs42_41.txt`
with full provenance; checker exclusions updated. It is a NEW solution, NOT Wang-Zhu's
published sequences: C,D flat score 124 vs their 140 (score is invariant under swap/
negation/reversal ⇒ inequivalent). First hit at n≥38 ever; first hit at n=41 — the flat
window+checkpoint architecture found it in the 8th window at ~4.9 h. HONEST FRAMING:
replication-class solver-capability result (WZ constructed 41-43); n=44 remains the record.
Checker fix: FIRSTHIT section previously did NOT grep FOUND banners inside firsthit files
(the hit surfaced only as arms_with_hits=1) — per-file grep now includes the banner,
sequences and VERIFY lines. Rest of fleet: Rorqual wave-5b `17637330-35` COMPLETE HITLESS,
~447M fresh tested on n=42 (7,11,0,0) (arms_summarized 164/178 = undercount); Fir siblings
`51517706/708` (skips 7/9) hitless, tested 31-34M with checkpoint telemetry live
(tested_cum/resume_pi present, aborted=0); Trillium wave-6 `1970159/160/162` still PD
(Priority); Nibi `18545816-24` still PD. NO submits — post-hit strategy is Daniel's call.
NEEDS_HUMAN: (1) DECIDE the pivot — n=41 class is cracked, obvious next move is
concentrating the fleet on n=42 (7,11,0,0)+reverse with checkpointed lanes (wave-7 verbatim
resubmits auto-resume), but that redeployment is yours to greenlight; (2) Trillium wave-6
lanes are PD and now partly redundant at n=41 — keep or scancel/repoint to n=42; (3) the
KOTSIREAS EMAIL — the brief predates this result and should now LEAD with it (first
independent replication of a WZ rung + ~590-680M-tested exclusion data); still unsent,
still the highest-leverage human action.**

**⚡ 2026-07-29 (Daniel session) — RESUME BUILD COMPLETE, ALL 5 GATES PASS (the loop's
"unvalidated WIP" note is superseded: the evening session HAD run the battery, holding the
commit for the overnight canary).** Gates: (a) n=19 resume-off BIT-IDENTICAL to HEAD
(idx=807/rank=2/nodes=8087); (c) resume-equivalence ×3 — mid-batch (ledger 20+17=37
EXACT), batch>0 (ckpt (215,1,5) → 15+18=33, identical solution AND final state),
non-buffered (400+407=807) — no gap, no overlap, identical hits every time; (d) CFGSIG
mismatch → refused loudly, fresh start; (b) n=29 blind re-find canary on the NEW binary:
FOUND idx=26694 EXACT fingerprint match to the 07-26 record (nodes=81320, ~149s),
`tools/verify_npaf.py` independent PASS (sig 0,-6,9,1; NPAF[s]=0 all s; WZ pair encoding
OK). Mechanism + env vars + deployment doctrine: spec
`docs/superpowers/specs/2026-07-28-per-arm-candidate-resume-design.md` + the 07-28 late
entry. **STRATEGIC RE-PRICING forced by waves 5+5b: the published n=41 class (0,2,9,9) now
totals ≈590-680M tested across both ends (front ~330-390M, reverse ~260-290M), hitless —
the COMBINED total has crossed the ~500M pessimistic band top. Under the band's own
assumptions P(still no hit) ≈ 30%, so not yet an exclusion — but the optimistic-to-mid
band is dead, and per the 07-28 pre-registered rule the GPU spike + Kotsireas methods ask
move from hedge to PLAN. Full per-end exclusion (~500M EACH end) needs ~2 more
resume-era rounds — which now cost zero re-tread.** Wave 6 TODAY (Fir + Trillium idle):
tar-pipe the ckpt build to all 4, then fresh lanes skip 7/8/9 per idle cluster; wave 7+ =
resubmit the SAME lanes verbatim (auto-resume). NEEDS_HUMAN: (1) today's 4-cluster
tar-pipe + Fir/Trillium wave-6 submits (blocks below); (2) greenlight the GPU feasibility
spike as the next session's build; (3) KOTSIREAS — now pre-registered-escalation, not just
leverage. Wave-6 paste blocks:
```
cd ~/Projects/BS45_Quantum_Explorer && tar -cf - src/solver/wz_match.cpp cluster/deploy/cluster_firsthit_probe.sh | ssh dangord@fir.alliancecan.ca 'cd $SCRATCH/bs45 && tar -xvf - && cp -f cluster/deploy/cluster_firsthit_probe.sh ./cluster_firsthit_probe.sh && for k in 7 8 9; do sbatch --requeue --export=ALL,WZ_N=41,WZ_A=0,WZ_B=2,WZ_C=9,WZ_D=9,WZ_FH_PROF_ORDER=1,WZ_FH_AB_BUDGET=50000000,FH_NARMS=178,WZ_FH_PROF_SKIP=$k ./cluster_firsthit_probe.sh; done; squeue -u dangord -h -o "%.14i %.10j %.2t %.11L %R"'
cd ~/Projects/BS45_Quantum_Explorer && tar -cf - src/solver/wz_match.cpp cluster/deploy/cluster_firsthit_probe.sh | ssh dangord@trillium.alliancecan.ca 'cd $SCRATCH/bs45 && tar -xvf - && cp -f cluster/deploy/cluster_firsthit_probe.sh ./cluster_firsthit_probe.sh && for k in 7 8 9; do sbatch --requeue --export=ALL,WZ_N=41,WZ_A=0,WZ_B=2,WZ_C=9,WZ_D=9,WZ_FH_PROF_ORDER=2,WZ_FH_AB_BUDGET=50000000,FH_NARMS=178,WZ_FH_PROF_SKIP=$k ./cluster_firsthit_probe.sh; done; squeue -u dangord -h -o "%.14i %.10j %.2t %.11L %R"'
cd ~/Projects/BS45_Quantum_Explorer && for c in rorqual nibi; do tar -cf - src/solver/wz_match.cpp cluster/deploy/cluster_firsthit_probe.sh | ssh dangord@$c.alliancecan.ca 'cd $SCRATCH/bs45 && tar -xvf - && cp -f cluster/deploy/cluster_firsthit_probe.sh ./cluster_firsthit_probe.sh'; done
```
(Rorqual/Nibi = tar-pipe only: Rorqual busy with 5b until ~03:00, Nibi PD; their queued
jobs compile the new source at start but run their old submit-time driver = checkpointing
stays OFF for them, bit-identical behavior — validated gate (a).)
**WAVE 6 SUBMITTED ~13:30 EDT, all pastes landed: Fir `51517706-708` (n=41 (0,2,9,9) flat,
CHECKPOINTED lanes skip 7/8/9, new binary+driver) · Trillium `1970159/160/162` (n=41
reverse, ckpt lanes skip 7/8/9) · Rorqual+Nibi tar-pipe landed (new source+driver staged;
their queued/running jobs unaffected). These are the FIRST checkpointed lanes — wave 7 =
resubmit these exact sbatch lines verbatim, arms auto-resume, zero re-tread. Checker: wave-6
IDs stay un-excluded until verdicts; expect `[driver] checkpoint lane:` + `[firsthit ckpt]`
lines in outputs and `tested_cum=`/`resume_pi_min/max=` in GATEB.**

**⚡ 2026-07-29 (daily loop) — WAVES 5+5b READ: HITLESS on both n=41 ends; the published
classes have now absorbed their deepest coverage yet.** Fir `51356688-90`+`51371189-91`
(n=41 (0,2,9,9) FLAT, skip windows 1-6) ALL COMPLETE, arms_with_hits=0, tested
39-53M/window ≈ **277M fresh tested this round** (aborted=0, 178/178 arms summarized).
Trillium `1946948-50`+`1955719-21` (n=41 REVERSE, windows 1-6) ALL COMPLETE hitless,
tested 36-41M/window ≈ **226M fresh**; note cells_done_sum=11-25 — first jobs ever to
COMPLETE whole cells (the skip windows land on smaller cells, as designed). Rorqual wave-5
`17528790-92` (n=42 (7,11,0,0) flat, skips 1/2/3) COMPLETE hitless, tested 76-95M/job
≈ 265M (arms_summarized 165/178 → tested is an undercount by ~13 arms/job).
**Pre-registered abort check: skip windows BEHAVED (tested ≫ 0, aborted=0) → Rorqual
wave-5b `17637330-35` rides — RUNNING ~2h in at check time, done ~03:00 EDT 07-30.** Nibi
`18545816-24` still PD (Priority). No FOUND banners anywhere; `rung_status check` =
EXHAUSTED (deliberate, no SA refill). Checker exclusions updated for all 15 finished wave
IDs (17637330-35 stay un-excluded). NO submits — post-wave-5b continuation (windows 7+,
resume-build-first, or GPU spike) is Daniel's call per the no-autonomous-probe-resubmit
rule. **Working tree holds the resume build MID-IMPLEMENTATION (wz_match.cpp +159 lines,
driver +23, per the 07-28 spec) — left uncommitted by the evening session; it passes
`g++ -fsyntax-only` but has NOT run its validation battery, so the loop left it untouched
and committed bookkeeping files only.** Kotsireas brief STILL unsent — standing
highest-leverage human action.**

**⚡ 2026-07-28 (evening session) — WAVE 5b SUBMITTED: the NEXT concentration windows, +12
node-days on the two published classes, zero code changes.** Rorqual `17637330-335` (n=42
(7,11,0,0): flat skips 4/5/6 + REVERSE skips 1/2/3 — reverse windows mirror the n=41
both-ends structure; PD, start as wave-5 nodes free ~19:30 EDT tonight) · Fir `51371189-191`
(n=41 (0,2,9,9) flat skips 4/5/6; PD behind the same maintenance fence → SIX Fir jobs start
in parallel when it lifts) · Trillium `1955719-721` (n=41 (0,2,9,9) reverse skips 4/5/6, PD
Resources). Nibi untouched (its 9 PD reverse jobs = its share). All windows disjoint from
waves 1-5 by construction (same NARMS=178/order/class, fresh skip values). **Trillium
wave-5 `1946948-50` RESTARTED post-preemption ~14:30 EDT — squeue `%L` is Time*Left*
(11:31 left = ~30 min in), finish ~2:50 AM.** Pre-registered abort: if tonight's Rorqual
wave-5 outputs show the skip windows misbehaving (GATEB tested=~0 or skip not engaged in
arm configs), scancel the PD continuations — else they ride. Wave-5b IDs stay UN-excluded
in the checker until verdicts are read. Checker driver QoL: `check_all_retry.sh` now honors
`CLUSTERS="fir nibi"` override for partial re-checks (default = all four, loop unchanged).
**WZ paper re-audit (Daniel asked "did we miss something"): Thm 2.4 PSD test is ALREADY
implemented** (joint bound wz_match.cpp:115, applied in the true pair stream) — with 2.11b +
2.12 closed on 07-15→26, the paper is fully mined; the remaining WZ delta is compute-shape,
not a missing trick. **RESUME BUILD GREENLIT** (approach B, flat-first-preserving per-arm
checkpoint; spec at `docs/superpowers/specs/2026-07-28-per-arm-candidate-resume-design.md`);
implementation + local validation next (n=19 bit-identical · n=29 blind re-find ·
two-half resume-equivalence). Kotsireas brief STILL unsent — the standing highest-leverage
human action.**

**⚡ 2026-07-28 (daily loop) — WAVE 5 MID-FLIGHT, no hits, no action. Rorqual `17528790-92`
(n=42 (7,11,0,0), skips 1/2/3) RUNNING ~6.2-6.5h of 12h (started 07:18-07:35 EDT, done
~19:30 tonight). Trillium `1946948-50` (n=41 (0,2,9,9) reverse, skips 1/2/3) STARTED this
morning (tri0307 10:24, tri1148/1150 11:53 EDT per output headers) but squeue shows PD
(Priority) again = preempted + requeued; `--requeue` restarts them from scratch on the same
deterministic window, no action needed. Fir + Nibi MISSED the Duo 180s window this check —
zero visibility, but Fir `51356688-90` was already PD behind its maintenance reservation and
Nibi's 9 n=42 reverse jobs (`18545816-24`) were PD; nothing to do there. No FOUND banners,
no gate sums, SA tail stale (deliberate lapse). Insufficient runtime = bookkeeping only:
no submits, no checker changes (wave-5 IDs stay un-excluded until their verdicts are read).
Tomorrow's loop should have the first real wave-5 data (Rorqual finishes tonight).
Kotsireas brief STILL ready-to-send — the standing highest-leverage human action.**

**⚡ 2026-07-27 (session close) — WAVE 5 SUBMITTED: THE CONCENTRATION WAVE. All fleet on
the two PUBLISHED-SIG classes (the only classes where solutions PROVABLY exist; WZ's n=41
scores 140 = inside our flat lane). Mechanism: WZ_FH_PROF_SKIP repurposed as a WINDOW
SELECTOR (not resume) — skip=k dedicates each arm's full day to its (k+1)-th flattest cell,
ZERO overlap between skip values: ~530 fresh arm-days on the flattest unexplored cells.
IDs: Fir `51356688-690` (n=41 (0,2,9,9) flat, skips 1/2/3 — ⚠️ PD behind an upcoming Fir
MAINTENANCE reservation, will start after) · Trillium `1946948-950` (n=41 reverse, skips
1/2/3) · Rorqual `17528790-792` (n=42 (7,11,0,0) flat, skips 1/2/3) · Nibi: its 9 PD n=42
reverse jobs cover its share. Checker exclusions will need these IDs post-completion.
**WZ paper re-read finding: they acknowledge the Nanjing HPC CENTER; no runtime published —
combined with Đoković's n=36 = 1,423 CPU-days (2010), record rungs historically cost
CPU-YEARS. We are ~7 core-years into n=41 with a solver ~100× stronger than 2 weeks ago:
the 'wall' is a measured mountain, ~25-45% climbed per class.** NEXT SESSION's standing
priorities: (1) read wave-5 results (first concentrated read on proven classes); (2) per-arm
candidate-level resume build (checkpoint file — env can't carry 178 values); (3) GPU
feasibility spike (the ×1000-class ceiling-breaker, now priced against 5-8 CPU-waves/class);
(4) streaming-side SIMD (bottleneck moved off completion); (5) **KOTSIREAS EMAIL — a week
'ready', still unsent, the highest-leverage human action in the campaign.** This chat ends
here by design: HANDOFF is canonical, the loop runs daily, fresh session starts from this
file + the bs45-campaign skill.**

**⚡ 2026-07-27 (Daniel session) — driver telemetry tar-piped to all 4; remaining tested
sums fetched. Trillium n=41 REVERSE: 33.7-61.6M tested/class. Rorqual n=42 flat-first:
17.1-105.6M tested/class (variance is real; `17481823` (5,9,0,8) = 0.89M, the
enumeration-bound class again). CUMULATIVE n=41 tested/class ≈ 120-220M of the ~500M
pessimistic band (~25-45%) across both ends — a measured slog, NOT a wall. Fork status:
concentration wave (all fleet -> the two PUBLISHED classes, NARMS-varied for disjointness)
is the zero-cost move; per-arm resume + GPU spike are the builds; Kotsireas email STILL the
unsent human lever.**

**⚡ 2026-07-27 (daily loop) — WAVE 4 COMPLETE AND HITLESS, and the DEPTH METRIC ITSELF was
wrong: `candidates=` counts STREAMED candidates, not tested ones.** 29/40 wave-4 jobs done
(Fir `51283846-54` 9x n=41 flat-first, Trillium `1940127-35` 9x n=41 reverse, Rorqual
`17481817-27` 11x n=42 flat-first — all 178-arm, aborted=0; Nibi `18545814/15` n=42 reverse
done ~100M streamed each, `18545816-24` still PD = live capacity, not idle). **Finding 1 —
arm-log audit (Fir 51283847 arms 0+100, read-only duo_run): every arm streams C,D candidates
into the 500k WZ_FH_CELL_ORDER buffer, then spends the rest of walltime DRAINING (completing)
it flat-first at ~9/s; ALL arms died mid-first-drain (cells_done=0 everywhere, arm summaries
show streamed 500-568k vs backtracks_entered 151-365k). So GATEB `candidates=` ≈ buffer cap ×
arms + score-rejects — a STREAMING artifact that reads ~100M/class regardless of completer
speed. True searched depth = backtracks_entered.** Fetched the real per-job tested sums on Fir
(n=41): `51283846`=50.3M `847`=87.2M `848`=110.2M `849`=54.8M `850`=75.6M `851`=73.9M
`852`=60.6M `853`=69.0M `854`=70.2M — **the 5x completer DID deliver: wave-4 TESTED 50-110M/class
at n=41 vs wave-3's ~5x less (wave 3's "95-105M/class" was streamed; its tested depth was
never aggregated). Ledger correction: n=41 front-end tested = wave-4's 50-110M/class (same
deterministic order as wave 3, superset); reverse-end tested (Trillium) not yet fetched.
Corollary: cell-level resume (WZ_FH_PROF_SKIP) is DEAD — no arm ever finishes cell 0; the
resume lever must be candidate-level PER ARM (tested_min).** **Finding 2 — the n=42
(5,9,0,8) anomaly is SOLVED and it is NOT a slow node: wave-4 `17481823` reproduced wave-3's
~7.1M on a different node, and its arms 0+100 streamed ZERO candidates in 11.5h — the class
is ENUMERATION-BOUND (stream DFS wall before the first shard leaf for most arms),
deterministic. Reverse-order coverage (Nibi PD jobs) is the near-term mitigation for that
class.** Rate math: ~9/s completion × 178 arms × 41.4ks ≈ 60-70M tested/class/wave ⇒ the
≥500M/class pessimistic band ≈ 5-8 more waves per class-end EVEN WITH exact resume — the
GPU-spike / methods-ask fork is now priced. CODE: `auto/2026-07-26` MERGED to main
(production-validated by wave 4); NEW branch `auto/2026-07-27` (85a41b7, R1-validated on
synthetic logs built from real arm summaries): GATEB line adds `tested=` + `tested_min=`.
`rung_status check` = EXHAUSTED (no SA refill, deliberate). No submits this round.
NEEDS_HUMAN — (1) tar-pipe the driver telemetry fix (driver-only, solver unchanged):
```
cd ~/Projects/BS45_Quantum_Explorer && git checkout auto/2026-07-27 && for c in fir rorqual nibi trillium; do tar -cf - cluster/deploy/cluster_firsthit_probe.sh | ssh dangord@$c.alliancecan.ca 'cd $SCRATCH/bs45 && tar -xvf - && cp -f cluster/deploy/cluster_firsthit_probe.sh ./cluster_firsthit_probe.sh'; done; git checkout main
```
(2) fetch the missing tested sums (Trillium shown; same on Rorqual j=17481817..27, Nibi
j=18545814..15 — Rorqual missed its Duo window this run):
```
ssh dangord@trillium.alliancecan.ca 'cd $SCRATCH/bs45 && for j in 1940127 1940128 1940129 1940130 1940131 1940132 1940133 1940134 1940135; do t=$(grep -h -oE "backtracks_entered=[0-9]+" fh_arms_$j/arm_*.log 2>/dev/null | awk -F= "{s+=\$2} END {print s+0}"); echo "$j tested_sum=$t"; done'
```
(3) wave-5 design call: candidate-level per-arm resume needs a per-arm checkpoint/skip file
(env alone cannot carry 178 values) — build it, or redirect to the GPU spike; and the
kotsireas brief is still READY TO SEND (methods ask = the door to 42+).**

**⚡ 2026-07-26 (Daniel session, evening) — 5x COMPLETER DEPLOYED to all 4 clusters
(auto/2026-07-26 tar-piped, verified independently: n=19 off-path bit-identical, n=29 blind
re-find PASS idx=26694 NPAF==0). Two findings from the resume-point fetch: (1) **cells_done
MIN = 0 on EVERY n=41/n=42 job** (Fir 178/178 arms, Rorqual 171-178) — with flat-first
ordering the densest cells are also the LARGEST, so the slowest arm in each job never
completed even its first cell in 11.5h. **Uniform WZ_FH_PROF_SKIP is therefore unusable here
(skip>=1 would open a coverage gap); skip=0 is the only sound value.** The real fix = PER-ARM
skip (each arm resumes its own cells_done) — NEXT lever, needs a per-arm env or a checkpoint
file. (2) Trillium `1926730` (n=41 published sig, pre-obs binary) = TIMEOUT 12:04:05, exit
0:0 — ran full walltime, blind negative, depth unknown; Trillium now FREE with the new binary.
**WAVE 4 DECISION (given skip=0): re-run flat-first (PROF_ORDER=1) with the NEW completer —
NOT a wasteful re-tread. It is 5x faster, so it re-reaches wave-3's ~100M mark in ~2.3h
(resolving wave-3's small abort residue on the way — the proven hit-hiding failure mode) then
newly exhausts to ~500M/class (the pessimistic x3/rung band) in the remaining ~9h. Primary
value = DEPTH; abort-cleanup = bonus.** Allocation: Fir n=41 flat-first + Trillium n=41 reverse
(far-end coverage insurance) ; Rorqual n=42 flat-first + Nibi n=42 reverse. Early-verdict ping
infra added (daily_auto.sh watcher + auto_prompt Step 1 writes results/interim_summary.txt
first) so long build sessions no longer leave Daniel waiting an hour between "check done" and
the summary. HANDOFF/checker updated. **WAVE 4 SUBMITTED ~evening 07-26, all 4 Duo-approved (40 jobs, 5x completer): Fir `51283846-854` (9x n=41 flat-first) + Trillium `1940127-135` (9x n=41 reverse) bracket n=41 both ends; Rorqual `17481817-827` (11x n=42 flat-first) + Nibi `18545814-824` (11x n=42 reverse) bracket n=42. Expected reach ~500M/class (x3/rung band) in one node-day each. Checker exclusions need +these on next loop run.**

**⚡ 2026-07-26 (daily loop) — WAVE 3 COMPLETE AND FULLY OBSERVED: still hitless, but the
flat-prioritized ordering delivered ~7-10× candidate throughput; the PROFILE-CONSTRAINED
A,B COMPLETION lever (named 07-25 as the deepest un-built lever) is BUILT + VALIDATED on
`auto/2026-07-26` — aborts collapse 424→2 per 1000 at n=29.** Wave-3 data (all 178-arm
jobs summarized, all deadline-interrupted = counts are lower bounds): **n=41 Fir
`51091778-786`: hitless, 95.4-104.5M candidates/class in ONE wave (~898M total; aborts
≤0.03%) vs 10-19M (wave 1) and 13.5-22.4M (wave 2) — ~7× throughput; each n=41 class now
≥100M searched (~120-145M cumulative), and because flat-first front-loads the measured
~35× solution-density enrichment, this negative is STRONGER than raw candidate count
suggests. n=42 Rorqual `17448745-55`: hitless, 105-116M/class (~1.07B total; ~10× prior
throughput) — EXCEPT `17448751` sig (5,9,0,8): only 7.1M cands / AB_nodes 1.2e12 (~20×
below siblings; ran on rc21802 vs siblings' rc13xxx — suspected slow/degraded node, NOT a
property of the class).** The optimistic ×2/rung density band (hit by ~40M/class) is now
dead; data says ≥×3/rung (≥500M/class) — but at wave-3 throughput that is ~4-5 more
node-days/class, no longer 25-30. NEW CODE (branch `auto/2026-07-26`, R1-validated,
NOT deployed — tar-pipe is Daniel's): (1) **WZ_FH_AB_PROF (default ON at m6)** — per-cell
allowed (k,r) A,B mod-6 profile lists (2.11a-exact + 2.11b-cancel + 2.12, both signed
targets ±a/±b for negation/reversal-canon retention), count_pairs22-style capacity pruning
down the pair-DFS + exact leaf membership; proven-dead cells are skipped entirely.
Validation: n=19 lever-off BIT-IDENTICAL to HEAD (idx=807/rank=2/nodes=8087); lever-on
same hits n=10/11/19 both moduli (odd+even L; n=19 m6 total nodes 488k→77k = 6.3×); n=29
BLIND RE-FIND canary PASS (FOUND idx=26694, NPAF==0, 148s 1-thread); n=29 1000-cand
sample: budget-aborts 424→2, clean 576→998, nodes 127.8M→47.8M (2.7×), wall 2×; **n=41
real-class A/B (sig (0,6,3,11), 200 cands, budget 5e7): nodes 1,142.5M→218.9M = 5.2×,
completion wall ~4.8×, 200/200 clean both ways; n=42 (sig (9,9,2,2), odd-L, 100 cands):
319.3M→48.3M = 6.6×**; setup sane at both (map build 1.0-1.5s, ~8-11MB/arm, max_list
≤15.3k under the 20k cap). **DECISIVE: the lever converts budget-aborts into hits — the
n=29 unconstrained completer, streaming the exact same candidates up to the known hit at
budget 2e5, aborted 2,326/5,006 INCLUDING the solution candidate and found NOTHING; the
constrained run resolved that candidate in 81,320 nodes and printed the solution. Deep-n
waves' abort fractions may have been sitting on hits.** (2) driver
GATEB line now emits **cells_done_min** (= the sound WZ_FH_PROF_SKIP for wave 4) and
cells_done_sum. Wave-3 arm logs ALREADY carry cells_done= (07-24 binary) — fetch the
per-job MIN, then wave 4 = new source + WZ_FH_PROF_SKIP=<min> per class: exact disjoint
resume, no re-tread. No submits this round (wave 4 without skip re-treads wave 3's
deterministic order; skip values + source both need Daniel). Trillium `1926730` STILL no
summary in output — check `sacct -j 1926730` when convenient. Checker exclusions
+51091778-86, +17448745-55. Kotsireas brief still READY TO SEND.
NEEDS_HUMAN paste blocks — (1) ship the lever to all clusters:
```
cd ~/Projects/BS45_Quantum_Explorer && git checkout auto/2026-07-26 && for c in fir rorqual nibi trillium; do tar -cf - src/solver/wz_match.cpp cluster/deploy/cluster_firsthit_probe.sh | ssh dangord@$c.alliancecan.ca 'cd $SCRATCH/bs45 && tar -xvf - && cp -f cluster/deploy/cluster_firsthit_probe.sh ./cluster_firsthit_probe.sh'; done; git checkout main
```
(2) fetch wave-3 per-job MIN cells_done (Fir shown; same on Rorqual with 17448745..55):
```
ssh dangord@fir.alliancecan.ca 'cd $SCRATCH/bs45 && for j in 51091778 51091779 51091780 51091781 51091782 51091783 51091784 51091785 51091786; do n=$(ls fh_arms_$j/arm_*.log 2>/dev/null | wc -l); m=$(grep -h -oE "cells_done=[0-9]+" fh_arms_$j/arm_*.log 2>/dev/null | cut -d= -f2 | sort -n | head -1); s=$(grep -l "cells_done=" fh_arms_$j/arm_*.log 2>/dev/null | wc -l); echo "$j arms=$n with_cells=$s min_cells_done=$m"; done'
```
⚠️ a job's min is a SOUND skip only if with_cells == arms == 178 (arms are interleaved
shards; an arm with no summary has unknown progress, and a global skip past it would
open a coverage gap that could skip the hit). Fir wave-3 = 178/178 everywhere; Rorqual
`17448747/48/49/50/52/53/54` summarized 171-177 → skip=0 there, or Daniel's call.
(3) wave-4 submit template (per class, after (1)+(2); Fir example, sig+skip per job):
```
sbatch --requeue --export=ALL,WZ_N=41,WZ_A=0,WZ_B=6,WZ_C=3,WZ_D=11,WZ_FH_PROF_ORDER=1,WZ_FH_AB_BUDGET=50000000,WZ_FH_PROF_SKIP=<min_cells_done> ./cluster_firsthit_probe.sh
```
**

**⚡ 2026-07-25 (late) — EFFICIENCY PUSH, two answers to "is there nothing else": (1) BUILT
TONIGHT: flat-first within-cell ordering (commit HEAD, `WZ_FH_CELL_ORDER`, default ON at m6) —
every arm now completes each cell's candidates flattest-first (the ~35× enrichment as an
ordering, zero coverage loss; WZ's own 41-43 solutions score 140/142/134). Validated: coverage
invariant 809==809, n=19 m3 bit-identical, n=41 streams+completes. (2) NAMED AND SPEC'D, the
deepest un-built lever: PROFILE-CONSTRAINED A,B COMPLETION — our 2.11b/2.12 filters PROVE a
compatible (k,r) A,B-profile exists per C,D cell, then the completer ignores that and searches
the whole A,B space; constraining the pair-DFS to compatible class-sums (count_pairs22-style
per-class capacity pruning) is the last structural gap vs WZ Step 5, expected ≥10× on
nodes-per-exhaust. NEXT SESSION's build, full retention validation. Wave-3 doctrine: waves 1+2
COMPLETED every streamed candidate, so re-ordering the searched window is pointless — wave 3
must reach NEW territory: PROF_ORDER=1 (flattest cells first) + cell_order = fully
flat-prioritized search; new binary emits cells_done → wave 4 gets exact PROF_SKIP resume.
GPU spike remains the strategic reserve if the completer levers don't bend the slope.**
**WAVE 3 SUBMITTED ~19:30, Duo-approved: Fir `51091778-786` (9× n=41) · Rorqual
`17448745-755` (11× n=42) — flat-prioritized end to end (PROF_ORDER=1 flattest cells first +
in-cell flat-first ordering), cells_done telemetry live ⇒ wave 4 gets exact PROF_SKIP resume.**

**⚡ 2026-07-25 (evening, Daniel's manual full re-check) — THE TWO-WAVE 40s MAP IS COMPLETE:
still hitless, and the depth lower-bound is now MEASURED tight.** The 13:50 "NEW HIT" ntfy
alert was NOISE (truncated Fir session; full re-check shows none-yet everywhere; the 17:38
daily rc=1 was an API ECONNRESET, no interpretation ran). The real data: **n=41 = 9 classes
× ~30-40M candidates cleanly resolved EACH, from BOTH ends** (wave 1 front-DFS 10-19M +
wave 2 reverse `50610008-16` 13.5-22.4M, disjoint by construction) — ~300M total, hitless.
**n=42 = all 11 classes telemetried hitless at 6.7-13.4M each** (`17194603-09` +
`16939414/16/17` + Trillium `1926731` published-sig 12.1M on the upgraded binary — the
tar-pipe race was WON). Trillium `1926730` (n=41 published sig) still no summary — likely
running. **Reading vs the pre-registered density extrapolation (20M-500M/class): the
optimistic ×2/rung band is now EXCLUDED at n=41 (searched past it, both ends); data
consistent with ×3/rung (~500M/class) or worse ⇒ brute persistence ≈ 25-30 node-days/class.
Decision fork sharpened: (a) precise-resume grind waves (cells_done data EXISTS in Fir
wave-2 arm logs — driver aggregation doesn't sum it yet; fetch or extend), (b) tier-lane
decomposition from the same arm logs (are ≤150-gated arms outperforming?), (c) the GPU
spike — the only lever sized to close a ×30 gap outright. Recommendation on file: fetch
arm-log stats next session, decide GPU spike with those numbers.** Checker exclusions
updated (+50610008-16, 17194603-09, 16939414/16/17, 1926731). No banks, no submits this
check; all clusters idle except Trillium 1926730.


**⚡ 2026-07-24 — FIRST FULLY-OBSERVED n=41 DATA (Daniel's manual fir+nibi re-check): the
obs fix WORKS (178/178 arms summarized on all 9 Fir jobs) and the depth picture is finally
MEASURED.** Fir `50267781-91` (9× n=41, canon+obs binary): hitless, but **10.6-19.4M
candidates CLEANLY RESOLVED per class-job** (abort rate collapsed to 0.006-0.27% at budget
5e7 — the canon levers made per-candidate search essentially exact), ~123M n=41 candidates
exhausted total, AB_nodes ~4.8e13/job (saturated node). **Depth math: density trend
(1/21k @29 thinning 2-3×/rung) ⇒ expected first hit at n=41 ≈ 20M-500M cand/class; one
node-day reaches 10-19M ⇒ wave 1 hit 0.03×-0.5× of expected depth. NOT a wall — a progress
bar.** Nibi `18168030/31/33` = blind (started 02:27, pre-tar-pipe) — bounded negatives, depth
unknown. Rorqual 10 jobs still PD; Trillium still reservation-held. **Provenance resolved:
`17194603-09` = Daniel's 07-23 resubmit of the 7 hitless n=42 classes, submitted AFTER the
obs tar-pipe → will carry real telemetry.** BUILT+VALIDATED today: `auto/2026-07-23` MERGED
to main + **`WZ_FH_PROF_SKIP` continuation lever + `cells_done` telemetry** (commit 39a24dc;
skip invariant exact at n=11: cells 916−k, complement streams correctly; n=19 bit-identical)
— wave 3+ resumes at measured disjoint depth. Wave 2 strategy (no cells data from wave 1):
**reverse profile order (`WZ_FH_PROF_ORDER=2`)** = disjoint-by-construction coverage from
the opposite end, no skip estimate needed. BOTH PASTED ~14:20: tar-pipe landed all 4 clusters (Rorqual's 10 PD upgrade at compile-time); Fir wave-2 = `50610008-016` (9× n=41, PROF_ORDER=2 reverse — disjoint from wave 1 by construction).

**⚡ 2026-07-24 — 7 UNATTRIBUTED FIRSTHIT JOBS ON RORQUAL (`17194603-09` PD) — PRESUMED
DANIEL'S RESUBMIT OF THE 7 HITLESS n=42 CLASSES; NEEDS CONFIRMATION + tar-pipe status
(loop run 16).** Checker 13:06: Fir and Nibi MISSED (no Duo approval in 180s — no data,
not idle evidence). Rorqual reached: `16939415` (n=42 sig (7,9,2,6)) completed
`arms_with_hits=0, GATEB: candidates=0` on the PRE-OBS binary → one more hitless-BLIND
bounded negative, depth unknown (same 07-23 artifact class, NOT empty, NOT searched out);
`16939414/16/17` still PD; **NEW: `17194603-09`, 7× FIRSTHIT PD, absent from HANDOFF/ledger
— exactly the count of the 7 hitless-blind n=42 classes from 07-23 NEEDS_HUMAN #2, so
presumed Daniel resubmitted them.** Unknown whether the `auto/2026-07-23` obs-fix tar-pipe
landed first (branch still unmerged; jobs compile at start, so if it landed their outputs
carry real `candidates_streamed=` summaries; if not they run blind too). Trillium reached:
`1926730/31` still PD behind maintenance reservation → NEEDS_HUMAN #1 tar-pipe window STILL
OPEN. `rung_status check` = EXHAUSTED n=38/budget-0 → no SA refill (deliberate). No FOUND
anywhere. Actions: checker exclusions +16939415, pending list updated (17194603-09 flagged
unconfirmed-provenance); no submits, no code changes. NEEDS_HUMAN: (1) confirm 17194603-09
— did you submit them, and did the obs-fix tar-pipe land first? If not, the 07-23 4-cluster
tar-pipe block still needs pasting (Trillium time-boxed, before its PD pair starts);
(2) Fir/Nibi unreached — next checker run needs Duo taps; (3) still standing: n=38/39 wave
greenlight, Task 3 call, kotsireas send (brief READY, carries n≤37 — the methods ask is the
door to 42+).**


---

## 🚀 QUICK REFERENCE — the current system (rewritten 2026-07-30)

**CURRENT PROGRAM (2026-09-22): PASS F / FR / F2 front-only tiling, budget 2e6, canon ON —
`docs/lever19_sweep_plan.md` is the operating table; `docs/external_review_brief.md` is
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
`docs/superpowers/specs/2026-07-28-per-arm-candidate-resume-design.md`). Deployed via
`cluster/deploy/cluster_firsthit_probe.sh` (178 single-core arms/node, 12h). SA ladder
RETIRED (ceiling ~n=33-35, archive); exhaustive/join RETIRED (archive).

**Ladder record (all NPAF-verified + banked in `results/champions/`):** 29→30→31 (SA) →
32→33→34→35→36→37 (firsthit, 2026-07-17..21) → **41 = BS(42,41) banked 2026-07-30**
(published WZ class (0,2,9,9), NEW inequivalent solution, score 124 vs their 140; first
hit ever at n≥38). n=42/43 = WZ's remaining rungs, under both-ends attack. **n=44 =
BS(45,44) = the OPEN WORLD RECORD** — 12-class frontier enumerated + all stream (2026-07-30);
program: `docs/n44_search_narrowing_research.md`.

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
n=32→37, retired deploys, superseded entries): **`HANDOFF_ARCHIVE.md`**.
