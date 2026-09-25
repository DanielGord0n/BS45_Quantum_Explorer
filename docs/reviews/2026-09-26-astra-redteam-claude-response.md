# Claude's verification of Astra's 2026-09-26 red-team

Every item was checked against the code, and every build was tested (six controls + small-n
corpora) before this note. Nothing here is deployed to the clusters.

| Item | Verdict | What was done |
|---|---|---|
| 1.1 truncated cell list | CONFIRMED (latent; n=44 lists are 0.48-1.04M, cap 20M) | First-hit mode now refuses to search when the cap is hit (`RESULT: CELL LIST TRUNCATED`). |
| 1.2 checkpoint meaning (budget not in CFGSIG) | CONFIRMED by design (budget was excluded deliberately) | Pass H namespace carries the attempt policy (K, B, budget) + list digest. |
| 1.3 resume at batch == DRAIN_BATCHES | **REPRODUCED AND FIXED** | Forged checkpoint: old solver did one full extra buffer (380 -> 382 at buf 4; at n=44 that is 500k completions). Fixed in `drain()`; `tools/test_resume_boundary.py` (5 configs) fails on the old solver, passes on the new. Rare in production (needs SIGTERM in a narrow window); ships with the next redeploy. |
| 1.4 interruption flags | CERTIFIED by code | The handler sets `g_fh_sigterm` then `fh_stop`; arms are single-threaded, so no state has one set without the other. Cell advancement now also requires `!g_fh_sigterm` (belt and braces). |
| 1.5 budget accounting | CONFIRMED | Middle trials charge `fh_nodes_total`, not `fh_cur`. Documented in the Pass H plan; no policy change. |
| 1.6 contradictory plan sections | DONE | `docs/plans/pass_h_plan.md` rewritten as v3. |
| Hall/PSD roundoff | CERTIFIED | Angles j*pi/100, table in double; worst-case error on the four squared sums < 1e-9 versus the 0.5 margin above 178 (a true solution is <= 178 exactly). No false rejection is possible. |
| FOUND but NPAF fails | CONFIRMED (silent) | Now prints `FH_INTERNAL_ERROR`. |
| Q-prune certificates | CONFIRMED (one sample per reason) | `WZ_FH_QPRUNE_DUMP` now also writes `<path>.certs`: one `orbit_id X Q(X) reason` line per dead orbit. |
| 2 breadth-versus-depth | ARITHMETIC VERIFIED | Recorded as a launch decision for Daniel in the Pass H plan (80/20 hedge, <= 2 node-day pilot first). |
| 3 middle signs | BUILT, default off (`WZ_FH_MID_SOLVE=1`) | Identical verdicts/hits/sequences/aborts in 192 runs; 1.8% fewer nodes at small n. Promote only on a >= 2% cluster timing. |
| 4 outer 2/3-quad tables | QUEUED | After completion timing shows where time goes. |
| 5 A,B exchange; A,B profile reachability | QUEUED | Same. |

All suites pass after these changes, including default-off identity against the c2a3813 baseline
and the n29 canary.
