# Gate B + Gate C measurements — the first-hit probe (2026-07-16/17)

*Instrument: `WZ_FIRSTHIT=1` mode in `src/solver/wz_match.cpp` (built this session).
Streams the Thm-2.2-constrained C,D PAIR stream in deterministic DFS order
(profiles sequential, single thread ⇒ exact candidate index), completes A,B per
candidate by backtracking under Definition 1.1 + Theorem 2.2 (mirror-pair DFS,
8 combos/pair, NPAF bound pruning, per-candidate node budget), stops at the
first NPAF==0 hit. Work order: `docs/fable_workorder_firsthit_n41.md`.
Pre-registered rules (set before numbers were seen): Gate C PASS = hit within
~10⁻³ of the stream under some ordering and no degradation n=29→31; Gate B
PASS = ≤~10 ms/candidate at n=31 single-core.*

## Task 0 — Wang-Zhu Table 1 ground truth: ALL THREE PASS (banked)

Transcribed from arXiv:2506.20296 PDF (programmatic extraction, zero hand-typing),
banked at `results/reference/wz_table1_bs{42_41,43_42,44_43}.txt`:

| n | sig (a,b,c,d) | verify_npaf | Thm 2.2 encoding | 2.11a+b m=3 | m=6 | retention (all filter levels) |
|---|---|---|---|---|---|---|
| 41 | (−2,0,9,9) | PASS (all s) | PASS | PASS | PASS | KEPT |
| 42 | (7,11,0,0) | PASS (all s) | PASS | PASS | PASS | KEPT |
| 43 | (8,−2,5,9) | PASS (all s) | PASS | PASS | PASS | KEPT |

**The entire theorem stack is validated AT the target rungs.** `canary_thm211b.py`
now includes these as permanent PASS fixtures (10/10). Free intelligence: mod-3
profile spaces at n=41/42/43 are TINY (A,B/C,D survivors: 1308/604, 709/1441,
1369/1398) and mod-6 in the low millions — the profile layer is not a wall.

## Instrument lesson (negative result worth keeping)

The unpaired A,B backtracker (ported verbatim from `wz_generate.cpp`, NPAF
bounds only) **burned a 200k-node budget on 100% of 200,000 candidates at n=19
with zero completions and zero clean exhausts** (~550 s, 40e9 nodes). Adding the
Thm-2.2 mirror-pair encoding to the backtracker — exactly what WZ Step 5 says
they do — made the same 200k candidates tractable: most exhaust in ~43k nodes.
**The encoding is not optional in Step 5; any first-hit build must pair-place.**

## Gate C — first-hit fractional depth (DFS order unless noted)

| n | sig | stream size | hit idx | frac depth | wall (1 core) | notes |
|---|---|---|---|---|---|---|
| 7 | (2,4,3,−1) | 91 | 3 | 3.3e-2 | <1 ms | validation |
| 11 | (2,4,−5,1) | 809 | 4 | 4.9e-3 | <1 ms | validation |
| 19 | (6,4,5,1) | 1,291,990 | 807 | **6.2e-4** | 1.9 s | inside PASS window |
| 19 | ″ flattest-profile-first | ″ | 335 | 2.6e-4 | 0.27 s | ordering helps 2.4× |
| 19 | ″ reverse-profile (control) | ″ | 1022 | 7.9e-4 | 4.4 s | control degrades ✓ |
| 19 | ″ score gate ≤30 | ″ | 842 streamed / **22 completed** | — | 0.098 s | flat candidates ~35× denser in solutions |
| 29 | (0,6,9,1) | 1.737e9 | RUNNING | — | — | 10⁻³-window probe |
| 30 | (1,−7,6,6) | ~5e9 est | RUNNING | — | — | ″ |
| 31 | (0,−6,9,−3) | ~1.4e10 est | queued | — | — | ″ |

n=19 independent re-verification: probe find sig (6,−4,5,1) PASS all shifts
(`verify_npaf.py`), distinct C,D from any banked artifact.

## Gate B — per-candidate A,B completion cost (from the same runs)

| n | candidates tested | avg nodes/cand | avg ms/cand (1 core) | budget aborts @200k | hit's own cost |
|---|---|---|---|---|---|
| 19 | 807 | ~43k | ~2.3 ms | 57/807 (7%) | 13,017 nodes |
| 29 | RUNNING | — | — | — | — |
| 31 | queued | — | — | — | — |

Early structural signal (n=11 and n=19): **solution-bearing candidates complete
in far fewer nodes than dead candidates take to exhaust** — the first-hit bet's
core asymmetry, observed directly.

## Verdicts (to fill when n=29/30/31 land — rules above, pre-registered)

- Gate C: —
- Gate B: —
