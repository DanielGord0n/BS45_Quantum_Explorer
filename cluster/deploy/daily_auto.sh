#!/bin/bash
# daily_auto.sh — the autonomous 1pm loop.
#   check (you tap Duo) -> headless Claude interprets/acts/commits/pushes -> phone summary.
#
# Pieces:
#   check_all_retry.sh   collects status (auto-types the Duo "1"; you tap the push)
#   checker_cmd.txt      the remote checker command — the agent keeps this current
#   auto_prompt.md       unattended agent instructions (incl. the 2 rails)
#   next_seeds.sh        deterministic seed allocation (model never picks seeds)
#   duo_run.sh           how the agent SUBMITS jobs (plain ssh can't pass Duo)
#
# SAFETY / CONTROL:
#   * Kill switch: `touch cluster/deploy/AUTOPILOT_OFF` disables the run.
#   * Full transcript: results/auto_YYYY-MM-DD.log
#   * ⚠️ CLAUDE_ARGS below uses --dangerously-skip-permissions so the agent can run
#     duo_run.sh / git / bash unattended. That means it can run ANY command in this
#     repo context without asking. That is the price of a hands-off loop — read the
#     logs, and use the kill switch if anything looks wrong.

set -uo pipefail
DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO="$(cd "$DIR/../.." && pwd)"
cd "$REPO"

[ -f "$DIR/notify.conf" ] && . "$DIR/notify.conf"
NTFY_URL="${NTFY_URL:-}"
CLAUDE_BIN="${CLAUDE_BIN:-claude}"

# --- model: primary + fallback ----------------------------------------------
# If your CLI uses different identifiers, fix them HERE (one place).
MODEL_PRIMARY="${MODEL_PRIMARY:-claude-fable-5}"
MODEL_FALLBACK="${MODEL_FALLBACK:-claude-opus-4-8}"
# Permissions: the agent must run bash (duo_run.sh) and git unattended.
CLAUDE_ARGS="${CLAUDE_ARGS:---dangerously-skip-permissions}"

STAMP="$(date +%Y-%m-%d)"
LOG="$REPO/results/auto_${STAMP}.log"
CHECK_OUTPUT="$REPO/results/latest_check.txt"
SUMMARY="$REPO/results/last_summary.txt"
export CHECK_OUTPUT
mkdir -p "$REPO/results"

log() { echo "[$(date +%H:%M:%S)] $*" | tee -a "$LOG"; }
ntfy_push() {  # title message [priority] [tags]
  [ -z "$NTFY_URL" ] && return 0
  curl -s -m 15 -H "Title: ${1}" -H "Priority: ${3:-default}" -H "Tags: ${4:-satellite}" \
       -d "${2}" "$NTFY_URL" >/dev/null 2>&1
}

if [ -f "$DIR/AUTOPILOT_OFF" ]; then
  log "AUTOPILOT_OFF present — skipping."
  ntfy_push "BS45 autopilot OFF" "Kill switch on; skipped today's run." "low" "no_entry"
  exit 0
fi
if ! command -v "$CLAUDE_BIN" >/dev/null 2>&1; then
  log "ERROR: '$CLAUDE_BIN' not on PATH."
  ntfy_push "BS45 autopilot error" "Claude CLI not found on PATH; loop did not run." "high" "warning"
  exit 1
fi

# --- 0. self-heal exec bits -------------------------------------------------
# An editor/rewrite silently drops +x and the loop dies with exit 126 (this
# already bit duo_run.sh once). Cheap to just guarantee it every run.
chmod +x "$DIR/check_all_retry.sh" "$DIR/duo_run.sh" "$DIR/next_seeds.sh" 2>/dev/null

# --- 1. check ---------------------------------------------------------------
log "Running checker…"
"$DIR/check_all_retry.sh" > "$CHECK_OUTPUT" 2>>"$LOG"

if ! grep -q "BS45\|---" "$CHECK_OUTPUT" 2>/dev/null || ! grep -q "NEW FOUND" "$CHECK_OUTPUT"; then
  log "No cluster answered (no Duo taps). Aborting before the agent."
  ntfy_push "BS45: no Duo taps" "Nothing was reachable — approve the pushes and re-run." "default" "warning"
  exit 0
fi

# --- 2. pick a model (probe once, so we never run the real loop twice) -------
choose_model() {
  if "$CLAUDE_BIN" -p "reply with just: OK" --model "$MODEL_PRIMARY" >/dev/null 2>&1; then
    echo "$MODEL_PRIMARY"
  else
    echo "$MODEL_FALLBACK"
  fi
}
MODEL="$(choose_model)"
log "Model: $MODEL (primary=$MODEL_PRIMARY fallback=$MODEL_FALLBACK)"

# --- 3. the agent (runs exactly ONCE — a retry could double-submit jobs) -----
log "Invoking headless Claude…"
: > "$SUMMARY"
# shellcheck disable=SC2086
"$CLAUDE_BIN" -p "$(cat "$DIR/auto_prompt.md")" --model "$MODEL" $CLAUDE_ARGS >>"$LOG" 2>&1
rc=$?
log "Claude exited rc=$rc"

# --- 4. phone summary -------------------------------------------------------
if [ -s "$SUMMARY" ]; then
  msg="$(head -c 900 "$SUMMARY")"
else
  msg="Run finished (rc=$rc) but no summary was written — see results/auto_${STAMP}.log."
fi

if grep -qi '^NEEDS_HUMAN' "$SUMMARY" 2>/dev/null; then
  ntfy_push "⚠️ BS45 needs you" "$msg" "high" "warning"
elif grep -qiE 'verified|champion banked' "$SUMMARY" 2>/dev/null; then
  ntfy_push "🏆 BS45 — verified result!" "$msg" "urgent" "rotating_light,tada"
else
  ntfy_push "BS45 daily" "$msg" "default" "satellite"
fi
log "Done."
