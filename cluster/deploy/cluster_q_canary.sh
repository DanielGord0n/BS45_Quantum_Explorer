#!/bin/bash
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=2
#SBATCH --mem=16G
#SBATCH --time=12:00:00
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
# Step 1 (count only): WZ_FH_CELLSIZE + WZ_FH_TARGET -> image idx, batch, rank_in_batch.
# Step 2 (real search): DRAIN_TOP=rank+1, DRAIN_BATCHES=batch+1 on that cell only.
# PRE-REGISTERED: PASS = step 2 prints "BS(43,42) FOUND" with NPAF==0 (then independently
# verified locally), at idx == step-1 idx when the hit is the target image. FAIL = step 2
# finishes the drained set without FOUND. INCONCLUSIVE = step 1 does not reach the image in
# time, or (batch+1)*(rank+1) > MAXWORK (too many completions for the walltime).
cd "$SLURM_SUBMIT_DIR"
module load StdEnv/2023
module load gcc/12.3
BIN=qc_bin_${SLURM_JOB_ID}
g++ -O3 -march=native -std=c++17 -fopenmp -o "$BIN" src/solver/wz_match.cpp || exit 1
MAXWORK=${MAXWORK:-300000}
echo "=== QCANARY job $SLURM_JOB_ID node $(hostname) $(date) sha $(sha256sum src/solver/wz_match.cpp | cut -c1-16) ==="

canary() {  # name C D
  local name=$1 C=$2 D=$3
  env WZ_FIRSTHIT=1 WZ_FH_ORBIT_CANON=1 WZ_FH_ORBIT_Q=1 WZ_FH_PROF_ORDER=1 OMP_NUM_THREADS=1 \
      WZ_FH_LOCATE_C="$C" WZ_FH_LOCATE_D="$D" ./"$BIN" 42 7 11 0 0 > "${name}_locate.log" 2>&1
  local pi; pi=$(grep -m1 -oE 'cell_idx=[0-9]+ canon_kept=YES' "${name}_locate.log" | grep -oE '[0-9]+' | head -1)
  [ -z "$pi" ] && { echo "[$name] VERDICT: INCONCLUSIVE (LOCATE found no kept cell)"; return; }
  local arm=$((pi % 178)) win=$((pi / 178))
  echo "[$name] step0: kept cell pi=$pi (window $win, arm $arm) | $(grep -m1 '^LOCATE_CANON' "${name}_locate.log")"
  local E="WZ_FIRSTHIT=1 WZ_FH_ORBIT_CANON=1 WZ_FH_ORBIT_Q=1 WZ_FH_PROF_ORDER=1 WZ_FH_NSHARD=178 WZ_FH_SHARD=$arm WZ_FH_PROF_SKIP=$win WZ_FH_PROF_END=$((win+1)) OMP_NUM_THREADS=1 WZ_FH_PROG_SEC=600"
  env $E WZ_FH_CELLSIZE=1000000000000 WZ_FH_TARGET_C="$C" WZ_FH_TARGET_D="$D" \
      ./"$BIN" 42 7 11 0 0 > "${name}_step1.log" 2>&1
  local T; T=$(grep -m1 '^TARGET' "${name}_step1.log")
  echo "[$name] step1: ${T:-NO TARGET LINE} | $(grep -m1 '^CELLSIZE' "${name}_step1.log")"
  [ -z "$T" ] && { echo "[$name] VERDICT: INCONCLUSIVE (image not reached)"; return; }
  local idx batch rank
  idx=$(sed -n 's/.* idx=\([0-9]*\).*/\1/p' <<<"$T"); batch=$(sed -n 's/.* batch=\([0-9]*\).*/\1/p' <<<"$T")
  rank=$(sed -n 's/.* rank_in_batch=\([0-9]*\).*/\1/p' <<<"$T")
  local work=$(( (batch + 1) * (rank + 1) ))
  if [ "$work" -gt "$MAXWORK" ]; then
    echo "[$name] VERDICT: INCONCLUSIVE (needs $work completions > MAXWORK=$MAXWORK)"; return
  fi
  env $E WZ_FH_AB_BUDGET=2000000 WZ_FH_DRAIN_TOP=$((rank + 1)) WZ_FH_DRAIN_BATCHES=$((batch + 1)) \
      ./"$BIN" 42 7 11 0 0 > "${name}_step2.log" 2>&1
  echo "[$name] step2: $(grep -m1 'FOUND \*\*\*' "${name}_step2.log") | $(grep -m1 '^FIRSTHIT:' "${name}_step2.log") | $(grep -m1 '^VERIFY:' "${name}_step2.log") | predicted idx=$idx"
  if grep -q 'FOUND \*\*\*' "${name}_step2.log"; then echo "[$name] VERDICT: PASS (verify locally)"
  else echo "[$name] VERDICT: FAIL (drained set exhausted without FOUND) $(grep -m1 '^RESULT' "${name}_step2.log")"; fi
  sed -n '/FOUND \*\*\*/,/^D = /p' "${name}_step2.log"
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
