---
description: Run the BS45 daily cluster loop — check, interpret, draft refills (approval-gated), update HANDOFF
---

# BS45 daily loop

Run the full daily campaign loop. **Invoke the `bs45-campaign` skill first** — it holds
the judgment (output-reading traps, verification discipline, measured-dead list, seed
rules). This command is only the orchestration; when they conflict, the skill wins.
$ARGUMENTS may carry extra intent (e.g. "bias arm on Fir this round") — honor it.

## 1. Check all clusters

```bash
./cluster/deploy/check_all_retry.sh
```

Tell me which clusters answered. If any came back `UNREACHABLE`, that just means I
haven't tapped its Duo push yet — carry on with the ones that responded and remind me to
re-run for the rest. Do not treat UNREACHABLE as a cluster problem.

## 2. Interpret — apply the skill's traps, not the raw lines

For each cluster that answered, read the QUEUE + LATEST OUTPUT + SOLUTIONS blocks and
report state, obeying the skill's output-reading traps. In particular:
- Trust ONLY the `*** REPRODUCTION CONFIRMED ... FOUND ***` banner — never the periodic
  `bestAB=` progress line (stale/per-signature; reads 8 on files that hold a real hit).
- `TIMEOUT` at full walltime = a completed 12h shot, not a failure. `CANCELLED … slurmstepd`
  tails = preemption/walltime; `--requeue` restarts fragments — normal.
- `grep -l FOUND` also matches OLD banked files — cross-check against results/champions/
  before calling anything new.
- Under `WZ_PSD_BIAS`, an odd `bestAB` (e.g. 9) includes the bias term; only `bestAB=0`/FOUND
  is exact.

State, per cluster: running vs pending arrays, floor reached, and whether it is IDLE
(nothing queued) and therefore needs a refill.

## 3. If there's a hit — verify before ANY claim

Do not call it a solution yet. Follow the skill's verification chain:
banner dump → `python3 tools/verify_npaf.py` PASS → write a champion file to
`results/champions/` with full provenance → update HANDOFF → commit. An unverified banner
is not a result.

## 4. Draft refills for idle clusters — then STOP for my approval

For each idle cluster, compute the next **disjoint** `WZ_SEED_BASE` from the ledger:
- Read the current rung and the seed ledger from HANDOFF (grep for `Seed ledger` / `next`
  in the QUICK REFERENCE snapshot). Bases advance by the stride recorded there (3,000,000),
  one fresh base per cluster, **never reused** — same base = identical trajectories = a
  wasted round. Recall `SEED = WZ_SEED_BASE + ARRAY_TASK_ID*100000`.
- Produce a paste-ready block PER cluster in this form (add `--account=def-ikotsire_cpu`
  for **Nibi only**):

```bash
ssh dangord@<cluster>.alliancecan.ca 'cd $SCRATCH/bs45 && sbatch --requeue \
  --export=ALL,WZ_N=<current-n>,WZ_SEED_BASE=<next-base> ./cluster_sa_ladder.sh'
```

If a round needs NEW or patched source on the clusters, use the tar-pipe from repo root
(scp does not expand `$SCRATCH`), per the skill.

**Then stop.** List the exact commands and the seed base each will consume, and wait for my
explicit go-ahead. Submitting jobs is state-changing and burns real cluster rounds — I
approve first. Because `check_all_retry.sh` left the connections open, these submits reuse
them with no new Duo push (if you're within the persist window; otherwise tap once more).

## 5. After I approve and the submits echo job IDs

- Record each new job ID against its cluster.
- Update HANDOFF: append the new job IDs to the LIVE ROUND line, advance the seed ledger
  (mark the consumed bases used, note the next free ones), and write a one-line round
  verdict. Follow "update HANDOFF whenever checker results change" from the skill.
- Commit with a short message.
