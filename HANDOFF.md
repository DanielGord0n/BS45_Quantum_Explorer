# CP493 — BS(45) Solver Project Handoff

**Date**: 2026-05-13  
**Student**: Daniel Gordon (dangord on Alliance clusters)  
**Supervisor account**: def-ikotsire  
**Goal**: Find BS(45,44) δ-codes — a world record. Currently validating with BS(43,42) and BS(28,27).

---

## What Is This Problem?

We are searching for **Balonin-Seberry δ-codes BS(n+1, n)**:  
Four ±1 sequences A (length n+1), B (length n+1), C (length n), D (length n) such that their joint **Normalised Periodic Autocorrelation Function (NPAF) = 0 at all nonzero shifts**:

```
NPAF[s] = Σ_{i=0}^{n-s} (A[i]*A[i+s] + B[i]*B[i+s]) + Σ_{i=0}^{n-1-s} (C[i]*C[i+s] + D[i]*D[i+s]) = 0
```

BS(43,42) is a known result we are trying to reproduce as validation. BS(45,44) is an open problem — nobody has found it. If we find it, that's a world record.

**Mathematical structure (Wang-Zhu Theorem 2.4)**: Each pair of sequences can be represented as mirror-pair 4-tuples that obey product/sum constraints:
- Pair d=0: product = -1 (AB) or free (CD) → `comb8_neg[8][4]` or `comb16[16][4]`
- Pairs d=1..(n/2-1): product = +1 → `comb8_pos[8][4]`
- Middle position (odd n): free ±1 ±1 → `comb4[4][2]`

**This encoding is critical**: it constrains the search space from ~2^(4n) to ~8^(n/2), making SA feasible. Any solver that doesn't use this will plateau and fail.

---

## Algorithm: Phased CD-then-AB SA (v8)

**Key insight**: Solve the problem in two phases instead of jointly.

**Phase 1 — CD SA**: Fix sequences C and D with a *relaxed* cost function that only penalizes NPAF values that AB cannot compensate (i.e., |corr_CD[s]| > 2*(n+1-s)). This is much easier than the joint problem.

**Phase 2 — AB SA**: With CD fixed, find A and B such that NPAF_AB[s] = -NPAF_CD[s] at every shift. Cost = Σ|corr_AB[s] + corr_CD[s]|.

**Joint SA was tried (v4–v7) and failed** — it consistently plateaus at cost 24 or 32. Do not use it.

### SA Parameters (in `SAParams` struct, line ~177 of wz_sa_v8.cpp)
```
initial_temp  = 50.0
cooling_rate  = 0.9999      (faster cooling for shorter cycles)
iterations    = 500000      (iterations per restart cycle)
restarts      = 20          (restart cycles per epoch)
reheat_threshold = 50000    (reheat if no improvement for this many iters)
reheat_ratio  = 0.25        (reheat to 25% of initial_temp)
```

---

## The Canonical Solver File

**`BS45_Quantum_Explorer/src/solver/wz_sa_v8.cpp`** (943 lines)

This is the only solver being used. All other solver files in the repo are historical.

### Key v8 Features (all present, all working):

