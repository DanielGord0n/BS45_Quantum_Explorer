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

## Steps

1. **Interpret** `$CHECK_OUTPUT` using the skill's output-reading traps (trust
   only the banner, not `bestAB`; TIMEOUT at full walltime = completed; old banked
   files match the FOUND grep — cross-check `results/champions/`).

2. **Decide** the single smartest action (skill doctrine: one option, one
   sentence of why). Branches:
   - **Jobs still running / too early for results** → do nothing but bookkeeping.
     Explicitly note "insufficient runtime, no action" — do NOT invent changes.
   - **Idle cluster(s), same method** → refill. Get seeds deterministically:
     `cluster/deploy/next_seeds.sh take <#idle>` (never pick seeds yourself).
     Submit with the standard block (add `--account=def-ikotsire_cpu` for Nibi):
     ```
     ssh dangord@<cluster>.alliancecan.ca 'cd $SCRATCH/bs45 && sbatch --requeue \
       --export=ALL,WZ_N=<n>,WZ_SEED_BASE=<base> ./cluster_sa_ladder.sh'
     ```
     (The checker left SSH masters open, so this usually needs no new Duo tap.)
   - **A code/deploy change is warranted** → make it, then obey **R1**. Ship new
     source via tar-pipe from repo root (scp does not expand `$SCRATCH`), then
     submit. If validation fails, R1 applies (branch + NEEDS_HUMAN).
   - **Verified hit** → obey **R2**; bank champion; if it clears a new rung, bump
     `next_seeds.sh set-n <n+1>`.

3. **Record.** Update HANDOFF (new job IDs on the LIVE ROUND line, round verdict,
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
