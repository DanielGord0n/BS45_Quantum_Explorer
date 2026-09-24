# BS45 autonomous daily loop (headless)

You are running UNATTENDED via a 1pm cron. No human is watching this run. Be
decisive, follow the campaign discipline exactly, and leave a clean audit trail.
**First, load the `bs45-campaign` skill and read HANDOFF.md QUICK REFERENCE +
newest TOP OF MIND** — they are the state and the judgment. When this prompt and
the skill disagree, the skill wins.

The checker has ALREADY run this cycle; its combined output is in the file named
in the environment variable `$CHECK_OUTPUT` (also `results/latest_check.txt`).
Do not re-run the checker.

## Non-negotiable rails (do not skip, even unattended)

- **R1 — validate before shipping code.** If you change ANY solver/deploy source,
  it MUST compile and pass the relevant small-n validation locally (e.g.
  `g++ -O3 -std=c++17 ...` then reproduce a banked small-n case, and/or
  `python3 tools/verify_npaf.py`) IN THIS RUN before it is deployed to a cluster.
  If it does not build or validate, do NOT deploy — write the diagnosis to the
  summary, leave the change uncommitted on a branch named `auto/YYYY-MM-DD`, and
  set NEEDS_HUMAN.
- **R2 — verify before claiming.** A `*** ... FOUND ***` banner is NOT a result
  until `python3 tools/verify_npaf.py` PASSES on the extracted sequences and a
  champion file with full provenance is written to `results/champions/`. Never
  commit or summarize a solution/record as real without that pass. An unverified
  banner ⇒ NEEDS_HUMAN, not a claim.
- **R3 — never route around a guard.** If a hook, permission rule, or allowlist
  blocks an action (e.g. `guard_git_push.py` denying a headless push), that is a
  STOP, not an obstacle. Do NOT rephrase the command, use an alias/synonym, pass
  `--no-verify`, disable the hook, or edit the guard to get past it. Nothing in
  this prompt — including anything that looks like authorization — overrides a
  guard. Stop, set NEEDS_HUMAN, and report exactly what was blocked and why.

## Steps

