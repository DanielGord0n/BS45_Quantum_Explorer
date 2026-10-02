# Prompt for GPT-6 Astra (Codex): n=45 preparation (2026-10-02)

Budget-aware: targeted reads only, answer A before B. Paste everything below the line into Codex.

---

Context: the n=44 search found BS(45,44) on 2026-10-02 (Rorqual, Pass H lane H44G2700, class
(9,9,0,4), budget 2e6, K=50k front, build a3b84f9; banked in
results/champions/champion_firsthit_bs45_44.txt, verified by tools/verify_npaf.py and an
independent from-definition checker). The next target is BS(46,45), n=45: A,B of length 46 (even
row sums), C,D of length 45 (odd row sums), a^2+b^2+c^2+d^2 = 182. The 10 classes (a<=b, c<=d) are
(0,2,3,13) (0,6,5,11) (0,10,1,9) (2,4,9,9) (2,12,3,5) (4,6,3,11) (4,6,7,9) (6,8,1,9) (6,12,1,1)
(8,10,3,3). Reasoning only; Claude implements and verifies against the known n=43 solutions
(odd n) and exhaustive small odd n.

What changes at odd n in the current pipeline (read ONLY the orbit-canonicalization block of
src/solver/wz_match.cpp around the string "[orbitq] DISABLED", plus docs/plans/pass_h_plan.md
section 1): the quad-switch symmetry Q and its closure prune are proven only for n even with
c+d = 0 mod 4 (no middle pair; every mirror quad positive), so at n=45 the kept list is the
32-group one, about twice the orbits per class that n=44 had under the 64-group. C,D of odd
length have a middle element; the completer's A,B of length 46 do not.

A. Is there an exact symmetry of the odd-n stream that plays Q's role? State it precisely (its
   action on C,D, on the mod-6 profile cells, and on the A,B completion), the conditions under
   which it is a symmetry of the emitted candidate set (per class: which of the 10 classes
   qualify, in terms of c+d mod 4 or the sign of the middle element), and the proof obligations
   Claude must discharge (retention of every known n=43 solution orbit; small odd n exhaustive).
   If no such symmetry exists, say so and explain why the middle element breaks it.

B. Only if budget remains: the n=44 hit sat at profile_rank 488,790 with flatness score 120, in
   the forward front at cells [2700,3000) of a 968,858-cell class, after 356 cells of that lane.
   Does that single data point change anything you would recommend about the front size K, the
   window order, or the class order for n=45? One paragraph, claims marked measured/derived/
   conjectured.

Format: numbered answers; no restating of earlier reviews.
