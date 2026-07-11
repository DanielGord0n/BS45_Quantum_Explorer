#!/bin/bash
# check_all_retry.sh — same output as check_all.sh, but you don't have to catch
# all four Duo pushes at once, and it can push status to your phone via ntfy.
#
# Difference from check_all.sh:
#   * ONE push per cluster, on a persistent SSH master connection. Approve
#     whenever you notice — each cluster keeps re-pushing until you tap (or until
#     MAX_WAIT). Miss the moment and it just tries again; no lost run.
#   * The master connection stays open for CONTROL_PERSIST after the check, so a
#     follow-up refill (sbatch over the same host) reuses it with NO new push.
#   * Phone notifications via ntfy (optional): a nudge when it starts, a one-line
#     status when it finishes, and a LOUD alert if a NEW solution banner appears.
#   * Desktop notification too, if osascript/notify-send is available.
#
# The remote command and output format are IDENTICAL to check_all.sh, so the
# bs45-campaign skill reads the result the same way.
#
# Phone setup: put your ntfy topic URL in cluster/deploy/notify.conf, e.g.
#     NTFY_URL="https://ntfy.sh/bs45-dangord-ddc7290c22a8"
# (that file is git-ignored). Leave it unset to disable phone pushes.
#
# Usage:  ./cluster/deploy/check_all_retry.sh
# Tunables (env): RETRY_INTERVAL, MAX_WAIT, CONTROL_PERSIST, PUSH_TIMEOUT.

set -uo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
[ -f "$SCRIPT_DIR/notify.conf" ] && . "$SCRIPT_DIR/notify.conf"
NTFY_URL="${NTFY_URL:-}"

USER_ID="${CLUSTER_USER:-dangord}"
RETRY_INTERVAL="${RETRY_INTERVAL:-180}"   # re-push every 3 min if not approved
MAX_WAIT="${MAX_WAIT:-7200}"              # give up on a cluster after 2 h
CONTROL_PERSIST="${CONTROL_PERSIST:-600}" # keep connection open 10 min for refills
PUSH_TIMEOUT="${PUSH_TIMEOUT:-70}"        # seconds to wait for one approval

CTRL_DIR="$(mktemp -d "${TMPDIR:-/tmp}/bs45mux.XXXXXX")"
TMPDIR_OUT="$(mktemp -d)"
FOUND_STATE="$REPO_ROOT/results/.ntfy_found_state"
trap 'rm -rf "$TMPDIR_OUT"' EXIT   # NOTE: master connections are left open on purpose

# --- notifications ----------------------------------------------------------
desktop_notify() {
  if command -v osascript >/dev/null 2>&1; then
    osascript -e "display notification \"$1\" with title \"BS45 cluster check\"" >/dev/null 2>&1
  elif command -v notify-send >/dev/null 2>&1; then
    notify-send "BS45 cluster check" "$1" >/dev/null 2>&1
  fi
}
# ntfy_push TITLE MESSAGE [PRIORITY] [TAGS]
ntfy_push() {
  [ -z "$NTFY_URL" ] && return 0
  curl -s \
    -H "Title: ${1}" \
    -H "Priority: ${3:-default}" \
    -H "Tags: ${4:-satellite}" \
    -d "${2}" "$NTFY_URL" >/dev/null 2>&1
}

ssh_base() {
  ssh -o ControlMaster=auto -o ControlPersist="$CONTROL_PERSIST" \
      -o ControlPath="$CTRL_DIR/%r@%h:%p" \
      -o ConnectTimeout="$PUSH_TIMEOUT" "$@"
}

# Remote status command — kept identical to check_all.sh.
remote_cmd() {
  local name="$1"
  cat <<REMOTE
    echo '=== QUEUE ==='
    squeue -u ${USER_ID} --format='%10i %20j %2t %12L' 2>/dev/null
    echo ''
    echo '=== LATEST OUTPUT (tail 8) ==='
    cd \$SCRATCH/bs45 2>/dev/null || exit 0
    for f in \$(ls -t bs43_v8_${name}_output_*.txt 2>/dev/null | head -4); do
      echo "--- \$f ---"
      tail -8 "\$f"
    done
    echo ''
    echo '=== SOLUTIONS FOUND ==='
    grep -rl 'REPRODUCTION CONFIRMED\|WORLD RECORD' \$SCRATCH/bs45/ 2>/dev/null || echo '(none yet)'
REMOTE
}

