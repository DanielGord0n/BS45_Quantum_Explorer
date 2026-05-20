# CP493 — BS(45) Solver Project Handoff

**Date**: 2026-05-17 (updated after Commits A–E + encoding verification)
**Student**: Daniel Gordon (dangord on Alliance clusters)
**Supervisor account**: def-ikotsire
**Goal**: Find BS(45,44) δ-codes — a world record. Currently validating with BS(43,42) and BS(28,27).

---

## Solver Validation Status

### BS(28,27) — pipeline sanity check PASSED (2026-05-20)

BS(28,27) is a known easy validation case (Daniel has found it before). The current solver
reproducing it just confirms the wz_sa_v8 Commit C–G pipeline works — it is **not** a result.
Found by Commit G's 3-pair polish (Fir job 40543567 task 0, seed 28700, sig (0,-2,-5,9)),
independently verified with `verify_npaf.py` (NPAF[s]=0 for all s, fits Wang-Zhu encoding).
Tuple kept here only as a pipeline-works proof:

```
A = {-1,-1,1,-1,-1,-1,1,1,-1,-1,-1,1,1,1,1,1,1,-1,1,-1,1,-1,1,1,-1,-1,1,-1}
B = {1,1,-1,-1,1,-1,1,1,1,-1,1,-1,-1,-1,-1,-1,-1,1,1,1,1,-1,1,-1,-1,1,-1,-1}
C = {1,1,-1,1,-1,-1,1,1,-1,1,-1,1,1,-1,-1,-1,1,-1,-1,-1,-1,-1,-1,1,-1,-1,1}
D = {1,-1,-1,1,1,1,1,-1,1,-1,1,1,1,1,-1,-1,-1,1,1,1,-1,1,1,1,-1,1,1}
```

### BS(43,42), BS(45,44) — NOT yet found

BS(43) SA still plateaus at coupled cost ~28-40 (polish trigger threshold is 16, so polish never
fires on BS(43)). BS(43) is the current gate before BS(45). See "Next Steps".

### Known cosmetic bugs (do not affect solution validity)

1. **stdout interleave**: the success block (`#pragma omp critical`) and the tid==0 progress
   logger are not mutually exclusive — on BS(28) the solution banner got interleaved with a
   progress line. The A/B/C/D arrays still printed on clean lines. Fix: have the tid==0 logger
   skip printing once `g_found` is set.
2. **`g_best_ab_cost` not updated by polish**: when `endgame_polish` finds a solution it bumps
   `g_polish_solutions` but not `g_best_ab_cost`, so the log kept showing `bestAB=8` after the
   real answer (0) was found. Fix: `update_min_atomic(g_best_ab_cost, 0)` in the polish success path.

---

## What Is This Problem?

We are searching for **Balonin-Seberry δ-codes BS(n+1, n)**:
Four ±1 sequences A (length n+1), B (length n+1), C (length n), D (length n) such that their joint **Normalised Periodic Autocorrelation Function (NPAF) = 0 at all nonzero shifts**:

```
NPAF[s] = Σ_{i=0}^{n-s} (A[i]*A[i+s] + B[i]*B[i+s]) + Σ_{i=0}^{n-1-s} (C[i]*C[i+s] + D[i]*D[i+s]) = 0
```

BS(43,42) is a known result we are trying to reproduce as validation. BS(45,44) is an open problem — nobody has found it. If we find it, that's a world record.

**Mathematical structure (Wang-Zhu Theorem 2.4)**: Each pair of sequences is represented as mirror-pair 4-tuples (A[d], B[d], A[n1-1-d], B[n1-1-d]) that obey product/sum constraints:

- AB pair d=0: product = -1 → `comb8_neg[8][4]`
- AB pairs d=1..(n1/2-1): product = +1 → `comb8_pos[8][4]`
- CD pair d=0: free (no product constraint) → `comb16[16][4]`
- CD pairs d=1..(n/2-1): product = +1 → `comb8_pos[8][4]`
- Middle position (odd n only): free ±1 ±1 → `comb4[4][2]`

