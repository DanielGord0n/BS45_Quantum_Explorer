#!/bin/bash
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=192
#SBATCH --time=24:00:00
#SBATCH --job-name=BS45_solver
#SBATCH --output=bs45_output_%j.txt
#SBATCH --mail-type=END,FAIL

# === BS(45) Solver — Trillium Job Script ===
# Submit from $SCRATCH/bs45 with: sbatch bs45_job.sh

cd $SLURM_SUBMIT_DIR

# Load compiler
module load StdEnv/2023
module load gcc/12.3

# Use all 192 cores for OpenMP
export OMP_NUM_THREADS=$SLURM_CPUS_PER_TASK

echo "=============================================="
echo "  BS(45) Solver — Trillium Supercomputer"
echo "  Job ID: $SLURM_JOB_ID"
echo "  Node:   $(hostname)"
echo "  Cores:  $OMP_NUM_THREADS"
echo "  Time:   $(date)"
echo "=============================================="

# Compile (in case binary isn't available)
if [ ! -f wz_sa ]; then
    echo "Binary not found, compiling..."
    g++ -O3 -march=native -std=c++17 -fopenmp -o wz_sa src/solver/wz_sa_trillium.cpp
    echo "Build complete."
fi

# Run the solver for BS(45,44)
./wz_sa 44

echo "=== Job finished at $(date) ==="
