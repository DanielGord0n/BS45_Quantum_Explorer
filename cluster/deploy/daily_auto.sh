#!/bin/bash
# daily_auto.sh — the fully autonomous 1pm loop.
#   check (you tap Duo) -> headless Claude interprets/acts/commits/pushes -> phone summary.
#
# Pieces:
#   check_all_retry.sh   collects status (retries Duo until you tap)
#   auto_prompt.md       the unattended agent instructions (with the 2 rails)
#   next_seeds.sh        deterministic seed allocation (model never picks seeds)
#
# SAFETY / CONTROL:
#   * Kill switch: `touch cluster/deploy/AUTOPILOT_OFF` to disable without
#     unloading launchd. Remove the file to re-enable.
#   * Everything is logged to results/auto_YYYY-MM-DD.log.
#   * Requires headless Claude CLI authenticated on this Mac (see AUTOMATION.md).
#
# Env you may override: CLAUDE_BIN, CLAUDE_ARGS.

set -uo pipefail
DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO="$(cd "$DIR/../.." && pwd)"
cd "$REPO"

[ -f "$DIR/notify.conf" ] && . "$DIR/notify.conf"
NTFY_URL="${NTFY_URL:-}"
CLAUDE_BIN="${CLAUDE_BIN:-claude}"
# Headless + non-interactive tool use. Adjust in AUTOMATION.md to taste.
CLAUDE_ARGS="${CLAUDE_ARGS:---permission-mode acceptEdits}"

STAMP="$(date +%Y-%m-%d)"
LOG="$REPO/results/auto_${STAMP}.log"
CHECK_OUTPUT="$REPO/results/latest_check.txt"
SUMMARY="$REPO/results/last_summary.txt"
export CHECK_OUTPUT
mkdir -p "$REPO/results"

log() { echo "[$(date +%H:%M:%S)] $*" | tee -a "$LOG"; }
ntfy_push() {  # title message priority tags
  [ -z "$NTFY_URL" ] && return 0
  curl -s -H "Title: ${1}" -H "Priority: ${3:-default}" -H "Tags: ${4:-satellite}" \
       -d "${2}" "$NTFY_URL" >/dev/null 2>&1
}

# --- kill switch ------------------------------------------------------------
if [ -f "$DIR/AUTOPILOT_OFF" ]; then
  log "AUTOPILOT_OFF present — skipping autonomous run."
  ntfy_push "BS45 autopilot OFF" "Kill switch is on; skipped today's run." "low" "no_entry"
  exit 0
fi

# --- 1. check (gentle retry so it nudges, not spams) ------------------------
log "Running checker…"
RETRY_INTERVAL="${RETRY_INTERVAL:-600}" MAX_WAIT="${MAX_WAIT:-5400}" \
  "$DIR/check_all_retry.sh" > "$CHECK_OUTPUT" 2>>"$LOG"

if grep -q "UNREACHABLE" "$CHECK_OUTPUT" && ! grep -qE "^[[:space:]]*[0-9]" "$CHECK_OUTPUT"; then
  log "No clusters answered (no Duo taps). Aborting before agent."
  ntfy_push "BS45: no Duo taps" "Nothing was reachable today — approve the pushes and re-run when you can." "default" "warning"
  exit 0
fi

# --- 2. headless agent ------------------------------------------------------
log "Invoking headless Claude…"
: > "$SUMMARY"
if ! command -v "$CLAUDE_BIN" >/dev/null 2>&1; then
  log "ERROR: '$CLAUDE_BIN' not found on PATH. See AUTOMATION.md."
  ntfy_push "BS45 autopilot error" "Claude CLI not found on PATH; loop did not run. Results collected only." "high" "warning"
  exit 1
fi

# shellcheck disable=SC2086
"$CLAUDE_BIN" -p "$(cat "$DIR/auto_prompt.md")" $CLAUDE_ARGS >>"$LOG" 2>&1
rc=$?
log "Claude exited rc=$rc"

# --- 3. phone summary -------------------------------------------------------
if [ -s "$SUMMARY" ]; then
  msg="$(head -c 900 "$SUMMARY")"
else
  msg="Run finished (rc=$rc) but no summary was written — check results/auto_${STAMP}.log."
fi

if grep -qi '^NEEDS_HUMAN' "$SUMMARY" 2>/dev/null; then
  ntfy_push "⚠️ BS45 needs you" "$msg" "high" "warning"
elif grep -qiE 'verified|record|champion banked' "$SUMMARY" 2>/dev/null; then
  ntfy_push "🏆 BS45 — verified result!" "$msg" "urgent" "rotating_light,tada"
else
  ntfy_push "BS45 daily" "$msg" "default" "satellite"
fi
log "Done."
