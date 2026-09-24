# Prompt for GPT-6 Astra (Codex): follow-up to the 2026-09-24 review

Paste everything below the line into the SAME Codex chat as your first review, or a new one.

---

Follow-up to your review docs/reviews/2026-09-24-astra-math-review.md. Same limits as before:
reasoning only, no code edits, runs, builds, tests, worktrees or commits.

Read ONLY docs/reviews/2026-09-24-astra-review-claude-response.md. It reports what Claude
verified and measured: your quad switch Q holds on all six known solutions and gives 45%
fewer n=44 cell orbits. A cluster re-find canary for Q is running.

Answer the three questions at the end of that file:
1. Is the "Q-closure prune" sound as stated? At n=44 every emitted candidate is
   quad-positive. Can a real cell whose Q-image is not in the enumerated cell list (so it
   fails a necessary cell filter, or has no binary realization) be skipped together with
   its whole 64-orbit? State exactly which properties of the enumerator the argument needs.
2. Should cells be ordered by the orbit's MINIMUM profile score (Q-invariant) instead of
   the kept representative's score? Is there any reason the representative's score is the
   better flatness prior?
3. Is there another involution of the same kind (profile-level, preserving pair NPAF and
   both sums, binary-to-binary under a condition the stream guarantees) that the 64-element
   group still misses? If none, say "none found".

Same output format as before: claim; proof or sketch; confidence; exact specification
Claude can implement; how to verify it never loses a solution; pre-registered pass/fail rule.
Write the answer to docs/reviews/2026-09-25-astra-followup.md and stop.
