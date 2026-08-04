#!/bin/bash
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=192
#SBATCH --time=12:00:00
#SBATCH --job-name=FIRSTHIT
#SBATCH --output=firsthit_output_%j.txt
#SBATCH --account=def-ikotsire
#SBATCH --mail-type=END,FAIL

# First-hit probe (Gate B + Gate C, work order 2026-07-16): stream the Thm-2.2
# C,D pair stream in deterministic order, backtrack A,B per candidate under
# Def 1.1 + Thm 2.2 (mirror-pair DFS), stop at the first NPAF==0 hit.
# Fans FH_NARMS single-core arms over INTERLEAVED profile shards (arm i takes
# profiles ≡ i mod N — fat-tail-safe). Driver polls for the first FOUND, gives
# the other arms a grace window, then aggregates.
#   sbatch --export=ALL,WZ_N=29,WZ_A=0,WZ_B=6,WZ_C=9,WZ_D=1 cluster_firsthit_probe.sh
# Optional: FH_NARMS (190) WZ_FH_AB_BUDGET (200000) WZ_FH_PROF_ORDER (0)
#           WZ_FH_SCORE_MAX (0) WZ_FH_STREAM_TOTAL (frac-depth denominator)
#           FH_GRACE (1800s after first FOUND)

cd $SLURM_SUBMIT_DIR
module load StdEnv/2023
module load gcc/12.3

N=${WZ_N:?need WZ_N}; A=${WZ_A:?}; B=${WZ_B:?}; C=${WZ_C:?}; D=${WZ_D:?}
NARMS=${FH_NARMS:-190}
GRACE=${FH_GRACE:-1800}
BIN=fh_bin_${SLURM_JOB_ID}
DIR=fh_arms_${SLURM_JOB_ID}
mkdir -p "$DIR"
# Per-arm candidate-level resume (2026-07-28): checkpoint dir is keyed by the
# SEARCH LANE (class + arms + order + skip), NOT the job id — the next job on
# the same lane auto-resumes every arm at its exact stopping candidate. One job
# per lane at a time (concurrent same-lane jobs would overwrite each other's
# checkpoints: no gaps, but wasted overlap). WZ_FH_RESUME=0 = fresh start.
CKDIR=fh_ckpt/${N}_${A}_${B}_${C}_${D}_${NARMS}_ord${WZ_FH_PROF_ORDER:-0}_skip${WZ_FH_PROF_SKIP:-0}_oc${WZ_FH_ORBIT_CANON:-0}
mkdir -p "$CKDIR"
export WZ_FH_CKPT_DIR=$CKDIR
echo "[driver] checkpoint lane: $CKDIR (resume=${WZ_FH_RESUME:-1})"

echo "=== FIRSTHIT probe BS($((N+1)),$N) sig ($A,$B,$C,$D) — $NARMS arms — node $(hostname) — $(date) ==="
g++ -O3 -march=native -std=c++17 -fopenmp -o "$BIN" src/solver/wz_match.cpp || exit 1

export WZ_FIRSTHIT=1 OMP_NUM_THREADS=1
export WZ_FH_AB_BUDGET=${WZ_FH_AB_BUDGET:-200000}
[ -n "$WZ_FH_PROF_ORDER" ]   && export WZ_FH_PROF_ORDER
[ -n "$WZ_FH_SCORE_MAX" ]    && export WZ_FH_SCORE_MAX
[ -n "$WZ_FH_STREAM_TOTAL" ] && export WZ_FH_STREAM_TOTAL
[ -n "$WZ_FH_PROF_SKIP" ]    && export WZ_FH_PROF_SKIP
[ -n "$WZ_FH_AB_PROF" ]      && export WZ_FH_AB_PROF
[ -n "$WZ_FH_RESUME" ]       && export WZ_FH_RESUME
[ -n "$WZ_FH_BUF_CAP" ]      && export WZ_FH_BUF_CAP
[ -n "$WZ_FH_ORBIT_CANON" ]  && export WZ_FH_ORBIT_CANON

