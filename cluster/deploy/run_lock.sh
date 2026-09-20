#!/bin/bash
# run_lock.sh — ONE global lock for every BS45 run (cron 1pm, button, reminder-driven).
# 2026-09-19: three agents ran concurrently on the same repo (1pm run + button run +
# auto-spawned supplementary) and two died after 4-6 h. From now on exactly one
# daily_auto.sh may be past the checker at a time.
#
#   run_lock.sh acquire <label> [wait_sec]  -> 0 when held (waits up to wait_sec), 1 if timed out
#   run_lock.sh release                     -> releases if this pid (or its parent) holds it
#   run_lock.sh busy                        -> 0 if held by a live process, 1 otherwise
#   run_lock.sh info                        -> "label since HH:MM (pid N)"
# Stale locks (dead pid) are reclaimed automatically.
set -u
LOCKD="${BS45_LOCK_DIR:-/tmp/bs45_run.lock.d}"
holder_alive() { [ -f "$LOCKD/pid" ] && kill -0 "$(cat "$LOCKD/pid" 2>/dev/null)" 2>/dev/null; }
case "${1:-}" in
  acquire)
    label="${2:-run}"; wait="${3:-0}"; t0=$(date +%s)
    while :; do
      if mkdir "$LOCKD" 2>/dev/null; then
        echo "$PPID" > "$LOCKD/pid"; echo "$label" > "$LOCKD/label"; date +%H:%M > "$LOCKD/since"; exit 0
      fi
      if ! holder_alive; then rm -rf "$LOCKD"; continue; fi          # stale -> reclaim
      [ $(( $(date +%s) - t0 )) -ge "$wait" ] && exit 1
      sleep 20
    done;;
  release)
    if [ -f "$LOCKD/pid" ]; then p="$(cat "$LOCKD/pid")"; if [ "$p" = "$PPID" ] || [ "$p" = "$$" ] || ! kill -0 "$p" 2>/dev/null; then rm -rf "$LOCKD"; fi; fi
    exit 0;;
  busy) holder_alive; exit $?;;
  info) [ -d "$LOCKD" ] && echo "$(cat "$LOCKD/label" 2>/dev/null) since $(cat "$LOCKD/since" 2>/dev/null) (pid $(cat "$LOCKD/pid" 2>/dev/null))" || echo "free"; exit 0;;
  *) echo "usage: run_lock.sh acquire <label> [wait_sec] | release | busy | info" >&2; exit 64;;
esac
