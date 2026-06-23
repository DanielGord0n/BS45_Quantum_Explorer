#!/bin/bash
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=192
#SBATCH --time=12:00:00
#SBATCH --job-name=WZ_MATCH
#SBATCH --output=wz_match_output_%A_%a.txt
#SBATCH --account=def-ikotsire
#SBATCH --mail-type=END,FAIL

# Hash-join "matching" base-sequence solver (wz_match) on a full 192-core node.
# Generate spectral+residue-filtered A,B (hash by autocorrelation) and C,D (look up
# the negating vector) — the Wang-Zhu architecture, parallelized with OpenMP.
# Target BS(n+1,n) at a fixed signature via env WZ_N / WZ_A WZ_B WZ_C WZ_D, e.g.:
#   sbatch --export=ALL,WZ_N=18,WZ_A=7,WZ_B=5,WZ_C=0,WZ_D=0 cluster_wz_match.sh   # milestone
#   sbatch --export=ALL,WZ_N=42,WZ_A=7,WZ_B=11,WZ_C=0,WZ_D=0 cluster_wz_match.sh  # BS(43,42) goal
# A hit prints "*** BS(n+1,n) FOUND ***" with A,B,C,D and an exact NPAF==0 VERIFY line.

cd $SLURM_SUBMIT_DIR
module load StdEnv/2023
module load gcc/12.3
export OMP_NUM_THREADS=$SLURM_CPUS_PER_TASK

N=${WZ_N:-18}; A=${WZ_A:-7}; B=${WZ_B:-5}; C=${WZ_C:-0}; D=${WZ_D:-0}
BIN=wz_match_bin_${SLURM_JOB_ID}

echo "=== BS($((N+1)),$N) sig ($A,$B,$C,$D) hash-join — $OMP_NUM_THREADS threads — node $(hostname) — $(date) ==="
g++ -O3 -march=native -std=c++17 -fopenmp -o "$BIN" src/solver/wz_match.cpp || exit 1
./"$BIN" "$N" "$A" "$B" "$C" "$D"
rm -f "$BIN"
echo "=== done $(date) ==="
