#!/bin/bash
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=192
#SBATCH --time=24:00:00
#SBATCH --job-name=BS45_nibi
#SBATCH --output=bs45_nibi_output_%A_%a.txt
#SBATCH --array=0-49
#SBATCH --account=def-ikotsire
#SBATCH --mail-type=END,FAIL

# === BS(45) Solver — Nibi Cluster (50 jobs) ===
# Run 4: Seed offsets 10000-10049 (AB retry fix)
# Total: 50 nodes × 192 cores = 9,600 cores searching

cd $SLURM_SUBMIT_DIR

module load StdEnv/2023
module load gcc/12.3

export OMP_NUM_THREADS=$SLURM_CPUS_PER_TASK

SEED_OFFSET=$((47000 + SLURM_ARRAY_TASK_ID))

echo "=============================================="
echo "  BS(45) Solver — Nibi Cluster"
echo "  Job: $SLURM_ARRAY_JOB_ID  Task: $SLURM_ARRAY_TASK_ID"
echo "  Node: $(hostname)  Cores: $OMP_NUM_THREADS"
echo "  Seed Offset: $SEED_OFFSET"
echo "  Time: $(date)"
echo "=============================================="

# Force recompile to pick up bug fix
rm -f wz_sa
echo "Compiling with bug fix..."
g++ -O3 -march=native -std=c++17 -fopenmp -o wz_sa src/solver/wz_sa_trillium.cpp
echo "Done."

./wz_sa 44 $SEED_OFFSET

echo "=== Task $SLURM_ARRAY_TASK_ID finished at $(date) ==="