This encoding constrains the search space from ~2^(4n) to ~8^(n/2). **Independently verified on 2026-05-17 (see "Encoding Verification" section below): both Wang-Zhu BS(43,42) and BS(44,43) tuples from the paper fit our encoding perfectly.** The search space is sound.

---

## CRITICAL FINDING (2026-05-14): The "phased" approach was broken

The original "phased CD-then-AB" approach (`wz_sa_bs43.cpp` and v8 pre-Commit-C) had a **mathematically vacuous Phase 1**. The CD "relaxed cost" penalized `|corr_CD[s]| > 2·(n1-s)` — but `corr_CD[s]` is a sum of `(n-s)` terms each in {-2,0,+2}, so `|corr_CD[s]| ≤ 2(n-s) = 2(n1-s)-2 < 2(n1-s)` **always**. The penalty was identically zero. Phase 1 only matched sums (sum_c=tc, sum_d=td) and then handed AB essentially random sum-correct (C,D) pairs, almost none of which admit a compensating (A,B).

This explains why for ~6 months **the solver never actually found any BS solution** (no git evidence of `REPRODUCTION CONFIRMED` in any historical log).

**The fix (Commit C — see Algorithm Evolution below): block coordinate descent (BCD) on the true coupled objective.** CDState::cost now takes an optional `ab_full` parameter; when provided, the CD cost becomes `Σ|corr_CD[s] + ab_full[s]|` (the coupled NPAF residual). The main loop alternates: freeze AB → CD optimizes to cancel AB → freeze CD → AB optimizes → repeat. Each half-step is guarded so it never regresses coupled cost.

---

## Algorithm Evolution (Commits A → E)

The current solver applies five layered improvements on top of the original phased structure.

| Commit | Date | Change | Effect on cluster plateaus |
|--------|------|--------|----------------------------|
| Pre-A | — | Naive phased (broken Phase 1) | BS(28)=16, BS(34)=32, BS(43)=40-48 |
| A | 2026-05-13 | AB-phase diagnostics (`shifts_top`, `term_hist`, `ABterm`, `ab_resid`) | None (additive only) |
| B | 2026-05-13 | AB champion sharing per sig + multi-AB-per-CD 5→15 | None — proved CD was the bottleneck |
| **C** | **2026-05-14** | **Alternating CD↔AB refinement (BCD on coupled objective)** | **BS(28) 16→8, BS(34) 32→16-24** |
| D | 2026-05-14 | Stall detector + CD perturbation kick (2-3 pairs) between BCD rounds | BS(43) 40-48 → 32-38; BS(28)/BS(34) unchanged |
| E | 2026-05-17 | Escalating kick magnitude (2..7 pairs) + AB-side kicks + per-sig bestAB tracking | Currently running |

### Commit A — Diagnostics (lines ~250-265 and ~880-905)
Added globals: `g_ab_shift_residual[128]`, `g_ab_term_hist[256]`, `g_ab_terminations`, `g_ab_sum_residual`, `g_ab_npaf_residual`. In `solve_AB_SA` before the final return, when `0 < best_cost < 64`, records the per-shift residual, the termination cost bucket, and splits the residual into sum-mismatch vs NPAF-penalty.

### Commit B — AB champion sharing (mirrors `g_cd_champ`)
Added `g_ab_champ` per-sig champion vector. In `solve_AB_SA` on restart>0, 30% chance to warm-start from champion. Champion's `corr` stays valid (AB self-correlation); the cost is recomputed against the current `cd_full` (different per CD success). Multi-AB-per-CD bumped 5→15 with adaptive early-exit (now superseded by Commit C structure).

### Commit C — Alternating refinement (THE structural fix)
- `CDState::cost(tc, td, n1, n, const int *ab_full=nullptr)` — optional coupled objective. When `ab_full` set: `pen = Σ|corr[s] + ab_full[s]|`. Cost=0 ⇒ full NPAF=0 solution.
- `solve_CD_SA(..., int sig_idx, const int *ab_full=nullptr)` — same param threaded through. Champion warm-start recomputes cost in refinement mode.
- Main loop restructured: initial AB pass (4 tries) against warm-start CD; then up to 16 refinement rounds doing freeze-AB→CD-step→freeze-CD→AB-step. Both block-steps guarded by `keep-better-cost`.
- `update_min_atomic(g_best_ab_cost, ...)` from refinement-mode CD solves so `bestAB` in logs reflects best coupled cost from either direction.

