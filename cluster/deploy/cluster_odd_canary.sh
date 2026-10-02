#!/bin/bash
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=2
#SBATCH --mem=16G
#SBATCH --time=24:00:00
#SBATCH --job-name=ODDCANARY
#SBATCH --output=oddcanary_%j.txt
#SBATCH --account=def-ikotsire
#
# Odd-n pipeline control for the n=45 launch (2026-10-02). Run from an ISOLATED dir
# ($SCRATCH/bs45_oddcanary), never the production tree. The EXACT n=45 stream settings
# (WZ_FH_ENDPOS=1 positive endpoint quad, orbit canon + Q + closure prune, ORDER=3, 177 arms,
# budget 2e6, A1 K=3 default) must re-find BOTH known BS(44,43) (ours-43 and WZ-43, class
# (2,8,5,9)): step 0 LOCATEs each solution's kept cell IN THIS JOB (cell indices are not portable
# across toolchains), step 1 streams that cell to the image and completes it (TARGET_COMPLETE).
# PRE-REGISTERED: PASS = both print "BS(44,43) FOUND" with VERIFY NPAF==0 (then verify_npaf.py
# locally); FAIL = LOCATE says the orbit is not kept, or TARGET_COMPLETE reached but no FOUND
# (do NOT launch n=45 on this pipeline); INCONCLUSIVE = image not reached in 24 h.
cd "$SLURM_SUBMIT_DIR"
module load StdEnv/2023
module load gcc/12.3
BIN=oc_bin_${SLURM_JOB_ID}
g++ -O3 -march=native -std=c++17 -fopenmp -o "$BIN" src/solver/wz_match.cpp || exit 1
echo "=== ODDCANARY job $SLURM_JOB_ID node $(hostname) $(date) sha $(sha256sum src/solver/wz_match.cpp | cut -c1-16) ==="
E="WZ_FIRSTHIT=1 WZ_FH_ENDPOS=1 WZ_FH_ORBIT_CANON=1 WZ_FH_ORBIT_Q=1 WZ_FH_ORBIT_QPRUNE=1 WZ_FH_PROF_ORDER=3 WZ_FH_ORD3_NODIGEST_OK=1 WZ_FH_NSHARD=177 OMP_NUM_THREADS=1"

canary() {  # name C D
  local name=$1 C=$2 D=$3
  env $E WZ_FH_LOCATE_C="$C" WZ_FH_LOCATE_D="$D" ./"$BIN" 43 2 8 5 9 > "${name}_locate.log" 2>&1
  echo "[$name] step0: $(grep -m1 '^\[orbitcanon\]' "${name}_locate.log") | $(grep -m1 '^\[order\]' "${name}_locate.log" | cut -c1-120) | $(grep -m1 '^LOCATE_CANON' "${name}_locate.log")"
  local pi; pi=$(grep -m1 -oE 'cell_idx=[0-9]+ canon_kept=YES' "${name}_locate.log" | grep -oE '[0-9]+' | head -1)
  [ -z "$pi" ] && { echo "[$name] VERDICT: FAIL (LOCATE found no KEPT cell: the odd-n group/prune dropped a known solution orbit)"; return; }
  local arm=$((pi % 177)) win=$((pi / 177))
  echo "[$name] step0: kept cell pi=$pi (window $win, arm $arm)"
  env $E WZ_FH_SHARD=$arm WZ_FH_PROF_SKIP=$win WZ_FH_PROF_END=$((win+1)) WZ_FH_PROG_SEC=600 \
      WZ_FH_CELLSIZE=1000000000000 WZ_FH_TARGET_C="$C" WZ_FH_TARGET_D="$D" WZ_FH_TARGET_COMPLETE=1 \
      WZ_FH_AB_BUDGET=2000000 ./"$BIN" 43 2 8 5 9 > "${name}_step1.log" 2>&1
  echo "[$name] step1: $(grep -m1 '^TARGET_COMPLETE' "${name}_step1.log" || echo NO TARGET_COMPLETE LINE) | $(grep -m1 '^CELLSIZE' "${name}_step1.log")"
  if ! grep -q '^TARGET_COMPLETE' "${name}_step1.log"; then echo "[$name] VERDICT: INCONCLUSIVE (image not reached)"; return; fi
  echo "[$name] result: $(grep -m1 'FOUND \*\*\*' "${name}_step1.log") | $(grep -m1 '^FIRSTHIT:' "${name}_step1.log") | $(grep -m1 '^VERIFY:' "${name}_step1.log")"
  if grep -q 'FOUND \*\*\*' "${name}_step1.log"; then echo "[$name] VERDICT: PASS (verify locally)"
  else echo "[$name] VERDICT: FAIL (image reached, completer did not complete it at 2e6) $(grep -m1 '^RESULT' "${name}_step1.log")"; fi
  sed -n '/FOUND \*\*\*/,/^D = /p' "${name}_step1.log"
}

canary ours43 "-1,-1,-1,1,-1,-1,1,1,1,-1,1,1,-1,-1,1,1,-1,1,1,-1,1,-1,1,1,1,-1,1,-1,1,1,-1,-1,1,1,1,-1,1,1,1,1,-1,-1,-1" \
              "1,-1,-1,1,-1,1,1,1,1,-1,1,-1,1,-1,1,1,1,1,1,-1,-1,-1,-1,1,1,-1,-1,-1,1,1,1,1,1,1,1,-1,1,-1,1,1,-1,-1,1" &
canary wz43 "1,1,-1,-1,-1,1,-1,-1,1,1,1,1,-1,1,-1,1,-1,1,-1,-1,-1,-1,1,-1,-1,1,-1,1,-1,1,1,-1,1,1,1,1,1,-1,1,-1,1,1,1" \
            "-1,-1,1,1,1,1,-1,-1,-1,1,1,-1,-1,1,1,1,1,1,-1,1,1,-1,-1,1,-1,1,1,1,1,1,1,1,1,1,-1,1,1,-1,-1,1,-1,-1,-1" &
wait
echo "=== ODDCANARY done $(date) ==="
