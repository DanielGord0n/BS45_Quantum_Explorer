#!/bin/bash
# daily_auto.sh — the autonomous 1pm loop.
#   check (you tap Duo) -> headless Claude interprets/acts/commits/pushes -> phone summary.
#
# 2026-07-12 changes (both from the 07-12 failure, where the run died at the session limit):
#   * SESSION-LIMIT RETRY. Hitting the Claude usage/session limit no longer kills the day.
#     The run WAITS and retries, and only when it is provably safe to do so (see below).
#   * MODEL PROBE REMOVED. The old choose_model() probe burned ~88 MINUTES on 07-12
#     (13:03 -> 14:31) and spent quota just to ask "which model?". We now run the real
#     agent directly and fall back only on a genuine model-unavailable error.
#
# RETRY SAFETY (this is the important part — a blind retry could DOUBLE-SUBMIT jobs):
#   We retry ONLY if the agent provably did NOTHING:
#       (a) the log shows a usage/session/rate limit, AND
#       (b) no summary file was written, AND
#       (c) git HEAD is unchanged (it committed nothing).
#   If any of those fail, we do NOT retry — a partial run must be inspected by a human.
#
# SAFETY / CONTROL:
#   * Kill switch: `touch cluster/deploy/AUTOPILOT_OFF`
#   * Full transcript: results/auto_YYYY-MM-DD.log
#   * ⚠️ CLAUDE_ARGS uses --dangerously-skip-permissions so the agent can run duo_run.sh /
#     git / bash unattended. It can run ANY command in this repo context. Read the logs.

set -uo pipefail
DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO="$(cd "$DIR/../.." && pwd)"
cd "$REPO"

[ -f "$DIR/notify.conf" ] && . "$DIR/notify.conf"
NTFY_URL="${NTFY_URL:-}"
CLAUDE_BIN="${CLAUDE_BIN:-claude}"

MODEL_PRIMARY="${MODEL_PRIMARY:-claude-fable-5}"
MODEL_FALLBACK="${MODEL_FALLBACK:-claude-opus-4-8}"
CLAUDE_ARGS="${CLAUDE_ARGS:---dangerously-skip-permissions}"

# Session-limit retry policy (portable: fixed backoff, no fragile date parsing).
RETRY_WAIT="${RETRY_WAIT:-1800}"     # 30 min between attempts
MAX_RETRY="${MAX_RETRY:-8}"          # 8 x 30min = up to 4h of waiting

STAMP="$(date +%Y-%m-%d)"
LOG="$REPO/results/auto_${STAMP}.log"
CHECK_OUTPUT="$REPO/results/latest_check.txt"
SUMMARY="$REPO/results/last_summary.txt"
export CHECK_OUTPUT
mkdir -p "$REPO/results"

