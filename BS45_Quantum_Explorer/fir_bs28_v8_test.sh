#!/bin/bash
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=192
#SBATCH --time=02:00:00
#SBATCH --job-name=BS28_v8_fir
#SBATCH --output=bs28_v8_fir_output_%A_%a.txt
#SBATCH --array=0-2
#SBATCH --account=def-ikotsire
#SBATCH --mail-type=END,FAIL

# === BS(28,27) v8 regression test — Fir ===
# Validates that the phased SA algorithm in v8 actually works on a small
# known-solvable case before burning more compute on BS(43).
# 3 tasks × 1h, seed offsets 28000-28002.

cd $SLURM_SUBMIT_DIR

module load StdEnv/2023
module load gcc/12.3

export OMP_NUM_THREADS=$SLURM_CPUS_PER_TASK
SEED_OFFSET=$((28600 + SLURM_ARRAY_TASK_ID))
BIN=wz_sa_v8_bs28_${SLURM_ARRAY_TASK_ID}

echo "=============================================="
echo "  BS(28,27) v8 regression — Fir"
echo "  Job: $SLURM_ARRAY_JOB_ID  Task: $SLURM_ARRAY_TASK_ID"
echo "  Node: $(hostname)  Cores: $OMP_NUM_THREADS"
echo "  Seed Offset: $SEED_OFFSET"
echo "  Time: $(date)"
echo "=============================================="

g++ -O3 -march=native -std=c++17 -fopenmp -o $BIN src/solver/wz_sa_v8.cpp || exit 1

./$BIN 27 $SEED_OFFSET

rm -f $BIN
echo "=== Task $SLURM_ARRAY_TASK_ID finished at $(date) ==="
