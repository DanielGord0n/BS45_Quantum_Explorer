#!/bin/bash
#SBATCH --gres=gpu:1
#SBATCH --cpus-per-task=4
#SBATCH --mem=16G
#SBATCH --time=00:30:00
#SBATCH --job-name=GPUSPIKE
#SBATCH --output=gpu_spike_output_%j.txt
#SBATCH --account=def-ikotsire

# GPU feasibility spike (lever 3, docs/n44_search_narrowing_research.md).
# Measures A,B-completer throughput: 1 CPU core vs 1 GPU, identical iterative
# DFS (validated verdict-exact vs production at n=19 + n=41, 2026-07-31).
# PRE-REGISTERED RULE: speedup >=300x => BUILD the production GPU completer;
# 30-300x => marginal (decide vs GPU-node availability); <30x => KILL, the
# record path stays CPU-grind + class triage + the Kotsireas collaboration.
cd $SLURM_SUBMIT_DIR
module load StdEnv/2023 cuda
nvidia-smi -L
BIN=fh_gpu_spike_${SLURM_JOB_ID}
nvcc -O3 -o "$BIN" src/solver/gpu/fh_gpu_spike.cu || exit 1
echo "=== primary: n=44 (1,7,8,8), budget 1e6, 20k candidates, cpu_sample 200 ==="
./"$BIN" 44 1 7 8 8 n44_cands_20k.txt 1000000 20000 200
echo "=== secondary: production budget 5e7, cpu_sample 50 ==="
./"$BIN" 44 1 7 8 8 n44_cands_20k.txt 50000000 20000 50
rm -f "$BIN"
echo "=== done $(date) ==="
