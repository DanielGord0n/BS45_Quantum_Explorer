# Prompt for GPT-6 Astra (Codex): deep dive, no token limit (2026-09-30)

Paste everything below the line into a NEW Codex chat.

---

You are the mathematics reviewer for the BS(45,44) search (base sequences, the open case of
the base-sequence conjecture). Claude implements, tests and deploys everything; you reason.
This time there is NO token limit: take the space you need, derive things fully, and write
proofs rather than sketches. Reasoning only: no code edits, runs, builds, tests, worktrees or
commits. Read the files below fully (not by targeted search this time), plus the solver
functions named in section C.

FILES: docs/briefs/external_review_brief.md; docs/reviews/2026-09-26-astra-redteam.md and
docs/reviews/2026-09-26-astra-redteam-claude-response.md; docs/reviews/2026-09-27-astra-policy.md;
docs/reviews/2026-09-28-astra-passh-launch.md; docs/plans/pass_h_plan.md;
docs/research/wz_paper_reconstruction.md; the newest ~10 entries of HANDOFF.md.

STATE (2026-09-30): Pass H is live on all four clusters: 64-group canonicalization (your quad
switch Q) + closure prune, deterministic orbit-min ordering, 177 arms per node (an even arm
count idled half the arms: kept cells sit on one index parity because orbits are contiguous
even-sized blocks), front-only K=50k of the first 500k, budget 2e6. Your decision-identical leaf
test made streaming 6.03x faster and is deployed, so the C,D stream is now ~15% of worker time
and the A,B completer ~85%. First H reps: 897-1324 kept cells per 12 h node-rep, 131% of Pass G.
Abort rates at 2e6: workhorse (3,13,0,0) 5-28%, (1,7,8,8) 35-50% (hot), (5,5,8,8) 11-18%,
(9,9,0,4) 19-40%, (5,9,6,6) ~36%, others 20-30%. Two classes have identical C,D cell lists:
(7,11,2,2) and (1,13,2,2). All six known solutions (ours and Wang-Zhu, n=41/42/43) and every
BS(n+1,n) for n=6,8,10 are retained under the deployed configuration.

WHAT WE WANT, IN PRIORITY ORDER:

A. THE COMPLETER IS NOW THE COST. Develop, fully and implementably, the strongest sound
   speedups for the A,B completion (fh_ab_search / fh_complete_ab: outside-in mirror-quad
   DFS, target -N_CD, |t-Dab| <= Kab bound, early outer-correlation check, profile-row
   constraint, A[0]=B[0]=+1 and reversal canon). You sketched two in your red-team: (i) the
   k=2/3 outer-quad reachable-tuple tables; (ii) exact A,B mirror-pair residual reachability
   against the profile rows, including the residue-4 middle. Give complete derivations, the
   exact tables/criteria, where in the DFS they apply, memory and per-node cost, what they
   can and cannot prune, and the identity test that proves they never drop a completion
   (verdict, hit, node counts may drop). Then look for a THIRD idea we have not discussed:
   e.g. a stronger bound than Kab from the already-placed outer pairs, variable order
   changes that keep exactness, symmetry in the A,B completion beyond exchange for a=b,
   bit-parallel evaluation of the correlation updates, or a cheap necessary test on the
   C,D candidate that predicts a certain abort (so the 2e6 nodes are not spent).

B. THE ABORT TAIL. 20-50% of completions hit the 2e6 cap. Is there structure in which
   candidates abort (score, class, profile-row count, residual pattern) that a sound test
   could detect BEFORE completion, or that argues for a class-specific budget? Derive what
   would need measuring; we can add counters to the completer cheaply.

C. FULL MATH RE-AUDIT, UNHURRIED. Re-read count_pairs22, survive_profiles6, thm212_ok,
   PairAutoSet, hall_ok*, the orbit canonicalization block, the Q-closure prune, ORDER=3,
   fh_ab_search and fh_complete_ab in src/solver/wz_match.cpp in full, and re-derive each
   necessary condition from Wang-Zhu's theorems. Anything unsound, or any condition that is
   weaker than it could be for free.

D. THEORY, OPEN-ENDED. With the space to think: is there any structural reason BS(45,44)
   might not exist, or any construction we have not considered (product, recursive, from
   Turyn-type or Golay-like objects, or from the twin-class structure)? Where do the six known
   solutions sit relative to each other in the invariants we track (flatness score, orbit,
   profile), and does that suggest where at n=44 to look first? Anything from the literature
   on base sequences and T-sequences beyond Wang-Zhu that changes our search.

OUTPUT: for each item, claim; full proof; confidence; exact implementation specification
(inputs, formulas, where it applies, cost); verification protocol (six controls + exhaustive
n=6/8/10, identity requirements); pre-registered pass/fail; expected effect in worker-time
terms given the 15/85 split. Rank by expected gain per implementation effort. Say plainly
where you find nothing. Write to docs/reviews/2026-09-30-astra-deep.md and stop.
