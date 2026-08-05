#!/bin/bash
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=1
#SBATCH --mem=4G
#SBATCH --time=00:30:00
#SBATCH --job-name=PLACEV2
#SBATCH --output=place_v2_bench_%j.txt
#SBATCH --account=def-ikotsire

# fh_place V2 (branchless/vectorizable) A/B bench — lever priced 2026-08-05.
# PRE-REGISTERED RULE: x86 speedup >=15% => V2 default-on wave 15+ (bit-identical,
# NOT in CFGSIG, lanes resume unaffected); 5-15% => keep env-opt-in; <5% => drop.
cd $SLURM_SUBMIT_DIR
module load StdEnv/2023 gcc/12.3
BIN=pv2_${SLURM_JOB_ID}
g++ -O3 -march=native -std=c++17 -fopenmp -o "$BIN" src/solver/wz_match.cpp || exit 1
export WZ_FIRSTHIT=1 OMP_NUM_THREADS=1 WZ_FH_PROF_ORDER=0 WZ_FH_MAX_CAND=800 WZ_FH_AB_BUDGET=50000000
echo "=== V1 (production baseline), n=44 (1,7,8,8), 800 cands ==="
t0=$(date +%s); ./"$BIN" 44 1 7 8 8 > /dev/null 2>&1; t1=$(date +%s); echo "V1_wall=$((t1-t0))s"
echo "=== V2 (WZ_FH_PLACE_V2=1), same workload ==="
export WZ_FH_PLACE_V2=1
t0=$(date +%s); ./"$BIN" 44 1 7 8 8 > /dev/null 2>&1; t1=$(date +%s); echo "V2_wall=$((t1-t0))s"
echo "=== correctness cross-check: verdict counts must MATCH ==="
unset WZ_FH_PLACE_V2; ./"$BIN" 29 0 6 9 1 2>/dev/null | grep -E "backtracks_entered" | tail -1
WZ_FH_PLACE_V2=1 ./"$BIN" 29 0 6 9 1 2>/dev/null | grep -E "backtracks_entered" | tail -1
rm -f "$BIN"; echo "=== done $(date) ==="
