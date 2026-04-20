#!/bin/bash
#SBATCH --job-name=BS28_v2_test
#SBATCH --account=def-ikotsire
#SBATCH --time=01:00:00
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=192
#SBATCH --mem-per-cpu=256M
#SBATCH --output=bs28_v2_output_%A_%a.txt
#SBATCH --array=0-2
#SBATCH --mail-type=END,FAIL

# === BS(28) v2 Validation Test — Fir Cluster ===
# Seeds 1000-1002. If any task finds BS(28), the v2 pipeline is validated.

cd $SLURM_SUBMIT_DIR

module load StdEnv/2023
module load gcc/12.3

export OMP_NUM_THREADS=$SLURM_CPUS_PER_TASK

SEED_OFFSET=$((1000 + SLURM_ARRAY_TASK_ID))

echo "=============================================="
echo "  BS(28) v2 VALIDATION TEST — Fir"
echo "  Job: $SLURM_ARRAY_JOB_ID  Task: $SLURM_ARRAY_TASK_ID"
echo "  Node: $(hostname)  Cores: $OMP_NUM_THREADS"
echo "  Seed Offset: $SEED_OFFSET"
echo "  Time: $(date)"
echo "=============================================="

rm -f wz_sa_v2
echo "Compiling wz_sa_v2..."
g++ -O3 -march=native -std=c++17 -fopenmp -o wz_sa_v2 src/solver/wz_sa_v2.cpp
echo "Done."

./wz_sa_v2 27 $SEED_OFFSET

echo "=== Task $SLURM_ARRAY_TASK_ID finished at $(date) ==="
