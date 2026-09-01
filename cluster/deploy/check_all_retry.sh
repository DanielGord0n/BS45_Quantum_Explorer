#!/bin/bash
# check_all_retry.sh — hands-off cluster check. YOU NEVER TOUCH THE LAPTOP.
#
# Runs your exact checker on all 4 clusters, ONE AT A TIME. For each cluster it
# auto-types "1" at the Duo menu, which sends the push to your phone; you tap to
# approve; it captures that cluster's output and moves on.
#
#   * One push per cluster per run — no spam, no overlapping logins.
#   * No SSH connection-sharing (that's what caused the prompt pile-up before).
#   * Output saved to results/latest_check.txt so Claude Code reads it directly
#     (no copy-paste), and a summary is texted to your phone via ntfy.
#
# If you don't approve a cluster's push within PUSH_WAIT, that cluster is
# skipped on the FIRST pass so the others are not held up. Then (2026-08-24,
# Daniel's ask) every missed cluster gets a phone nudge + a fresh Duo push every
# RETRY_INTERVAL seconds (default hourly), up to RETRY_MAX rounds (default 10),
# until you approve. Ctrl-C stops a manual run. The daily loop runs the first
# pass with RETRY_MAX=0 and does the hourly retries in a supplementary pass
# AFTER the main agent run, so the day's read is never delayed by a missed tap.
#
# Requires: python3 (ships with macOS Command Line Tools).
# Usage:  ./cluster/deploy/check_all_retry.sh
# Tunables (env): PUSH_WAIT (180s), RETRY_MAX (10), RETRY_INTERVAL (3600s),
#                 RETRY_NUDGE (45s between the phone nudge and the push), CLUSTER_USER.

set -uo pipefail
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
[ -f "$SCRIPT_DIR/notify.conf" ] && . "$SCRIPT_DIR/notify.conf"
NTFY_URL="${NTFY_URL:-}"

USER_ID="${CLUSTER_USER:-dangord}"
# Override to re-check only the clusters that missed their Duo window, e.g.
#   CLUSTERS="fir nibi" ./cluster/deploy/check_all_retry.sh
# Unset (the daily loop) = all four, unchanged.
CLUSTERS="${CLUSTERS:-fir nibi rorqual trillium}"
PUSH_WAIT="${PUSH_WAIT:-180}"     # seconds to wait for you to tap each push
RETRY_MAX="${RETRY_MAX:-10}"      # hourly re-push rounds for missed clusters (0 = none)
RETRY_INTERVAL="${RETRY_INTERVAL:-3600}"
RETRY_NUDGE="${RETRY_NUDGE:-45}"  # phone nudge lands, then the push follows this many s later
DUO="${DUO:-$SCRIPT_DIR/duo_ssh.py}"
# The remote checker command lives in checker_cmd.txt so Claude Code can evolve it
# (exclusion filter, rung label, gate probes) without touching this driver.
CMD_FILE="$SCRIPT_DIR/checker_cmd.txt"

mkdir -p "$REPO_ROOT/results"
OUT="$REPO_ROOT/results/latest_check.txt"
: > "$OUT"

ntfy_push() {  # title message [priority] [tags]
  [ -z "$NTFY_URL" ] && return 0
  curl -s -m 15 -H "Title: ${1}" -H "Priority: ${3:-default}" -H "Tags: ${4:-satellite}" \
       -d "${2}" "$NTFY_URL" >/dev/null 2>&1
}

if [ ! -f "$CMD_FILE" ]; then
  echo "Missing $CMD_FILE — the checker command lives there." >&2
  exit 2
fi
# Strip comment/blank lines; keep the command verbatim (no local expansion).
INNER="$(grep -vE '^[[:space:]]*(#|$)' "$CMD_FILE")"
REMOTE='echo ===BS45BEGIN===
'"$INNER"'
echo ===BS45END==='

