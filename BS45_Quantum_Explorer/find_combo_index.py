#!/usr/bin/env python3
"""Compute the wz_exact_t23 combo index of the KNOWN BS(43,42) solution.

The solver fixes the first 3 layers (ab0,cd0,ab1,cd1,ab2,cd2) and exhaustively
searches the rest. If we know which combo index the published solution falls
into, we can point the solver at just that combo and reproduce BS(43,42) in
seconds -- proving the prune chain does not kill the real solution.
"""

def parse(s1, s2):
    return [1 if c == '+' else -1 for c in (s1 + s2) if c in '+-']

A = parse("++--++--+-+++-+--+-++-", "-+----+++-+-+-+++++++")
B = parse("+++++++++---+-+-+++--+", "+-+-+-++++--++-++--+-")
C = parse("+++++----+++--+++-++-+", "--+----+--+--+++--+-")
D = parse("++------+---++++---+-+", "-+++--+-++-+-++-+++-")

n1 = len(A); n = len(C)
assert len(B) == n1 and len(D) == n and n1 == n + 1, (n1, len(B), n, len(D))

# --- signature + NPAF sanity ---
sa, sb, sc, sd = sum(A), sum(B), sum(C), sum(D)
print(f"lengths: A,B={n1}  C,D={n}")
print(f"signature (sumA,sumB,sumC,sumD) = ({sa},{sb},{sc},{sd})")
print(f"a^2+b^2+c^2+d^2 = {sa*sa+sb*sb+sc*sc+sd*sd}  (target 4n+2 = {4*n+2})")

def npaf(s):
    c = 0
    if s < n1:
        for i in range(n1 - s): c += A[i]*A[i+s] + B[i]*B[i+s]
    if s < n:
        for i in range(n - s): c += C[i]*C[i+s] + D[i]*D[i+s]
    return c
bad = [s for s in range(1, n1+1) if npaf(s) != 0]
print(f"NPAF zero at all shifts: {'YES' if not bad else 'NO at '+str(bad)}")

# --- build comb tables exactly as init_combs() in wz_exact_t23.cpp ---
comb16 = []
comb8_pos = []
comb8_neg = []
for i in range(16):
    t = [1 if (i & 8) else -1, 1 if (i & 4) else -1,
         1 if (i & 2) else -1, 1 if (i & 1) else -1]
    comb16.append(t)
    if t[0]*t[1]*t[2]*t[3] == 1:
        comb8_pos.append(t)
    else:
        comb8_neg.append(t)

def idx_in(table, tup):
    for k, t in enumerate(table):
        if t == list(tup):
            return k
    return None

# --- layer tuples: AB pair d = (A[d],B[d],A[n-d],B[n-d]);
#                   CD pair d = (C[d],D[d],C[n-1-d],D[n-1-d]) ---
def ab_pair(d):  return (A[d], B[d], A[n-d],   B[n-d])
def cd_pair(d):  return (C[d], D[d], C[n-1-d], D[n-1-d])

# verify the WHOLE encoding (all layers), not just the first 3
print("\n--- full-encoding check (all layers) ---")
ok = True
half = n // 2
for d in range(half):
    abt, cdt = ab_pair(d), cd_pair(d)
    ab_tab = comb8_neg if d == 0 else comb8_pos
    cd_tab = comb16 if d == 0 else comb8_pos
    ai, ci = idx_in(ab_tab, abt), idx_in(cd_tab, cdt)
    if ai is None or ci is None:
        ok = False
        print(f"  layer {d:2d}: AB{abt} prod={abt[0]*abt[1]*abt[2]*abt[3]:+d} -> {ai}"
              f"   CD{cdt} prod={cdt[0]*cdt[1]*cdt[2]*cdt[3]:+d} -> {ci}   *** NO MATCH ***")
print("  all layers fit the Wang-Zhu encoding" if ok else "  *** ENCODING MISMATCH ***")

# --- combo index from first 3 layers ---
ab0 = idx_in(comb8_neg, ab_pair(0)); cd0 = idx_in(comb16, cd_pair(0))
ab1 = idx_in(comb8_pos, ab_pair(1)); cd1 = idx_in(comb8_pos, cd_pair(1))
ab2 = idx_in(comb8_pos, ab_pair(2)); cd2 = idx_in(comb8_pos, cd_pair(2))
print("\n--- first-3-layer combo encoding ---")
print(f"ab0={ab0} cd0={cd0} ab1={ab1} cd1={cd1} ab2={ab2} cd2={cd2}")
combo = ab0 | (cd0 << 3) | (ab1 << 7) | (cd1 << 10) | (ab2 << 13) | (cd2 << 16)
print(f"\n>>> COMBO INDEX = {combo}")

slices = {"Fir":(0,131072), "Rorqual":(131072,262144),
          "Nibi":(262144,393216), "Trillium":(393216,524288)}
for name,(lo,hi) in slices.items():
    if lo <= combo < hi:
        print(f">>> falls in {name}'s slice [{lo},{hi})")

# symmetry-pin check for sig (7,11,0,0): pins require C[0]=+1 and D[0]=+1
print(f"\nsym-pin canonical? C[0]={C[0]:+d} D[0]={D[0]:+d} "
      f"({'OK - representative is searched' if C[0]==1 and D[0]==1 else 'NEEDS negation'})")
print(f"\nReproduce locally with:\n  ./wz_exact_t23 42 7 11 0 0 {combo} {combo+1}")
