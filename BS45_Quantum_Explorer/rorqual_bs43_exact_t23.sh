#!/bin/bash
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=192
#SBATCH --time=24:00:00
#SBATCH --job-name=BS43_t23_rorqual
#SBATCH --output=bs43_t23_rorqual_output_%A_%a.txt
#SBATCH --array=0-9
#SBATCH --account=def-ikotsire
#SBATCH --mail-type=END,FAIL

# === BS(43,42) EXHAUSTIVE wz_exact_t23 (Thm 2.3 prune) — Rorqual ===
# Sig-targeted (7,11,0,0): Wang-Zhu BS(43,42) signature.
# Rorqual searches combo quarter [8388608,16777216) of the 33554432 first-4-layer combos (WZ_SPLIT=4).

cd $SLURM_SUBMIT_DIR
module load StdEnv/2023
module load gcc/12.3
export OMP_NUM_THREADS=$SLURM_CPUS_PER_TASK
export WZ_SPLIT=4   # Optimization D: 4-layer split (33.5M combos) for load balancing

N=42
SIG_A=7
SIG_B=11
SIG_C=0
SIG_D=0

CLUSTER_LO=8388608
CLUSTER_HI=16777216
NTASKS=10
SPAN=$(( (CLUSTER_HI - CLUSTER_LO + NTASKS - 1) / NTASKS ))
TASK_LO=$(( CLUSTER_LO + SLURM_ARRAY_TASK_ID * SPAN ))
TASK_HI=$(( TASK_LO + SPAN ))
if [ $TASK_HI -gt $CLUSTER_HI ]; then TASK_HI=$CLUSTER_HI; fi
# Checkpoint-resume (replaces the old WAVE hack): each task does its FULL slice and
# resumes from a per-task checkpoint, so every resubmit ADVANCES instead of redoing
# the same prefix. The solver writes the watermark to WZ_CKPT every ~30s.
LO=$TASK_LO
HI=$TASK_HI
export WZ_CKPT=$SCRATCH/bs45/ckpt_rorqual_${SLURM_ARRAY_TASK_ID}.txt

# --- Auto-chain: task 0 submits the next generation (afterany on this whole
# array) at STARTUP, so the chain survives walltime kills (code after the solver
# never runs at walltime). Checkpoints make each generation resume + advance.
# Stops when: a solution is found, all task slices are checkpoint-complete, or
# CHAIN hits MAXCHAIN. To kill everything: scancel -u dangord (pending chain too).
CHAIN=${CHAIN:-0}
MAXCHAIN=${MAXCHAIN:-10}
if ls bs43_t23_*output*.txt >/dev/null 2>&1 && \
   grep -l 'REPRODUCTION CONFIRMED\|WORLD RECORD' bs43_t23_*output*.txt >/dev/null 2>&1; then
  echo "solution already found; not running or chaining"; exit 0
fi
if [ "$SLURM_ARRAY_TASK_ID" -eq 0 ] && [ "$CHAIN" -lt "$MAXCHAIN" ]; then
  # Single-lineage guard: if another pending generation of this campaign already
  # exists (double submit, or a requeued task 0), don't start a second chain.
  PEND_OTHER=$(squeue -h -u dangord -n BS43_t23_rorqual -t PD -o '%i' 2>/dev/null | grep -vc "^${SLURM_ARRAY_JOB_ID}_")
  ALL_DONE=1
  for t in $(seq 0 $((NTASKS-1))); do
    TLO=$(( CLUSTER_LO + t * SPAN )); THI=$(( TLO + SPAN ))
    [ $THI -gt $CLUSTER_HI ] && THI=$CLUSTER_HI
    CK=$SCRATCH/bs45/ckpt_rorqual_$t.txt
    if [ ! -f "$CK" ] || [ "$(cat "$CK" 2>/dev/null || echo 0)" -lt "$THI" ]; then ALL_DONE=0; break; fi
  done
  if [ "$ALL_DONE" -eq 0 ] && [ "${PEND_OTHER:-0}" -eq 0 ]; then
    sbatch --export=ALL,CHAIN=$((CHAIN+1)) --dependency=afterany:$SLURM_ARRAY_JOB_ID rorqual_bs43_exact_t23.sh \
      && echo "chained next generation (CHAIN=$((CHAIN+1)))"
  else
    echo "no chain (all_done=$ALL_DONE pending_other=${PEND_OTHER:-0})"
  fi
fi

BIN=wz_exact_t23_${SLURM_ARRAY_TASK_ID}

echo "=============================================="
echo "  BS(43,42) wz_exact_t23 — Rorqual"
echo "  Sig: ($SIG_A,$SIG_B,$SIG_C,$SIG_D)"
echo "  Job: $SLURM_ARRAY_JOB_ID  Task: $SLURM_ARRAY_TASK_ID"
echo "  Combo range: [$LO,$HI)   Node: $(hostname)"
echo "  Time: $(date)"
echo "=============================================="

g++ -O3 -march=native -std=c++17 -fopenmp -o $BIN src/solver/wz_exact_t23.cpp || exit 1
./$BIN $N $SIG_A $SIG_B $SIG_C $SIG_D $LO $HI
rm -f $BIN
echo "=== Task $SLURM_ARRAY_TASK_ID finished at $(date) ==="