log() { echo "[$(date +%H:%M:%S)] $*" | tee -a "$LOG"; }
ntfy_push() {
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
  ntfy_push "BS45 autopilot error" "Claude CLI not found on PATH." "high" "warning"
  exit 1
fi

chmod +x "$DIR/check_all_retry.sh" "$DIR/duo_run.sh" "$DIR/next_seeds.sh" \
         "$DIR/rung_status.sh" 2>/dev/null

# --- 1. check ---------------------------------------------------------------
log "Running checker…"
"$DIR/check_all_retry.sh" > "$CHECK_OUTPUT" 2>>"$LOG"
if ! grep -q "NEW FOUND" "$CHECK_OUTPUT" 2>/dev/null; then
  log "No cluster answered (no Duo taps). Aborting before the agent."
  ntfy_push "BS45: no Duo taps" "Nothing was reachable — approve the pushes and re-run." "default" "warning"
  exit 0
fi

# --- 2. agent, with session-limit deferral ----------------------------------
HEAD_BEFORE="$(git rev-parse HEAD 2>/dev/null || echo none)"
TREE_BEFORE="$(git status --porcelain 2>/dev/null)"
: > "$SUMMARY"

limit_hit() {   # did the LAST attempt die on a usage/session/rate limit?
  # 07-15: the real wording is "You're out of usage credits · resets Jul 17 at 11pm".
  # The old pattern said "out of (credit|quota)" and MISSED it (plural + "usage" in the
  # middle), so the retry never engaged and the phone got a useless "no summary" text.
  # Keep this loose: match the shapes, not one exact sentence.
  tail -25 "$LOG" | grep -qiE "session limit|usage limit|rate limit|quota exceeded|out of .*(credit|quota)|hit your (usage|session) limit|insufficient (credit|quota)"
}
# A hard CREDIT exhaustion (days, not hours) is different from a rolling session limit:
# retrying every 30 min for 4h cannot help. Detect it and stop cleanly.
credits_gone() {
  tail -25 "$LOG" | grep -qiE "out of .*credit|insufficient credit"
}
limit_reset_note() {   # pull the "resets ..." text so the phone says something useful
  tail -25 "$LOG" | grep -oiE "resets [^·]*" | head -1
}
did_nothing() { # provably safe to retry: no summary, no commit, no working-tree change.
  # The tree check matters: taking seeds (next_seeds.sh) or advancing the rung
  # ledger dirties tracked files WITHOUT moving HEAD — an agent that died mid-run
  # after a duo_run.sh submit would otherwise look like "did nothing" and get
  # retried into a double-submit.
  [ ! -s "$SUMMARY" ] \
    && [ "$(git rev-parse HEAD 2>/dev/null || echo none)" = "$HEAD_BEFORE" ] \
    && [ "$(git status --porcelain 2>/dev/null)" = "$TREE_BEFORE" ]
}

MODEL="$MODEL_PRIMARY"
attempt=1
rc=1
while : ; do
  log "Invoking headless Claude (model=$MODEL, attempt $attempt/$((MAX_RETRY+1)))…"
  # shellcheck disable=SC2086
  "$CLAUDE_BIN" -p "$(cat "$DIR/auto_prompt.md")" --model "$MODEL" $CLAUDE_ARGS >>"$LOG" 2>&1
  rc=$?
  log "Claude exited rc=$rc"

  # Success: it wrote a summary. Done.
  [ "$rc" -eq 0 ] && [ -s "$SUMMARY" ] && break

  # Model unavailable (NOT a limit) -> swap to the fallback once, retry immediately.
  if [ "$MODEL" = "$MODEL_PRIMARY" ] && ! limit_hit \
     && tail -25 "$LOG" | grep -qiE "model.*(not found|unavailable|invalid|unknown)"; then
    log "Primary model '$MODEL_PRIMARY' unavailable — switching to '$MODEL_FALLBACK'."
    MODEL="$MODEL_FALLBACK"
    continue
  fi

  # Credits exhausted (resets in DAYS) -> retrying for 4h is pointless. Stop cleanly.
  if credits_gone; then
    log "OUT OF CREDITS ($(limit_reset_note)) — not retrying; a 4h backoff cannot fix a multi-day reset."
    ntfy_push "BS45 blocked — out of credits" \
      "Agent could not run: out of usage credits ($(limit_reset_note)). Nothing was submitted; clusters keep computing. Re-run ./cluster/deploy/daily_auto.sh once credits reset." \
      "default" "no_entry"
    break
  fi

  # Rolling session/usage limit AND the agent provably did nothing -> defer and retry.
  if limit_hit && did_nothing && [ "$attempt" -le "$MAX_RETRY" ]; then
    log "USAGE LIMIT hit and the agent did nothing (no summary, HEAD unchanged) — safe to retry."
    log "Waiting ${RETRY_WAIT}s, then attempt $((attempt+1))."
    ntfy_push "BS45 deferred — usage limit" \
      "Agent blocked by the Claude usage limit. Nothing was submitted. Retrying in $((RETRY_WAIT/60)) min (attempt $((attempt+1))/$((MAX_RETRY+1)))." \
      "low" "hourglass"
    sleep "$RETRY_WAIT"
    attempt=$((attempt+1))
    continue
  fi

  # Anything else (incl. a PARTIAL run) -> stop. A human must look.
  if limit_hit && ! did_nothing; then
    log "USAGE LIMIT hit but the agent had ALREADY acted (summary or commit present)."
    log "NOT retrying — a retry could double-submit. Human review required."
    ntfy_push "⚠️ BS45 — partial run, needs you" \
      "Usage limit hit mid-run after the agent had already acted. NOT retried (double-submit risk). Check results/auto_${STAMP}.log and squeue." \
      "high" "warning"
  fi
  break
done

# --- 3. phone summary -------------------------------------------------------
if [ -s "$SUMMARY" ]; then
  msg="$(head -c 900 "$SUMMARY")"
elif credits_gone; then
  msg="BLOCKED — out of Claude usage credits ($(limit_reset_note)). Nothing was submitted, nothing changed; the clusters keep computing regardless. Re-run ./cluster/deploy/daily_auto.sh once credits reset."
elif limit_hit; then
  msg="BLOCKED by the Claude usage limit after $attempt attempt(s). Nothing was submitted, nothing changed. Re-run ./cluster/deploy/daily_auto.sh once your limit resets."
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