1. **Wang-Zhu pair encoding** — comb16/comb8_pos/comb8_neg/comb4 tables (lines 62–88)
2. **Odd-n middle position support** — critical fix: for odd n (e.g., n=27 for BS(28)), position n/2 was never initialized, leaving C[13]=D[13]=0. Now handled with `cd_init_random()` (lines 213–241) and mid-position mutation branch in SA loop (lines 365–413)
3. **Strong RNG seeding** — `std::random_device` + `seed_seq` mixing tid, seed_offset, nanosecond timestamp. Each thread has truly independent mt19937 (lines 837–845)
4. **Per-signature CD champion sharing** — `g_cd_champ` vector (one per signature). Each thread pushes its best CD state; 30% chance to warm-start from champion on restart (lines 244–248, 280–300, 503–515)
5. **k-pair kick escape** — when stuck (no_improve > 30000 && best_cost > 0): resample 2–3 random pairs simultaneously, recompute corr from scratch, reset temp to 0.5×initial. Both CD and AB have this (lines 326–360 for CD, lines 648–681 for AB)
6. **Multi-AB-per-CD** — 5 AB attempts per CD success to amortize expensive CD work (lines 868–874)
7. **Diagnostic logging** per epoch from thread 0 (lines 921–936):
   ```
   [<time>s] epochs=N speed=S bestCD=X bestAB=Y CDok=A/B ABtry=C
   ```
   - bestCD: best CD cost ever seen (0 = CD phase solved)
   - bestAB: best AB cost ever seen (0 = full solution found)
   - CDok/total: how many CD calls succeeded / total attempted
   - ABtry: total AB attempts (should be ~5× CDok)

### Compilation command (used in every SLURM script):
```bash
g++ -O3 -march=native -std=c++17 -fopenmp -o wz_sa_v8 src/solver/wz_sa_v8.cpp
```

### Invocation:
```bash
./wz_sa_v8 <n> <seed_offset>
# e.g., ./wz_sa_v8 42 13100   (for BS(43,42) with seed offset 13100)
# e.g., ./wz_sa_v8 27 28100   (for BS(28,27) with seed offset 28100)
```

---

## SLURM Scripts (in `BS45_Quantum_Explorer/`)

All scripts: 192 cores/node, `--account=def-ikotsire` (Nibi: `def-ikotsire_cpu`)

| Script | Cluster | Problem | Seed Range | Array | Time |
|--------|---------|---------|-----------|-------|------|
| `fir_bs28_v8_test.sh` | Fir | BS(28,27) | 28100–28102 | 0-2 | 2h |
| `fir_bs34_v8_test.sh` | Fir | BS(34,33) | 34100–34102 | 0-2 | 2h |
| `fir_bs43_v8_job.sh` | Fir | BS(43,42) | 12000–12009 | 0-9 | 24h |
| `rorqual_bs43_v8_job.sh` | Rorqual | BS(43,42) | 13100–13109 | 0-9 | 24h |
| `trillium_bs43_v8_job.sh` | Trillium | BS(43,42) | 13300–13309 | 0-9 | 24h |
| `nibi_bs43_v8_job.sh` | Nibi | BS(43,42) | 12200–12209 | 0-9 | 24h |

**Note**: Fir BS(43) uses seed offsets 12000–12009; Nibi uses 12200–12209. These are different from Rorqual (13100–13109) and Trillium (13300–13309) to avoid exploring identical RNG trajectories.

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
The entire `BS45_Quantum_Explorer/` folder is synced there (including `src/solver/wz_sa_v8.cpp`).

To submit a job (from `$SCRATCH/bs45`):
```bash
sbatch fir_bs28_v8_test.sh    # on Fir
sbatch rorqual_bs43_v8_job.sh # on Rorqual
# etc.
```

---

## Last Known Job State (as of ~2026-05-12 evening, before Warp session loss)

| Cluster | Job ID | Status | What it's running | Last known output |
|---------|--------|--------|--------------------|-------------------|
| Fir | 39732325 | PD (maintenance) | BS(28,27) test, tasks 0-2 | Not started yet |
| Fir | 39732326 | PD (maintenance) | BS(34,33) test, tasks 0-2 | Not started yet |
| Rorqual | 11992681 | RUNNING (10 tasks) | BS(43,42), seeds 13100-13109 | **bestAB=40-48 after ~15h** |
| Trillium | 1539237 | PD (priority) | BS(43,42), seeds 13300-13309 | Not started |
| Nibi | 13607933 | PD (priority) | BS(43,42), seeds 12200-12209 | Not started |

**Most important signal**: Rorqual showing `bestAB=40–48` is genuine progress — the CD phase is working (CDok > 0) and AB is getting close. bestAB=0 means a full BS(43) solution found.