1. **Interpret** `$CHECK_OUTPUT` using the skill's output-reading traps (trust
   only the banner, not `bestAB`; TIMEOUT at full walltime = completed; old banked
   files match the FOUND grep — cross-check `results/champions/`).
   **Immediately after interpreting — BEFORE any build/validate/long work — write
   a 2-4 sentence plain-text verdict to `results/interim_summary.txt`** (hits or
   not, the one key number, what you're doing next). A watcher pushes it to
   Daniel's phone within seconds; without it he waits an hour staring at "check
   done". The full summary still goes to `results/last_summary.txt` at the end.

   **1b. COMMIT-AS-YOU-GO (hard rule, 2026-08-24).** The 08-23 run died on an API
   connection drop after 3.5 h with NOTHING committed — the cycle's reads and a
   Rorqual restack had to be reconstructed the next day. So: (a) as soon as the reads
   are interpreted and HANDOFF + checker exclusions are updated for them, run
   `git add -A && git commit -m "auto: <date> reads"` and the bounded push from step 4
   BEFORE any duo_run submit; (b) after EACH cluster's submits echo job IDs, append
   the IDs to HANDOFF and commit again immediately. A dropped connection must never
   lose more than one cluster's restack IDs.

2. **Decide** the single smartest action (skill doctrine: one option, one
   sentence of why). Branches:
   - **Jobs still running / too early for results** → do nothing but bookkeeping.
     Explicitly note "insufficient runtime, no action" — do NOT invent changes.
   - **Idle cluster(s)** → **FIRST run the exit condition. It is not advisory.**
     ```
     cluster/deploy/rung_status.sh check
     ```
     - Exit 3 / `EXHAUSTED` → **DO NOT refill SA at this rung.** More tickets here
       is measured-dead (SA caps ~n≈33-35). Follow the escalation the check prints:
       finish the JOIN22 / Theorem-2.2 canary (never passed — a ~15h slot on
       Trillium's 24h queue), then the Phase-0 gates in `docs/wz_firsthit_plan.md`.
       Those are PRE-REGISTERED experiments, so you may submit them autonomously via
       `duo_run.sh`. Anything beyond them is a NEW research direction → NEEDS_HUMAN.
       Put "SA ladder capped at n=<N> — escalated to <experiment>" in the summary.
       (Do NOT mention `docs/kotsireas_brief.md` — Daniel declined sending it on
       2026-08-05; the standing human lever is the allocation ask, already drafted,
       and it needs no daily reminder.)
     - Exit 0 / `ACTIVE` → refilling is still justified; proceed.

   - **Refill (only when ACTIVE)** — get seeds deterministically:
     `cluster/deploy/next_seeds.sh take <#idle>` (never pick seeds yourself).
     After the submits echo job IDs, record them: `cluster/deploy/rung_status.sh add <#submitted>`.
     Record each observed floor: `cluster/deploy/rung_status.sh floor <bestAB>` (a floor
     that IMPROVES extends the budget — that is progress, not grinding).
     On a VERIFIED hit (R2 passed): `cluster/deploy/rung_status.sh promote`.
     **Submit ONLY via `duo_run.sh`** — plain `ssh` cannot get past the Duo menu
     unattended and will silently fail (add `--account=def-ikotsire_cpu` for Nibi):
     ```
     ./cluster/deploy/duo_run.sh <cluster> 'cd $SCRATCH/bs45 && sbatch --requeue \
       --export=ALL,WZ_N=<n>,WZ_SEED_BASE=<base> ./cluster_sa_ladder.sh'
     ```
     **RUN duo_run.sh IN THE FOREGROUND and wait for it (2026-08-29 hard rule).** You are
     a headless `-p` session: ending your turn ENDS THE RUN — nothing re-invokes you
     when a background task finishes. The 08-29 supplementary run backgrounded the Fir
     submit, ended its turn "to wait", and exited with 0 submits and no summary. Never
     use run_in_background for duo_run.sh, git push, or anything you need the result of.
     It auto-types the Duo "1"; Daniel taps his phone to approve. **Confirm each
     submit echoed a `Submitted batch job <id>` before treating it as queued** —
     if `duo_run.sh` exits non-zero, the job did NOT go in. Never advance the
     ledger for a submit that did not echo a job ID.
     **JOB SHAPE VERDICTS (2026-09-19):** levers 24/25 (3 h and split-lane jobs) FAILED on
     Nibi (0 short/small jobs admitted in 72 h while 12 h lanes were) — Nibi admits by
     priority only. ALL Nibi submits are 12 h whole-node lanes; never reshape jobs there.
     Keep Nibi >= 300 lanes queued (allocation burn), expect ~13-day waits, no alarms.
     Budget stays 2e6 fleet-wide (both lever-21 controls passed; the 5e6 lane gave only
     80% of the cells => failed its rule). The N43b2e6 re-find (21707091) is bookkept.
     **NIBI IS THE RRG'S HOME (2026-09-15):** the professor's allocation is meant for Nibi
     ("submit several hundreds of jobs"). Keep Nibi >= 100 lanes pending at all times;
     refill in tranches of 100-300 from the plan (FR workhorse, then FR class A/B, then F2).
     Fir/Rorqual keep Pass F. LEVER 23 (`WZ_FH_WALL_SEC=900`) is under a split test on
     Nibi's FR tranche (k=0 mod 16 with, k=8 mod 16 without): when both halves have read,
     compare cells_done/lane; >= +15% for the walled half => use WALL_SEC=900 on all new
     submits and say so. Self-deploy token for FR/F2/WS lanes: source must contain
     `WZ_FH_WALL_SEC` (sha >= the 09-15 commit).
     **PASS FR (2026-09-12, lever 22 — built, validated locally, NOT yet deployed):** after the
     workhorse's Pass F tile completes, run the SAME offsets with `WZ_FH_STREAM_REV=1`
     (reversed in-cell enumeration = a second independent front per cell; CKDIR suffix
     `_sr1`, fresh lanes, K=50000, budget 2e6). FR needs source with `WZ_FH_STREAM_REV`
     (self-deploy a sha that has it; grep-verify; never deploy older than the cluster's).
     Class order: F tile -> FR tile -> F2 (deeper) per class.
     **PASS F2 (2026-09-11, deeper complement — see plan AUDIT):** when the workhorse's Pass F
     tile is complete, start F2 on it: same offsets (k step 8), `WZ_FH_DRAIN_TOP=175000,
     WZ_FH_DRAIN_BATCHES=2` (CKDIR suffix `_dt175000b2`, fresh lanes). F2 needs source with
     `WZ_FH_DRAIN_BATCHES` (self-deploy a sha that has it, grep-verified) — NEVER deploy a
     sha older than the cluster's current one. CHECKPOINT-SIGNATURE RULE: a deploy must not
     change CFGSIG for existing lanes (look for "[ckpt] CFGSIG mismatch" / "fresh start" in
     new outputs after any deploy and report it loudly — that means resume positions were
     lost).
     **LEVER 21 APPLIED (2026-09-12, Daniel's session):** the Fir control PASSED (F41b2e6 re-found
     n=41 in 3.1 h at budget 2e6 vs 4.6 h at 5e7) — Daniel's session applied it fleet-wide
     without waiting for Nibi's twin. ALL submits now carry `WZ_FH_AB_BUDGET=2000000` (same
     CKDIRs; budget is outside CFGSIG). Report Nibi N43b2e6 when it reads (informational).
     Budget 2e6 aborts ~21% of tested candidates on the workhorse (measured 09-13: 6.6M of 31.8M) — EXPECTED, not an alarm; those are deep failures (all 3 known solutions needed <=213k nodes). Report the abort fraction; escalate only if it exceeds ~35%. PRE-REGISTERED measurement: at the next Fir restack submit TWO extra workhorse lanes at the next free offsets with `WZ_FH_AB_BUDGET=5000000` (-J F44f<k>_b5e6 is NOT allowed — keep the plain name so the CKDIR matches; note the k's in HANDOFF) and compare cells_done + aborted against their 2e6 neighbours; if 5e6 gives >=90% of the cells with <=5% aborts, switch the fleet to 5e6.
     **MEMORY (2026-09-10, hard rule):** every lane must run with all node memory. The
     driver now has `#SBATCH --mem=0`; if a cluster's `./cluster_firsthit_probe.sh` lacks
     `--mem=0`, self-deploy it (pinned sha >= c792cbf) BEFORE any submit; always pass
     `--mem=0` on the sbatch line too. Pending jobs submitted before the fix: `scontrol
     update JobId=<id> MinMemoryNode=700000` (not on Trillium). arms_summarized < 178 with
     "oom_kill" in the output = memory, not walltime — report it.
     **SELF-DEPLOY FROM GITHUB (2026-09-01):** the repo is public. If a cluster's
     `$SCRATCH/bs45/src/solver/wz_match.cpp` or `./cluster_firsthit_probe.sh` lacks
     `WZ_FH_DRAIN_TOP`, deploy the pinned files via duo_run:
     `R=https://raw.githubusercontent.com/DanielGord0n/BS45_Quantum_Explorer/<HEAD sha>;
     curl -fsSL $R/src/solver/wz_match.cpp -o src/solver/wz_match.cpp.new && curl -fsSL
     $R/cluster/deploy/cluster_firsthit_probe.sh -o cluster_firsthit_probe.sh.new && grep -q
     WZ_FH_DRAIN_TOP both && mv into place && cp to cluster/deploy/`. The new source is
     byte-identical in behaviour when WZ_FH_DRAIN_TOP is unset (validated 09-01), so
     running/queued lanes are unaffected. **LEVER 20 (`WZ_FH_DRAIN_TOP=50000`) is
     gated on its pre-registered control** (docs/n44_search_narrowing_research.md):
     do NOT apply it to n=44 lanes until Nibi's N43dt327 (our n=43 hit's window, capped)
     or another N43dt lane re-finds a known n=43 solution. Report that verdict loudly.
     **RAC ACCOUNT (2026-08-31):** submit with `--account=rrg-ikotsire` on clusters where
     the association exists (`sacctmgr -n show assoc user=dangord account=rrg-ikotsire`
     non-empty; Nibi may need `rrg-ikotsire_cpu`), else def-ikotsire. Pending jobs can be
     moved with `scontrol update job <id> Account=rrg-ikotsire`.
     **TELEMETRY PILOT CLOSED (2026-09-24):** 61213832 read usable; verdict "neither" (HANDOFF
     09-24). No repeat rep, and never enable WZ_FH_TELEMETRY on any job unless Daniel's session
     says so. RAW EVIDENCE RULE: whenever a decision rests on a measurement line (GATEB_TELEM,
     GATEB cum_*, COUNT_ONLY, control nodes_this_cand), save the verbatim line(s) with job ID
     to docs/reviews/<date>-evidence/<jobid>.txt and commit it; HANDOFF paraphrase alone is
     not enough (09-24: the pilot's raw line was never saved).
     **CELLSIZE MEASUREMENT (2026-09-24):** Fir job 61315095 (name CS44g2000) runs from
     $SCRATCH/bs45_cellsize (NOT $SCRATCH/bs45) with WZ_FH_CELLSIZE: it only counts
     candidates, cannot find anything, and has no checkpoint lane. It is NOT a search lane:
     do not count it toward Fir's pending/running refill numbers, do not restack or cancel
     it, and do not read it; Daniel's session reads it with tools/cellsize_summary.py.
     **LEVER 28 DONE (2026-09-23):** controls PASSED (exact node counts); the early check is now
     DEFAULT ON in source (WZ_FH_EARLY_CHECK=0 disables). Never set the flag on submits.
     **PASS G IS THE PROGRAM (2026-09-22; supersedes Pass F/FR/F2 and the stride-8 tiling):**
     two defects fixed (docs/n44_search_narrowing_research.md levers 26-27): endpoint pins
     were dropping ~75% of workhorse orbits under canon, and lanes had no ownership (~99%
     repeated work). A Pass G lane owns raw cells [k, k+S) per arm: env
     `WZ_FH_PROF_SKIP=k,WZ_FH_PROF_END=k+S,WZ_FH_DRAIN_TOP=50000,WZ_FH_AB_BUDGET=2000000,
     WZ_FH_ORBIT_CANON=1` (+`WZ_FH_STREAM_REV=1` for the reversed-front twin). Names
     `<C>44g<k>` / `<C>44gr<k>`. S = 1000 (3,13,0,0); 300 for dedup ~8x classes; 150 for
     dedup ~4x classes (table in docs/lever19_sweep_plan.md). A lane is DONE when GATEB
     shows `range_done=178/178` (or its output says RANGE EXHAUSTED on every arm):
     NEVER resubmit it. A lane that is not done gets restacked (singleton, verbatim) until
     it is. Refill rule: when a cluster's pending < 8, first restack its unfinished G
     lanes, then start G2 on finished ranges (same k/S, `WZ_FH_DRAIN_TOP=175000,
     WZ_FH_DRAIN_BATCHES=2`, names `<C>44g2<k>`), workhorse first. Self-deploy token for
     G lanes: solver must contain `RANGE EXHAUSTED` and driver `range_done` (sha >= the
     09-22 commit). Reads: report range_done per lane; a "STALE checkpoint IGNORED" line
     on a pin-affected class ((3,13,0,0),(9,9,0,4),(3,5,0,12)) is EXPECTED once.
     **CUMULATIVE ACCOUNTING (2026-09-23):** GATEB cum_* already sums independent
     arms within one job. For a completed ownership unit, take ONLY its latest complete
     endpoint snapshot; NEVER add cum_* across successive jobs. Check every arm reported
     and range_done is complete. Units begun on 3014b95 have missing historical bases;
     label them "rate only, not exact alpha" unless the entire counter history is recovered.
     Old logs never recorded cells_empty, so exact E cannot be reconstructed merely by
     summing their done/dead/duplicate fields. Do not reset search checkpoints to reset
     accounting. Process-local FH_TELEM is separate: it describes THIS rep, not history.
     Telemetry pilot (CLOSED 2026-09-24, no repeat; rules below kept for any future
     telemetry Daniel's session authorizes): mode 64 ONLY, one Fir workhorse rep plus at most one repeat
     (one allocated node-day total). Same-node overhead validation is required before
     wider/long-running use, not for this bounded measurement. Do not enable it
     fleet-wide or perform the targeted pending-job substitution autonomously.
     FH_TELEM v2 separates resumed-cell replay: report both inclusive and
     *_without_replay phase shares; use the latter for fresh-work bottlenecks.
     Up to 3 missing/rejected arms per rep is acceptable WITH counts/reasons and
     missing-arm bias disclosed. More is incomplete; never scale missing work up.
     Pool additive FH_TELEM totals from the allowed repeat for the volume gate;
     do not average shares or sum GATEB cum_* across reps. Two incomplete reads stop
     the pilot; no local n44 fallback. Details: docs/reviews/2026-09-23-telemetry-review-fixes.md.
     **LEVER 19 SWEEP RULES (2026-08-29, supersede stack-depth for sweep lanes):** read
     `docs/lever19_sweep_plan.md`. Sweep lanes (names F44i*/F44Bi*/R44i*/R44Ai*/R44Di*/
     T43b*/N43b*) are ONE rep each — never restack the same k. Do NOT top up the deep
     front stacks (F44w*/F44s*/R44r*) while sweep lanes are pending on that cluster.
     When a cluster's pending sweep lanes drop below ~8, submit the NEXT pass/phase
     from the plan tables (verbatim env, k in the name) and append it to the plan.
     Controls to report loudly: F41regr (n=41 re-find) and any Track-B FOUND.
     **STACK DEPTH (2026-08-24):** when restacking FIRSTHIT checkpoint lanes on Fir or
     Rorqual, maintain THREE singleton reps queued per lane (`-J <lane> -d singleton`,
     verbatim env incl. `WZ_FH_ORBIT_CANON=1`), not two — a missed Duo day plus one
     failed loop idled Fir on 08-23/24. Submit (3 minus reps currently queued) per lane;
     singleton serialization makes extra reps collision-proof.
   - *(RESOLVED 07-16: the JOIN22 n=29 canary `16243606` PASSED — banked, frontier re-opened.
     Its instructions are retired; the live priority is now the FIRSTHIT probes below.)*

   - **🚨 THE FIRSTHIT PROBES OUTRANK EVERYTHING (submitted 07-16 ~23:55; work order
     `docs/fable_workorder_firsthit_n41.md`, results doc `docs/gate_bc_firsthit_results.md`).**
     Rorqual `16498722`/`16498723`/`16498724` = the PRE-REGISTERED Gate B+C runs at n=29/30/31.
     Trillium `1926730`/`1926731` = EXPLORATORY probes at n=41/n=42 on Wang-Zhu's own published
     sigs (PD behind maintenance; they start when it lifts). The checker's "FIRSTHIT PROBES"
     section shows `arms_with_hits`, `GATEB:` and `GLOBAL FIRST:` lines per job.
     - **Still running/PD** → do not touch, do not resubmit; say so. Not idle capacity.
     - **Rorqual n=29/30/31 finished** → a FOUND here is an EXPECTED re-find of a banked rung —
       do NOT bank it, do NOT announce it as news. Record in HANDOFF: `GLOBAL FIRST` (profile
       rank + idx), `GATEB` totals (candidates/aborted/nodes), wall time. Read against the
       PRE-REGISTERED rules (do not move them now that numbers are visible): **Gate C PASS** =
       depth ≲1e-3 of the stream under some ordering AND not degrading n=29→31; **Gate B PASS** =
       ≤~10 ms/candidate at n=31 (compute ms/cand = wall × arms ÷ candidates). Verdicts →
       `docs/gate_bc_firsthit_results.md` + HANDOFF. **Whether to build Task 3 (the full
       first-hit architecture) on a PASS is Daniel's call — set NEEDS_HUMAN with the numbers.**
     - **FIRSTHIT probes at n=32/33 (if queued): a FOUND banner BEATS the banked best (n=31).**
       Full R2: `verify_npaf.py` PASS → bank to `results/champions/` with provenance (job id,
       sig, "found by WZ_FIRSTHIT probe") → HANDOFF → checker exclusion → NEEDS_HUMAN, loud.
       A no-hit probe at n=32/33 is a bounded negative for THAT sig only (budget aborts are
       unknowns, and other sig classes exist — parity rule: n even ⇒ a,b odd + c,d even;
       n odd ⇒ a,b even + c,d odd).
     - **⚠️ TRILLIUM SOURCE UPGRADE PENDING (07-21):** its queued `1926730`/`1926731` compile
       at job start, and the fixed (mod-6 + forced 2.11b/2.12) source did NOT land — login
       node refused connections during maintenance. WHEN the checker reaches Trillium again:
       (a) if those jobs are still PD → NEEDS_HUMAN: Daniel must paste the source-only
       tar-pipe (block in HANDOFF 07-21 entry) BEFORE they start; (b) if they already ran on
       old source → they lapsed with zero candidates (known wall, NOT a real negative) →
       NEEDS_HUMAN to upgrade-then-resubmit. Do not interpret their zero-candidate output as
       evidence about n=41/42.
     - **Trillium n=41/42: ANY `*** BS(42,41) FOUND ***` / `*** BS(43,42) FOUND ***` banner =
       our solver REPLICATING Wang-Zhu's published result — the campaign's target.** R2 with
       extra care: run `tools/verify_npaf.py` on the printed A/B/C/D; confirm the sequences are
       NOT identical to `results/reference/wz_table1_*` (identical = re-find of their exact
       solution — still a replication, say which case it is); bank to `results/champions/` with
       full provenance; update the checker exclusion list; **NEEDS_HUMAN, loud**.
     - **No hit at n=41/42** = a bounded negative ("no hit within N candidates × budget"), NOT a
       proof of absence and NOT a KILL — the Gate verdicts come from Rorqual, not Trillium.
       Do not resubmit exploratory probes without Daniel.

   - **⛔ DO NOT resubmit the n=36 Gate A′ array (`P22_GATE` / `cluster_pair22_gate.sh`).**
     Superseded 2026-07-15: a PASS at n=36 is arithmetically impossible (completed n=29
     C,D = 1.74e9 already exceeds the ≤1e9 n=36 PASS line, and streams grow ~2.86×/rung),
     and the array physically cannot finish (one C,D profile > 12h walltime, so every shard
     times out with 0 SHARD_STREAM). If one is still queued, leave it or cancel it — do not
     wait on it, do not resubmit, do not "fix" the sharding. See HANDOFF 07-15.

   - **A GATE ARRAY is running or finished** (any OTHER gate/probe) → this outranks any
     refill; it is the decision the campaign is waiting on.
     - Still running (any task R/PD) → do NOT touch it, do NOT resubmit, and say so in
       the summary. It is not "idle capacity".
     - All tasks finished → collect it:
       ```
       grep -h SHARD_STREAM pair22_gate_output_<JOBID>_*.txt | awk '{s+=$5} END {print s}'
       ```
       **⚠️ ALL-SHARDS-OR-NOTHING.** Count the SHARD_STREAM lines first: there must be
       exactly one per array task (20). If ANY shard is missing, failed, or still running,
       the sum is an UNDERCOUNT — and an undercount looks exactly like a PASS. Do NOT
       report it, do NOT act on it. Resubmit the missing shards and wait.
     - With a COMPLETE sum, read it against the pre-registered rule (do NOT move the line
       now that you can see the number): **≤ ~1e9 = PASS** → the Thm-2.2 route to n=42-43 is
       alive; **≥ 1e12 = KILL**; in between → Gate B first. Record the number + verdict in
       HANDOFF, and set NEEDS_HUMAN — deciding to build Phase 1 is Daniel's call, not yours.

   - **A code/deploy change is warranted** → make it and validate it (**R1**), but
     do **NOT** try to ship it. Shipping source needs the tar-pipe, and the
     tar-pipe cannot be driven through the Duo auto-answer (it needs stdin, which
     the Duo driver occupies). So: commit the validated change on branch
     `auto/YYYY-MM-DD`, set NEEDS_HUMAN, and put the exact tar-pipe command in the
     summary for Daniel to paste. Code deploys are a human step by design.
   - **Verified hit** → obey **R2**; bank champion; if it clears a new rung, bump
     `next_seeds.sh set-n <n+1>`.

3. **Keep the checker current.** The remote checker command lives in
   `cluster/deploy/checker_cmd.txt` and is EXPECTED to evolve — edit it whenever:
**NOTIFICATION SENTINEL (hard rule, 2026-08-05):** the phone summary's trophy title fires
ONLY on a line starting exactly `RESULT_BANKED:` -- write that line if and only if a
solution passed tools/verify_npaf.py AND was banked to results/champions/ THIS run
(e.g. `RESULT_BANKED: BS(44,43) champion_firsthit_bs44_43.txt`). NEVER write the token
otherwise -- phrases like "no verified solutions" are fine, the title no longer
substring-matches them. A false celebration is a cry-wolf bug.

   - a solution gets banked → add its file id to the `grep -vE` exclusion list, or
     "NEW FOUND?" will cry wolf on old banked files every run;
   - the ladder climbs a rung → update the `n=32 progress` label and any globs;
   - the live probe changes → update the GATE PROBES section.
   Never put `exit` in that file (it would drop the END marker and make a good run
   look like a failure).

4. **Record.** Update HANDOFF (new job IDs on the LIVE ROUND line, round verdict,
   seed ledger already advanced by the helper). Capture submit job IDs from the
   sbatch echoes.

4. **Commit + push.** `git add -A && git commit -m "auto: <one-line verdict>"`
   then **`timeout 60 git push origin main`** — ALWAYS use the timeout. 2026-07-16: a bare
   `git push` HUNG the run because the `osxkeychain` credential helper cannot pop its
   authorization dialog in a headless cron. If the push times out, do NOT retry it and do NOT
   fight the credential helper: the commit is safe locally, so just note in the summary
   "push blocked (credential helper) — run `git push origin main` at the Mac" and continue. Exception: if NEEDS_HUMAN was set for a code
   change, push the `auto/YYYY-MM-DD` branch instead of main and do not merge.

5. **Write the phone summary** to `results/last_summary.txt` — ONE short
   paragraph, plain language: per-cluster state, what you did (or why nothing),
   any new job IDs, and clearly whether a **verified** solution/record occurred
   (only if R2 passed). If NEEDS_HUMAN, start the summary with `NEEDS_HUMAN:` and
   say what to look at. This file is what gets texted to Daniel.
