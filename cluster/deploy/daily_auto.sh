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
# Start stamp BEFORE anything that could stall (2026-09-24: the 1pm run's first log line
# was 13:21:43 with the Mac awake; no record showed whether launchd fired late or the
# script hung before logging). The gap to the first auto_*.log line answers that.
mkdir -p "$REPO/results"
echo "$(date '+%F %T') start pid=$$ ppid=$PPID button=${BUTTON:-0} supp=${SUPPLEMENTARY:-0}" >> "$REPO/results/loop_starts.log"

[ -f "$DIR/notify.conf" ] && . "$DIR/notify.conf"
NTFY_URL="${NTFY_URL:-}"
# Newest Claude Code CLI available (2026-09-23): the Homebrew cask does not self-update
# (it sat at 2.1.187, which rejects Fable 5.1 / Opus 5.5 and maps its aliases to older
# models), while the VS Code extension's bundled CLI auto-updates. Pick whichever
# candidate reports the highest version on every run. Override with CLAUDE_BIN=...
pick_claude() {
  local best="" bestv="" c v
  for c in /opt/homebrew/bin/claude /usr/local/bin/claude "$HOME/.local/bin/claude" "$HOME/.claude/local/claude" \
           $(ls -d "$HOME"/.vscode/extensions/anthropic.claude-code-*/resources/native-binary/claude \
                   "$HOME"/.cursor/extensions/anthropic.claude-code-*/resources/native-binary/claude 2>/dev/null); do
    [ -x "$c" ] || continue
    v="$("$c" --version 2>/dev/null | awk '{print $1}')"
    [ -z "$v" ] && continue
    if [ -z "$bestv" ] || [ "$(printf '%s\n%s\n' "$bestv" "$v" | sort -V | tail -1)" = "$v" ] && [ "$v" != "$bestv" ]; then best="$c"; bestv="$v"; fi
  done
  echo "${best:-claude}"
}
CLAUDE_BIN="${CLAUDE_BIN:-$(pick_claude)}"

MODEL_PRIMARY="${MODEL_PRIMARY:-fable}"   # CLI alias -> newest Fable the CLI knows (5.1 on 2.1.280)
MODEL_FALLBACK="${MODEL_FALLBACK:-opus}"  # CLI alias -> newest Opus (5.5 on 2.1.280); aliases advance with CLI updates
CLAUDE_ARGS="${CLAUDE_ARGS:---dangerously-skip-permissions}"

# Session-limit retry policy (portable: fixed backoff, no fragile date parsing).
RETRY_WAIT="${RETRY_WAIT:-1800}"     # 30 min between attempts
MAX_RETRY="${MAX_RETRY:-8}"          # 8 x 30min = up to 4h of waiting

STAMP="$(date +%Y-%m-%d)"
# SUPPLEMENTARY=1 (2026-08-24; since 2026-09-19 launched ONLY by the button): a pass that
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
  local act=()
  [ -n "${NTFY_CONTROL_URL:-}" ] && act=(-H "Actions: http, Run check now, ${NTFY_CONTROL_URL}, method=POST, body=check ${NTFY_CONTROL_TOKEN:-}, clear=true")
  curl -s -m 15 -H "Title: ${1}" -H "Priority: ${3:-default}" -H "Tags: ${4:-satellite}" "${act[@]}" \
       -d "${2}" "$NTFY_URL" >/dev/null 2>&1
}

if [ -f "$DIR/AUTOPILOT_OFF" ]; then
  log "AUTOPILOT_OFF present — skipping."
  ntfy_push "BS45 autopilot OFF" "Kill switch on; skipped today's run." "low" "no_entry"
  exit 0
fi
# ONE global lock for every run (2026-09-19: three concurrent agents, two died). The
# cron run waits up to 30 min; a button run waits up to 2 h (queued behind the running
# check, then re-derives what is still unread today). Released on any exit.
LOCK_WAIT_SEC="${LOCK_WAIT_SEC:-1800}"
LOCK_LABEL="$( [ "${BUTTON:-0}" = 1 ] && echo button || { [ "$SUPPLEMENTARY" = 1 ] && echo supplementary || echo 1pm; } )"
if "$DIR/run_lock.sh" busy; then
  log "Another run holds the lock ($("$DIR/run_lock.sh" info)) — waiting up to ${LOCK_WAIT_SEC}s."
