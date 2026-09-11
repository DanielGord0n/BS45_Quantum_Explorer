#!/bin/bash
# ntfy_listener.sh — one-tap "Run check now" (2026-09-10).
# Subscribes to the private control topic; when a message "check <token>" arrives
# (the button on any BS45 notification publishes it), runs a supplementary
# daily_auto pass on all four clusters RIGHT NOW (Duo pushes follow within seconds,
# while Daniel is holding the phone). Guards: token, a lock against concurrent runs,
# and a 3-minute debounce. Runs under launchd (com.dangord.bs45listener), restarts
# itself on disconnect. DRY_RUN=1 = log only.
set -uo pipefail
DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"; REPO="$(cd "$DIR/../.." && pwd)"
. "$DIR/notify.conf"
LOG="$REPO/results/ntfy_listener.log"; LOCK="/tmp/bs45_check.lock"
log(){ echo "[$(date '+%F %T')] $*" >> "$LOG"; }
ntfy(){ curl -s -m 15 -H "Title: $1" -H "Priority: ${3:-default}" -H "Tags: ${4:-satellite}" -d "$2" "$NTFY_URL" >/dev/null 2>&1; }
last_run=0
log "listener up (topic $(basename "$NTFY_CONTROL_URL"))"
while :; do
  curl -s --no-buffer -m 3600 "$NTFY_CONTROL_URL/json?since=30s" 2>/dev/null | while IFS= read -r line; do
    msg="$(printf '%s' "$line" | python3 -c 'import sys,json
try: d=json.loads(sys.stdin.read()); print(d.get("message","") if d.get("event")=="message" else "")
except Exception: print("")')"
    [ -z "$msg" ] && continue
    case "$msg" in
      "check $NTFY_CONTROL_TOKEN"*) ;;
      *) log "ignored message: ${msg:0:40}"; continue;;
    esac
    now=$(date +%s)
    if [ $((now-last_run)) -lt 180 ]; then log "debounced"; continue; fi
    if [ -e "$LOCK" ] && kill -0 "$(cat "$LOCK" 2>/dev/null)" 2>/dev/null; then
      ntfy "BS45: a check is already running" "Tap again when it finishes (its Duo pushes are on the way)." "low" "hourglass"; log "busy"; continue
    fi
    last_run=$now
    if [ "${DRY_RUN:-0}" = 1 ]; then log "DRY_RUN: would start check"; continue; fi
    log "button tap -> starting supplementary check on all clusters"
    ntfy "BS45: check starting now" "4 Duo pushes coming one at a time (fir nibi rorqual trillium). Tap each." "high" "bell"
    ( cd "$REPO" && echo $$ > "$LOCK" && SUPPLEMENTARY=1 CLUSTERS="fir nibi rorqual trillium" RETRY_MAX=0 ./cluster/deploy/daily_auto.sh >> "$LOG" 2>&1; rm -f "$LOCK" ) &
  done
  log "subscription dropped — reconnecting in 15s"; sleep 15
done
