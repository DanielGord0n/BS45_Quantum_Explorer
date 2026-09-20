#!/bin/bash
# ntfy_listener.sh — one-tap "Run check now" (2026-09-10, rewritten 2026-09-19).
#
# Subscribes to the private control topic; when "check <token>" arrives (the button
# on any BS45 notification publishes it) it runs a supplementary daily_auto pass for
# ONLY the clusters not yet read today (per-day ledger results/reached_<date>.txt).
# Nothing missing => full re-check of all four.
#
# Concurrency (the 09-19 failure: a tap during the 1pm run started a second agent):
# daily_auto.sh itself holds ONE global lock for every run (cron, button, reminder);
# a tap while a run is in progress is queued — daily_auto waits for the lock, then
# recomputes what is still unread. The phone is told either way.
#
# Runs under launchd (com.dangord.bs45listener), restarts on disconnect. DRY_RUN=1 = log only.
set -uo pipefail
DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"; REPO="$(cd "$DIR/../.." && pwd)"
. "$DIR/notify.conf"
LOG="$REPO/results/ntfy_listener.log"
log(){ echo "[$(date '+%F %T')] $*" >> "$LOG"; }
ntfy(){ curl -s -m 15 -H "Title: $1" -H "Priority: ${3:-default}" -H "Tags: ${4:-satellite}" -d "$2" "$NTFY_URL" >/dev/null 2>&1; }

whats_left() {  # prints the clusters not in today's ledger (all four if none read)
  local ledger="$REPO/results/reached_$(date +%F).txt" done_today="" todo="" c
  [ -f "$ledger" ] && done_today="$(awk '{print $2}' "$ledger" | sort -u | tr '\n' ' ')"
  for c in fir nibi rorqual trillium; do
    case " $done_today " in *" $c "*) ;; *) todo="$todo $c";; esac
  done
  todo="$(echo $todo)"; [ -z "$todo" ] && todo="fir nibi rorqual trillium"
  echo "$todo"
}

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
    if [ $((now-last_run)) -lt 120 ]; then log "debounced"; continue; fi
    last_run=$now
    todo="$(whats_left)"
    n=$(echo $todo | wc -w | tr -d ' ')
    if [ "${DRY_RUN:-0}" = 1 ]; then log "DRY_RUN: would run check on: $todo"; continue; fi
    if "$DIR/run_lock.sh" busy; then
      log "button tap -> run in progress ($("$DIR/run_lock.sh" info)); queuing: $todo"
      ntfy "BS45: check queued" "A check is already running ($("$DIR/run_lock.sh" info)). Yours starts when it finishes, for: $todo" "low" "hourglass"
    else
      log "button tap -> supplementary check on: $todo"
      ntfy "BS45: check starting now" "$n Duo push(es) coming one at a time ($todo). Tap each." "high" "bell"
    fi
    # stdin MUST be detached (2026-09-20): `claude -p` blocks until stdin EOF, and a child
    # launched inside this `curl | while read` loop inherits the hour-long stream as stdin —
    # every button-run agent sat idle until the stream ended (3-6 h runs, 09-19/20).
    ( cd "$REPO" && exec nohup env SUPPLEMENTARY=1 CLUSTERS="$todo" RETRY_MAX=0 LOCK_WAIT_SEC=7200 BUTTON=1 ./cluster/deploy/daily_auto.sh </dev/null >> "$LOG" 2>&1 ) &
  done
  log "subscription dropped — reconnecting in 15s"; sleep 15
done
