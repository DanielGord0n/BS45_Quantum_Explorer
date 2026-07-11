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
# skipped (rather than re-pushing at you). Just re-run to pick it up.
#
# Requires: python3 (ships with macOS Command Line Tools).
# Usage:  ./cluster/deploy/check_all_retry.sh
# Tunables (env): PUSH_WAIT (default 180s), CLUSTER_USER.

set -uo pipefail
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
[ -f "$SCRIPT_DIR/notify.conf" ] && . "$SCRIPT_DIR/notify.conf"
NTFY_URL="${NTFY_URL:-}"

USER_ID="${CLUSTER_USER:-dangord}"
CLUSTERS="fir nibi rorqual trillium"
PUSH_WAIT="${PUSH_WAIT:-180}"     # seconds to wait for you to tap each push
DUO="$SCRIPT_DIR/duo_ssh.py"
# Banked-solution IDs hidden from "NEW FOUND?" — keep in sync with HANDOFF.
EXCLUDE="46274622_4|14923090_[26]|16945067_3"

mkdir -p "$REPO_ROOT/results"
OUT="$REPO_ROOT/results/latest_check.txt"
: > "$OUT"

ntfy_push() {  # title message [priority] [tags]
  [ -z "$NTFY_URL" ] && return 0
  curl -s -m 15 -H "Title: ${1}" -H "Priority: ${3:-default}" -H "Tags: ${4:-satellite}" \
       -d "${2}" "$NTFY_URL" >/dev/null 2>&1
}

INNER='squeue -u '"$USER_ID"' -h -o "%.14i %.10j %.2t %.11L %R" 2>/dev/null; cd $SCRATCH/bs45 2>/dev/null || exit 0; echo "--- NEW FOUND? ---"; grep -l "FOUND" sa_ladder_*.txt 2>/dev/null | grep -vE "'"$EXCLUDE"'" || echo "(none yet)"; echo "--- n=32 progress ---"; for f in $(ls -t sa_ladder_*.txt 2>/dev/null | head -3); do hdr=$(grep -oE "BS\([0-9]+,[0-9]+\)" "$f" | head -1); best=$(grep -oE "bestAB=[0-9]+" "$f" | sort -t= -k2 -n | head -1 | grep -oE "[0-9]+$"); echo "$(basename $f) [$hdr] bestAB_min=$best | $(tail -1 "$f" | cut -c1-55)"; done; echo "--- GATE PROBES ---"; for f in $(ls -t wz_match_output_*.txt 2>/dev/null | head -2); do echo "=== $f ==="; grep -A5 "SUMMARY (n=" "$f" || tail -3 "$f"; done'
REMOTE='echo ===BS45BEGIN===; '"$INNER"'; echo ===BS45END==='

ntfy_push "BS45 check starting" "4 Duo pushes coming one at a time — tap each to approve." "low" "hourglass"

reached=""
missed=""
for c in $CLUSTERS; do
  echo ""                                    | tee -a "$OUT"
  echo "════════════════ $c ════════════════" | tee -a "$OUT"
  echo ">> [$c] Duo push sent — approve on your phone (waiting up to ${PUSH_WAIT}s)…"
  raw="$(python3 "$DUO" "${USER_ID}@${c}.alliancecan.ca" "$REMOTE" "$PUSH_WAIT" 2>/dev/null)"
  # tr -d '\r': the pty turns every newline into CRLF; strip the CRs or every
  # downstream match (incl. the "(none yet)" filter) silently fails.
  body="$(printf '%s' "$raw" | tr -d '\r' | awk '/===BS45BEGIN===/{f=1;next} /===BS45END===/{f=0} f')"
  if [ -n "$body" ]; then
    printf '%s\n' "$body" | tee -a "$OUT"
    reached="$reached $c"
  else
    echo "(no approval within ${PUSH_WAIT}s — skipped; re-run to retry $c)" | tee -a "$OUT"
    missed="$missed $c"
  fi
done

# --- phone summary ----------------------------------------------------------
hits="$(awk '/--- NEW FOUND\? ---/{f=1;next} /--- n=32 progress ---/{f=0} f' "$OUT" \
        | grep -vE '^\(none yet\)$' | grep -E '\S' | sort -u)"
msg="reached:${reached:- none};"
[ -n "$missed" ] && msg="$msg missed:${missed};"
if [ -n "$hits" ]; then
  ntfy_push "🚨 BS45 possible NEW hit" "NEW FOUND line(s): ${hits}. Verify with tools/verify_npaf.py before claiming." "urgent" "rotating_light"
  msg="$msg NEW HIT — verify!"
fi
ntfy_push "BS45 check done" "$msg" "default" "satellite"

echo ""
echo "Summary:$msg"
echo "Saved to results/latest_check.txt"
