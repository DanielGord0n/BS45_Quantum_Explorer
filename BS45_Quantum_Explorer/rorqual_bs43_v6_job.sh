#!/bin/bash
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=192
#SBATCH --time=24:00:00
#SBATCH --job-name=BS43_v6_rorqual
#SBATCH --output=bs43_v6_rorqual_output_%A_%a.txt
#SBATCH --array=0-9
#SBATCH --account=def-ikotsire
#SBATCH --mail-type=END,FAIL

# === BS(43,42) v6 — Rorqual ===
# Seed offsets 10100-10109.

cd $SLURM_SUBMIT_DIR

module load StdEnv/2023
module load gcc/12.3

export OMP_NUM_THREADS=$SLURM_CPUS_PER_TASK
SEED_OFFSET=$((10100 + SLURM_ARRAY_TASK_ID))
BIN=wz_sa_v6_${SLURM_ARRAY_TASK_ID}

echo "=============================================="
echo "  BS(43,42) v6 — Rorqual"
echo "  Job: $SLURM_ARRAY_JOB_ID  Task: $SLURM_ARRAY_TASK_ID"
echo "  Node: $(hostname)  Cores: $OMP_NUM_THREADS"
echo "  Seed Offset: $SEED_OFFSET"
echo "  Time: $(date)"
echo "=============================================="

g++ -O3 -march=native -std=c++17 -fopenmp -o $BIN src/solver/wz_sa_v6.cpp || exit 1

./$BIN 42 $SEED_OFFSET

rm -f $BIN
echo "=== Task $SLURM_ARRAY_TASK_ID finished at $(date) ==="
