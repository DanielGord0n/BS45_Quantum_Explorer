#!/bin/bash
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=2
#SBATCH --mem=16G
#SBATCH --time=36:00:00
#SBATCH --job-name=QCANARY
#SBATCH --output=qcanary_%j.txt
#SBATCH --account=def-ikotsire
#
# Quad-switch (WZ_FH_ORBIT_Q) cluster re-find canary, 2026-09-24. Run from an ISOLATED dir
# ($SCRATCH/bs45_qcanary), never the production tree. n=42 class (7,11,0,0), the Pass G
# stream env (flat order, orbit canon, budget 2e6, early check default) plus ORBIT_Q=1.
# Two targets in parallel, each restricted to the ONE kept cell holding its image.
# v2 (2026-09-25): the cell is located IN THIS JOB (step 0, LOCATE, same binary as the
# search). v1 used an index computed on macOS; cells tied on profile score are ordered by
# std::sort's unspecified tie behaviour, so libc++ and Fir's libstdc++ disagree inside a
# tie (proven: libc++ tie randomization moves ours42 from 582325 to 549054) and v1 hit an
# orbit-duplicate cell. ours42's kept cell holds ONLY a Q-image (the real test); wz42's is
# also reachable by the old group (secondary).
# Also: writes the Q-closure-prune dead-cell list for (3,13,0,0) in THIS toolchain's order,
# so the CELLSIZE job 61315095's pi values can be checked against it.
# v3 (2026-09-26): v2 located both images but ours42's sits at batch 47, in-cell rank ~301k
# (14.5M completions under the front policy) and wz42's cell had not finished streaming at
# 12 h. v3 streams each kept cell (count-only) to the image and completes THAT image directly
# (WZ_FH_TARGET_COMPLETE=1, canary budget 5e7 nodes), then stops. 36 h walltime, 2 cores.
# PRE-REGISTERED: PASS = "BS(43,42) FOUND" with VERIFY NPAF==0 (then tools/verify_npaf.py
# locally) and the printed C,D in the target's 64-orbit. FAIL = TARGET_COMPLETE printed but no
# FOUND (completer could not complete a known solution's image => do NOT deploy Q).
# INCONCLUSIVE = image not reached in time (no TARGET_COMPLETE line).
cd "$SLURM_SUBMIT_DIR"
module load StdEnv/2023
module load gcc/12.3
BIN=qc_bin_${SLURM_JOB_ID}
g++ -O3 -march=native -std=c++17 -fopenmp -o "$BIN" src/solver/wz_match.cpp || exit 1
echo "=== QCANARY job $SLURM_JOB_ID node $(hostname) $(date) sha $(sha256sum src/solver/wz_match.cpp | cut -c1-16) ==="

canary() {  # name C D
  local name=$1 C=$2 D=$3
  env WZ_FIRSTHIT=1 WZ_FH_ORBIT_CANON=1 WZ_FH_ORBIT_Q=1 WZ_FH_PROF_ORDER=1 OMP_NUM_THREADS=1 \
      WZ_FH_LOCATE_C="$C" WZ_FH_LOCATE_D="$D" ./"$BIN" 42 7 11 0 0 > "${name}_locate.log" 2>&1
  local pi; pi=$(grep -m1 -oE 'cell_idx=[0-9]+ canon_kept=YES' "${name}_locate.log" | grep -oE '[0-9]+' | head -1)
  [ -z "$pi" ] && { echo "[$name] VERDICT: INCONCLUSIVE (LOCATE found no kept cell)"; return; }
  local arm=$((pi % 178)) win=$((pi / 178))
  echo "[$name] step0: kept cell pi=$pi (window $win, arm $arm) | $(grep -m1 '^LOCATE_CANON' "${name}_locate.log")"
  env WZ_FIRSTHIT=1 WZ_FH_ORBIT_CANON=1 WZ_FH_ORBIT_Q=1 WZ_FH_PROF_ORDER=1 WZ_FH_NSHARD=178 WZ_FH_SHARD=$arm \
      WZ_FH_PROF_SKIP=$win WZ_FH_PROF_END=$((win+1)) OMP_NUM_THREADS=1 WZ_FH_PROG_SEC=600 \
      WZ_FH_CELLSIZE=1000000000000 WZ_FH_TARGET_C="$C" WZ_FH_TARGET_D="$D" WZ_FH_TARGET_COMPLETE=1 \
      WZ_FH_AB_BUDGET=50000000 ./"$BIN" 42 7 11 0 0 > "${name}_step1.log" 2>&1
  echo "[$name] step1: $(grep -m1 '^TARGET_COMPLETE' "${name}_step1.log" || echo NO TARGET_COMPLETE LINE) | $(grep -m1 '^CELLSIZE' "${name}_step1.log")"
  if ! grep -q '^TARGET_COMPLETE' "${name}_step1.log"; then echo "[$name] VERDICT: INCONCLUSIVE (image not reached)"; return; fi
  echo "[$name] result: $(grep -m1 'FOUND \*\*\*' "${name}_step1.log") | $(grep -m1 '^FIRSTHIT:' "${name}_step1.log") | $(grep -m1 '^VERIFY:' "${name}_step1.log")"
  if grep -q 'FOUND \*\*\*' "${name}_step1.log"; then echo "[$name] VERDICT: PASS (verify locally)"
  else echo "[$name] VERDICT: FAIL (image reached, completer did not complete it) $(grep -m1 '^RESULT' "${name}_step1.log")"; fi
  sed -n '/FOUND \*\*\*/,/^D = /p' "${name}_step1.log"
}

env WZ_FIRSTHIT=1 WZ_FH_ORBIT_CANON=1 WZ_FH_ORBIT_Q=1 WZ_FH_ORBIT_QPRUNE=1 WZ_FH_PROF_ORDER=1 \
    WZ_FH_ORBIT_AUDIT=1 WZ_FH_QPRUNE_DUMP=qprune_dead_3_13_0_0.txt OMP_NUM_THREADS=1 \
    ./"$BIN" 44 3 13 0 0 > qprune_3_13_0_0.log 2>&1
echo "[qprune] $(grep -m1 'qprune\] missing' qprune_3_13_0_0.log) | dead raw cells in windows 2000-2999: $(awk '$1>=356000 && $1<534000' qprune_dead_3_13_0_0.txt | wc -l)"
canary ours42 \
  "1,-1,1,-1,1,1,1,-1,-1,-1,-1,-1,1,-1,1,1,-1,-1,1,-1,1,1,-1,1,1,1,-1,-1,1,1,-1,-1,1,-1,-1,1,1,-1,1,1,-1,-1" \
  "1,-1,1,-1,1,1,-1,1,1,1,-1,-1,-1,-1,-1,-1,1,1,-1,1,1,1,1,-1,-1,-1,1,1,1,-1,-1,-1,-1,1,1,-1,1,-1,1,1,-1,-1" &
canary wz42 \
  "1,1,1,1,1,-1,-1,-1,-1,1,1,1,-1,-1,1,1,1,-1,1,1,-1,1,-1,-1,1,-1,-1,-1,-1,1,-1,-1,1,-1,-1,1,1,1,-1,-1,1,-1" \
  "1,1,-1,-1,-1,-1,-1,-1,1,-1,-1,-1,1,1,1,1,-1,-1,-1,1,-1,1,-1,1,1,1,-1,-1,1,-1,1,1,-1,1,-1,1,1,-1,1,1,1,-1" &
wait
echo "=== QCANARY done $(date) ==="
