# Prompt for GPT-6 Astra (Codex): Pass H design audit + next lever

Paste everything below the line into Codex.

---

Mathematics and design review for the BS(45,44) search. Same limits as your earlier
reviews: reasoning only; no code edits, runs, builds, tests, worktrees or commits.

Read ONLY:
1. docs/plans/pass_h_plan.md (the next fleet-wide change: Q + closure prune + orbit-min ordering)
2. docs/reviews/2026-09-24-astra-review-claude-response.md (what was verified and measured)
3. src/solver/wz_match.cpp ONLY by targeted search for `count_pairs22` if item 2 needs it

WHAT WE NEED (priority order):
1. PASS H AUDIT. Before this restarts every lane: can any orbit end up searched by no
   lane (ownership gaps from reordering, the prune, or the Pass G -> H transition)? Is it
   sound to change group, prune and order together in one namespace, given the kept set
   is checked to be a permutation-invariant set? Answer the three open questions in the
   plan. If you would change the plan, give the exact change and its check.
2. NEXT LEVER, STREAM COST (conditional). Candidate generation (the C,D mirror-pair DFS in
   count_pairs22 plus its Hall/PSD leaf tests) is about half of worker time; completion is
   the other half. A running job will soon report leaves per emitted candidate and seconds
   per 500k. Measured dead already: incremental PSD pruning inside the A,B backtracker.
   Propose SOUND ways to prune the C,D DFS earlier (bounds on partial mirror-quad
   assignments, profile/residue feasibility tested per level, reordering levels, etc.).
   For each: which measured quantity would make it worth building, and the pre-registered
   rule. Say "none found" if nothing is sound and cheap.

OUTPUT FORMAT (each item): claim; proof or sketch; confidence; exact specification Claude can
implement; how to verify it never loses a solution (six controls + exhaustive n=6/8/10);
pre-registered pass/fail rule. Keep it short. Write to docs/reviews/2026-09-25-astra-passh.md
and stop.
