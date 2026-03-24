#!/bin/bash
# === Multi-Cluster Deployment Script ===
# Deploys BS solver to Nibi, Fir, and Rorqual clusters
# Run this script manually and authenticate each SSH session with Duo
#
# Cluster assignments:
#   Nibi (UWaterloo)  → BS(45) search, seeds 100-119
#   Fir (SFU)         → BS(45) search, seeds 200-219
#   Rorqual (McGill)  → BS(43) search, seeds 300-319
#   Trillium (UofT)   → BS(45) search, seeds 0-49 (already submitted)

set -e

USERNAME="dangord"
SRC_DIR="$(cd "$(dirname "$0")" && pwd)"

# Files to copy to each cluster
FILES=(
    "src/solver/wz_sa_trillium.cpp"
)

deploy_cluster() {
    local CLUSTER=$1
    local HOST=$2
    local JOB_SCRIPT=$3
    local TARGET="BS45"
    
    if [[ "$JOB_SCRIPT" == *"bs43"* ]]; then
        TARGET="BS43"
    fi
    
    echo ""
    echo "=========================================="
    echo "  Deploying $TARGET to $CLUSTER"
    echo "  Host: $HOST"
    echo "=========================================="
    
    # Create remote directory
    echo "[1/4] Creating remote directory..."
    ssh ${USERNAME}@${HOST} "mkdir -p \$SCRATCH/bs45/src/solver"
    
    # Copy source code
    echo "[2/4] Copying source code..."
    scp "$SRC_DIR/src/solver/wz_sa_trillium.cpp" ${USERNAME}@${HOST}:"\$SCRATCH/bs45/src/solver/"
    
    # Copy job script
    echo "[3/4] Copying job script..."
    scp "$SRC_DIR/$JOB_SCRIPT" ${USERNAME}@${HOST}:"\$SCRATCH/bs45/"
    
    # Submit job
    echo "[4/4] Compiling and submitting job..."
    ssh ${USERNAME}@${HOST} "cd \$SCRATCH/bs45 && module load StdEnv/2023 gcc/12.3 && g++ -O3 -march=native -std=c++17 -fopenmp -o wz_sa src/solver/wz_sa_trillium.cpp && sbatch $JOB_SCRIPT && squeue -u \$USER"
    
    echo "=== $CLUSTER deployment complete ==="
}

echo "=== Multi-Cluster BS Solver Deployment ==="
echo "This script will deploy to 3 clusters."
echo "You will need to authenticate with Duo for EACH SSH connection."
echo ""

# Deploy to each cluster one at a time
deploy_cluster "Nibi" "nibi.alliancecan.ca" "nibi_bs45_job.sh"
deploy_cluster "Fir" "fir.alliancecan.ca" "fir_bs45_job.sh"
deploy_cluster "Rorqual" "rorqual.alliancecan.ca" "rorqual_bs43_job.sh"

echo ""
echo "=== ALL DEPLOYMENTS COMPLETE ==="
echo "Monitoring commands:"
echo "  Nibi:    ssh ${USERNAME}@nibi.alliancecan.ca 'squeue -u \$USER'"
echo "  Fir:     ssh ${USERNAME}@fir.alliancecan.ca 'squeue -u \$USER'"
echo "  Rorqual: ssh ${USERNAME}@rorqual.alliancecan.ca 'squeue -u \$USER'"
