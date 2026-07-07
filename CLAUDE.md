# BS45_Quantum_Explorer — instructions for Claude

**Start every session by reading `HANDOFF.md`** — QUICK REFERENCE (structure, checker,
deploy) + the newest ⚡ TOP OF MIND entries. It is the canonical project state; this file and
the skill are just the routing layer. Then invoke the **`bs45-campaign`** skill
(`.claude/skills/bs45-campaign/SKILL.md`) before acting on anything cluster- or
campaign-related — it holds the playbook, the output-reading traps, the measured-dead list,
and the verification discipline.

## Hard rules

1. **Verify before claiming.** A solution exists only after `python3 tools/verify_npaf.py`
   passes on it, a champion file with provenance is written to `results/champions/`, and
   HANDOFF is updated. The solver's progress `bestAB` lines are stale/inflated — only the
   `REPRODUCTION CONFIRMED ... FOUND` banner + independent verification count.
2. **No heavy solvers on this laptop** — clusters only. Local runs are for compiling,
   small-n (≤13) validation, and the NPAF verifier.
3. **You cannot reach the clusters; Daniel pastes commands** (each ssh = a Duo push on his
   phone). Produce exact, copy-paste-ready blocks; batch per cluster.
4. **Don't rebuild what's measured dead** (exhaustion n≥36, hash-join above n≈29,
   incremental PSD pruning, bias above n=30, >12h walltimes) — the skill has the list with
   the numbers. Re-litigating these costs real cluster rounds.
5. **Honest framing:** ladder finds are solver-capability results, not records. n≤40 is
   known; 41–43 are Wang-Zhu's; n=44 is the open record and needs new mathematics.

## Layout (post-2026-06-29 reorg)

`src/{solver,verifier}/` C++ solvers (active: `wz_sa_v8.cpp`; probe: `wz_match.cpp`) ·
`cluster/deploy/` active SLURM scripts · `tools/verify_npaf.py` independent checker ·
`results/champions/` banked solutions · `docs/` incl. `kotsireas_brief.md` · `sarukhanian/`
separate sub-project. HANDOFF sections older than 2026-06-29 use pre-reorg paths.
