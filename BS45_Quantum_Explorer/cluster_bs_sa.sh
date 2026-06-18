#!/bin/bash
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=192
#SBATCH --time=12:00:00
#SBATCH --job-name=BS_SA
#SBATCH --output=bs_sa_output_%A_%a.txt
#SBATCH --array=0-19
#SBATCH --account=def-ikotsire
#SBATCH --mail-type=END,FAIL

# Simulated-annealing search for base sequences BS(n+1,n) with wz_sa_v8 — the
# HEURISTIC solver that originally found BS(28,27). Heuristic search is better at
# FINDING than the exhaustive solver (which only blind-reaches ~n=18). Each array
# task is an independent 192-core SA run with a distinct seed; 20 tasks = 20
# parallel attempts on this cluster = far better odds of hitting a solution.
#
# Target length via env WZ_N; seeds start at SEED_BASE. Deploy example:
#   sbatch --export=ALL,WZ_N=33,SEED_BASE=1000 cluster_bs_sa.sh
# A hit prints "*** REPRODUCTION CONFIRMED: BS(N+1,N) FOUND ***" with A,B,C,D.

cd $SLURM_SUBMIT_DIR
module load StdEnv/2023
module load gcc/12.3
export OMP_NUM_THREADS=$SLURM_CPUS_PER_TASK

N=${WZ_N:-33}
SEED=$(( ${SEED_BASE:-1000} + SLURM_ARRAY_TASK_ID ))
BIN=wz_sa_v8_${SLURM_ARRAY_TASK_ID}

echo "=== BS($((N+1)),$N) SA  task=$SLURM_ARRAY_TASK_ID  seed=$SEED  node=$(hostname)  $(date) ==="
g++ -O3 -march=native -std=c++17 -fopenmp -o "$BIN" src/solver/wz_sa_v8.cpp || exit 1
./"$BIN" "$N" "$SEED"
rm -f "$BIN"
echo "=== task $SLURM_ARRAY_TASK_ID done $(date) ==="
