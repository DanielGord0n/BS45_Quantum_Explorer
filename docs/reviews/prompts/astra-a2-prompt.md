# Prompt for GPT-6 Astra (Codex): A1 result + A2 spec check (2026-10-01)

Budget-aware (Plus plan): targeted reads only, answer A first, stop when the budget runs low.
Paste everything below the line into Codex.

---

Follow-up to your 2026-09-30 completer deep dive (docs/reviews/2026-09-30-astra-deep.md).
Reasoning only; no code edits, runs, builds, tests or commits. Claude implements and verifies.

RESULT OF A1 (your outer-lag reachable-tuple tables, k=3), measured on Fir, paired on the
production Pass H stream, 4 cells x 40,000 candidates each, budget 2e6, same node, both settings:
- 160,000/160,000 candidates paired, 0 identity violations (hit->hit, clean-no->clean-no,
  abort->{resolved,abort}).
- Completion time 5189 s -> 2743 s (-47.1%); charged nodes 46.84e9 -> 9.32e9 (5.02x fewer).
- EVERY one of the 1,941 budget aborts under K=0 became a clean exhausted-no under K=3 (0 aborts
  remain in the sample; hits 0 -> 0 as expected at n=44).
- Trivial cell (every candidate dies at 34 nodes): +2.7% (table lookup overhead on instant rejects).
- Six known controls (complete-only): identical A,B, 3.0-4.1x fewer nodes. Tables matched an
  independent enumeration from real arrays. Default flipped to K=3 fleet-wide today.
Evidence: docs/reviews/evidence/cpilot_62283881.txt (read only its last 45 lines).

QUESTIONS, in priority order (answer A fully before B):

A. A2 (exact A,B profile-row reachability), before Claude builds it. Read ONLY your own A2
   section and, in src/solver/wz_match.cpp, ONLY fh_abp_filter, fh_abp_leaf_ok and the Kab
   bookkeeping inside fh_ab_search (targeted search, not the file).
   1. State the exact predicate Claude must implement at each depth d (inputs available there:
      rowA/rowB per residue, PA/PB placed so far, q remaining mirror quads, free middle), the
      proof it is necessary (never prunes a completable node), and whether it subsumes the
      existing per-residue capacity test or must run alongside it.
   2. The verification obligations: the exhaustive small-n corpus (which n, which classes), the
      six controls, and the identity rule under caps (A1 showed a capped hit may move to an
      earlier candidate that stopped aborting; is the same retention rule right for A2?).
   3. Expected effect now that A1 removed the abort tail in this sample: is A2 still the
      highest-value completer item, or has A1 absorbed most of what A2 would prune? Give the
      cheapest measurement that decides this BEFORE a full build (e.g. a counter at the Kab
      filter on the production stream).

B. Budget rule after A1. Your section B said: re-measure abort rates after any promoted prune,
   never combine a prune with a budget change in the first comparison. With 0 aborts in 160k
   production candidates at 2e6, is there any reason to touch the budget at all, in either
   direction? One paragraph.

C. Only if budget remains: with completion now ~45-50% of worker time instead of 85%, the C,D
   stream (count_pairs22 + survive_profiles6 + canonicalization) is the other half. Name the
   single stream item you would price first, with its exactness obligation.

Format: numbered answers, claims marked measured/derived/conjectured, no restating of the
deep dive. If a question cannot be answered from the targeted reads, say so instead of guessing.
