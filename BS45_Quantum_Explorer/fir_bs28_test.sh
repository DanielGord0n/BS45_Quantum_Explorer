#!/bin/bash
#SBATCH --job-name=BS28_test
#SBATCH --account=def-ikotsire
#SBATCH --time=03:00:00
#SBATCH --nodes=1
#SBATCH --cpus-per-task=192
#SBATCH --mem-per-cpu=256M
#SBATCH --output=bs28_test_output.txt

module load StdEnv/2023 gcc/12.3

export OMP_NUM_THREADS=$SLURM_CPUS_PER_TASK

echo "=============================================="
echo "  BS(28) VALIDATION TEST"
echo "  If this fails, we have a fundamental bug."
echo "=============================================="

./wz_sa 27 99999
