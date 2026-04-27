#!/bin/bash
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=192
#SBATCH --time=12:00:00
#SBATCH --job-name=BS43_v3_fir
#SBATCH --output=bs43_v3_fir_output_%A_%a.txt
#SBATCH --array=0-19
#SBATCH --account=def-ikotsire
#SBATCH --mail-type=END,FAIL

# === BS(43,42) v2 reproduction — Fir ===
# v2 uses Wang-Zhu pair encoding, known to contain the BS(43,42) solution.
# Seed offsets 5000-5019 (disjoint across clusters).

cd $SLURM_SUBMIT_DIR

module load StdEnv/2023
module load gcc/12.3

export OMP_NUM_THREADS=$SLURM_CPUS_PER_TASK
SEED_OFFSET=$((5000 + SLURM_ARRAY_TASK_ID))
BIN=wz_sa_v3_${SLURM_ARRAY_TASK_ID}

echo "=============================================="
echo "  BS(43,42) v3 — Fir"
echo "  Job: $SLURM_ARRAY_JOB_ID  Task: $SLURM_ARRAY_TASK_ID"
echo "  Node: $(hostname)  Cores: $OMP_NUM_THREADS"
echo "  Seed Offset: $SEED_OFFSET"
echo "  Time: $(date)"
echo "=============================================="

g++ -O3 -march=native -std=c++17 -fopenmp -o $BIN src/solver/wz_sa_v3.cpp || exit 1

./$BIN 42 $SEED_OFFSET

rm -f $BIN
echo "=== Task $SLURM_ARRAY_TASK_ID finished at $(date) ==="
