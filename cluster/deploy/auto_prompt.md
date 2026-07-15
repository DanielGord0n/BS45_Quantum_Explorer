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
       Put "SA ladder capped at n=<N> — escalated to <experiment>" in the summary,
       and remind Daniel that `docs/kotsireas_brief.md` is READY TO SEND (the methods
       ask is the door to 42+; compute is not).
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
     It auto-types the Duo "1"; Daniel taps his phone to approve. **Confirm each
     submit echoed a `Submitted batch job <id>` before treating it as queued** —
     if `duo_run.sh` exits non-zero, the job did NOT go in. Never advance the
     ledger for a submit that did not echo a job ID.
   - **🚨 THE JOIN22 n=29 CANARY OUTRANKS EVERYTHING (job `16243606`, Rorqual, `WZ_MATCH`).**
     Context (HANDOFF 07-15 + `docs/wz_paper_reconstruction.md`): the complete join FINDS and
     self-verifies solutions — measured on a 4-core laptop, BS(20,19) in **26 s**, cost ~2.67×/rung
     ⇒ n=29 ≈ **3 h** on a 192-core node. The old "join dead above n≈29" verdict used
     **pre-Thm-2.2** counts inflated ~10⁵× and is STALE.
     - **Still running** → do not touch, do not resubmit, and SAY SO in the summary. It is the
       decision the campaign is waiting on.
     - **Finished** → look for `*** BS(30,29) FOUND ***` (the checker's new `wz_match_output_*.txt`
       "NEW FOUND?" glob catches it; a `resolve …/342 FOUND` progress line is NOT a pass).
       **PASS** ⇒ the join re-found the banked n=29 class ⇒ the frontier is re-opened. Set
       NEEDS_HUMAN and recommend walking the join UP the ladder (n=31 → 32 → 33), one signature per
       job, `WZ_JOIN22=1`, `WZ_JOIN22_SLOTS_LOG2` sized from **distinct keys ≈ stream/6** (NOT the
       raw stream — the 34 GB figure in old notes was over-sized), sharded by A,B slice.
       **Deciding to open that campaign is Daniel's call, not yours.**
       **FAIL / walltime again** ⇒ report the phase it died in; do not silently resubmit.
     - **A join FOUND banner is still only a claim** — R2 applies: `verify_npaf.py` must PASS and a
       champion file must be written before it is called a result.

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
   then `git push origin main`. Exception: if NEEDS_HUMAN was set for a code
   change, push the `auto/YYYY-MM-DD` branch instead of main and do not merge.

5. **Write the phone summary** to `results/last_summary.txt` — ONE short
   paragraph, plain language: per-cluster state, what you did (or why nothing),
   any new job IDs, and clearly whether a **verified** solution/record occurred
   (only if R2 passed). If NEEDS_HUMAN, start the summary with `NEEDS_HUMAN:` and
   say what to look at. This file is what gets texted to Daniel.
