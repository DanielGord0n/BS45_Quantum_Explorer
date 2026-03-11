#!/bin/bash
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=192
#SBATCH --time=24:00:00
#SBATCH --job-name=BS42_find
#SBATCH --output=bs42_output_%A_%a.txt
#SBATCH --array=0-19
#SBATCH --mail-type=END,FAIL

# === BS(42) Reproduction — 10 independent searches ===
# Each task: 192 cores with a unique seed.
# Total: 10 × 192 = 1,920 cores searching for BS(42).
#
# Submit with: sbatch bs42_job.sh

cd $SLURM_SUBMIT_DIR

module load StdEnv/2023
module load gcc/12.3

export OMP_NUM_THREADS=$SLURM_CPUS_PER_TASK

if [ ! -f wz_sa ]; then
    g++ -O3 -march=native -std=c++17 -fopenmp -o wz_sa src/solver/wz_sa_trillium.cpp
fi

echo "=============================================="
echo "  BS(42) Reproduction — Trillium"
echo "  Job: $SLURM_ARRAY_JOB_ID  Task: $SLURM_ARRAY_TASK_ID"
echo "  Node: $(hostname)  Cores: $OMP_NUM_THREADS"
echo "  Time: $(date)"
echo "=============================================="

# BS(42,41) = ./wz_sa 41
./wz_sa 41 $SLURM_ARRAY_TASK_ID

echo "=== Finished at $(date) ==="
