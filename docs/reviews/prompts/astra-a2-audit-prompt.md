# Prompt for GPT-6 Astra (Codex): audit the A2 implementation (2026-10-01, evening)

Budget-aware: one diff, one question. Paste everything below the line into Codex.

---

Audit, reasoning only (no edits, runs or commits): Claude implemented your A2 predicate today as
commit b2cf92e in src/solver/wz_match.cpp. Read ONLY these symbols (targeted search): init_a2_tables,
a2_self_ok, a2_row_ok, fh_abp_filter, and the a2_cut block inside fh_ab_search (search "A2 shadow").
Do not read anything else. The gate tools/test_a2_rows.py passed (formulas == brute force for
q <= 4; solver blocks and counts == position enumeration for L=45,6,7,9,12,13 and m=3,6; C++ ==
python on 150k random rows; 84 small-n identity runs with 0 hits under would-be cuts; six controls
re-complete with their own row). A pilot (Fir 62438417) is measuring shadow and prune on the
production stream.

Answer only:
1. Does the code implement your A2 exactly? In particular: the reflection r -> (L-1-r) mod m and
   block construction; the free-middle test being applied at every filter depth (the middle is
   placed only at the leaf, after the last filter call); using residuals row - partial sums where
   the partials already include the root quad; the mod-4 test written as (t0|t1|t2|t3) & 3 on
   possibly negative ints; a2_self_ok's parity test ((u - v) & 3) == 0.
2. Is there any input on which the predicate rejects a completable row? If yes, give the
   counterexample; if no, say so in one line.
3. In the shadow accounting: is "saved nodes = nodes under the first would-be cut per path"
   the quantity you asked for, and is counting a hit under a cut as hit_under_cut the right
   soundness alarm?

Format: three numbered answers, each at most a short paragraph, claims marked derived/conjectured.
