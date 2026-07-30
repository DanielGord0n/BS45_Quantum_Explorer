# CP493 — BS(45) Solver Project Handoff

**Date**: 2026-07-30 (read TOP OF MIND newest-first; QUICK REFERENCE below has the current
system. Pre-2026-07-24 history — SA era, join saga, firsthit ramp n=32→37 — lives in
`HANDOFF_ARCHIVE.md`; measured-dead list in `.claude/skills/bs45-campaign/SKILL.md`.)

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
ssh dangord@<cluster>.alliancecan.ca 'cd $SCRATCH/bs45 && sbatch --requeue --export=ALL,WZ_N=<n>,WZ_A=<a>,WZ_B=<b>,WZ_C=<c>,WZ_D=<d>,WZ_FH_PROF_ORDER=<1 flat|2 reverse>,WZ_FH_AB_BUDGET=50000000,FH_NARMS=178,WZ_FH_PROF_SKIP=<k> ./cluster_firsthit_probe.sh'
```
Nibi adds `--account=def-ikotsire_cpu`. Ship source via tar-pipe (scp does NOT expand
$SCRATCH): `tar -cf - src/solver/wz_match.cpp cluster/deploy/cluster_firsthit_probe.sh |
ssh dangord@<c>.alliancecan.ca 'cd $SCRATCH/bs45 && tar -xvf - && cp -f
cluster/deploy/cluster_firsthit_probe.sh ./cluster_firsthit_probe.sh'`.

**Reading GATEB:** `tested=` (backtracks_entered sum) is true depth; `candidates=` is
STREAMED (inflated by the drain buffer); `tested_cum=` is cross-wave cumulative from
checkpoints; `resume_pi_min/max=` is the lane frontier. TIMEOUT@12h = normal completion.
squeue `%L` is time LEFT.

**Window/lane ledger:** n=41 (0,2,9,9): SOLVED (flat windows 0-8 burned, reverse 0-6).
n=42 (7,11,0,0): flat 0-6 burned + lanes 7/8/9 live (Fir); reverse 0-3 burned/pending +
lanes 4/5/6 (Rorqual) + 7/8/9 (Trillium) live. n=43 (8,-2,5,9): flat lanes 0/1/2 (Fir) +
reverse 0/1/2 (Rorqual) live. n=44: flat skip-0 lanes on (5,9,6,6),(5,7,2,10),(1,13,2,2)
(Fir). Nibi: 9× n=42 reverse skip-0 (all classes, old driver, no ckpt).

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
