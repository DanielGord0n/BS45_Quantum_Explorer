#!/bin/bash
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=192
#SBATCH --time=12:00:00
#SBATCH --job-name=BS43_v3_nibi
#SBATCH --output=bs43_v3_nibi_output_%A_%a.txt
#SBATCH --array=0-19
#SBATCH --account=def-ikotsire_cpu
#SBATCH --mail-type=END,FAIL

# === BS(43,42) v2 reproduction — Nibi ===
# Seed offsets 5200-5219.

cd $SLURM_SUBMIT_DIR

module load StdEnv/2023
module load gcc/12.3

export TMPDIR=$SCRATCH/tmp
mkdir -p $TMPDIR

export OMP_NUM_THREADS=$SLURM_CPUS_PER_TASK
SEED_OFFSET=$((5200 + SLURM_ARRAY_TASK_ID))
BIN=wz_sa_v3_${SLURM_ARRAY_TASK_ID}

echo "=============================================="
echo "  BS(43,42) v3 — Nibi"
echo "  Job: $SLURM_ARRAY_JOB_ID  Task: $SLURM_ARRAY_TASK_ID"
echo "  Node: $(hostname)  Cores: $OMP_NUM_THREADS"
echo "  Seed Offset: $SEED_OFFSET"
echo "  Time: $(date)"
echo "=============================================="

g++ -O3 -march=native -std=c++17 -fopenmp -o $BIN src/solver/wz_sa_v3.cpp || exit 1

./$BIN 42 $SEED_OFFSET

rm -f $BIN
echo "=== Task $SLURM_ARRAY_TASK_ID finished at $(date) ==="
