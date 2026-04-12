#!/bin/bash
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=192
#SBATCH --time=24:00:00
#SBATCH --job-name=BS45_array
#SBATCH --output=bs45_output_%A_%a.txt
#SBATCH --array=0-49
#SBATCH --account=def-ikotsire
#SBATCH --mail-type=END,FAIL

# === BS(45) Solver — Trillium Array Job (Run 4: AB retry fix) ===
# Each array task gets a unique seed offset so it explores
# a completely different region of the search space.
# Run 4: Seed offsets 12000-12049
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

# Force recompile to pick up bug fix
rm -f wz_sa
echo "Compiling with bug fix..."
g++ -O3 -march=native -std=c++17 -fopenmp -o wz_sa src/solver/wz_sa_trillium.cpp
echo "Done."

SEED_OFFSET=$((24000 + SLURM_ARRAY_TASK_ID))

# Run solver with unique seed offset
./wz_sa 44 $SEED_OFFSET

echo "=== Task $SLURM_ARRAY_TASK_ID finished at $(date) ==="
