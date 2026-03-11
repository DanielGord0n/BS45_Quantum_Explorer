#!/bin/bash
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=192
#SBATCH --time=24:00:00
#SBATCH --job-name=BS45_array
#SBATCH --output=bs45_output_%A_%a.txt
#SBATCH --array=0-49
#SBATCH --mail-type=END,FAIL

# === BS(45) Solver — Array Job (50 independent searches) ===
# Each array task gets a unique seed offset so it explores
# a completely different region of the search space.
# Total: 50 nodes × 192 cores = 9,600 cores searching simultaneously
#
# Submit with: sbatch bs45_array_job.sh

cd $SLURM_SUBMIT_DIR

# Load compiler
module load StdEnv/2023
module load gcc/12.3

# Use all 192 cores for OpenMP
export OMP_NUM_THREADS=$SLURM_CPUS_PER_TASK

echo "=============================================="
echo "  BS(45) Solver — Trillium Array Job"
echo "  Job ID: $SLURM_ARRAY_JOB_ID  Task: $SLURM_ARRAY_TASK_ID"
echo "  Node:   $(hostname)"
echo "  Cores:  $OMP_NUM_THREADS"
echo "  Seed:   $SLURM_ARRAY_TASK_ID"
echo "  Time:   $(date)"
echo "=============================================="

# Compile if binary doesn't exist
if [ ! -f wz_sa ]; then
    echo "Compiling..."
    g++ -O3 -march=native -std=c++17 -fopenmp -o wz_sa src/solver/wz_sa_trillium.cpp
    echo "Done."
fi

# Run solver with array task ID as seed offset
./wz_sa 44 $SLURM_ARRAY_TASK_ID

echo "=== Task $SLURM_ARRAY_TASK_ID finished at $(date) ==="
