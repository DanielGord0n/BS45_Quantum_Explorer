# Claude's verification of Astra's 2026-09-30 completer deep dive

Solver commit 38dfbd9; gate `tools/test_outer_tables.py` (PASS); pilot Fir 62283881.

| Item | Status |
|---|---|
| A1 outer-lag tables (k=2/3) | BUILT, default off (`WZ_FH_OUTER_K`). Tables reproduced by an independent enumeration from real arrays and real lags (K2 96 / K3 2848 entries; filler quads shown irrelevant). Identity: 84 class/budget runs, nodes never increase; uncapped verdict/hit/sequences identical; under caps 11 hits moved to an earlier candidate that stopped aborting (your allowed case) and every old hit candidate still completes with fewer nodes. Six controls: identical A,B, 3.0-4.1x fewer nodes. Paired completion-time pilot on Fir (62283881: 4 shards x {off,on}, 40k candidates per arm, production budget 2e6). |
| A2 profile-row reachability | Queued next, after the A1 pilot reads. Derivation accepted. |
| A3 coupled coefficient bound | Queued after A2, gated on the late-node counters. |
| B abort-tail counters | Queued; `WZ_FH_CAND_LOG` (per-candidate outcome, nodes, ns) is the first one and feeds the pilot. |
| C1 internal error | DONE: fatal exit 5 before the candidate is marked attempted; injected-disagreement test passes. |
| C2 depth-only Kab table | Queued (bookkeeping only). |
| D | Recorded, no action. |

Pre-registered pilot rule: per-candidate identity (hit stays hit with <= nodes, clean-no stays,
abort may resolve) else FAIL; PASS at >= 15% completion-time saving with no drop in resolved
candidates; CLOSE below 5%; at least 2000 paired candidates.
