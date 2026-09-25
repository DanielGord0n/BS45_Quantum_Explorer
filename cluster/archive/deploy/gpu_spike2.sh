#!/bin/bash
#SBATCH --gres=gpu:h100:1
#SBATCH --cpus-per-task=4
#SBATCH --mem=16G
#SBATCH --time=00:30:00
#SBATCH --job-name=GPUSPIKE2
#SBATCH --output=gpu_spike2_output_%j.txt
#SBATCH --account=def-ikotsire

# GPU spike v2 (2026-08-06): the LAST unpriced lever. Two divergence-tolerant
# variants vs the naive kernel, all at PRODUCTION budget 5e7:
#   V2A = naive kernel + host-side flatness sort (warps retire together)
#   V2B = warp-cooperative kernel (one candidate per warp, lane-parallel node work)
# PRE-REGISTERED RULE (vs 1 CPU core, production budget): >=200x => production
# build justified (>=1 CPU-node-equivalent per GPU); 60-200x => marginal, decide
# vs idle GPU quota; <60x => GPU lever closed permanently.
cd $SLURM_SUBMIT_DIR
module load StdEnv/2023 cuda
nvidia-smi -L
BIN=fh_gpu_spike2_${SLURM_JOB_ID}
nvcc -O3 -o "$BIN" src/solver/gpu/fh_gpu_spike.cu || exit 1
./"$BIN" 44 1 7 8 8 n44_cands_20k.txt 50000000 6000 50
rm -f "$BIN"
echo "=== done $(date) ==="
