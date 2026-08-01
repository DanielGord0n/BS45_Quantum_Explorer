# Class-level compression triage: a signature class of BS(n+1,n) can contain a
# solution ONLY IF a periodic-complementary COMPRESSED quadruple exists with the
# right entry ranges/parities and sums (DK compression, arXiv:1302.0571, applied
# to the zero-padded quadruple at length L=n+1 = d*m). If none exists -> the
# class is RIGOROUSLY DEAD. Feasible = no information (expected common case).
import itertools, sys

def paf_vec(x, d):
    return tuple(sum(x[i] * x[(i + s) % d] for i in range(d)) for s in range(1, d // 2 + 1))

_memo = {}
def side_vecs_ab(d, m, target_sum):
    key = ("ab", d, m, target_sum)
    if key in _memo: return _memo[key]
    _memo[key] = _side_vecs_ab(d, m, target_sum)
    return _memo[key]
def _side_vecs_ab(d, m, target_sum):
    # A/B side: d entries, each a sum of m +-1s (values m-2j), total = +-target
    vals = list(range(-m, m + 1, 2))
    out = set()
    for combo in itertools.product(vals, repeat=d):
        if abs(sum(combo)) == target_sum:
            out.add(paf_vec(combo, d))
    return out

def side_vecs_cd(d, m, target_sum):
    key = ("cd", d, m, target_sum)
    if key in _memo: return _memo[key]
    _memo[key] = _side_vecs_cd(d, m, target_sum)
    return _memo[key]
def _side_vecs_cd(d, m, target_sum):
    # C/D side padded with ONE zero (WLOG in the last position class by rotation
    # invariance): d-1 entries are sums of m +-1s, one entry is a sum of (m-1).
    vals_full = list(range(-m, m + 1, 2))
    vals_pad = list(range(-(m - 1), m, 2))
    out = set()
    for combo in itertools.product(*([vals_full] * (d - 1) + [vals_pad])):
        if abs(sum(combo)) == target_sum:
            out.add(paf_vec(combo, d))
    return out

def feasible(d, L, sig):
    m = L // d
    a, b, c, cd_ = sig
    SA = side_vecs_ab(d, m, a); SB = side_vecs_ab(d, m, b)
    SC = side_vecs_cd(d, m, c); SD = side_vecs_cd(d, m, cd_)
    SAB = set(tuple(x + y for x, y in zip(u, v)) for u in SA for v in SB)
    SCD = set(tuple(x + y for x, y in zip(u, v)) for u in SC for v in SD)
    for v in SAB:
        if tuple(-x for x in v) in SCD:
            return True
    return False

# positive control: the SOLVED n=41 class (0,2,9,9), L=42 = d*m
print("=== positive control: n=41 (0,2,9,9), must be FEASIBLE at every d ===")
for d, m in ((2, 21), (3, 14), (6, 7), (7, 6)):
    ok = feasible(d, 42, (0, 2, 9, 9))
    print(f"  d={d}: {'FEASIBLE (control OK)' if ok else 'INFEASIBLE — METHOD BUG!'}")
print("=== n=44 classes, L=45: d=3 (m=15) and d=5 (m=9) ===")
SIGS = [(1,7,8,8),(1,13,2,2),(3,3,4,12),(3,5,0,12),(3,13,0,0),(5,5,8,8),
        (5,7,2,10),(5,9,6,6),(5,11,4,4),(7,7,4,8),(7,11,2,2),(9,9,0,4)]
for sig in SIGS:
    r3 = feasible(3, 45, sig)
    r5 = feasible(5, 45, sig)
    verdict = "alive" if (r3 and r5) else "*** CLASS KILLED ***"
    print(f"  sig {sig}: d=3 {'ok' if r3 else 'INFEASIBLE'}  d=5 {'ok' if r5 else 'INFEASIBLE'}  -> {verdict}", flush=True)
