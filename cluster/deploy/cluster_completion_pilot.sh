#!/bin/bash
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=12
#SBATCH --mem=36G
#SBATCH --time=8:00:00
#SBATCH --job-name=CPILOT
#SBATCH --output=cpilot_%j.txt
#SBATCH --account=def-ikotsire
#
# Paired COMPLETION-timing pilot (2026-09-30; Astra deep dive A1-A3 gates). Run from an ISOLATED
# dir ($SCRATCH/bs45_cpilot). Production Pass H stream (ORDER=3 + Q + prune, workhorse window
# 2000..3000, 177 arms) but every arm completes ALL of its first NCAND streamed candidates
# (BUF_CAP = DRAIN_TOP = MAX_CAND = NCAND) at the production budget 2e6, with WZ_FH_CAND_LOG=1
# writing one line per candidate. 4 shards x {PILOT_VAR=0, PILOT_VAR=PILOT_ON} side by side on ONE
# node: the same candidates, same node, both settings.
# PRE-REGISTERED (tools/completion_pilot_summary.py): IDENTITY per candidate: hit stays hit (same
# nodes or fewer), clean-no stays clean-no, an abort may stay, resolve or hit; anything else = FAIL.
# PASS if completion CPU (sum ns over paired candidates) falls >= 15% AND resolved count does not
# fall; CLOSE if the saving is < 5%; otherwise INCONCLUSIVE (one repeat on another window).
# 2026-10-01: PILOT_ON may list several on-modes ("1 2": A2 shadow + A2 prune) and SHARDS the
# shards; (shards) x (1 + modes) processes must fit --cpus-per-task. The summary tool reports
# every on-mode against mode 0 and prints the A2SHADOW counters.
cd "$SLURM_SUBMIT_DIR"
module load StdEnv/2023
module load gcc/12.3
PILOT_VAR=${PILOT_VAR:-WZ_FH_OUTER_K}; PILOT_ON=${PILOT_ON:-3}; NCAND=${NCAND:-40000}; SHARDS=${SHARDS:-10+56+102+148}
PILOT_ON=${PILOT_ON//+/ }; SHARDS=${SHARDS//+/ }  # lists are '+'-joined on the sbatch line (no spaces survive --export)
BIN=cp_bin_${SLURM_JOB_ID}
g++ -O3 -march=native -std=c++17 -fopenmp -o "$BIN" src/solver/wz_match.cpp || exit 1
echo "=== CPILOT $PILOT_VAR=0/$PILOT_ON shards=$SHARDS ncand=$NCAND job $SLURM_JOB_ID node $(hostname) $(lscpu | grep -m1 'Model name' | cut -c1-60) $(date) sha $(sha256sum src/solver/wz_match.cpp | cut -c1-16) ==="
E="WZ_FIRSTHIT=1 WZ_FH_ORBIT_CANON=1 WZ_FH_ORBIT_Q=1 WZ_FH_ORBIT_QPRUNE=1 WZ_FH_PROF_ORDER=3 WZ_FH_ORD3_NODIGEST_OK=1 WZ_FH_NSHARD=177 WZ_FH_PROF_SKIP=2000 WZ_FH_PROF_END=3000 WZ_FH_AB_BUDGET=2000000 WZ_FH_BUF_CAP=$NCAND WZ_FH_DRAIN_TOP=$NCAND WZ_FH_MAX_CAND=$NCAND WZ_FH_CAND_LOG=1 WZ_FH_PROG_SEC=600 OMP_NUM_THREADS=1"
pids=()
for shard in $SHARDS; do
  for mode in 0 $PILOT_ON; do
    env $E WZ_FH_SHARD=$shard $PILOT_VAR=$mode ./"$BIN" 44 3 13 0 0 > "cp_s${shard}_m${mode}.log" 2>&1 &
    pids+=($!)
  done
done
( sleep $(( 8*3600 - 1200 )); kill -TERM "${pids[@]}" 2>/dev/null ) &
wait "${pids[@]}"
for f in cp_s*_m*.log; do grep -h -E '^(CAND|A2SHADOW|CELLSIZE|FIRSTHIT SUMMARY|candidates_streamed=|backtracks_entered=)' "$f" | sed "s/^/$f /"; done
echo "=== CPILOT done $(date) ==="
