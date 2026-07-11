#!/bin/bash
# next_seeds.sh — deterministic seed-base allocator for the autonomous loop.
# Keeps seed selection OUT of the model so a base is never reused.
#
#   next_seeds.sh peek <count>     # print next <count> bases, do NOT advance
#   next_seeds.sh take <count>     # print next <count> bases AND advance ledger
#   next_seeds.sh n                # print current rung N
#   next_seeds.sh set-n <value>    # set rung N (when the ladder climbs)
#
# Bases are STRIDE apart. Output: one base per line.

set -euo pipefail
DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
LEDGER="$DIR/seed_ledger.txt"

get() { grep -E "^$1=" "$LEDGER" | tail -1 | cut -d= -f2; }
set_kv() {  # set_kv KEY VALUE — rewrite via temp file (no .bak litter, portable)
  local tmp; tmp="$(mktemp)"
  if grep -qE "^$1=" "$LEDGER"; then
    awk -v k="$1" -v v="$2" 'BEGIN{FS=OFS="="} $1==k{$2=v} {print}' "$LEDGER" > "$tmp"
  else
    cat "$LEDGER" > "$tmp"; echo "$1=$2" >> "$tmp"
  fi
  cat "$tmp" > "$LEDGER"; rm -f "$tmp"
}

cmd="${1:-}"; arg="${2:-}"
case "$cmd" in
  n)      get N ;;
  set-n)  [ -n "$arg" ] || { echo "set-n needs a value" >&2; exit 2; }
          set_kv N "$arg" ;;
  peek|take)
    [ -n "$arg" ] || { echo "$cmd needs a count" >&2; exit 2; }
    base="$(get NEXT_BASE)"; stride="$(get STRIDE)"
    for ((i=0; i<arg; i++)); do echo $(( base + i*stride )); done
    if [ "$cmd" = "take" ]; then set_kv NEXT_BASE $(( base + arg*stride )); fi
    ;;
  *) echo "usage: next_seeds.sh {peek|take <count>|n|set-n <value>}" >&2; exit 2 ;;
esac
