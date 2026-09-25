#!/bin/bash
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=192
#SBATCH --time=12:00:00
#SBATCH --job-name=BS43_repro294887
#SBATCH --output=bs43_t23_repro294887_output_%j.txt
#SBATCH --account=def-ikotsire
#SBATCH --mail-type=END,FAIL

# === Targeted BS(43,42) reproduction ===
# Combo 294887 holds the published Wang-Zhu BS(43,42) solution (verified by
# find_combo_index.py: all 21 layers fit the encoding, sig (7,11,0,0)).
# We search a 192-combo window starting at 294887 so every core is busy (gives
# a clean v5 rate reading) while the core on 294887 grinds the known solution.
# Any combo in the window that hits NPAF=0 is an equally valid reproduction.

cd $SLURM_SUBMIT_DIR
module load StdEnv/2023
module load gcc/12.3
export OMP_NUM_THREADS=$SLURM_CPUS_PER_TASK

echo "=============================================="
echo "  BS(43,42) targeted reproduction — combo 294887 (v5/opt-C)"
echo "  Window: [294887, 295079)   Node: $(hostname)   $(date)"
echo "=============================================="

g++ -O3 -march=native -std=c++17 -fopenmp -o wz_exact_t23_repro src/solver/wz_exact_t23.cpp || exit 1
./wz_exact_t23_repro 42 7 11 0 0 294887 295079
rm -f wz_exact_t23_repro
echo "=== repro294887 finished at $(date) ==="
