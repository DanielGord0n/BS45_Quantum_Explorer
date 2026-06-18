#!/bin/bash
# ==========================================================================
# Reproduce the published BS(43,42) base sequence (delta-code) with our solver.
#
# Blind discovery at n=42 is infeasible (search-tree wall — see HANDOFF.md).
# This fixes the first K of the 21 layers of the known published solution and
# BLINDLY searches the remaining (21-K) layers with wz_exact_t23 (Wang-Zhu
# Thm 2.3 residue prune + Thm 2.4 spectral filter), then INDEPENDENTLY verifies
# the result with verify_npaf.py (NPAF[s]=0 for all s).
#
# Usage:  ./reproduce_bs43.sh [K]      (default K=13 -> ~15s; lower K = more
#                                       layers searched blind, slower)
# ==========================================================================
set -e
cd "$(dirname "$0")"
K=${1:-13}

echo "building wz_exact_t23 ..."
g++ -O3 -std=c++17 -fopenmp -o /tmp/wz_repro src/solver/wz_exact_t23.cpp 2>/dev/null \
  || g++ -O3 -std=c++17 -o /tmp/wz_repro src/solver/wz_exact_t23.cpp

PFX=$(python3 - "$K" <<'PY'
import sys
K=int(sys.argv[1])
def parse(s1,s2): return [1 if c=='+' else -1 for c in (s1+s2) if c in '+-']
A=parse('++--++--+-+++-+--+-++-','-+----+++-+-+-+++++++')
B=parse('+++++++++---+-+-+++--+','+-+-+-++++--++-++--+-')
C=parse('+++++----+++--+++-++-+','--+----+--+--+++--+-')
D=parse('++------+---++++---+-+','-+++--+-++-+-++-+++-')
n=len(C)
c16=[[1 if(i&8)else -1,1 if(i&4)else -1,1 if(i&2)else -1,1 if(i&1)else -1] for i in range(16)]
cp=[t for t in c16 if t[0]*t[1]*t[2]*t[3]==1]
cn=[t for t in c16 if t[0]*t[1]*t[2]*t[3]==-1]
def ab(d): return [A[d],B[d],A[n-d],B[n-d]]
def cd(d): return [C[d],D[d],C[n-1-d],D[n-1-d]]
p=[str(cn.index(ab(0))),str(c16.index(cd(0)))]
for L in range(1,K): p+=[str(cp.index(ab(L))),str(cp.index(cd(L)))]
print(','.join(p))
PY
)

echo "reproducing BS(43,42): first $K of 21 layers fixed, blind-searching the rest ..."
WZ_PREFIX="$PFX" /tmp/wz_repro 42 7 11 0 0 | tee /tmp/repro42.txt \
  | grep -E "REPRODUCTION CONFIRMED|^sig|^[ABCD] =|^Time"

echo ""
echo "=== independent verification (verify_npaf.py) ==="
python3 verify_npaf.py < /tmp/repro42.txt
