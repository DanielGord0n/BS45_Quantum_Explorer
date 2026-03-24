#!/bin/bash
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=192
#SBATCH --time=24:00:00
#SBATCH --job-name=BS42_rorqual
#SBATCH --output=bs42_rorqual_output_%A_%a.txt
#SBATCH --array=0-19
#SBATCH --account=def-ikotsire
#SBATCH --mail-type=END,FAIL

# === BS(42) Solver — Rorqual Cluster (20 jobs) ===
# Seed offsets 300-319. Dedicated BS(42) reproduction.
# Total: 20 nodes × 192 cores = 3,840 cores searching

cd $SLURM_SUBMIT_DIR

module load StdEnv/2023
module load gcc/12.3

export OMP_NUM_THREADS=$SLURM_CPUS_PER_TASK

SEED_OFFSET=$((300 + SLURM_ARRAY_TASK_ID))

echo "=============================================="
echo "  BS(42) Solver — Rorqual Cluster"
echo "  Job: $SLURM_ARRAY_JOB_ID  Task: $SLURM_ARRAY_TASK_ID"
echo "  Node: $(hostname)  Cores: $OMP_NUM_THREADS"
echo "  Seed Offset: $SEED_OFFSET"
echo "  Time: $(date)"
echo "=============================================="

if [ ! -f wz_sa ]; then
    echo "Compiling..."
    g++ -O3 -march=native -std=c++17 -fopenmp -o wz_sa src/solver/wz_sa_trillium.cpp
    echo "Done."
fi

# BS(42,41) = ./wz_sa 41
./wz_sa 41 $SEED_OFFSET

echo "=== Task $SLURM_ARRAY_TASK_ID finished at $(date) ==="
