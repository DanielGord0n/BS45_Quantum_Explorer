#!/bin/bash
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=192
#SBATCH --time=24:00:00
#SBATCH --job-name=BS_repro
#SBATCH --output=bs_repro_output_%A_%a.txt
#SBATCH --array=0-29
#SBATCH --mail-type=END,FAIL

# === BS(42), BS(43), BS(44) Reproduction — Array Job ===
# Reproduces known results from the Wang-Zhu (Chinese) paper.
#
# Array tasks 0-9:   Search for BS(42) (./wz_sa 41)
# Array tasks 10-19: Search for BS(43) (./wz_sa 42)
# Array tasks 20-29: Search for BS(44) (./wz_sa 43)
#
# Each task gets 192 cores and a unique seed offset.
# Total: 30 nodes × 192 cores = 5,760 cores
#
# Submit with: sbatch bs_repro_job.sh

cd $SLURM_SUBMIT_DIR

module load StdEnv/2023
module load gcc/12.3

export OMP_NUM_THREADS=$SLURM_CPUS_PER_TASK

# Compile if needed
if [ ! -f wz_sa ]; then
    g++ -O3 -march=native -std=c++17 -fopenmp -o wz_sa src/solver/wz_sa_trillium.cpp
fi

# Determine which BS(n) to search based on array task ID
TASK_ID=$SLURM_ARRAY_TASK_ID

if [ $TASK_ID -lt 10 ]; then
    N=41    # BS(42,41)
    BS_NAME="BS(42)"
    SEED=$TASK_ID
elif [ $TASK_ID -lt 20 ]; then
    N=42    # BS(43,42)
    BS_NAME="BS(43)"
    SEED=$((TASK_ID - 10))
else
    N=43    # BS(44,43)
    BS_NAME="BS(44)"
    SEED=$((TASK_ID - 20))
fi

echo "=============================================="
echo "  $BS_NAME Reproduction — Trillium"
echo "  Job ID: $SLURM_ARRAY_JOB_ID  Task: $TASK_ID"
echo "  Node:   $(hostname)"
echo "  Cores:  $OMP_NUM_THREADS"
echo "  N=$N  Seed=$SEED"
echo "  Time:   $(date)"
echo "=============================================="

./wz_sa $N $SEED

echo "=== Task $TASK_ID ($BS_NAME) finished at $(date) ==="
