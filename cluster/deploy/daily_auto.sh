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
# SUPPLEMENTARY=1 (2026-08-24): a second pass, spawned by the main run, that
# re-pushes Duo every hour for the cluster(s) missed at 1pm and — once approved —
# reads/restacks ONLY those. Own log file; never spawns another supplementary.
SUPPLEMENTARY="${SUPPLEMENTARY:-0}"
LOG="$REPO/results/auto_${STAMP}.log"
[ "$SUPPLEMENTARY" = 1 ] && LOG="$REPO/results/auto_${STAMP}_supp.log"
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
ALL_CLUSTERS="fir nibi rorqual trillium"
if [ "$SUPPLEMENTARY" = 1 ]; then
  log "SUPPLEMENTARY pass for missed cluster(s): ${CLUSTERS:-?} — hourly Duo re-push (RETRY_MAX=${RETRY_MAX:-10})."
  "$DIR/check_all_retry.sh" > "$CHECK_OUTPUT" 2>>"$LOG"          # CLUSTERS from env; hourly retries inside
else
  log "Running checker (first pass, no waiting on missed taps)…"
  RETRY_MAX=0 "$DIR/check_all_retry.sh" > "$CHECK_OUTPUT" 2>>"$LOG"
fi
# Which clusters missed their push? Parsed from the checker's own Summary line.
MISSED="$(grep -m1 '^Summary:' "$CHECK_OUTPUT" 2>/dev/null | sed -n 's/.*missed:\([^;(]*\).*/\1/p' | xargs)"
if ! grep -q "NEW FOUND" "$CHECK_OUTPUT" 2>/dev/null; then
  if [ "$SUPPLEMENTARY" = 1 ]; then
    log "Supplementary pass: no approval after the hourly retries — giving up for today."
    ntfy_push "BS45: gave up on ${CLUSTERS:-?}" \
      "No Duo approval after the hourly retries. ${CLUSTERS:-?} stays unread today (the clusters keep computing). When you can: CLUSTERS=\"${CLUSTERS:-?}\" ./cluster/deploy/check_all_retry.sh" \
      "default" "warning"
    exit 0
  fi
  log "No cluster answered on the first pass — switching to hourly Duo re-push for all of them."
  ntfy_push "BS45: no Duo taps yet" "Nothing was reachable on the first pass. I'll re-push every hour (up to 10 h) — tap when you see it." "default" "hourglass"
  SUPPLEMENTARY=1 CLUSTERS="${CLUSTERS:-$ALL_CLUSTERS}" exec "$DIR/daily_auto.sh"
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

# Early-verdict ping (2026-07-26): the agent writes results/interim_summary.txt
# with a 2-4 sentence verdict as its FIRST action after reading the checker.
# This watcher pushes it to the phone within seconds — long build/validate
# sessions after interpretation (an hour+ is normal and productive) no longer
# leave Daniel waiting blind between "check done" and the full summary.
INTERIM="$REPO/results/interim_summary.txt"
rm -f "$INTERIM"
(
  for _ in $(seq 1 180); do   # watch up to 15 min
    if [ -s "$INTERIM" ]; then
      ntfy_push "BS45 early read" "$(cat "$INTERIM")" "default" "satellite"
      exit 0
    fi
    sleep 5
  done
) &
INTERIM_PID=$!

PROMPT="$(cat "$DIR/auto_prompt.md")"
if [ "$SUPPLEMENTARY" = 1 ]; then
  PROMPT="SUPPLEMENTARY READ (${STAMP}): today's main run already happened and handled every other cluster (see HANDOFF's newest entry and results/last_summary.txt). Only these cluster(s) were re-checked after missed Duo pushes and appear in \$CHECK_OUTPUT: ${CLUSTERS:-?}. Interpret, bookkeep, and (if idle) restack ONLY them. Do not touch or re-summarize the others. Append a short '(supplementary)' entry to HANDOFF instead of rewriting today's main entry.

$PROMPT"
fi