fi
if ! "$DIR/run_lock.sh" acquire "$LOCK_LABEL" "$LOCK_WAIT_SEC"; then
  log "Lock still held after ${LOCK_WAIT_SEC}s ($("$DIR/run_lock.sh" info)) — giving up this run."
  ntfy_push "BS45: run skipped (another check still running)" "$("$DIR/run_lock.sh" info). Tap Run check now once it finishes." "low" "hourglass"
  exit 0
fi
trap '"$DIR/run_lock.sh" release' EXIT
if [ "${BUTTON:-0}" = 1 ]; then
  # re-derive what is still unread now that we hold the lock (a queued tap may be stale)
  ledger="$REPO/results/reached_$(date +%F).txt"; todo=""
  for c in $CLUSTERS; do
    if [ -f "$ledger" ] && awk -v c="$c" '$2==c{f=1} END{exit f?0:1}' "$ledger"; then continue; fi
    todo="$todo $c"
  done
  todo="$(echo $todo)"
  if [ -z "$todo" ]; then log "Button run: everything already read today — nothing to do."; ntfy_push "BS45: all clusters already read today" "Nothing left to check. Tap again tomorrow, or after the 1pm run." "low" "satellite"; exit 0; fi
  CLUSTERS="$todo"; export CLUSTERS
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
# Deterministic digest to the phone within a minute of the check (2026-09-20) — numbers
# first, narrative later; also the fallback text if the agent fails.
MECH="$(python3 "$DIR/summarize_check.py" "$CHECK_OUTPUT" 2>/dev/null)"
if [ -n "$MECH" ]; then
  log "digest: $MECH"
  if grep -q "FOUND banners: [1-9]" <<<"$MECH"; then ntfy_push "🚨 BS45 check: FOUND banner" "$MECH" "urgent" "rotating_light"
  else ntfy_push "BS45 check: numbers" "$MECH" "low" "satellite"; fi
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
  log "No cluster answered on the first pass — arming hourly reminders (no unattended pushes)."
  ntfy_push "BS45: no Duo taps — nothing read yet" "Tap Run check now when you can; I'll remind you hourly (no pushes until you tap)." "default" "hourglass"
  python3 "$DIR/spawn_detached.py" "$DIR/remind_unread.sh" ${CLUSTERS:-$ALL_CLUSTERS} >/dev/null
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
# Why the primary was skipped, never empty (2026-09-24: the CLI said only "You're out of
# usage credits. Switch to another model..." with no "resets" clause, so the phone read
# "fable blocked ()").
block_reason() {
  local why="model unavailable" r
  if credits_gone; then why="out of usage credits"; elif limit_hit; then why="usage limit"; fi
  r="$(limit_reset_note 2>/dev/null)"
  echo "${why}${r:+, $r}"
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
# Remembered primary block (2026-09-25): a credit-exhausted primary stays blocked for days,
# so re-trying it every run only adds a CLI start (09-25: that start stalled 31 min).
BLOCK_FILE="$REPO/results/model_block_${MODEL_PRIMARY}.txt"
if [ -f "$BLOCK_FILE" ] && [ "$(awk 'NR==1{print $1}' "$BLOCK_FILE" 2>/dev/null || echo 0)" -gt "$(date +%s)" ] 2>/dev/null; then
  log "Primary '$MODEL_PRIMARY' still blocked ($(cut -d' ' -f2- "$BLOCK_FILE")); using '$MODEL_FALLBACK' directly."
  MODEL="$MODEL_FALLBACK"
fi
attempt=1
rc=1
PARTIAL=0
SUBMITS_THIS_RUN=0
while : ; do
  log "Invoking headless Claude (model=$MODEL, cli=$("$CLAUDE_BIN" --version 2>/dev/null | awk '{print $1}') at $CLAUDE_BIN, attempt $attempt/$((MAX_RETRY+1)))…"
  # shellcheck disable=SC2086
  # Hard cap (2026-09-19): normal runs take 20-60 min; 4-6 h runs died on API timeouts.
  MAX_AGENT_SEC="${MAX_AGENT_SEC:-5400}"
  # STARTUP STALL GUARD (2026-09-25): twice the CLI sat 20-31 min BEFORE starting its
  # session while the Mac was idle with the display off, and continued within ~15 s of the
  # display waking (09-24 13:21:27 -> 13:21:43; 09-25 13:33:11 -> 13:33:22). Not reproduced
  # with a 20 s display-off, so the mechanism is unproven. Mitigation: declare user
  # activity (wakes the display) right before launch, and a watchdog that re-wakes every
  # 2 min until a session transcript appears, pushing an alert at 4 min.
  caffeinate -u -t 5 >/dev/null 2>&1 &
  START_MARK="$REPO/results/.agent_start_mark"; touch "$START_MARK"
  (
    for m in 2 4 6 8 10 12 14 16 18 20; do
      sleep 120
      find "$HOME/.claude/projects" -name '*.jsonl' -newer "$START_MARK" 2>/dev/null | grep -q . && exit 0
      caffeinate -u -t 3 >/dev/null 2>&1
      log "Agent session not started after ${m} min (CLI startup stall) — woke the display."
      [ "$m" = 4 ] && ntfy_push "BS45: agent stalled at startup" \
        "The Claude CLI has not started after 4 min (it stalls while the Mac is idle). The loop keeps waking the display; touching the Mac also releases it." "high" "warning"
    done
  ) &
  WATCH_PID=$!
  # </dev/null: claude -p blocks until stdin EOF (verified 2026-09-20: 25 s with an open
  # pipe vs 2 s with no stdin) — never let it inherit a pipe. caffeinate -i: no idle
  # sleep while the agent runs. No IDE auto-connect for headless runs (VS Code may be
  # throttled while the Mac is idle, and the loop never needs it).
  CLAUDE_CODE_AUTO_CONNECT_IDE=false CLAUDE_CODE_IDE_SKIP_AUTO_INSTALL=1 \
    python3 "$DIR/run_with_timeout.py" "$MAX_AGENT_SEC" caffeinate -i "$CLAUDE_BIN" -p "$PROMPT" --model "$MODEL" $CLAUDE_ARGS </dev/null >>"$LOG" 2>&1
  rc=$?
  pkill -P "$WATCH_PID" 2>/dev/null; kill "$WATCH_PID" 2>/dev/null || true
  [ "$rc" -eq 124 ] && log "Agent exceeded ${MAX_AGENT_SEC}s and was killed (rc=124)."
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
     && { limit_hit || tail -25 "$LOG" | grep -qiE "model.*(not found|unavailable|invalid|unknown)|does not support this model"; }; then
    WHY="$(block_reason)"
    # remember a credit block for 20 h (next run goes straight to the fallback; a real
    # reset shows up the day after, when the primary is tried again)
    credits_gone && echo "$(( $(date +%s) + 72000 )) $WHY (since $(date '+%F %H:%M'))" > "$BLOCK_FILE"
    log "Primary '$MODEL_PRIMARY' blocked ($WHY) — falling back to '$MODEL_FALLBACK'."
    ntfy_push "BS45 — falling back to $MODEL_FALLBACK" \
      "$MODEL_PRIMARY blocked ($WHY). Running on $MODEL_FALLBACK instead." "low" "arrows_counterclockwise"
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
    SUBMITS_THIS_RUN=$(grep -c "Submitted batch job" "$LOG" 2>/dev/null); SUBMITS_THIS_RUN=${SUBMITS_THIS_RUN:-0}
    if limit_hit; then why="Usage limit hit mid-run"
    elif [ "$rc" -eq 124 ]; then why="Agent hit the ${MAX_AGENT_SEC:-5400}s time cap and was killed"
    elif [ "$rc" -eq 0 ]; then why="Agent exited cleanly (rc=0) but wrote NO summary (ended its turn early — e.g. waiting on a backgrounded duo_run)"
    else why="Agent died mid-run (rc=$rc, e.g. API connection dropped)"; fi
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
  msg="Agent did not finish (${why:-rc=$rc}; ${SUBMITS_THIS_RUN:-0} submit echo(es) logged, edits committed as PARTIAL). The reads still count — numbers: ${MECH:-n/a}. Unprocessed outputs re-show next check."
else
  msg="Agent wrote no summary (rc=$rc). Numbers from the check: ${MECH:-n/a}. Unprocessed outputs re-show next check."
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
  log "Missed Duo on: ${MISSED} — arming hourly reminders (pushes only when Daniel taps)."
  ntfy_push "BS45: ${MISSED} unread today" \
    "Missed the Duo push for ${MISSED}. Tap Run check now when you can — it will check only ${MISSED}. I'll remind you hourly." "default" "hourglass"
  python3 "$DIR/spawn_detached.py" "$DIR/remind_unread.sh" $MISSED >/dev/null
fi
log "Done."
