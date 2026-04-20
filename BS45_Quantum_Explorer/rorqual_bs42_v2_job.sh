#!/bin/bash
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=192
#SBATCH --time=24:00:00
#SBATCH --job-name=BS42_v2_rorqual
#SBATCH --output=bs42_v2_rorqual_output_%A_%a.txt
#SBATCH --array=0-49
#SBATCH --account=def-ikotsire
#SBATCH --mail-type=END,FAIL

# === BS(42) v2 Solver — Rorqual (50 jobs, reproduction) ===
# Seed offsets 4100-4149

cd $SLURM_SUBMIT_DIR

module load StdEnv/2023
module load gcc/12.3

export OMP_NUM_THREADS=$SLURM_CPUS_PER_TASK

SEED_OFFSET=$((4100 + SLURM_ARRAY_TASK_ID))

echo "=============================================="
echo "  BS(42) v2 Solver — Rorqual"
echo "  Job: $SLURM_ARRAY_JOB_ID  Task: $SLURM_ARRAY_TASK_ID"
echo "  Node: $(hostname)  Cores: $OMP_NUM_THREADS"
echo "  Seed Offset: $SEED_OFFSET"
echo "  Time: $(date)"
echo "=============================================="

rm -f wz_sa_v2
echo "Compiling wz_sa_v2..."
g++ -O3 -march=native -std=c++17 -fopenmp -o wz_sa_v2 src/solver/wz_sa_v2.cpp
echo "Done."

# BS(42,41) = ./wz_sa_v2 41
./wz_sa_v2 41 $SEED_OFFSET

echo "=== Task $SLURM_ARRAY_TASK_ID finished at $(date) ==="