MODEL="$MODEL_PRIMARY"
attempt=1
rc=1
PARTIAL=0
SUBMITS_THIS_RUN=0
while : ; do
  log "Invoking headless Claude (model=$MODEL, attempt $attempt/$((MAX_RETRY+1)))…"
  # shellcheck disable=SC2086
  "$CLAUDE_BIN" -p "$PROMPT" --model "$MODEL" $CLAUDE_ARGS >>"$LOG" 2>&1
  rc=$?
  log "Claude exited rc=$rc"

  # Success: it wrote a summary. Done.
  [ "$rc" -eq 0 ] && [ -s "$SUMMARY" ] && break

  # ---- FALL BACK TO THE OTHER MODEL ----------------------------------------
  # 07-15 lesson: credits are PER-MODEL, not account-wide. Fable was out of credits
  # while Opus still worked fine — but the old code only fell back on "model not
  # found/unavailable", so a credit-exhausted primary just killed the day instead of
  # running on the fallback. Any primary-model blocker (unavailable OR out of
  # credits/limit) now falls back, as long as the agent provably did nothing yet.
  if [ "$MODEL" = "$MODEL_PRIMARY" ] && did_nothing \
     && { limit_hit || tail -25 "$LOG" | grep -qiE "model.*(not found|unavailable|invalid|unknown)"; }; then
    log "Primary '$MODEL_PRIMARY' blocked ($(limit_reset_note 2>/dev/null || echo 'unavailable')) — falling back to '$MODEL_FALLBACK'."
    ntfy_push "BS45 — falling back to $MODEL_FALLBACK" \
      "$MODEL_PRIMARY blocked ($(limit_reset_note)). Running on $MODEL_FALLBACK instead." "low" "arrows_counterclockwise"
    MODEL="$MODEL_FALLBACK"
    continue
  fi

  # Credits exhausted on the FALLBACK too (i.e. everything is out) -> stop cleanly;
  # a 30-min backoff cannot fix a multi-day reset.
  if credits_gone; then
    log "OUT OF CREDITS on both '$MODEL_PRIMARY' and '$MODEL_FALLBACK' ($(limit_reset_note)) — not retrying."
    ntfy_push "BS45 blocked — out of credits" \
      "Both models are out of usage credits ($(limit_reset_note)). Nothing was submitted; clusters keep computing. Re-run ./cluster/deploy/daily_auto.sh once credits reset." \
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
  # 2026-08-24: this branch used to fire ONLY on a usage limit. The 08-23 run died on
  # "API Error: Connection closed mid-response" after 3.5 h with a Rorqual restack
  # already done and NOTHING committed -> fell through to a bare "no summary" text and
  # the whole cycle's bookkeeping was lost until the next day reconstructed it.
  # Now: any death after the agent acted -> preserve its edits as a PARTIAL commit,
  # count the submit echoes in this run's log, and say so on the phone.
  if ! did_nothing; then
    PARTIAL=1
    SUBMITS_THIS_RUN=$(grep -c "Submitted batch job" "$LOG" 2>/dev/null || echo 0)
    if limit_hit; then why="Usage limit hit mid-run"; else why="Agent died mid-run (rc=$rc, e.g. API connection dropped)"; fi
    log "$why after the agent had ALREADY acted (summary/commit/tree change present)."
    log "NOT retrying — a retry could double-submit. Human review required."
    if [ -n "$(git status --porcelain 2>/dev/null)" ]; then
      if git add -A && git commit -q -m "auto: PARTIAL run ${STAMP} (rc=$rc) — agent edits preserved for review; ${SUBMITS_THIS_RUN} submit echo(es) in log"; then
        log "Preserved uncommitted agent edits in a PARTIAL commit."
        # bounded push (no coreutils timeout on macOS; osxkeychain can hang headless)
        python3 - >>"$LOG" 2>&1 <<'PY' || log "push of PARTIAL commit did not complete — run git push origin main at the Mac"
import subprocess, sys
try:
    subprocess.run(["git","push","-q","origin","main"], timeout=60, check=True)
except Exception:
    sys.exit(1)
PY
      else
        log "Could not commit partial edits (see git status)."
      fi
    fi
    ntfy_push "⚠️ BS45 — partial run, needs you" \
      "$why after the agent had already acted. NOT retried (double-submit risk). ${SUBMITS_THIS_RUN} 'Submitted batch job' echo(es) in this run's log; bookkeeping edits were committed as PARTIAL. Check results/auto_${STAMP}.log and squeue." \
      "high" "warning"
  fi
  break
done
kill "$INTERIM_PID" 2>/dev/null || true

# --- 3. phone summary -------------------------------------------------------
if [ -s "$SUMMARY" ]; then
  msg="$(head -c 900 "$SUMMARY")"
elif credits_gone; then
  msg="BLOCKED — out of Claude usage credits ($(limit_reset_note)). Nothing was submitted, nothing changed; the clusters keep computing regardless. Re-run ./cluster/deploy/daily_auto.sh once credits reset."
elif limit_hit; then
  msg="BLOCKED by the Claude usage limit after $attempt attempt(s). Nothing was submitted, nothing changed. Re-run ./cluster/deploy/daily_auto.sh once your limit resets."
elif [ "${PARTIAL:-0}" = 1 ]; then
  msg="PARTIAL run (rc=$rc): the agent acted (${SUBMITS_THIS_RUN:-0} submit echo(es) logged) but died before writing a summary. Its bookkeeping edits were committed as PARTIAL. Review results/auto_${STAMP}.log + squeue; the next loop folds this cycle in."
else
  msg="Run finished (rc=$rc) but no summary was written — see results/auto_${STAMP}.log."
fi

if grep -qi '^NEEDS_HUMAN' "$SUMMARY" 2>/dev/null; then
  ntfy_push "⚠️ BS45 needs you" "$msg" "high" "warning"
elif grep -qE '^RESULT_BANKED' "$SUMMARY" 2>/dev/null; then
  # Sentinel contract (2026-08-05): the agent writes a line starting exactly
  # RESULT_BANKED only when a solution passed verify_npaf and was banked THIS
  # run. The old substring match ('verified') fired on "no verified solutions"
  # -- a false trophy that trains the reader to ignore the real one.
  ntfy_push "🏆 BS45 — verified result!" "$msg" "urgent" "rotating_light,tada"
else
  if [ "$SUPPLEMENTARY" = 1 ]; then ntfy_push "BS45 supplementary (${CLUSTERS:-?})" "$msg" "default" "satellite"
  else ntfy_push "BS45 daily" "$msg" "default" "satellite"; fi
fi

# --- 4. hourly re-push for clusters missed at 1pm (2026-08-24) --------------
# Runs AFTER the main agent pass, so today's read was never delayed. Sequential:
# the supplementary agent only starts once this pass is completely done.
if [ "$SUPPLEMENTARY" != 1 ] && [ -n "$MISSED" ]; then
  log "Missed Duo on: ${MISSED} — starting hourly re-push + supplementary read."
  ntfy_push "BS45: will retry ${MISSED}" \
    "You missed the Duo push(es) for ${MISSED}. I'll re-push every hour (up to 10 h) and read/restack as soon as you approve." "default" "hourglass"
  SUPPLEMENTARY=1 CLUSTERS="$MISSED" "$DIR/daily_auto.sh"
fi
log "Done."