---

## History of What Was Tried and Why It Failed

### Previous solvers (all abandoned):
- **`wz_sa.cpp`** — original, joint (A,B,C,D) SA, no Wang-Zhu encoding. Plateaued at cost=8 for BS(28).
- **`wz_sa_bs43.cpp`** — correct Wang-Zhu encoding, but only even-n CD (no odd-n support). Basis for v8.
- **`wz_sa_trillium.cpp`** (v4–v7) — dropped Wang-Zhu encoding entirely, used unconstrained ±1 flips. Fundamental regression. Plateaued at cost=24/32.

### Critical bugs fixed in v8:
1. **Odd-n CD encoding bug (CRITICAL)**: `for (d=0; d<n/2; d++)` only filled positions 0..12 and 14..26 for n=27, leaving C[13]=D[13]=0. `CDok=0/523076` — CD was never solvable. Fixed with `cd_init_random()` middle-position branch.
2. **Missing `#include <tuple>`**: `std::tie` used without the header. Caught by gcc on clusters. Fixed.

### Why joint SA doesn't work:
The joint (A,B,C,D) cost function has a flat landscape. At cost=24 or cost=32, the "hole" toward cost=0 is surrounded by an exponential number of states at higher cost. Phased SA avoids this by solving the easier CD subproblem first with a relaxed cost.

---

## How to Interpret Progress Logs

Sample log line:
```
[54321.2s] epochs=18432 speed=0.339 bestCD=0 bestAB=40 CDok=127/38291 ABtry=635
```

- `bestCD=0` — CD phase is working correctly ✓
- `bestAB=40` — AB is getting closer (0 = solution)
- `CDok=127/38291` — 127 CD successes out of 38291 attempts (~0.3% rate, normal)
- `ABtry=635` — 635 AB attempts = ~5× CDok (multi-AB-per-CD working ✓)
- `speed=0.339` — epochs/second

**Warning signs**:
- `CDok=0/N` after >1000 attempts → CD phase broken (was caused by odd-n bug, should be fixed)
- `bestCD >> 0` after hours → CD can't find valid configurations (check signature enumeration)
- `bestAB` stuck at same value for hours → AB getting trapped in local basin

---

## What Needs to Happen Next (in order)

1. **Get logs** (user will provide after checking clusters):
   - Check if Fir BS(28)/BS(34) tests started after maintenance
   - Check Rorqual BS(43) current bestAB (was 40-48; want to see it converging to 0)
   - Check Trillium/Nibi if they started

2. **If Rorqual BS(43) still running and bestAB still stuck around 40–48**:  
   Consider improvements to the AB phase. Options (not yet implemented):
   - **AB champion sharing**: same as CD champion, share best AB state across threads per-CD. This would let multiple threads build on near-misses.
   - **Longer AB SA**: increase iterations or restarts for AB when CD succeeds (AB is cheap relative to CD)
   - **Looser CD criterion**: currently CD requires exact `|corr_CD[s]| ≤ 2*(n1-s)`. Could try `≤ 2*(n1-s)+2` to let more CD states through to AB.

3. **If BS(28) not solved yet** (Fir maintenance delayed it): Should solve quickly once it runs — BS(28) is small, and CDok should be high.

4. **If BS(43) found**: Immediately move to BS(45):
   - Create job scripts: `fir_bs45_v8_job.sh`, `rorqual_bs45_v8_job.sh`, `nibi_bs45_v8_job.sh`, `trillium_bs45_v8_job.sh`
   - Use `./wz_sa_v8 44 <seed_offset>` (n=44 for BS(45,44))
   - Disjoint seed ranges across clusters: Fir 45000–45049, Rorqual 45100–45149, Nibi 45200–45249, Trillium 45300–45349

5. **If jobs have timed out/completed without finding**: Resubmit with new seed ranges (add 1000 to all existing offsets).

---

## Cluster Check Script (copy-paste, each cluster needs separate MFA)

