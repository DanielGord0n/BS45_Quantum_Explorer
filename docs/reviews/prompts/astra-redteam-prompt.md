# Prompt for GPT-6 Astra (Codex): full red-team of soundness + efficiency (2026-09-25)

Paste everything below the line into Codex.

---

Red-team review of the BS(45,44) search. The goal is the highest achievable probability of
finding BS(45,44) per node-day. Same limits as before: reasoning only; no code edits, runs,
builds, tests, worktrees or commits. Claude implements and verifies everything you propose
against the six known solutions and exhaustive small-n corpora.

READ: docs/briefs/external_review_brief.md, docs/plans/pass_h_plan.md,
docs/reviews/2026-09-24-astra-review-claude-response.md. In src/solver/wz_match.cpp read ONLY
by targeted search: count_pairs22, survive_profiles6, enum_class_sums, thm212_ok,
PairAutoSet, fh_abp_filter, fh_abp_leaf_ok, fh_ab_search (including the early outer-
correlation check), fh_complete_ab, flat_score, the orbit-canonicalization block (with
WZ_FH_ORBIT_Q and WZ_FH_ORBIT_QPRUNE), and the front-only drain (fh_drain_top).

NUMBERS YOU CAN USE (measured unless marked):
- 12 n=44 classes; kept cell orbits 1,460,098 (32-group, running now) -> 759,190 with Q + prune.
- Per 12 h node-rep (178 single-thread arms): ~430-950 cells, 21-48M completed candidates,
  budget aborts 4-40% of completions (budget 2e6 nodes; 5e6 measured WORSE on throughput).
- Worker time: stream (C,D generation) 47.6%, completion 52.4%; 96.8% of charged completer
  nodes at late depths; completion cost flat across flatness rank; aborts highest in the
  flattest decile.
- Policy: per cell, the flattest K=50k of the first 500k streamed candidates (front-only).
  The known n=42 hit was in its cell's SECOND 500k buffer, at the ~32nd flatness percentile.
- Estimate: one front pass ~1,040 node-days (Pass G) or ~540 (Pass H); ~13 nodes running.

WHAT WE NEED (priority order; say "none found" when true):
1. SILENT-LOSS AUDIT. Walk every point where a candidate, cell or orbit is rejected (cell
   enumeration, eq 2.11a/2.11b/2.12, Hall/PSD tests, mirror-quad tables and endpoint rules,
   orbit canon + Q + closure prune, A,B normalization A[0]=B[0]=+1 and the reversal rules,
   the early check, profile-constrained completion rows, checkpoint resume, lane ownership).
   For each: is it a NECESSARY condition for every solution in the declared domain, and does
   it stay necessary when COMBINED with the others? The 09-22 pin bug was exactly a
   representative-only convention combined with orbit canonicalization. List any
   combination you cannot certify.
2. COMPLETION-SIDE MATH (the other ~52%). Sound prunes or reorderings for the A,B completer:
   an exact residual-reachability test on the A,B mirror quads against the target -N_CD,
   stronger early outer-correlation checks, better variable order. For each: why it is
   exact, and what measurement would justify building it.
3. POLICY FOR P(hit) PER NODE-DAY. Given the numbers above and the n=41/42/43 hit
   positions, is front-only K=50k/500k plus budget 2e6 plus flat-first cell order a
   defensible allocation? Would a different split win in expectation: fewer cells and deeper
   per cell, two buffers, a different K, per-class allocation weights, or re-completing
   aborted candidates later at a higher budget? Give an explicit expected-value argument with
   stated assumptions, not a preference.
4. ANYTHING ELSE that could raise P(find n=44): a mathematical reduction, a missed symmetry
   in the A,B completion, a class-level argument, or a known flaw in our reasoning.

OUTPUT FORMAT (each item): claim; proof or sketch; confidence; exact specification Claude can
implement; how to verify it never loses a solution (six controls + exhaustive n=6/8/10
corpus); pre-registered pass/fail rule; expected effect. Rank by expected gain in P(hit)
per node-day per unit of implementation effort. Write to
docs/reviews/2026-09-26-astra-redteam.md and stop.