### Commit D — Stall + CD perturbation
Tracks `prev_coupled`, `stall_count`. When 2 consecutive rounds fail to improve coupled cost, kicks `best_cd` by resampling 2-3 random pairs from the comb tables (same mechanism as the in-SA k-pair kick, lifted to BCD level). Counters: `g_refine_rounds`, `g_refine_kicks`.

### Commit E — Escalating + AB-side perturbation + per-sig tracking
- Restructured stall check to END-of-round (after both block-steps), using post-round coupled cost. This is what lets AB kicks actually affect the next round's CD-step target.
- 50/50 coin flip between CD-kick and AB-kick.
- `kick_level` grows by 1 per kick (cap 4 → max 7-pair kicks); resets to 0 on any coupled-cost improvement. Counter: `g_refine_kick_escalations` (logged as `kesc`).
- `g_sig_best_ab[1024]` atomic array — tracks lowest coupled cost seen per signature. Updated from both `solve_AB_SA` diagnostic block AND each refinement round. Logged as `sig_best=idx:cost,...` (top-5 lowest).
- `kRefineRounds` 12 → 16.

---

## Current SA Parameters (`SAParams` struct, line ~177)

```
initial_temp     = 50.0
cooling_rate     = 0.9999     (faster cooling for shorter cycles)
iterations       = 500000     (iterations per restart cycle)
restarts         = 20         (restart cycles per epoch)
reheat_threshold = 50000      (reheat if no improvement for this many iters)
reheat_ratio     = 0.25       (reheat to 25% of initial_temp)
```

Plus the in-SA k-pair kick: triggers at `no_improve > 30000 && best_cost > 0`, resamples 2-3 pairs, reheats to 0.5×initial.

---

## The Canonical Solver File

**`BS45_Quantum_Explorer/src/solver/wz_sa_v8.cpp`** (now ~1320 lines after Commits A-E)

This is the only solver being used. All other solver files in the repo are historical or deleted.

### Compilation (used in every SLURM script):
```bash
g++ -O3 -march=native -std=c++17 -fopenmp -o wz_sa_v8 src/solver/wz_sa_v8.cpp
```

### Invocation:
```bash
./wz_sa_v8 <n> [seed_offset] [a,b,c,d]
# e.g., ./wz_sa_v8 42 13900           (BS(43,42), seed offset 13900, random sig)
# e.g., ./wz_sa_v8 27 28500           (BS(28,27))
# e.g., ./wz_sa_v8 42 80000 7,11,0,0  (BS(43,42) LOCKED to known-good sig — diagnostic)
```

The optional 3rd arg `a,b,c,d` locks signature selection to a single sig — used by `rorqual_bs43_debug_lockedsig.sh` to test whether SA can find the known BS(43,42) when given the correct sig.

---

## Encoding Verification (2026-05-17)

Critical sanity check: do the known Wang-Zhu solutions actually live in the search space our solver explores? **YES.** Verified via Python check against the hardcoded sequences in [`src/verifier/verify_bs43.cpp`](BS45_Quantum_Explorer/src/verifier/verify_bs43.cpp):

| Property | BS(43,42) | BS(44,43) |
|----------|-----------|-----------|
| Signature (a,b,c,d) | (7, 11, 0, 0) | (8, -2, 5, 9) |
| a²+b²+c²+d² = 4n+2 | 170 = 170 ✓ | 174 = 174 ✓ |
| Parity match `get_sigs` filter | a,b odd; c,d even ✓ | a,b even; c,d odd ✓ |
| AB pair d=0 product = -1 (comb8_neg) | ✓ | ✓ |
| AB pairs d=1..⌊n1/2⌋ product = +1 | ✓ (all 20) | ✓ (all 21) |
| CD pairs d=1..⌊n/2⌋ product = +1 | ✓ (all 20) | ✓ (all 20) |
| NPAF[s] = 0 for all s | ✓ | ✓ |

