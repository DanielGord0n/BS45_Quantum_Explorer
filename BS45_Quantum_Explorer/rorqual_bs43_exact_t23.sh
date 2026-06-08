#!/bin/bash
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=192
#SBATCH --time=24:00:00
#SBATCH --job-name=BS43_t23_rorqual
#SBATCH --output=bs43_t23_rorqual_output_%A_%a.txt
#SBATCH --array=0-9
#SBATCH --account=def-ikotsire
#SBATCH --mail-type=END,FAIL

# === BS(43,42) EXHAUSTIVE wz_exact_t23 (Thm 2.3 prune) — Rorqual ===
# Sig-targeted (7,11,0,0): Wang-Zhu BS(43,42) signature.
# Rorqual searches combo quarter [8388608,16777216) of the 33554432 first-4-layer combos (WZ_SPLIT=4).

cd $SLURM_SUBMIT_DIR
module load StdEnv/2023
module load gcc/12.3
export OMP_NUM_THREADS=$SLURM_CPUS_PER_TASK
export WZ_SPLIT=4   # Optimization D: 4-layer split (33.5M combos) for load balancing

N=42
SIG_A=7
SIG_B=11
SIG_C=0
SIG_D=0

CLUSTER_LO=8388608
CLUSTER_HI=16777216
NTASKS=10
SPAN=$(( (CLUSTER_HI - CLUSTER_LO + NTASKS - 1) / NTASKS ))
TASK_LO=$(( CLUSTER_LO + SLURM_ARRAY_TASK_ID * SPAN ))
TASK_HI=$(( TASK_LO + SPAN ))
if [ $TASK_HI -gt $CLUSTER_HI ]; then TASK_HI=$CLUSTER_HI; fi
# Checkpoint-resume (replaces the old WAVE hack): each task does its FULL slice and
# resumes from a per-task checkpoint, so every resubmit ADVANCES instead of redoing
# the same prefix. The solver writes the watermark to WZ_CKPT every ~30s.
LO=$TASK_LO
HI=$TASK_HI
export WZ_CKPT=$SCRATCH/bs45/ckpt_rorqual_${SLURM_ARRAY_TASK_ID}.txt

BIN=wz_exact_t23_${SLURM_ARRAY_TASK_ID}

echo "=============================================="
echo "  BS(43,42) wz_exact_t23 — Rorqual"
echo "  Sig: ($SIG_A,$SIG_B,$SIG_C,$SIG_D)"
echo "  Job: $SLURM_ARRAY_JOB_ID  Task: $SLURM_ARRAY_TASK_ID"
echo "  Combo range: [$LO,$HI)   Node: $(hostname)"
echo "  Time: $(date)"
echo "=============================================="

g++ -O3 -march=native -std=c++17 -fopenmp -o $BIN src/solver/wz_exact_t23.cpp || exit 1
./$BIN $N $SIG_A $SIG_B $SIG_C $SIG_D $LO $HI
rm -f $BIN
echo "=== Task $SLURM_ARRAY_TASK_ID finished at $(date) ==="
