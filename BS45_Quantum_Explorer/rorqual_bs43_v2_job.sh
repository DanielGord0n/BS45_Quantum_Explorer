#!/bin/bash
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=192
#SBATCH --time=24:00:00
#SBATCH --job-name=BS43_v2_rorqual
#SBATCH --output=bs43_v2_rorqual_output_%A_%a.txt
#SBATCH --array=0-19
#SBATCH --account=def-ikotsire
#SBATCH --mail-type=END,FAIL

# === BS(43) v2 Solver — Rorqual (20 jobs, reproduction of known result) ===
# Seed offsets 4000-4019

cd $SLURM_SUBMIT_DIR

module load StdEnv/2023
module load gcc/12.3

export OMP_NUM_THREADS=$SLURM_CPUS_PER_TASK

SEED_OFFSET=$((4000 + SLURM_ARRAY_TASK_ID))

echo "=============================================="
echo "  BS(43) v2 Solver — Rorqual"
echo "  Job: $SLURM_ARRAY_JOB_ID  Task: $SLURM_ARRAY_TASK_ID"
echo "  Node: $(hostname)  Cores: $OMP_NUM_THREADS"
echo "  Seed Offset: $SEED_OFFSET"
echo "  Time: $(date)"
echo "=============================================="

rm -f wz_sa_v2
echo "Compiling wz_sa_v2..."
g++ -O3 -march=native -std=c++17 -fopenmp -o wz_sa_v2 src/solver/wz_sa_v2.cpp
echo "Done."

# BS(43,42) = ./wz_sa_v2 42
./wz_sa_v2 42 $SEED_OFFSET

echo "=== Task $SLURM_ARRAY_TASK_ID finished at $(date) ==="
