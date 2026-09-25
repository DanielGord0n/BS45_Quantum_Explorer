#!/bin/bash
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=192
#SBATCH --time=24:00:00
#SBATCH --job-name=BS45_v8_fir
#SBATCH --output=bs45_v8_fir_output_%A_%a.txt
#SBATCH --array=0-9
#SBATCH --account=def-ikotsire
#SBATCH --mail-type=END,FAIL

# === BS(45,44) v8 — Fir ===
# WORLD-RECORD target. Do not submit until BS(43,42) has been reproduced
# with the current binary. Seed offsets 45000-45009 (handoff range
# 45000-45049 — leaves room for 4 more 10-task waves).

cd $SLURM_SUBMIT_DIR

module load StdEnv/2023
module load gcc/12.3

export OMP_NUM_THREADS=$SLURM_CPUS_PER_TASK
SEED_OFFSET=$((45000 + SLURM_ARRAY_TASK_ID))
BIN=wz_sa_v8_bs45_${SLURM_ARRAY_TASK_ID}

echo "=============================================="
echo "  BS(45,44) v8 — Fir — WORLD RECORD ATTEMPT"
echo "  Job: $SLURM_ARRAY_JOB_ID  Task: $SLURM_ARRAY_TASK_ID"
echo "  Node: $(hostname)  Cores: $OMP_NUM_THREADS"
echo "  Seed Offset: $SEED_OFFSET"
echo "  Time: $(date)"
echo "=============================================="

g++ -O3 -march=native -std=c++17 -fopenmp -o $BIN src/solver/wz_sa_v8.cpp || exit 1

./$BIN 44 $SEED_OFFSET

rm -f $BIN
echo "=== Task $SLURM_ARRAY_TASK_ID finished at $(date) ==="
