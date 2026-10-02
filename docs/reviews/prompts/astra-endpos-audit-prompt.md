# Prompt for GPT-6 Astra (Codex): audit the odd-n Q implementation before the n=45 launch (2026-10-02)

Budget-aware: one diff, three questions. Paste everything below the line into Codex.

---

Audit, reasoning only (no edits, runs or commits). Claude implemented your n=45 note A today as
commits 3eee03a and the one after it in src/solver/wz_match.cpp. Read ONLY (targeted search):
the comment block above `static bool G_ENDPOS`, the root-quad selection in count_pairs22
(search "d0free"), the Q guard (search "[orbitq] DISABLED"), the CFGSIG tag ".ep1", and the
cell filter (search "[endpos] cells"). Gate tools/test_endpos_q.py passed: verdicts identical
with ENDPOS off/on for every class n=5..13 (both directions), Q on == off and closure prune
identical under ENDPOS at odd n, brute-force retention n=5,7,9, all 14 banked odd-n solutions
(33..43) LOCATE-retained on the production ORDER=3 list, every closure-pruned odd-n cell
streams empty, CFGSIG carries .ep1. Measured at n=45: kept orbits 1,079,164 (32-group) ->
582,683 (ENDPOS + Q + prune); the cell-level parity filter removes 0 cells at n=45 (the
existing mod-6 profile filters already imply it). A cluster control (Fir 62595724) is
re-finding both known BS(44,43) under the exact n=45 pipeline.

1. Is the endpoint argument implemented exactly: ENDPOS restricts ONLY the C,D root quad
   (d=0, C,D side) to the 8 positive-product quads; the A,B root stays P22_NEG; the odd-n
   middle is untouched. Any way this drops a completable C,D?
2. The Q guard now reads safe = ENDPOS || (n even && c+d = 0 mod 4). Under ENDPOS at odd n,
   is the existing profile-level Q (p' = (p+q+Rp-Rq)/2 with R the mod-6 reflection
   r -> (n-1-r) mod 6) the correct image of the sequence-level Q when the middle residue is
   self-reflected? Confirm or give the counterexample.
3. The closure prune's "not_realizable" certificates were derived for even n. Under ENDPOS at
   odd n the test shows unknown=0 and every pruned cell empty at n=7,9,11. State what, if
   anything, in the certificate logic depends on n being even, or confirm nothing does.

Format: three numbered answers, each a short paragraph, claims marked derived/conjectured.
