#!/bin/bash
# remind_unread.sh <clusters...> — hourly phone REMINDERS (no Duo pushes) for clusters
# still unread today. Replaces the unattended hourly re-push (2026-09-19): pushes now
# happen only at 1pm or when Daniel taps "Run check now". Stops when the cluster is read
# (per-day ledger), stays quiet while the cluster is in a listed outage, gives up after
# MAX_REMINDERS. Every reminder carries the button.
set -uo pipefail
DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"; REPO="$(cd "$DIR/../.." && pwd)"
[ -f "$DIR/notify.conf" ] && . "$DIR/notify.conf"
NTFY_URL="${NTFY_URL:-}"; MAX_REMINDERS="${MAX_REMINDERS:-6}"; INTERVAL="${REMIND_INTERVAL:-3600}"
LOG="$REPO/results/reminders.log"
log(){ echo "[$(date '+%F %T')] $*" >> "$LOG"; }
ntfy_push() {
  [ -z "$NTFY_URL" ] && return 0
  local act=()
  [ -n "${NTFY_CONTROL_URL:-}" ] && act=(-H "Actions: http, Run check now, ${NTFY_CONTROL_URL}, method=POST, body=check ${NTFY_CONTROL_TOKEN:-}, clear=true")
  curl -s -m 15 -H "Title: ${1}" -H "Priority: ${3:-default}" -H "Tags: ${4:-bell}" "${act[@]}" -d "${2}" "$NTFY_URL" >/dev/null 2>&1
}
status_text() { curl -sm 20 -A "Mozilla/5.0 (Macintosh)" https://status.alliancecan.ca 2>/dev/null | python3 -c "import sys,re,html; t=html.unescape(re.sub(r'<[^>]+>',' ',sys.stdin.read())); print(re.sub(r'\s+',' ',t))"; }
in_outage() { local n; n="$(printf '%s' "$1" | awk '{print toupper(substr($0,1,1)) substr($0,2)}')"; status_text | grep -qE "\b${n} cloud_off\b"; }
unread() {  # of the given clusters, those not in today's ledger
  local ledger="$REPO/results/reached_$(date +%F).txt" out="" c
  for c in "$@"; do
    if [ -f "$ledger" ] && awk -v c="$c" '$2==c{f=1} END{exit f?0:1}' "$ledger"; then continue; fi
    out="$out $c"
  done
  echo $out
}
targets="$*"; log "reminders armed for: $targets"
for i in $(seq 1 "$MAX_REMINDERS"); do
  sleep "$INTERVAL"
  left="$(unread $targets)"; [ -z "$left" ] && { log "all read — done"; exit 0; }
  say=""; for c in $left; do in_outage "$c" && { log "$c in outage — quiet"; continue; }; say="$say $c"; done
  [ -z "$say" ] && continue
  ntfy_push "BS45: still unread today —$say" "Reminder $i/$MAX_REMINDERS: tap Run check now when you can (pushes only come when you tap). Queues keep computing meanwhile." "low" "bell"
  log "reminder $i sent for:$say"
done
left="$(unread $targets)"; [ -n "$left" ] && ntfy_push "BS45: $left unread today" "No more reminders today. Tomorrow's 1pm run reads it; or tap Run check now." "low" "bell"
exit 0
