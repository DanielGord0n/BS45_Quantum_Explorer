# Prompt for GPT-6 Astra (Codex): Pass H launch review (2026-09-28)

Paste everything below the line into Codex.

---

Pre-launch review of Pass H for the BS(45,44) search. Same limits: reasoning only; no code
edits, runs, builds, tests, worktrees or commits.

READ ONLY: docs/plans/pass_h_plan.md, docs/plans/passh_manifest.json (counts, digests, lane
ranges for all 12 classes), one submit file (docs/plans/passh_submit_fir.txt), and in
src/solver/wz_match.cpp ONLY the block starting "WZ_FH_PROF_ORDER=3 (Pass H" (ordering key,
digests, refusal) and the CFGSIG lines ".oq1", ".qp1", ".bud".

WHAT WAS BUILT AND VERIFIED (Claude, 09-28): ordering key = (orbit-min profile score over
listed members, orbit id, cell key), a total order; list and kept-bitmap digests printed by
every arm; WZ_FH_EXPECT_DIGEST in every submit line, arm refuses on mismatch; CFGSIG carries
ord3, .oq1, .qp1, the node budget and the kept digest. Gates passed: digests, counts, kept sets
and LOCATE positions identical between a normal build and a build that randomizes std::sort
tie order (23 classes n=8..13); verdicts identical to ORDER=1; six controls + every n=6/8/10
solution retain a witness under Q + prune + ORDER=3; Q canary v3 re-found the known n=42
solution through the Q path on Fir; the leaf-test speedup (6.03x stream) is deployed
fleet-wide (decision-identical). Manifest: 759,190 kept orbits, 444 lane-units (fwd + rev),
windows partitioned into S = 1000/300/150 as in Pass G, ownership asserted by simulation.

QUESTIONS:
1. Is anything in the ordering key, digest binding or refusal rule insufficient for the
   determinism and exactly-one-ownership requirements in your 09-25 review? Name any gap and
   its exact check.
2. Transition: at launch, PENDING Pass G jobs are cancelled (their checkpoints untouched for
   rollback), RUNNING G reps finish and are read, no G is resubmitted; H starts every range at
   its lower bound. Any way this leaves a kept orbit unsearched by both passes, or
   double-searches one in a way that matters?
3. Two classes share identical C,D cell lists and digests: (7,11,2,2) and (1,13,2,2) (both
   c=d=2; 7^2+11^2 = 1^2+13^2). Is completing each streamed C,D candidate against BOTH A,B
   targets in one lane sound and worth it (the stream is now ~15% of worker time, so the gain
   is bounded), and does the same apply to (1,7,8,8)/(5,5,8,8), whose lists differ slightly?
4. With the stream 6x cheaper, restate the breadth-hedge break-even (K=175k vs 50k) and say
   whether the 20% hedge should launch with H, wait for H's first reps, or be dropped.

OUTPUT as before: claim; proof or sketch; confidence; exact specification; verification;
pre-registered pass/fail; expected effect. Write to docs/reviews/2026-09-28-astra-passh-launch.md
and stop.
