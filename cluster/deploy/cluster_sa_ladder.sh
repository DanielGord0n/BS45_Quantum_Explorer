#!/bin/bash
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=192
#SBATCH --time=12:00:00
#SBATCH --job-name=SA_LADDER
#SBATCH --output=sa_ladder_%A_%a.txt
#SBATCH --account=def-ikotsire
#SBATCH --array=0-7
#SBATCH --mail-type=END,FAIL

# ───────────────────────────────────────────────────────────────────────────
# Massively-parallel simulated-annealing "ladder" for base sequences BS(n+1,n).
#
# WHY THIS (not the hash-join wz_match): metaheuristic search finds ONE solution
# without materializing the candidate set, so memory is O(n) per chain — it never
# OOMs. wz_sa_v8 already found + Kotsireas-verified BS(28,27); this scales it.
#
# Each array task = one full 192-thread node of independent SA chains that share
# per-signature champions. 8 array tasks (--array=0-7) with distinct RNG seed
# bases ≈ 8 × 192 ≈ 1536 independent chains per cluster at the target n.
#
# Target n via env WZ_N; optional locked signature via WZ_SIG="a,b,c,d".
#   sbatch --export=ALL,WZ_N=30 cluster_sa_ladder.sh   # blind BS(31,30)
#   sbatch --export=ALL,WZ_N=32 cluster_sa_ladder.sh   # blind BS(33,32)
#   sbatch --export=ALL,WZ_N=33 cluster_sa_ladder.sh   # blind BS(34,33) (stretch)
#
# A hit prints "*** REPRODUCTION CONFIRMED: BS(n+1,n) FOUND ***" with A,B,C,D,
# the Time/Seed, and an NPAF==0 banner. Cancel the rest of the array on a hit:
#   scancel <jobid>           # cancels all remaining array tasks
# ───────────────────────────────────────────────────────────────────────────

cd $SLURM_SUBMIT_DIR
module load StdEnv/2023
module load gcc/12.3
export OMP_NUM_THREADS=$SLURM_CPUS_PER_TASK

N=${WZ_N:-30}
# Distinct seed base per array task (stride ≫ 192 threads ⇒ no chain overlap).
SEED=$(( ${WZ_SEED_BASE:-1000} + SLURM_ARRAY_TASK_ID * 100000 ))
SIG=${WZ_SIG:-}
BIN=wz_sa_v8_bin_${SLURM_JOB_ID}_${SLURM_ARRAY_TASK_ID}

# Robust to deploy layout: src tree (git checkout) OR flat scp into $SCRATCH/bs45.
SRC=src/solver/wz_sa_v8.cpp; [ -f "$SRC" ] || SRC=wz_sa_v8.cpp
echo "=== BS($((N+1)),$N) SA-ladder — node $(hostname) — task ${SLURM_ARRAY_TASK_ID} seed ${SEED} — ${OMP_NUM_THREADS} threads — src ${SRC} — $(date) ==="
g++ -O3 -march=native -std=c++17 -fopenmp -o "$BIN" "$SRC" || exit 1
if [ -n "$SIG" ]; then
  ./"$BIN" "$N" "$SEED" "$SIG"
else
  ./"$BIN" "$N" "$SEED"
fi
rm -f "$BIN"
echo "=== done task ${SLURM_ARRAY_TASK_ID} $(date) ==="