**This means**: every commit of compute has been searching the right space. The remaining problem is purely search hardness, not search-space exclusion. The Wang-Zhu encoding does admit real BS solutions.

---

## Log Line Format (current, post-Commit E)

```
[t s] epochs=N speed=S bestCD=X bestAB=Y CDok=A/B ABtry=C ABterm=D ABchamp=E
       ABskip=F refine=G kicks=H kesc=I ab_resid=sum:X/npaf:Y
       shifts_top=s7:240,s3:180,... term_hist=40:34,48:21,...
       sig_best=14:4,32:8,7:8,...
```

Field meanings:
- `bestCD`: best CD cost ever seen (always 0 in normal runs — sum-matching trivial)
- `bestAB`: **best coupled cost ever seen** (from either AB phase or CD-against-AB phase). **0 = full NPAF=0 solution found**
- `CDok / total`: CD successes / total CD attempts. Rate ~0.3% (just hitting sum targets)
- `ABtry`: total AB SA invocations (~15-50× CDok with current settings)
- `ABterm`: AB attempts that terminated in the sampled plateau range (0<cost<64)
- `ABchamp`: warm-starts taken from `g_ab_champ` per-sig pool
- `ABskip`: AB attempts skipped by old adaptive early-exit (Commit B; mostly dead in C/D/E)
- `refine`: total alternating BCD rounds executed
- `kicks`: perturbation kicks fired between BCD rounds (Commits D/E)
- `kesc`: kicks fired with escalated magnitude (`kick_level > 0`, Commit E)
- `ab_resid=sum:X/npaf:Y`: accumulated residual broken into sum-mismatch (X) vs NPAF (Y)
- `shifts_top`: top-5 shifts by accumulated NPAF residual
- `term_hist`: top-5 termination cost buckets by count (modal = where AB usually gets stuck)
- `sig_best`: **top-5 signatures by lowest coupled cost seen** (Commit E key diagnostic — if one sig is far ahead of others, focus future compute there)

---

## Plateau values across commits (empirical data)

| Commit | BS(28) bestAB | BS(34) bestAB | BS(43) bestAB |
|--------|---------------|---------------|---------------|
| Pre-A (vacuous Phase 1) | 16 | 32 | 40-48 |
| B | 16 | 24-32 | 40-48 |
| **C** | **8** | **12-24** | (no clean data — old jobs cancelled mid-run) |
| D | 8 | 16 | **32-38** |
| E | TBD (jobs queued 2026-05-17) | TBD | TBD |

**Solution requires bestAB = 0.** Each halving of the plateau is progress; reaching 0 is the goal.

---

## Cluster Access

```
ssh dangord@fir.alliancecan.ca
ssh dangord@rorqual.alliancecan.ca
ssh dangord@nibi.alliancecan.ca
ssh dangord@trillium.alliancecan.ca
```

**All require Duo MFA** — Daniel must approve each connection manually. Cannot be scripted end-to-end.

Working directory on every cluster: `$SCRATCH/bs45`
The entire `BS45_Quantum_Explorer/` folder is synced there.

### Single-Duo deploy pattern (per cluster)

`tar | ssh` bundles upload + extract + sbatch in one SSH session = **one Duo prompt per cluster**:

```bash
cd /Users/danielgordon/School/CP468/CP468-Assignments/CP468-Sarukhanian/BS45_Quantum_Explorer && \
  tar -cf - src/solver/wz_sa_v8.cpp <script1>.sh <script2>.sh ... | \
  ssh dangord@<cluster>.alliancecan.ca '
    scancel --user=dangord --name=<job_name> 2>/dev/null;
    cd $SCRATCH/bs45 && tar -xvf - &&
    sbatch <script>.sh &&
    squeue -u dangord --format="%10i %25j %2t %12L %R"'
```

`tar` ignores macOS resource forks (`._*` and `LIBARCHIVE.xattr...` warnings are harmless).

