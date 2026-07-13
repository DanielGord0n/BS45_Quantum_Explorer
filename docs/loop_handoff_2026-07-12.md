# Handoff — autonomous loop + campaign state (2026-07-12, end of day)

*For a fresh session (Fable in Claude Code). Read `HANDOFF.md` QUICK REFERENCE + newest
TOP OF MIND first, then invoke the `bs45-campaign` skill. This file covers only what the
last three days added, which HANDOFF does not fully carry yet.*

## State right now — three questions in flight, all answerable

| Cluster | Job | What it answers |
|---|---|---|
| Trillium | `1921290` | **THE GATE.** n=36 C,D stream under Thm 2.2, 20-task array. Decides the n=42-43 route. |
| Trillium | `1921309` | **THE CANARY.** JOIN22 v2 n=29, 24h. Has NEVER passed — until it does, no negative from that path means anything. |
| Fir | `48409027` | SA ladder n=32, seed 105M. Rung is 13/27 — still ACTIVE, still justified. |
| Nibi | `17518826` | Old gate array, stuck PD behind a maintenance reservation. Harmless; cancel whenever. |

**Nothing else is worth submitting until the gate answers.** There is no solver that
functions at n=42 (SA caps ~33-35; exhaustion n>=36 is 6-15 orders short; hash-join OOMs at
36 and 42). Compute does not generate methods — it tests them. Two hypotheses are in flight;
that is the whole testable surface right now.

## Collect the gate (do this first)

```
grep -h SHARD_STREAM pair22_gate_output_1921290_*.txt | awk '{s+=$5;n++} END {print "shards="n"/20  CD_stream="s}'
```
**ALL-SHARDS-OR-NOTHING: the sum is valid only at shards=20.** A missing/failed/running shard
undercounts — and an undercount looks exactly like a PASS. Never mix job ids (the checker now
groups per-job for this reason).

**Pre-registered rule (do NOT move the line now that the number is visible):**
- **<= ~1e9 → PASS** ⇒ Thm-2.2 route to n=41-43 is alive; resume Phase 1 (joint-pair generation,
  `docs/wz_firsthit_plan.md`). Building Phase 1 is Daniel's explicit call, not the loop's.
- **>= 1e12 → KILL** ⇒ the Thm-2.2 lift is not the lever either.
- **in between → run Gate B** (per-candidate A,B completion cost) before judging.

## What the loop is (built 07-10..07-12, all in `cluster/deploy/`)

`daily_auto.sh` — 1pm launchd → checker → headless Claude (Fable) → commit/push → ntfy.
`check_all_retry.sh` + `duo_ssh.py` — auto-types the Duo "1"; you tap the phone. One push per
cluster, sequential. `checker_cmd.txt` — the remote checker command; **it is meant to evolve**
(exclusion filter after each bank, rung label, gate probes).
`duo_run.sh <cluster> '<cmd>'` — how anything gets submitted (plain ssh cannot pass Duo).
`next_seeds.sh` / `seed_ledger.txt` — deterministic seed bases; the model never picks seeds.
`rung_status.sh` / `rung_state.txt` — **the exit condition**. Pre-registered budget (27 arrays
at n=32 = 3x the ~9 that cracked n=31). On EXHAUSTED the loop stops buying SA tickets and
escalates to the method experiments. A floor improvement extends the budget; a verified hit
promotes the rung.
`auto_prompt.md` — the unattended agent's rails: R1 validate-before-ship, R2 verify-before-claim,
R3 never route around a guard.
`guard_git_push.py` — copy to `~/.claude/tools/`. Blocks headless pushes to main unless the
commit is bookkeeping-only (HANDOFF, seed_ledger, rung_state, checker_cmd, results/).

## Hard-won facts (do not rediscover)

- **macOS ships bash 3.2.** No `${x^^}`, and `local a=$1 b=$a` on one line breaks under `set -u`.
- **Duo is interactive**: it prints a menu and waits for "1" to be typed. Nothing auto-pushes.
  `duo_ssh.py` types it under a pty. **pty output is CRLF — strip `\r` or every match fails.**
- **Never use SSH ControlMaster here** — the persistent masters leak Duo prompts and pile up.
- Claude usage limits: `daily_auto.sh` now defers and retries, but ONLY when the agent provably
  did nothing (no summary + HEAD unchanged). A partial run is never retried — double-submit risk.
- The pair22 counter was OpenMP-parallel within a node but could not span nodes; that alone is
  why the gate was unreachable for 4 days. `WZ_PROF_LO`/`WZ_PROF_HI` shard it (exact: validated
  at n=10, a 3-way partition summed to the unsharded total).

## Do NOT

- Do not point clusters at n=42 "to make progress" — no solver functions there; it returns nothing.
- Do not build Phase 1 before the gate answers (the campaign's own doctrine: *measure before
  building* — a count-only probe already killed a multi-week build with two numbers).
- Do not kill SA. It is the only thing that has ever found anything (28→29→30→31). It caps
  ~33-35; it is not dead at 32, and n=32 is still inside budget.
- Do not claim a result without `verify_npaf.py` + a champion file. Honest framing: n<=40 is
  known, 41-43 are Wang-Zhu's, **n=44 is the open record and needs new mathematics.**
