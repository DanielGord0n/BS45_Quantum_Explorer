#!/bin/bash
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=8
#SBATCH --mem=24G
#SBATCH --time=8:00:00
#SBATCH --job-name=CDPILOT
#SBATCH --output=cdpilot_%j.txt
#SBATCH --account=def-ikotsire
#
# WZ_FH_CD_PRUNE paired stream-timing pilot (2026-09-25, Astra Pass H review items 2-3).
# Run from an ISOLATED dir ($SCRATCH/bs45_cdpilot). Workhorse (3,13,0,0), the F44g2000 cells
# (ORDER=1, canon, PROF_SKIP=2000, PROF_END=3000, 178-arm sharding), count-only CELLSIZE mode
# capped at the production 500k prefix. 4 shards x {CD_PRUNE=0, CD_PRUNE=3} run side by side on
# ONE node, so every cell is timed under both settings on the same hardware.
# PRE-REGISTERED (tools/cd_prune_pilot_summary.py): IDENTITY must hold (same cells, same cand per
# cell, else FAIL = do not deploy). PASS if the median per-cell time ratio off/on >= 1.25 (>= 20%
# less stream time) over >= 8 paired finished cells; CLOSE if < 1.05; otherwise INCONCLUSIVE
# (one repeat on another window). Stream-identical => a PASS deploys with no CFGSIG change.
cd "$SLURM_SUBMIT_DIR"
module load StdEnv/2023
module load gcc/12.3
BIN=cdp_bin_${SLURM_JOB_ID}
g++ -O3 -march=native -std=c++17 -fopenmp -o "$BIN" src/solver/wz_match.cpp || exit 1
echo "=== CDPILOT job $SLURM_JOB_ID node $(hostname) $(lscpu | grep -m1 'Model name' | cut -c1-60) $(date) sha $(sha256sum src/solver/wz_match.cpp | cut -c1-16) ==="
E="WZ_FIRSTHIT=1 WZ_FH_ORBIT_CANON=1 WZ_FH_PROF_ORDER=1 WZ_FH_NSHARD=178 WZ_FH_PROF_SKIP=2000 WZ_FH_PROF_END=3000 WZ_FH_CELLSIZE=500000 WZ_FH_PROG_SEC=600 OMP_NUM_THREADS=1"
pids=()
for shard in 11 57 103 149; do
  for mode in 0 3; do
    env $E WZ_FH_SHARD=$shard WZ_FH_CD_PRUNE=$mode ./"$BIN" 44 3 13 0 0 > "cdp_s${shard}_m${mode}.log" 2>&1 &
    pids+=($!)
  done
done
# stop everything 20 min before walltime so every process prints its last (partial) cell
( sleep $(( 8*3600 - 1200 )); kill -TERM "${pids[@]}" 2>/dev/null ) &
wait "${pids[@]}"
for f in cdp_s*_m*.log; do grep -h -E '^(CELLSIZE|CDSTAT)' "$f" | sed "s/^/$f /"; done
echo "=== CDPILOT done $(date) ==="
