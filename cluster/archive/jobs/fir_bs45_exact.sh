#!/bin/bash
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=192
#SBATCH --time=24:00:00
#SBATCH --job-name=BS45_exact_fir
#SBATCH --output=bs45_exact_fir_output_%A_%a.txt
#SBATCH --array=0-9
#SBATCH --account=def-ikotsire
#SBATCH --mail-type=END,FAIL

# === BS(45,44) EXHAUSTIVE backtracking (wz_exact) — Fir — WORLD RECORD ===
# DO NOT submit until wz_exact has reproduced BS(43,42). n=44.
# Fir searches combo quarter [0,2048) of the 8192 first-two-layer combos.

cd $SLURM_SUBMIT_DIR
module load StdEnv/2023
module load gcc/12.3
export OMP_NUM_THREADS=$SLURM_CPUS_PER_TASK

N=44
CLUSTER_LO=0
CLUSTER_HI=2048
NTASKS=10
SPAN=$(( (CLUSTER_HI - CLUSTER_LO + NTASKS - 1) / NTASKS ))
LO=$(( CLUSTER_LO + SLURM_ARRAY_TASK_ID * SPAN ))
HI=$(( LO + SPAN ))
if [ $HI -gt $CLUSTER_HI ]; then HI=$CLUSTER_HI; fi

BIN=wz_exact_${SLURM_ARRAY_TASK_ID}

echo "=============================================="
echo "  BS(45,44) EXHAUSTIVE — Fir — WORLD RECORD ATTEMPT"
echo "  Job: $SLURM_ARRAY_JOB_ID  Task: $SLURM_ARRAY_TASK_ID"
echo "  Combo range: [$LO,$HI)   Node: $(hostname)"
echo "  Time: $(date)"
echo "=============================================="

g++ -O3 -march=native -std=c++17 -fopenmp -o $BIN src/solver/wz_exact.cpp || exit 1
./$BIN $N $LO $HI
rm -f $BIN
echo "=== Task $SLURM_ARRAY_TASK_ID finished at $(date) ==="
