# CP493 — BS(45) Solver Project Handoff

**Date**: 2026-08-27 (read TOP OF MIND newest-first; QUICK REFERENCE below has the current
system. Pre-2026-07-24 history — SA era, join saga, firsthit ramp n=32→37 — lives in
`HANDOFF_ARCHIVE.md`; measured-dead list in `.claude/skills/bs45-campaign/SKILL.md`.)

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
