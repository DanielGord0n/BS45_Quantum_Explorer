#!/bin/bash
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=192
#SBATCH --time=12:00:00
#SBATCH --job-name=P22_GATE
#SBATCH --output=pair22_gate_output_%A_%a.txt
#SBATCH --array=0-19
#SBATCH --account=def-ikotsire
#SBATCH --mail-type=END,FAIL

# ============================================================================
# GATE A' — the decisive measurement for the route to n=42-43.
#
# WHY THIS EXISTS: the WZ_COUNT_PAIR22 C,D count is OpenMP-parallel WITHIN a node
# but could not span nodes. At n=36 it finished only 96 of 985 profiles in a 12h
# walltime (~19.6 thread-hours/profile => ~19,300 thread-hours total), so THE gate
# number was unreachable by any single job. Nibi 17434023 was marked do-not-rerun
# for exactly this reason — it wasn't a bad result, it was an unfinishable one.
#
# This shards the profile list across a SLURM ARRAY (WZ_PROF_LO/WZ_PROF_HI, added
# 2026-07-11). 20 tasks x 192 threads = 3,840 threads => ~5h. Fits one walltime.
# Sharding is EXACT: validated at n=10 (sum over a 3-way partition == unsharded
# total, 92+125+87 = 304).
#
# THE PRE-REGISTERED RULE (docs/wz_firsthit_plan.md — do not move the line after
# seeing the number):
#     C,D stream <= ~1e9 at n=36  -> PASS  -> build Phase 1 (joint-pair generation)
#     C,D stream >= 1e12 at n=36  -> KILL  -> the Thm-2.2 lift is not the lever
#     in between                  -> measure Gate B (A,B completion cost) first
#
# Early partial from the dead job was `leaves~0 stream~0` at 96/985 — if that holds
# this passes by orders of magnitude. It is also NOT yet evidence. Get the number.
#
# SUBMIT (n=36, sig 5,11,0,0 — norm 146 = 4*36+2):
#   ./cluster/deploy/duo_run.sh nibi 'cd $SCRATCH/bs45 && sbatch --requeue \
#      --account=def-ikotsire_cpu --export=ALL,WZ_N=36,WZ_A=5,WZ_B=11,WZ_C=0,WZ_D=0 \
#      ./cluster_pair22_gate.sh'
#
# COLLECT (after all 20 tasks finish) — sum the per-shard streams:
#   grep -h SHARD_STREAM pair22_gate_output_<JOBID>_*.txt | awk '{s+=$5} END {print "TOTAL C,D STREAM =", s}'
#   Then compare to the gate rule above. Sum ALL 20 shards or the number is a lie —
#   a missing shard silently under-reports the stream and would fake a PASS.
# ============================================================================

cd $SLURM_SUBMIT_DIR
module load StdEnv/2023
module load gcc/12.3
export OMP_NUM_THREADS=$SLURM_CPUS_PER_TASK

N=${WZ_N:-36}; A=${WZ_A:-5}; B=${WZ_B:-11}; C=${WZ_C:-0}; D=${WZ_D:-0}
NPROF=${WZ_NPROF:-985}                   # C,D profile count at n=36 (from the dead job)
NTASKS=${WZ_NTASKS:-20}                  # must match --array size above
SHARD=$(( (NPROF + NTASKS - 1) / NTASKS ))
LO=$(( SLURM_ARRAY_TASK_ID * SHARD ))
HI=$(( LO + SHARD ))
[ "$HI" -gt "$NPROF" ] && HI=$NPROF

BIN=pair22_bin_${SLURM_JOB_ID}_${SLURM_ARRAY_TASK_ID}

echo "=== GATE A' pair22 C,D — n=$N sig ($A,$B,$C,$D) — shard [$LO,$HI) of $NPROF ==="
echo "=== task $SLURM_ARRAY_TASK_ID/$NTASKS — $OMP_NUM_THREADS threads — $(hostname) — $(date) ==="

g++ -O3 -march=native -std=c++17 -fopenmp -o "$BIN" src/solver/wz_match.cpp || exit 1

WZ_COUNT_PAIR22=1 \
WZ_PAIR22_SIDE=CD \
WZ_PROF_LO=$LO \
WZ_PROF_HI=$HI \
./"$BIN" "$N" "$A" "$B" "$C" "$D"

rm -f "$BIN"
echo "=== shard [$LO,$HI) done $(date) — grep SHARD_STREAM and sum column 5 across all tasks ==="