---

## Current SLURM Scripts (in `BS45_Quantum_Explorer/`)

All scripts: 192 cores/node, `--account=def-ikotsire` (Nibi: `def-ikotsire_cpu`). **Seed offsets bumped by +100 on every redeploy** to avoid reusing RNG trajectories.

| Script | Cluster | Problem | Current seed range (post-E) | Array | Time |
|--------|---------|---------|------------------------------|-------|------|
| `fir_bs28_v8_test.sh` | Fir | BS(28,27) | 28500–28502 | 0-2 | 2h |
| `fir_bs34_v8_test.sh` | Fir | BS(34,33) | 34500–34502 | 0-2 | 2h |
| `fir_bs43_v8_job.sh` | Fir | BS(43,42) | 12400–12409 | 0-9 | 24h |
| `rorqual_bs43_v8_job.sh` | Rorqual | BS(43,42) | 13900–13909 | 0-9 | 24h |
| `nibi_bs43_v8_job.sh` | Nibi | BS(43,42) | 12600–12609 | 0-9 | 24h |
| `trillium_bs43_v8_job.sh` | Trillium | BS(43,42) | 14000–14009 | 0-9 | 24h |
| `fir_bs45_v8_job.sh` | Fir | BS(45,44) | 45000–45009 | 0-9 | 24h |
| `rorqual_bs45_v8_job.sh` | Rorqual | BS(45,44) | 45100–45109 | 0-9 | 24h |
| `nibi_bs45_v8_job.sh` | Nibi | BS(45,44) | 45200–45209 | 0-9 | 24h |
| `trillium_bs45_v8_job.sh` | Trillium | BS(45,44) | 45300–45309 | 0-9 | 24h |
| `rorqual_bs43_debug_lockedsig.sh` | Rorqual | BS(43,42) | 80000–80002 | 0-2 | 24h |

**DO NOT submit the BS(45) scripts until BS(43,42) has been reproduced** (currently never has). Range allowed per handoff is 50 per cluster (45000-45049, etc.) — leaves room for 4 more 10-task waves once we start.

The `rorqual_bs43_debug_lockedsig.sh` script runs `./wz_sa_v8 42 80000 7,11,0,0` — locks all 192 threads to the known-good BS(43) signature (7,11,0,0). Diagnostic: if it finds the solution under that constraint, SA works given the right sig. If it can't even with the right sig, SA itself is the bottleneck and we need a different algorithm.

---

## Current Job State (as of 2026-05-17, Commit E deploy)

| Cluster | Job ID | Status | What it's running |
|---------|--------|--------|--------------------|
| Fir | 40313876 | PD (None) | BS(28,27), seeds 28500-28502, Commit E |
| Fir | 40313877 | PD (None) | BS(34,33), seeds 34500-34502, Commit E |
| Rorqual | 12546331 | PD (None) | BS(43,42), seeds 13900-13909, Commit E |
| Nibi | 14186890 | PD (None) | BS(43,42), seeds 12600-12609, Commit E |
| Trillium | 1595709 | PD (None) | BS(43,42), seeds 14000-14009, Commit E |

The sig-locked diagnostic (`rorqual_bs43_debug_lockedsig.sh`) has NOT been deployed yet — user has the deploy command and can submit when ready.

---

## Cluster Check Script

```bash
for c in fir rorqual nibi trillium; do echo ""; echo "════════════════════ $c ════════════════════"; ssh dangord@${c}.alliancecan.ca "echo '--- QUEUE ---'; squeue -u dangord --format='%10i %25j %2t %12L %R' 2>/dev/null; echo ''; cd \$SCRATCH/bs45 2>/dev/null || exit 0; echo '--- NEW SOLUTIONS ---'; find . -maxdepth 1 -name '*.txt' -mtime -1 -exec grep -l 'REPRODUCTION CONFIRMED\|WORLD RECORD' {} + 2>/dev/null || echo '(none yet)'; echo ''; echo '--- LATEST PROGRESS (last 5 lines each) ---'; for f in \$(ls -t bs43_v8_*_output_*.txt bs28_v8_*_output_*.txt bs34_v8_*_output_*.txt bs45_v8_*_output_*.txt 2>/dev/null | head -4); do echo \"=== \$f ===\"; tail -5 \"\$f\"; echo; done"; done
```