n_clusters="$(echo $CLUSTERS | wc -w | tr -d ' ')"
ntfy_push "BS45 check starting" "${n_clusters} Duo push(es) coming one at a time (${CLUSTERS}) — tap each to approve." "low" "hourglass"

# --- outage awareness (2026-09-01) ------------------------------------------
# During Fir's 09-01 Slurm upgrade the login still showed the Duo menu but no push
# ever reached the phone, and the hourly re-push nagged Daniel all afternoon. Now:
# consult https://status.alliancecan.ca before every push; a cluster in a listed
# outage ("cloud_off" marker) is skipped WITHOUT a push or a "tap to approve" nudge
# (one quiet note when first seen, one when it clears). STATUS_SRC=<file> overrides
# the fetch for tests; a failed fetch = assume operational (old behaviour).
STATUS_SRC="${STATUS_SRC:-}"
status_text() {
  if [ -n "$STATUS_SRC" ]; then cat "$STATUS_SRC" 2>/dev/null; return; fi
  curl -sm 20 -A "Mozilla/5.0 (Macintosh)" https://status.alliancecan.ca 2>/dev/null \
    | python3 -c "import sys,re,html; t=html.unescape(re.sub(r'<[^>]+>',' ',sys.stdin.read())); print(re.sub(r'\s+',' ',t))"
}
cluster_outage() {  # $1 = cluster -> prints the outage title and returns 0 if in a listed outage
  local c="$1" name txt
  name="$(printf '%s' "$c" | awk '{print toupper(substr($0,1,1)) substr($0,2)}')"
  txt="$(status_text)"
  [ -z "$txt" ] && return 1
  printf '%s' "$txt" | grep -oE "\\b${name} cloud_off .{0,90}" | head -1 | sed -E "s/^${name} cloud_off //; s/ (check|cloud_off|event|warning_amber|power_off) .*//" | cut -c1-80
  printf '%s' "$txt" | grep -qE "\b${name} cloud_off\b"
}
OUTAGE_NOTED=" "   # bash-3.2 safe membership list (macOS bash lacks associative arrays)
noted()   { case "$OUTAGE_NOTED" in *" $1 "*) return 0;; *) return 1;; esac; }
note_add(){ noted "$1" || OUTAGE_NOTED="$OUTAGE_NOTED$1 "; }
note_del(){ OUTAGE_NOTED="$(printf "%s" "$OUTAGE_NOTED" | sed "s/ $1 / /")"; }
check_one() {  # $1 = cluster -> 0 if its output was captured, 1 if the push was missed
  local c="$1" raw body
  echo ""                                    | tee -a "$OUT"
  echo "════════════════ $c ════════════════" | tee -a "$OUT"
  echo ">> [$c] Duo push sent — approve on your phone (waiting up to ${PUSH_WAIT}s)…"
  raw="$(python3 "$DUO" "${USER_ID}@${c}.alliancecan.ca" "$REMOTE" "$PUSH_WAIT" 2>/dev/null)"
  # tr -d '\r': the pty turns every newline into CRLF; strip the CRs or every
  # downstream match (incl. the "(none yet)" filter) silently fails.
  body="$(printf '%s' "$raw" | tr -d '\r' | awk '/===BS45BEGIN===/{f=1;next} /===BS45END===/{f=0} f')"
  if [ -n "$body" ]; then
    printf '%s\n' "$body" | tee -a "$OUT"
    return 0
  fi
  echo "(no approval within ${PUSH_WAIT}s — skipped this attempt)" | tee -a "$OUT"
  return 1
}

