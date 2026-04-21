#!/bin/bash
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=192
#SBATCH --time=4:00:00
#SBATCH --job-name=BS28_v3_fir
#SBATCH --output=bs28_v3_fir_output_%A_%a.txt
#SBATCH --array=0-9
#SBATCH --account=def-ikotsire
#SBATCH --mail-type=END,FAIL

# === BS(28,27) v3 validation — Fir ===
# v3 uses the full unconstrained manifold — proven to solve BS(12,11) locally
# in under a minute on 4 threads.  Expect BS(28,27) within minutes on 192 cores.
# Seed offsets 6000-6009.

cd $SLURM_SUBMIT_DIR

module load StdEnv/2023
module load gcc/12.3

export OMP_NUM_THREADS=$SLURM_CPUS_PER_TASK
SEED_OFFSET=$((6000 + SLURM_ARRAY_TASK_ID))
BIN=wz_sa_v3_${SLURM_ARRAY_TASK_ID}

echo "=============================================="
echo "  BS(28,27) v3 — Fir"
echo "  Job: $SLURM_ARRAY_JOB_ID  Task: $SLURM_ARRAY_TASK_ID"
echo "  Node: $(hostname)  Cores: $OMP_NUM_THREADS"
echo "  Seed Offset: $SEED_OFFSET"
echo "  Time: $(date)"
echo "=============================================="

g++ -O3 -march=native -std=c++17 -fopenmp -o $BIN src/solver/wz_sa_v3.cpp || exit 1

./$BIN 27 $SEED_OFFSET

rm -f $BIN
echo "=== Task $SLURM_ARRAY_TASK_ID finished at $(date) ==="
