# Prompt for GPT-6 Astra (Codex): mathematics review, specification only

Paste everything below the line into a NEW Codex chat.

---

You are the mathematics reviewer for an open-problem search: existence of base sequences
BS(45,44) (the next unsolved case of the base sequence conjecture). Another agent (Claude)
does ALL implementation, testing and cluster work. Your job is reasoning only.

HARD LIMITS (token budget is small):
- Do NOT edit code, run the solver, build, or run tests. No worktrees, no commits.
- Read ONLY these files unless one is insufficient for a specific claim:
  1. docs/briefs/external_review_brief.md (problem, solver, results, what was tried)
  2. docs/research/n44_search_narrowing_research.md, sections "Lever 26" onward
  3. docs/reviews/2026-09-22-n44-plan-claude-review.md
  4. src/solver/wz_match.cpp ONLY by targeted search for a function you need
     (fh_abp_filter, fh_ab_search, count_pairs22, the orbit-canonicalization block)
- Write your answer to docs/reviews/<date>-astra-math-review.md and stop.

CONTEXT YOU CAN TRUST: the three new solutions (n=41,42,43) are verified; the endpoint-pin
bug and lane overlap you found are fixed; your early outer-correlation check passed
cluster controls with exact node counts and is now default on; Pass G tiles all 12 n=44
classes at K=50k. A telemetry job is measuring stream/score/sort/complete shares and the
node-by-depth histogram; do not propose work that depends on those numbers except as
conditional recommendations.

WHAT WE NEED (in priority order):
1. NEW NECESSARY CONDITIONS or CLASS ELIMINATIONS at n=44 beyond Wang-Zhu 2.11a/2.11b/2.12,
   the 200-angle PSD bound, compression (measured 0-0.6% extra rejection) and your joint
   mirror-quad reachability. Anything that could prove one of the 12 signature classes
   empty, or shrink the (C,D) stream by a large factor, is the highest-value output.
   Consider periodic/aperiodic relations, Hall-polynomial identities at special angles,
   mod-2^k and mod-5/9/11 constraints on sums and correlations, and interactions with the
   2.11b residue structure.
2. EXTRA SYMMETRY: the full 4,096-element group (global alternation, the C,D quad
   transformation 4<->5) vs our 32-variant C,D orbit canonicalization. Specify a SOUND joint
   canonicalization that gains up to 2x without repeating the pin/canon conflict: state
   exactly which transformations act on cells, which on in-cell candidates, and what may
   be fixed at each level.
3. CONSTRUCTION ROUTES: any composition, product, or recursive construction that could
   yield BS(45,44) from known objects (normal/near-normal are empty at 44), or a proof that
   none of the standard ones apply.
4. SELECTION HYPOTHESIS: is "flattest candidates across the whole cell" (bounded top-K
   heap over the full stream) better justified than "flattest within the first 500k
   enumeration prefix"? Give an argument or a cheap test using the 3+3 known solutions.
5. FLAWS: anything in the brief that is mathematically wrong or unjustified.

OUTPUT FORMAT (for each item): claim; proof or proof sketch; confidence; an exact
specification Claude can implement (inputs, formula, where it applies, how to verify it
never rejects the six known solutions); expected effect and how to measure it; and the
pass/fail rule you would pre-register. Rank all items by expected value per implementation
effort. Say plainly when you find nothing: "no new condition found" is a valid result.
