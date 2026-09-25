#!/bin/bash
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=192
#SBATCH --time=24:00:00
#SBATCH --job-name=BS43_exact_rorqual
#SBATCH --output=bs43_exact_rorqual_output_%A_%a.txt
#SBATCH --array=0-9
#SBATCH --account=def-ikotsire
#SBATCH --mail-type=END,FAIL

# === BS(43,42) EXHAUSTIVE backtracking (wz_exact) — Rorqual ===
# Rorqual searches combo quarter [2048,4096) of the 8192 first-two-layer combos.

cd $SLURM_SUBMIT_DIR
module load StdEnv/2023
module load gcc/12.3
export OMP_NUM_THREADS=$SLURM_CPUS_PER_TASK

N=42
# 3-layer split: 524288 total combos, quartered across the 4 clusters.
CLUSTER_LO=131072
CLUSTER_HI=262144
NTASKS=10
SPAN=$(( (CLUSTER_HI - CLUSTER_LO + NTASKS - 1) / NTASKS ))
TASK_LO=$(( CLUSTER_LO + SLURM_ARRAY_TASK_ID * SPAN ))
TASK_HI=$(( TASK_LO + SPAN ))
if [ $TASK_HI -gt $CLUSTER_HI ]; then TASK_HI=$CLUSTER_HI; fi
# Wave-offset: each task's slice is split into NWAVES non-overlapping sub-ranges.
# Submit with sbatch --export=ALL,WAVE=<n>, n in [0, NWAVES). Default WAVE=0.
NWAVES=3
WAVE=${WAVE:-0}
SUB_SPAN=$(( (SPAN + NWAVES - 1) / NWAVES ))
LO=$(( TASK_LO + WAVE * SUB_SPAN ))
HI=$(( LO + SUB_SPAN ))
if [ $HI -gt $TASK_HI ]; then HI=$TASK_HI; fi
if [ $LO -ge $TASK_HI ]; then echo "wave $WAVE past slice end; nothing to do"; exit 0; fi

BIN=wz_exact_${SLURM_ARRAY_TASK_ID}

echo "=============================================="
echo "  BS(43,42) EXHAUSTIVE — Rorqual"
echo "  Job: $SLURM_ARRAY_JOB_ID  Task: $SLURM_ARRAY_TASK_ID"
echo "  Combo range: [$LO,$HI)   Node: $(hostname)"
echo "  Time: $(date)"
echo "=============================================="

g++ -O3 -march=native -std=c++17 -fopenmp -o $BIN src/solver/wz_exact.cpp || exit 1
./$BIN $N $LO $HI
rm -f $BIN
echo "=== Task $SLURM_ARRAY_TASK_ID finished at $(date) ==="
