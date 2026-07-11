#!/bin/bash
# duo_run.sh <cluster> '<remote command>'
#
# Run ANY command on a cluster with the Duo "1" auto-typed for you. The push
# lands on your phone; you tap; the command runs; its output is printed clean
# (Duo chatter stripped). Same mechanism as the checker — you never touch the
# laptop.
#
# This is how new jobs get queued hands-off, e.g.:
#
#   ./cluster/deploy/duo_run.sh fir \
#     'cd $SCRATCH/bs45 && sbatch --requeue --export=ALL,WZ_N=32,WZ_SEED_BASE=69000000 ./cluster_sa_ladder.sh'
#
#   ./cluster/deploy/duo_run.sh nibi \
#     'cd $SCRATCH/bs45 && sbatch --requeue --account=def-ikotsire_cpu --export=ALL,WZ_N=32,WZ_SEED_BASE=72000000 ./cluster_sa_ladder.sh'
#
# Exit code is 0 only if the command actually ran (i.e. you approved in time).

set -uo pipefail
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
USER_ID="${CLUSTER_USER:-dangord}"
PUSH_WAIT="${PUSH_WAIT:-180}"

if [ $# -lt 2 ]; then
  echo "usage: duo_run.sh <cluster> '<remote command>'" >&2
  exit 64
fi
c="$1"; shift
cmd="$*"

REMOTE='echo ===BS45BEGIN===; '"$cmd"'; echo ===BS45END==='

echo ">> [$c] Duo push sent — approve on your phone (up to ${PUSH_WAIT}s)…" >&2
raw="$(python3 "$SCRIPT_DIR/duo_ssh.py" "${USER_ID}@${c}.alliancecan.ca" "$REMOTE" "$PUSH_WAIT" 2>/dev/null)"
body="$(printf '%s' "$raw" | tr -d '\r' | awk '/===BS45BEGIN===/{f=1;next} /===BS45END===/{f=0} f')"

if [ -z "$body" ]; then
  echo "!! [$c] no approval within ${PUSH_WAIT}s — nothing ran." >&2
  exit 2
fi
printf '%s\n' "$body"
