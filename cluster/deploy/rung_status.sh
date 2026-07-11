#!/bin/bash
# rung_status.sh — the loop's EXIT CONDITION. Deterministic, so the model can't
# rationalize "just one more round" at round 30.
#
#   rung_status.sh check              -> prints ACTIVE | EXHAUSTED (+ reason); exit 0/3
#   rung_status.sh add <n_arrays>     -> record arrays just submitted at this rung
#   rung_status.sh floor <value>      -> record an observed floor (keeps the best/lowest)
#   rung_status.sh promote            -> a VERIFIED hit: N+1, reset counters, rebudget
#   rung_status.sh show               -> dump state
#
# EXHAUSTED means: stop buying SA tickets at this rung. Escalate to the method
# experiments. It does NOT mean the project is stuck — it means the SA lever is
# spent here, which the campaign already measured (~n≈33-35 ceiling).

set -euo pipefail
DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
S="$DIR/rung_state.txt"

get() { grep -E "^$1=" "$S" | tail -1 | cut -d= -f2; }
set_kv() {
  local tmp; tmp="$(mktemp)"
  if grep -qE "^$1=" "$S"; then
    awk -v k="$1" -v v="$2" 'BEGIN{FS=OFS="="} $1==k{$2=v} {print}' "$S" > "$tmp"
  else
    cat "$S" > "$tmp"; echo "$1=$2" >> "$tmp"
  fi
  cat "$tmp" > "$S"; rm -f "$tmp"
}

cmd="${1:-show}"; arg="${2:-}"
N="$(get N)"; A="$(get ARRAYS_AT_RUNG)"; F="$(get BEST_FLOOR)"
B="$(get BUDGET)"; P="$(get PREV_RUNG_ARRAYS)"

case "$cmd" in
  show)
    echo "rung n=$N | arrays=$A/$B | best_floor=$F | status=$(get STATUS)"
    ;;

  add)
    [ -n "$arg" ] || { echo "add needs a count" >&2; exit 2; }
    set_kv ARRAYS_AT_RUNG "$(( A + arg ))"
    echo "arrays at n=$N: $(( A + arg ))/$B"
    ;;

  floor)
    [ -n "$arg" ] || { echo "floor needs a value" >&2; exit 2; }
    # Keep the BEST (lowest) floor ever seen at this rung.
    if [ "$arg" -lt "$F" ]; then
      set_kv BEST_FLOOR "$arg"
      # Floor MOVED -> the rung is still yielding. Extend the budget: this is
      # progress, not grinding. (Re-arms the exit condition honestly.)
      set_kv BUDGET "$(( A + P ))"
      echo "floor improved $F -> $arg at n=$N; rung still productive, budget extended to $(( A + P ))"
    else
      echo "floor $arg (no improvement on $F)"
    fi
    ;;

  promote)
    set_kv PREV_RUNG_ARRAYS "$A"          # what this rung actually cost
    set_kv N "$(( N + 1 ))"
    set_kv ARRAYS_AT_RUNG 0
    set_kv BEST_FLOOR 9999
    set_kv BUDGET "$(( A * 3 ))"          # next rung: 3x the last rung's real cost
    set_kv STATUS ACTIVE
    echo "PROMOTED to n=$(( N + 1 )) (previous rung cost $A arrays; new budget $(( A * 3 )))"
    ;;

  check)
    if [ "$A" -ge "$B" ]; then
      set_kv STATUS EXHAUSTED
      cat <<EOF
EXHAUSTED
Rung n=$N has absorbed $A arrays (pre-registered budget $B, = 3x the $P arrays
that cracked n=$((N-1))) with the floor stuck at $F. Per the campaign's own
measurements the SA lever caps around n≈33-35 — more tickets here is NOT a lever.

DO NOT refill SA at this rung. Escalate instead:
  1. Finish the JOIN22 / Theorem-2.2 canary (n=29). It has NEVER passed — until it
     does, no negative from that path means anything. Needs a ~15h slot (Trillium 24h).
  2. Run the Phase-0 measurement gates in docs/wz_firsthit_plan.md (cheap, decisive,
     pre-registered pass/KILL criteria).
  3. Tell Daniel to send docs/kotsireas_brief.md — it is READY TO SEND and is a
     METHODS ask, not a compute ask. That is the door to 42+.
EOF
      exit 3
    fi
    echo "ACTIVE — $A/$B arrays at n=$N (floor $F). Refilling is still justified."
    exit 0
    ;;

  *) echo "usage: rung_status.sh {check|add <n>|floor <v>|promote|show}" >&2; exit 2 ;;
esac
