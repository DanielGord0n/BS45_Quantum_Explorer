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
#
# 2026-08-24: a phone nudge now precedes every push, and a missed push is retried
# once after RUN_RETRY_WAIT (bounded — this runs INSIDE the agent session, where
# hours of blocking would drop the API connection). The hours-long hourly re-push
# for missed CHECKER reads lives in check_all_retry.sh / daily_auto.sh instead.
# Tunables: RUN_RETRIES (1), RUN_RETRY_WAIT (300s), PUSH_WAIT (180s).

set -uo pipefail
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
[ -f "$SCRIPT_DIR/notify.conf" ] && . "$SCRIPT_DIR/notify.conf"
NTFY_URL="${NTFY_URL:-}"
USER_ID="${CLUSTER_USER:-dangord}"
PUSH_WAIT="${PUSH_WAIT:-180}"
RUN_RETRIES="${RUN_RETRIES:-1}"
RUN_RETRY_WAIT="${RUN_RETRY_WAIT:-300}"
DUO="${DUO:-$SCRIPT_DIR/duo_ssh.py}"
ntfy_push() {  # title message [priority] [tags]
  [ -z "$NTFY_URL" ] && return 0
  curl -s -m 15 -H "Title: ${1}" -H "Priority: ${3:-default}" -H "Tags: ${4:-satellite}" \
       -d "${2}" "$NTFY_URL" >/dev/null 2>&1
}

if [ $# -lt 2 ]; then
  echo "usage: duo_run.sh <cluster> '<remote command>'" >&2
  exit 64
fi
c="$1"; shift
cmd="$*"

REMOTE='echo ===BS45BEGIN===; '"$cmd"'; echo ===BS45END==='

attempt=0
while : ; do
  attempt=$((attempt+1))
  ntfy_push "BS45 Duo push: $c" "Approve the $c push now — a submit is waiting on it (attempt $attempt/$((RUN_RETRIES+1)))." "high" "bell"
  sleep 5
  echo ">> [$c] Duo push sent — approve on your phone (up to ${PUSH_WAIT}s)…" >&2
  raw="$(python3 "$DUO" "${USER_ID}@${c}.alliancecan.ca" "$REMOTE" "$PUSH_WAIT" 2>/dev/null)"
  body="$(printf '%s' "$raw" | tr -d '\r' | awk '/===BS45BEGIN===/{f=1;next} /===BS45END===/{f=0} f')"
  [ -n "$body" ] && break
  echo "!! [$c] no approval within ${PUSH_WAIT}s (attempt $attempt/$((RUN_RETRIES+1)))." >&2
  if [ "$attempt" -gt "$RUN_RETRIES" ]; then
    echo "!! [$c] giving up — nothing ran." >&2
    exit 2
  fi
  echo ">> [$c] retrying in ${RUN_RETRY_WAIT}s…" >&2
  sleep "$RUN_RETRY_WAIT"
done
printf '%s\n' "$body"