---

## Independent NPAF Verifier (2026-05-17)

**`BS45_Quantum_Explorer/verify_npaf.py`** — standalone Python script. Independent code path from `wz_sa_v8.cpp::npaf_at` — critical for any world-record claim (don't trust the same code that found it).

Usage:
```bash
# Self-test against Wang-Zhu BS(43) and BS(44)
python3 verify_npaf.py --self-test

# Verify a solver output (paste the A = {...}; B = {...}; C = ...; D = ...; block)
python3 verify_npaf.py < solver_output.txt

# Inline
python3 verify_npaf.py --A "1,-1,1,..." --B "..." --C "..." --D "..."
```

Checks NPAF[s]=0 for all s=1..n+1, the signature equation a²+b²+c²+d²=4n+2, parity, and the Wang-Zhu pair encoding.

---

## How to Interpret Progress Logs

Sample healthy Commit E log line:
```
[7000s] epochs=4416 speed=0.62 bestCD=0 bestAB=8 CDok=141/6482 ABtry=9484
        ABterm=9299 ABchamp=53973 ABskip=0 refine=2232 kicks=557 kesc=120
        ab_resid=sum:8/npaf:248392 shifts_top=s1:11890,s3:11826,s2:11610,s4:11582,s8:11390
        term_hist=24:4885,16:1487,32:1033,52:336,28:312 sig_best=12:8,7:8,...
```

**Good signs**:
- `bestCD=0` always (trivial in v8)
- `bestAB` dropping over time (the only metric that matters for solving)
- `refine` and `kicks` growing (BCD machinery active)
- `sig_best` showing some sigs lower than others → sig selection has room to improve
- `bestAB=0` and `*** REPRODUCTION CONFIRMED ***` → SUCCESS

**Warning signs**:
- `CDok=0/N` after >1000 attempts → CD phase broken (the odd-n bug, should be fixed)
- `bestAB` stuck at exactly the same value across all tasks and many hours → either a structural floor OR sig-selection drowning out good sigs
- All `sig_best` entries clustering at the same value → structural ceiling; SA itself can't go lower
- `term_hist` shows no terminations below `bestAB` → BCD never finds the right basin

---

## Next Steps (decision tree)

### When Commit E logs arrive (~6-12h after first job starts running)

**Branch 1 — Commit E breaks through (bestAB < 8 on BS(28), or any REPRODUCTION CONFIRMED)**
→ Move directly to BS(45). Submit the 4 BS(45) scripts.

**Branch 2 — `sig_best` shows one sig FAR ahead (e.g., one at 0-4, others at 16+)**
→ Build "biased sig selection" (Commit F): instead of uniform random, weight signatures inversely by `g_sig_best_ab` (lower-cost sigs get more compute). Re-deploy.

**Branch 3 — All `sig_best` cluster at the same value (e.g., all 8 ± 2 on BS(28))**
→ Structural ceiling, not search problem. Run the sig-locked diagnostic (`rorqual_bs43_debug_lockedsig.sh`). If it ALSO plateaus at the same value with the known-good sig, SA itself is the bottleneck — need different algorithm (CP/SAT for one block, hybrid, theoretical construction from Wang-Zhu paper).

**Branch 4 — Commit E performs similar to D (no further improvement)**
→ Run the sig-locked diagnostic to separate SA-bottleneck from sig-selection-bottleneck. Path forward depends on outcome.

### If BS(43,42) IS finally reproduced
1. Run `verify_npaf.py` on the output to independently confirm.
2. Save the (A,B,C,D) tuple and signature in this handoff.
3. Deploy the 4 BS(45) SLURM scripts: `fir_bs45_v8_job.sh`, `rorqual_bs45_v8_job.sh`, `nibi_bs45_v8_job.sh`, `trillium_bs45_v8_job.sh` (all ready, seed offsets pre-set at 45000s/45100s/45200s/45300s).

---

## Important Constraints / Don't Do These

- **Do not run locally** — macOS doesn't have 192 cores; benchmarks are meaningless. Deploy straight to clusters. (Compile-checking with `g++ -c` for syntax is fine.)
- **Do not use joint (A,B,C,D) SA** — historical attempts (v4-v7) plateaued at 24/32 past n≈27.
- **Do not revert the alternating refinement (Commit C)** — without it, Phase 1 is mathematically vacuous and the solver finds nothing. The CDState::cost coupled-mode is the load-bearing change.
- **Do not modify the Wang-Zhu encoding** — verified to admit real BS solutions; comb16/comb8_pos/comb8_neg/comb4 are mathematically required.
- **Do not reuse the same seed offsets** across redeploys — increment by 100 each time.
- **Do not remove the odd-n middle position code** — needed for BS(28) n=27 and BS(34) n=33.
- **Always confirm with user before pushing to clusters** — they need to approve Duo MFA for each cluster.
- **Do not commit changes without explicit user request** — user prefers to review commits.
- **Do not speculatively build next improvements before data arrives** — costly lesson from Commit B. Wait for diagnostic signal (sig_best, kicks, kesc, plateau values) before deciding what to build next.

---

## History of What Was Tried and Why It Failed

### Previous solvers (deleted in commit `6df3b1c` "Codebase Cleanup")
- **`wz_sa.cpp`** — original, joint (A,B,C,D) SA, no Wang-Zhu encoding. Plateaued at cost=8 for BS(28).
- **`wz_sa_bs43.cpp`** — correct Wang-Zhu encoding, original "phased CD-then-AB" structure. **Now known to have mathematically vacuous Phase 1.** Never actually reproduced BS(43,42) despite the file name. Source for v8.
- **`wz_sa_trillium.cpp`** (v4–v7) — dropped Wang-Zhu encoding entirely, used unconstrained ±1 flips. Fundamental regression. Plateaued at cost=24/32.

### Critical bugs fixed in v8 lineage
1. **Odd-n CD encoding bug (CRITICAL, pre-Commit-A)**: `for (d=0; d<n/2; d++)` only filled positions 0..12 and 14..26 for n=27, leaving C[13]=D[13]=0. `CDok=0/523076` — CD was never solvable. Fixed with `cd_init_random()` middle-position branch.
2. **Missing `#include <tuple>`** — `std::tie` used without the header. Caught by gcc on clusters. Fixed.
3. **Vacuous CD relaxed cost (Commit C, the big one)** — `|corr_CD[s]| ≤ 2(n1-s)` always true; CD was just sum-matching. Fixed by adding coupled objective + alternating refinement.

### Why joint SA doesn't work (per the original analysis)
The joint (A,B,C,D) cost function has a flat landscape. At cost=24 or cost=32, the "hole" toward cost=0 is surrounded by an exponential number of states at higher cost. The Commit C alternating refinement effectively does joint optimization while exploiting the Wang-Zhu block structure for tractable per-block SA.

---

## Files That Matter

```
BS45_Quantum_Explorer/
├── src/
│   ├── solver/
│   │   └── wz_sa_v8.cpp                ← THE solver (~1320 lines after A-E)
│   └── verifier/
│       └── verify_bs43.cpp             ← C++ verifier with hardcoded BS(43)/BS(44) tuples
├── verify_npaf.py                       ← NEW: standalone Python NPAF verifier (2026-05-17)
├── fir_bs28_v8_test.sh                  ← BS(28) test on Fir
├── fir_bs34_v8_test.sh                  ← BS(34) test on Fir
├── fir_bs43_v8_job.sh                   ← BS(43) on Fir
├── rorqual_bs43_v8_job.sh               ← BS(43) on Rorqual
├── nibi_bs43_v8_job.sh                  ← BS(43) on Nibi
├── trillium_bs43_v8_job.sh              ← BS(43) on Trillium
├── fir_bs45_v8_job.sh                   ← BS(45) on Fir (HOLD until BS(43) reproduced)
├── rorqual_bs45_v8_job.sh               ← BS(45) on Rorqual (HOLD)
├── nibi_bs45_v8_job.sh                  ← BS(45) on Nibi (HOLD)
├── trillium_bs45_v8_job.sh              ← BS(45) on Trillium (HOLD)
└── rorqual_bs43_debug_lockedsig.sh      ← NEW: sig-locked BS(43) diagnostic (2026-05-17)
```

Historical/deleted from git:
- `src/solver/wz_sa_bs43.cpp` — original, even-n only, vacuous Phase 1. In git history at `6df3b1c~1`.
- `src/solver/wz_sa_trillium.cpp` — broken, lost Wang-Zhu encoding. In git history.
- `src/solver/wz_sa_v2.cpp` through `wz_sa_v6.cpp` — historical iterations.

---

## Success Criteria

- **BS(28) found**: output contains `*** REPRODUCTION CONFIRMED: BS(28,27) FOUND ***`
- **BS(43) found**: output contains `*** REPRODUCTION CONFIRMED: BS(43,42) FOUND ***`
- **BS(45) found**: output contains `*** WORLD RECORD DISCOVERY: BS(45,44) FOUND ***`

When any solution is found, the output prints the full A, B, C, D arrays and the signature (a,b,c,d). **Always verify independently**:
1. Run `python3 verify_npaf.py < <output_file>` — independent code path.
2. The output should report `PASS: NPAF[s]=0 for all s=1..n+1` and `PASS: fits Wang-Zhu pair encoding`.
3. Save the (A,B,C,D) tuple, signature, cluster, job ID, and seed offset in this handoff for posterity.

For BS(45,44), the world-record claim requires:
- Independent verification via verify_npaf.py
- Manuscript/publication-ready writeup of (A,B,C,D), the signature, the search method, total compute
- Comparison against the Wang-Zhu paper's open-problem statement

---

## Known Wang-Zhu BS(43,42) signature (target for sig-lock diagnostic)

The Wang-Zhu BS(43,42) sequences (in `src/verifier/verify_bs43.cpp`) have:
- **Signature: (a=7, b=11, c=0, d=0)**
- a²+b²+c²+d² = 49+121+0+0 = 170 = 4·42+2 ✓
- All Wang-Zhu pair-product constraints satisfied

This is what `rorqual_bs43_debug_lockedsig.sh` locks the solver to. If the solver can find BS(43,42) under that single-sig constraint, SA works given the right sig. If not, SA itself is the bottleneck.

The Wang-Zhu BS(44,43) sequences have signature **(a=8, b=-2, c=5, d=9)**, a²+b²+c²+d² = 64+4+25+81 = 174 = 4·43+2 ✓.

---

## Quick Reference: Active Commits in wz_sa_v8.cpp

Key line ranges (approximate, post-Commit E; may shift with edits):
- Lines 23-37: includes, namespace
- Lines 42-60: globals + `update_min_atomic` helper
- Lines 62-88: Wang-Zhu comb tables
- Lines 100-175: Sig struct + `get_sigs()` signature enumeration
- Lines 177-184: `SAParams` struct
- Lines 191-208: `CDState` + `CDState::cost(...)` (with optional `ab_full` from Commit C)
- Lines 213-241: `cd_init_random`
- Lines 244-265: `CDChampion`, `ABChampion` declarations + diagnostic globals (Commits A, B, D, E)
- Lines 263-540: `solve_CD_SA(..., int sig_idx, const int *ab_full=nullptr)`
- Lines 540-560: `ABState` + `ABState::cost`
- Lines 560-902: `solve_AB_SA(..., int sig_idx)` (champion sharing + k-pair kick + diagnostic)
- Lines 905-925: `main()` arg parsing (incl. `--lock-sig` from sig-lock CLI)
- Lines 935-970: signature load, champion init, per-sig array init
- Lines 970-1140: thread loop (initial AB + alternating refinement with kicks)
- Lines 1140-1235: solution verification and output
- Lines 1237-1320: tid==0 log block (all the new fields)