```bash
for c in fir rorqual nibi trillium; do echo ""; echo "════════════════════ $c ════════════════════"; ssh dangord@${c}.alliancecan.ca "echo '--- QUEUE ---'; squeue -u dangord --format='%10i %25j %2t %12L %R' 2>/dev/null; echo ''; cd \$SCRATCH/bs45 2>/dev/null || exit 0; echo '--- SOLUTIONS ---'; find . -maxdepth 1 -name '*.txt' -mtime -1 -exec grep -l 'REPRODUCTION CONFIRMED\|WORLD RECORD' {} + 2>/dev/null || echo '(none yet)'; echo ''; echo '--- LATEST PROGRESS ---'; for f in \$(ls -t bs43_v8_*output*.txt bs28_v8_*output*.txt bs34_v8_*output*.txt 2>/dev/null | head -4); do echo \"=== \$f ===\"; tail -5 \"\$f\"; echo; done"; done
```

---

## Resubmit Commands (if jobs expired)

**On Fir** (`ssh dangord@fir.alliancecan.ca`, then `cd $SCRATCH/bs45`):
```bash
sbatch fir_bs28_v8_test.sh
sbatch fir_bs34_v8_test.sh
sbatch fir_bs43_v8_job.sh
```

**On Rorqual** (`ssh dangord@rorqual.alliancecan.ca`, then `cd $SCRATCH/bs45`):
```bash
sbatch rorqual_bs43_v8_job.sh
```

**On Trillium** (`ssh dangord@trillium.alliancecan.ca`, then `cd $SCRATCH/bs45`):
```bash
sbatch trillium_bs43_v8_job.sh
```

**On Nibi** (`ssh dangord@nibi.alliancecan.ca`, then `cd $SCRATCH/bs45`):
```bash
sbatch nibi_bs43_v8_job.sh
```

---

## Important Constraints / Don't Do These

- **Do not run locally** — macOS doesn't have 192 cores; benchmarks are meaningless. Deploy straight to clusters.
- **Do not use joint SA** — always phased CD-then-AB
- **Do not modify the Wang-Zhu encoding** — it is mathematically required
- **Do not reuse the same seed offsets** across multiple submissions of the same job — increment by 100 each time to explore new RNG trajectories
- **Do not remove the odd-n middle position code** — needed for BS(28) n=27 and BS(34) n=33
- **Always confirm with user before pushing to clusters** — they need to approve Duo MFA

---

## Files That Matter

```
BS45_Quantum_Explorer/
├── src/solver/
│   └── wz_sa_v8.cpp              ← THE solver (943 lines, see detailed description above)
├── fir_bs28_v8_test.sh            ← BS(28) test on Fir
├── fir_bs34_v8_test.sh            ← BS(34) test on Fir  
├── fir_bs43_v8_job.sh             ← BS(43) on Fir (seeds 12000-12009)
├── rorqual_bs43_v8_job.sh         ← BS(43) on Rorqual (seeds 13100-13109)
├── trillium_bs43_v8_job.sh        ← BS(43) on Trillium (seeds 13300-13309)
└── nibi_bs43_v8_job.sh            ← BS(43) on Nibi (seeds 12200-12209)
```

Historical/reference only (do not run):
- `src/solver/wz_sa_bs43.cpp` — original, even-n only, no odd-n fix
- `src/solver/wz_sa_trillium.cpp` — broken, lost Wang-Zhu encoding

---

## Success Criteria

- **BS(28) found**: output contains `*** REPRODUCTION CONFIRMED: BS(28,27) FOUND ***`
- **BS(43) found**: output contains `*** REPRODUCTION CONFIRMED: BS(43,42) FOUND ***`
- **BS(45) found**: output contains `*** WORLD RECORD DISCOVERY: BS(45,44) FOUND ***`

When any solution is found, the output will also print the full A, B, C, D arrays and the signature (a,b,c,d). Verify independently using `NPAF[s] == 0` for all s=1..n.