reached=""
missed=""
for c in $CLUSTERS; do
  if o="$(cluster_outage "$c")"; then
    echo "" | tee -a "$OUT"; echo "════════════════ $c ════════════════" | tee -a "$OUT"
    echo "(in a listed outage on status.alliancecan.ca: ${o:-maintenance} — no push sent)" | tee -a "$OUT"
    ntfy_push "BS45: $c is down for maintenance" "status.alliancecan.ca lists $c in an outage (${o:-maintenance}). No Duo push was sent; I'll re-check quietly each hour and push only once it is back." "low" "construction"
    note_add "$c"
    missed="$missed $c"
    continue
  fi
  if check_one "$c"; then reached="$reached $c"; else missed="$missed $c"; fi
done

# --- hourly re-push for missed clusters (2026-08-24) ------------------------
# One nudge + one push per missed cluster per round; nothing overlaps. A cluster
# that answers leaves the missed list; the loop ends when the list is empty or
# RETRY_MAX rounds are used (bounded so a run can never collide with tomorrow's).
round=0
while [ -n "$missed" ] && [ "$round" -lt "$RETRY_MAX" ]; do
  round=$((round+1))
  echo ">> missed:${missed} — Duo retry round $round/$RETRY_MAX in ${RETRY_INTERVAL}s" | tee -a "$OUT"
  sleep "$RETRY_INTERVAL"
  still=""
  for c in $missed; do
    if o="$(cluster_outage "$c")"; then
      echo ">> [$c] still in a listed outage (${o:-maintenance}) — no push this round" | tee -a "$OUT"
      noted "$c" || ntfy_push "BS45: $c is down for maintenance" "status.alliancecan.ca lists $c in an outage (${o:-maintenance}). No Duo push sent; re-checking quietly each hour." "low" "construction"
      note_add "$c"
      still="$still $c"
      continue
    fi
    if noted "$c"; then
      ntfy_push "BS45: $c is back" "$c cleared its outage — a Duo push follows in ${RETRY_NUDGE}s. Tap to approve." "high" "bell"
      note_del "$c"
    elif [ "$round" -ge 3 ]; then
      ntfy_push "BS45 Duo retry ${round}/${RETRY_MAX}: $c (low)" \
        "Third+ push for $c with no approval. If no Duo request reaches your phone, $c's Duo delivery is down — ignore these; I keep retrying hourly." "low" "bell"
    else
      ntfy_push "BS45 Duo retry ${round}/${RETRY_MAX}: $c" \
        "You missed the $c push earlier — a fresh one arrives in ${RETRY_NUDGE}s. Tap to approve (open Duo Mobile if no banner shows)." "high" "bell"
    fi
    sleep "$RETRY_NUDGE"
    if check_one "$c"; then reached="$reached $c"; else still="$still $c"; fi
  done
  missed="$still"
done

# --- phone summary ----------------------------------------------------------
# End the section at ANY next "--- … ---" header: checker_cmd.txt's rung label is
# EXPECTED to change (n=32 -> n=33 …); hardcoding it here would turn every
# post-climb run into a false "possible NEW hit" alert (validated 2026-07-12).
# Also end at a cluster banner, a retry line, or a missed-push line (2026-08-24:
# with hourly re-pushes those can directly follow a section).
hits="$(awk '/--- NEW FOUND\? ---/{f=1;next} /^--- |^════|^>> |^\(no approval/{f=0} f' "$OUT" \
        | grep -vE '^\(none yet\)$' | grep -E '\S' | sort -u)"
msg="reached:${reached:- none};"
if [ -n "$missed" ]; then
  if [ "$RETRY_MAX" -gt 0 ]; then msg="$msg missed:${missed} (gave up after $RETRY_MAX retry rounds);"
  else msg="$msg missed:${missed};"; fi
fi
if [ -n "$hits" ]; then
  ntfy_push "🚨 BS45 possible NEW hit" "NEW FOUND line(s): ${hits}. Verify with tools/verify_npaf.py before claiming." "urgent" "rotating_light"
  msg="$msg NEW HIT — verify!"
fi
ntfy_push "BS45 check done" "$msg" "default" "satellite"

echo ""
echo "Summary:$msg"
echo "Saved to results/latest_check.txt"