open_conn() {
  local host="$1" deadline=$(( $(date +%s) + MAX_WAIT ))
  while :; do
    if ssh_base -f -N "$host" 2>/dev/null; then return 0; fi
    if (( $(date +%s) >= deadline )); then return 1; fi
    sleep "$RETRY_INTERVAL"
  done
}

check_cluster() {
  local name="$1" host="$2" out="$TMPDIR_OUT/$name.txt"
  if open_conn "$host"; then
    ssh_base "$host" "$(remote_cmd "$name")" > "$out" 2>&1
  else
    echo "UNREACHABLE — no Duo approval within ${MAX_WAIT}s. Re-run to retry $name." > "$out"
  fi
}

# --- run --------------------------------------------------------------------
echo "Sending Duo pushes for all clusters — tap whenever you notice (each retries every ${RETRY_INTERVAL}s)..."
desktop_notify "Duo pushes sent — approve on your phone"
ntfy_push "BS45 check running" "Approve the 4 Duo pushes on your phone when you can." "low" "hourglass"
echo ""

check_cluster fir      "${USER_ID}@fir.alliancecan.ca"      &
check_cluster rorqual  "${USER_ID}@rorqual.alliancecan.ca"  &
check_cluster nibi     "${USER_ID}@nibi.alliancecan.ca"     &
check_cluster trillium "${USER_ID}@trillium.alliancecan.ca" &
wait

for name in fir rorqual nibi trillium; do
  echo ""
  echo "════════════════════════════════════════"
  echo "  CLUSTER: ${name^^}"
  echo "════════════════════════════════════════"
  cat "$TMPDIR_OUT/$name.txt"
done

# --- build phone summary (factual only; interpretation is /daily-loop's job) --
summary=""
for name in fir rorqual nibi trillium; do
  f="$TMPDIR_OUT/$name.txt"
  if grep -q "UNREACHABLE" "$f"; then
    summary+="${name}: no-tap; "
    continue
  fi
  q=$(awk '/=== QUEUE ===/{f=1;next} /=== LATEST/{f=0} f' "$f" | grep -cE '^[[:space:]]*[0-9]')
  if [ "$q" -eq 0 ]; then summary+="${name}: IDLE; "; else summary+="${name}: ${q} jobs; "; fi
done

# --- NEW-solution detection (diff against last run; avoids daily false alarms) -
current_found="$(for name in fir rorqual nibi trillium; do
  awk '/=== SOLUTIONS FOUND ===/{f=1;next} f' "$TMPDIR_OUT/$name.txt"
done | grep -E '/' | grep -v '(none yet)' | sort -u)"
new_found=""
if [ -n "$current_found" ]; then
  if [ -f "$FOUND_STATE" ]; then
    new_found="$(comm -13 <(sort -u "$FOUND_STATE") <(printf '%s\n' "$current_found"))"
  else
    new_found="$current_found"   # first run ever: treat existing as baseline, don't alert
    printf '%s\n' "$current_found" > "$FOUND_STATE"
    new_found=""
  fi
fi
printf '%s\n' "$current_found" > "$FOUND_STATE"

echo ""
echo "Summary: ${summary}"
if [ -n "$new_found" ]; then
  echo "*** NEW solution banner(s): ${new_found} — VERIFY with tools/verify_npaf.py before claiming. ***"
  ntfy_push "🚨 BS45 NEW BANNER — verify now" "New FOUND file(s): ${new_found}. Run verify_npaf.py before claiming." "urgent" "rotating_light,tada"
fi
ntfy_push "BS45 status" "${summary}${new_found:+ | NEW BANNER — verify!}" "default" "satellite"
desktop_notify "Cluster check complete"
echo "(Connections stay open ${CONTROL_PERSIST}s — refills to these hosts won't re-push.)"
