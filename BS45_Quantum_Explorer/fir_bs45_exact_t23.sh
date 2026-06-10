#!/bin/bash
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=192
#SBATCH --time=24:00:00
#SBATCH --job-name=BS45_t23_fir
#SBATCH --output=bs45_t23_fir_output_%A_%a.txt
#SBATCH --array=0-9
#SBATCH --account=def-ikotsire
#SBATCH --mail-type=END,FAIL

# === BS(45,44) WORLD-RECORD ATTEMPT — wz_exact_t23, Fir ===
# Sig-targeted (13,3,0,0): the prime BS(45,44) candidate (4x symmetry pins,
# direct analog of BS(43)'s (7,11,0,0); T23Filter = 47484 valid tuples).
# Fir searches combo quarter [0,8388608) of the 33554432 first-4-layer combos.
# DO NOT SUBMIT until BS(43,42) is blind-reproduced (validates the pipeline).

cd $SLURM_SUBMIT_DIR
module load StdEnv/2023
module load gcc/12.3
export OMP_NUM_THREADS=$SLURM_CPUS_PER_TASK
export WZ_SPLIT=4

N=44
SIG_A=13
SIG_B=3
SIG_C=0
SIG_D=0

CLUSTER_LO=0
CLUSTER_HI=8388608
NTASKS=10
SPAN=$(( (CLUSTER_HI - CLUSTER_LO + NTASKS - 1) / NTASKS ))
TASK_LO=$(( CLUSTER_LO + SLURM_ARRAY_TASK_ID * SPAN ))
TASK_HI=$(( TASK_LO + SPAN ))
if [ $TASK_HI -gt $CLUSTER_HI ]; then TASK_HI=$CLUSTER_HI; fi
LO=$TASK_LO
HI=$TASK_HI
export WZ_CKPT=$SCRATCH/bs45/ckpt_bs45_fir_${SLURM_ARRAY_TASK_ID}.txt

# Auto-chain (same machinery as the BS43 campaign; see BS43 script comments).
CHAIN=${CHAIN:-0}
MAXCHAIN=${MAXCHAIN:-10}
if ls bs45_t23_*output*.txt >/dev/null 2>&1 && \
   grep -l 'REPRODUCTION CONFIRMED\|WORLD RECORD' bs45_t23_*output*.txt >/dev/null 2>&1; then
  echo "solution already found; not running or chaining"; exit 0
fi
if [ "$SLURM_ARRAY_TASK_ID" -eq 0 ] && [ "$CHAIN" -lt "$MAXCHAIN" ]; then
  PEND_OTHER=$(squeue -h -u dangord -n BS45_t23_fir -t PD -o '%i' 2>/dev/null | grep -vc "^${SLURM_ARRAY_JOB_ID}_")
  ALL_DONE=1
  for t in $(seq 0 $((NTASKS-1))); do
    TLO=$(( CLUSTER_LO + t * SPAN )); THI=$(( TLO + SPAN ))
    [ $THI -gt $CLUSTER_HI ] && THI=$CLUSTER_HI
    CK=$SCRATCH/bs45/ckpt_bs45_fir_$t.txt
    if [ ! -f "$CK" ] || [ "$(cat "$CK" 2>/dev/null || echo 0)" -lt "$THI" ]; then ALL_DONE=0; break; fi
  done
  if [ "$ALL_DONE" -eq 0 ] && [ "${PEND_OTHER:-0}" -eq 0 ]; then
    sbatch --export=ALL,CHAIN=$((CHAIN+1)) --dependency=afterany:$SLURM_ARRAY_JOB_ID fir_bs45_exact_t23.sh \
      && echo "chained next generation (CHAIN=$((CHAIN+1)))"
  else
    echo "no chain (all_done=$ALL_DONE pending_other=${PEND_OTHER:-0})"
  fi
fi

# Distinct binary name from the BS43 campaign: recompiling over a still-running
# binary fails with ETXTBSY if both campaigns ever overlap on a cluster.
BIN=wz45_${SLURM_ARRAY_TASK_ID}

echo "=============================================="
echo "  BS(45,44) WORLD-RECORD attempt — Fir"
echo "  Sig: ($SIG_A,$SIG_B,$SIG_C,$SIG_D)  CHAIN=$CHAIN"
echo "  Job: $SLURM_ARRAY_JOB_ID  Task: $SLURM_ARRAY_TASK_ID"
echo "  Combo range: [$LO,$HI)   Node: $(hostname)"
echo "  Time: $(date)"
echo "=============================================="

g++ -O3 -march=native -std=c++17 -fopenmp -o $BIN src/solver/wz_exact_t23.cpp || exit 1
./$BIN $N $SIG_A $SIG_B $SIG_C $SIG_D $LO $HI
rm -f $BIN
echo "=== Task $SLURM_ARRAY_TASK_ID finished at $(date) ==="
