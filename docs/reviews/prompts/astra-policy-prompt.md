# Prompt for GPT-6 Astra (Codex): policy after the three measurements (2026-09-26)

Paste everything below the line into Codex.

---

Follow-up to your 2026-09-26 red-team of the BS(45,44) search. Same limits: reasoning only;
no code edits, runs, builds, tests, worktrees or commits. Read ONLY
docs/reviews/2026-09-26-astra-redteam-claude-response.md and docs/plans/pass_h_plan.md.

NEW MEASUREMENTS (Fir, workhorse class (3,13,0,0), 12 h reps, 178 single-thread arms):
1. Cell sizes: every one of 530 finished live cells exceeded the 4M-candidate cap, i.e. >= 8x
   the 500k front prefix (the known n=42 solution's kept cell streamed to 24.0M candidates).
   Streaming the 500k prefix takes a median 0.62 h per arm. Whole-cell top-K is dead (KILL).
2. The stream's cost is the leaf spectral test: median 581 DFS leaves per emitted candidate
   (p90 1,569), ~220k leaves/s/arm. DFS prunes (your endpoint filter + residual reachability)
   cut 12% of DFS visits for 1% of wall time (CLOSE). A decision-identical leaf test (pair test
   implies both singles; angles ordered by measured rejection frequency) is in a paired pilot now.
3. Under the 64-element (Q) canonicalization, the known n=42 solution's image in its kept cell
   sits at stream index 23.8M, drain batch 47, in-cell flatness rank ~257k-301k of that batch,
   i.e. ~14.5M completions deep under the front policy. Under the old 32-group its image was in
   batch 2 at the ~32nd percentile. So a solution's in-cell position depends on which orbit
   representative is kept; the front policy captures a representative-dependent 50k-of-N sample.
4. Abort rates (budget 2e6): class (5,9,6,6) pools at 36.4% (lanes 4-52%), (7,11,2,2) 27%,
   (1,13,2,2) 25%, workhorse 6-31%.

QUESTIONS (say "none" or "no change" when that is the answer):
1. REPRESENTATIVE CHOICE. Given (3), is there a sound rule for choosing the kept representative
   of each orbit that makes front capture more likely or the tile cheaper: e.g. the listed member
   with the smallest cell (fewest candidates: cannot be counted cheaply, but is there a proxy?),
   or the member whose first 500k prefix is flattest? State what it must preserve (exhaustive
   reachability, determinism across toolchains, the manifest) and how to verify it.
2. POLICY UPDATE. Redo your breadth-versus-depth expected-value argument with the measured
   0.476/0.524 split, cells >= 8x the prefix, and the fact that per-orbit capture is a random
   sample under representative choice. Does the 80/20 hedge stand, change, or drop? Is there a
   cheap way to measure the in-buffer rank distribution of solutions at n=44 without a hit?
3. ABORTS. With 25-36% of completions unresolved at 2e6 nodes, is there any policy (per-class
   budget, replaying aborts only in the flattest decile, or a lower budget with more candidates)
   whose expected value beats the incumbent under stated assumptions? Give the threshold in
   measurable quantities.
4. Anything in (1)-(4) that changes a soundness conclusion from your red-team.

OUTPUT FORMAT as before: claim; proof or sketch; confidence; exact specification; verification
(six controls + exhaustive n=6/8/10); pre-registered pass/fail; expected effect. Write to
docs/reviews/2026-09-27-astra-policy.md and stop.