# FH_SCORE_TIERS="t1,t2" (optional): first quarter of arms complete only
# candidates with flatness score <= t1, second quarter <= t2, rest ungated.
# Flat candidates measured ~35x denser in solutions at n=19; hit scores are
# now printed per FIRSTHIT line, so tier values should come from prior rounds'
# data. Every arm still streams its full shard — tiers only skip completions.
# Prefer FH_SCORE_T1/FH_SCORE_T2 (sbatch --export MANGLES commas — the 07-20
# wave's FH_SCORE_TIERS=110\,130 arrived as both-tiers-110). FH_SCORE_TIERS
# kept for backward compat only.
T1=${FH_SCORE_T1:-}; T2=${FH_SCORE_T2:-}
if [ -z "$T1" ] && [ -n "$FH_SCORE_TIERS" ]; then
  T1=${FH_SCORE_TIERS%%,*}; T2=${FH_SCORE_TIERS##*,}
fi
[ -n "$T1" ] && [ -z "$T2" ] && T2=$T1
if [ -n "$T1" ]; then
  echo "[driver] score tiers: arms 0-$((NARMS/4-1)) <=$T1, $((NARMS/4))-$((NARMS/2-1)) <=$T2, rest ungated"
fi
pids=()
for ((i=0; i<NARMS; i++)); do
  tier=""
  if [ -n "$T1" ] && [ "$i" -lt $((NARMS/4)) ]; then tier=$T1
  elif [ -n "$T2" ] && [ "$i" -lt $((NARMS/2)) ]; then tier=$T2; fi
  WZ_FH_SHARD=$i WZ_FH_NSHARD=$NARMS WZ_FH_SCORE_MAX=$tier ./"$BIN" "$N" "$A" "$B" "$C" "$D" \
    > "$DIR/arm_$i.log" 2>&1 &
  pids+=($!)
done
echo "[driver] $NARMS arms launched $(date)"

# Poll: first FOUND starts the grace clock; 11.5h is the hard aggregation
# deadline (30 min before slurm walltime kill).
DEADLINE=$(( $(date +%s) + 11*3600 + 1800 ))
FOUND_AT=0
while :; do
  alive=0
  for p in "${pids[@]}"; do kill -0 "$p" 2>/dev/null && alive=$((alive+1)); done
  now=$(date +%s)
  if [ "$FOUND_AT" = 0 ] && grep -l "FOUND \*\*\*" "$DIR"/arm_*.log >/dev/null 2>&1; then
    FOUND_AT=$now
    echo "[driver] first FOUND at $(date) — grace ${GRACE}s for remaining arms"
  fi
  [ "$alive" = 0 ] && break
  if [ "$FOUND_AT" != 0 ] && [ $((now - FOUND_AT)) -ge "$GRACE" ]; then
    echo "[driver] grace over — stopping $alive remaining arms"; kill "${pids[@]}" 2>/dev/null; break
  fi
  if [ "$now" -ge "$DEADLINE" ]; then
    echo "[driver] deadline — stopping $alive remaining arms"; kill "${pids[@]}" 2>/dev/null; break
  fi
  sleep 30
done
wait 2>/dev/null

echo ""
echo "=== FIRSTHIT AGGREGATE (n=$N, sig $A,$B,$C,$D, $NARMS arms) ==="
hits=0
for f in "$DIR"/arm_*.log; do
  if grep -q "FOUND \*\*\*" "$f"; then
    hits=$((hits+1))
    echo "--- $(basename $f) ---"
    grep -A 10 "FOUND \*\*\*" "$f" | head -12
  fi
done
echo "arms_with_hits=$hits / $NARMS"
# Gate B aggregation from every arm's summary/progress lines
tot_cand=0; tot_nodes=0; tot_abort=0; cd_min=-1; cd_sum=0; tt_sum=0; tt_min=-1
tc_sum=0; rp_min=-1; rp_max=0
for f in "$DIR"/arm_*.log; do
  line=$(grep -E "candidates_streamed=" "$f" | tail -1)
  c=$(echo "$line" | grep -oE "candidates_streamed=[0-9]+" | cut -d= -f2)
  [ -n "$c" ] && tot_cand=$((tot_cand+c))
  # tested aggregation: candidates_streamed counts ENUMERATED candidates (incl.
  # ones sitting untested in the cell-order buffer at kill time) — wave 4 proved
  # streamed can read ~100M while tested is a fraction of it. backtracks_entered
  # is the true searched depth; MIN across arms = sound per-arm resume floor.
  tt=$(grep -oE "backtracks_entered=[0-9]+" "$f" | tail -1 | cut -d= -f2)
  if [ -n "$tt" ]; then
    tt_sum=$((tt_sum+tt))
    if [ "$tt_min" = -1 ] || [ "$tt" -lt "$tt_min" ]; then tt_min=$tt; fi
  fi
  # Cumulative (cross-wave) depth from the checkpoint ledger, and the resume
  # frontier: min/max resume_pi across arms = the wave-over-wave depth meter.
  tc=$(grep -oE "tested_cum=[0-9]+" "$f" | tail -1 | cut -d= -f2)
  [ -n "$tc" ] && tc_sum=$((tc_sum+tc))
  rp=$(grep -oE "resume_pi=[0-9]+" "$f" | tail -1 | cut -d= -f2)
  if [ -n "$rp" ]; then
    if [ "$rp_min" = -1 ] || [ "$rp" -lt "$rp_min" ]; then rp_min=$rp; fi
    if [ "$rp" -gt "$rp_max" ]; then rp_max=$rp; fi
  fi
  a=$(grep -oE "budget_aborted=[0-9]+" "$f" | tail -1 | cut -d= -f2)
  [ -n "$a" ] && tot_abort=$((tot_abort+a))
  nn=$(grep -oE "total_AB_nodes=[0-9]+" "$f" | tail -1 | cut -d= -f2)
  [ -n "$nn" ] && tot_nodes=$((tot_nodes+nn))
  # cells_done aggregation: MIN across arms = the sound WZ_FH_PROF_SKIP for the
  # next continuation wave (overlap is idempotent, a gap would be unsound).
  cd=$(grep -oE "cells_done=[0-9]+" "$f" | tail -1 | cut -d= -f2)
  if [ -n "$cd" ]; then
    cd_sum=$((cd_sum+cd))
    if [ "$cd_min" = -1 ] || [ "$cd" -lt "$cd_min" ]; then cd_min=$cd; fi
  fi
done
# Arms killed before their summary now still report via SIGTERM-dumped
# summaries / periodic progress lines; count both so a partial aggregate can
# never read as an empty stream again (the 07-22/23 zero-candidate artifact).
summarized=$(grep -l "FIRSTHIT SUMMARY" "$DIR"/arm_*.log 2>/dev/null | wc -l)
interrupted=$(grep -l "RESULT: INTERRUPTED" "$DIR"/arm_*.log 2>/dev/null | wc -l)
echo "GATEB: candidates=$tot_cand tested=$tt_sum tested_min=$tt_min tested_cum=$tc_sum resume_pi_min=$rp_min resume_pi_max=$rp_max aborted=$tot_abort AB_nodes=$tot_nodes arms_summarized=$(echo $summarized)/$NARMS arms_interrupted=$(echo $interrupted) cells_done_min=$cd_min cells_done_sum=$cd_sum"
# Global first hit = min by (profile_rank, idx) across arms
grep -h "FIRSTHIT:" "$DIR"/arm_*.log 2>/dev/null \
  | sed -E 's/.*idx=([0-9]+) profile_rank=([0-9]+).*/\2 \1 &/' \
  | sort -n -k1,1 -k2,2 | head -1 | cut -d' ' -f3- \
  | sed 's/^/GLOBAL FIRST: /'
rm -f "$BIN"
echo "=== done $(date) ==="
