#!/bin/bash
# Check job status and latest output across all clusters.
# Runs all 4 SSH connections in parallel — approve all Duo pushes quickly.

TMPDIR=$(mktemp -d)
trap "rm -rf $TMPDIR" EXIT

check_cluster() {
  local name=$1
  local host=$2
  local out=$TMPDIR/$name.txt

  ssh -o ConnectTimeout=20 "$host" "
    echo '=== QUEUE ==='
    squeue -u dangord --format='%10i %20j %2t %12L' 2>/dev/null

    echo ''
    echo '=== LATEST OUTPUT (tail 8) ==='
    cd \$SCRATCH/bs45 2>/dev/null || exit 0
    for f in \$(ls -t bs28_v3_${name}_output_*.txt bs43_v2_${name}_output_*.txt bs45_v3_${name}_output_*.txt 2>/dev/null | head -4); do
      echo \"--- \$f ---\"
      tail -8 \"\$f\"
    done

    echo ''
    echo '=== SOLUTIONS FOUND ==='
    grep -rl 'REPRODUCTION CONFIRMED\|WORLD RECORD' \$SCRATCH/bs45/ 2>/dev/null || echo '(none yet)'
  " > "$out" 2>&1
}

echo "Connecting to all clusters in parallel — approve Duo pushes on your phone..."
echo ""

check_cluster fir      dangord@fir.alliancecan.ca       &
check_cluster rorqual  dangord@rorqual.alliancecan.ca   &
check_cluster nibi     dangord@nibi.alliancecan.ca      &
check_cluster trillium dangord@trillium.scinet.utoronto.ca &

wait

for name in fir rorqual nibi trillium; do
  echo ""
  echo "════════════════════════════════════════"
  echo "  CLUSTER: ${name^^}"
  echo "════════════════════════════════════════"
  cat "$TMPDIR/$name.txt"
done
