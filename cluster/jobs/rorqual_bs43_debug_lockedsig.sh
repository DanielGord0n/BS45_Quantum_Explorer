#!/bin/bash
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=192
#SBATCH --time=24:00:00
#SBATCH --job-name=BS43_v8_lockSig
#SBATCH --output=bs43_v8_lockedsig_output_%A_%a.txt
#SBATCH --array=0-2
#SBATCH --account=def-ikotsire
#SBATCH --mail-type=END,FAIL

# === BS(43,42) DIAGNOSTIC: signature-locked single-sig run ===
# Locks all 192 cores to the known-good Wang-Zhu signature (7,11,0,0)
# instead of the usual uniform-random sig pick. If the solver finds
# BS(43,42) under this constraint, the SA itself works given the right
# sig and our only remaining problem is finding the right sig in a large
# enumeration. If it CAN'T find it even with the right sig, then SA is
# the bottleneck and we need a different algorithm (CP/SAT, hybrid, etc).
#
# Seed offsets 80000-80002 — far from any real campaign range.

cd $SLURM_SUBMIT_DIR

module load StdEnv/2023
module load gcc/12.3

export OMP_NUM_THREADS=$SLURM_CPUS_PER_TASK
SEED_OFFSET=$((80000 + SLURM_ARRAY_TASK_ID))
BIN=wz_sa_v8_locksig_${SLURM_ARRAY_TASK_ID}

echo "=============================================="
echo "  BS(43,42) DIAGNOSTIC — Rorqual"
echo "  LOCKED to Wang-Zhu sig (7,11,0,0)"
echo "  Job: $SLURM_ARRAY_JOB_ID  Task: $SLURM_ARRAY_TASK_ID"
echo "  Node: $(hostname)  Cores: $OMP_NUM_THREADS"
echo "  Seed Offset: $SEED_OFFSET"
echo "  Time: $(date)"
echo "=============================================="

g++ -O3 -march=native -std=c++17 -fopenmp -o $BIN src/solver/wz_sa_v8.cpp || exit 1

./$BIN 42 $SEED_OFFSET 7,11,0,0

rm -f $BIN
echo "=== Task $SLURM_ARRAY_TASK_ID finished at $(date) ==="
