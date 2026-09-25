#!/usr/bin/env python3
"""Read-only proof probes for exact quad/profile residual reachability.

Exhaustion is bounded to six quads (12 positions). Larger banked sequences
are only checked along their actual, already-known paths. No solver search.
"""
from itertools import product
from pathlib import Path

H = ((1,1,1,1), (1,1,-1,-1), (1,-1,1,-1), (1,-1,-1,1))
PLUS = [q for q in product((-1,1),repeat=4) if q[0]*q[1]*q[2]*q[3] == 1]
ROOT = [(1,1,1,-1), (1,1,-1,1)]

def quad_sums_ok(v, k):
    t4 = [sum(h[i]*v[i] for i in range(4)) for h in H]
    if any(x % 4 for x in t4): return False
    l1 = sum(abs(x) for x in t4)//4
    return l1 <= k and (k-l1) % 2 == 0

def self_sums_ok(v, k):
    return all(x % 2 == 0 for x in v) and sum(v) % 4 == 0 and max(map(abs,v)) <= 2*k

def contributions(q, j, L, m, classes):
    out = [0]*(2*len(classes))
    for pos, av, bv in ((j,q[0],q[1]), (L-1-j,q[2],q[3])):
        offset = 2*classes.index(pos % m)
        out[offset] += av
        out[offset+1] += bv
    return out

def residual_ok(A, B, m, depth, ab):
    L = len(A)
    target = [[0,0] for _ in range(m)]
    for pos in range(depth, L-depth):
        target[pos % m][0] += A[pos]
        target[pos % m][1] += B[pos]
    components = {}
    for j in range(depth, L//2):
        key = tuple(sorted(set((j%m,(L-1-j)%m))))
        components.setdefault(key, []).append(j)
    if L % 2:
        components.setdefault((L//2 % m,), [])
    for classes, positions in components.items():
        v = tuple(value for r in classes for value in target[r])
        exceptional = [(0,)*len(v)]
        normals = len(positions)
        if ab and 0 in positions:
            normals -= 1
            exceptional = [tuple(contributions(q,0,L,m,classes)) for q in ROOT]
        if L % 2 and L//2 % m in classes:
            offset = 2*classes.index(L//2 % m)
            extended = []
            for e in exceptional:
                for av,bv in product((-1,1),repeat=2):
                    q = list(e); q[offset] += av; q[offset+1] += bv
                    extended.append(tuple(q))
            exceptional = extended
        check = self_sums_ok if len(classes) == 1 else quad_sums_ok
        if not any(check(tuple(x-y for x,y in zip(v,e)), normals) for e in exceptional):
            return False
    return True

def load(path):
    lines = [s.strip() for s in path.read_text().splitlines() if s.strip() and not s.lstrip().startswith('#')]
    n = int(lines[0].split()[0])
    seqs = [[int(x) for x in line.split()] for line in lines[2:6]]
    assert [len(s) for s in seqs] == [n+1,n+1,n,n]
    assert all(x in (-1,1) for seq in seqs for x in seq)
    return n, seqs

def main():
    reachable = {(0,0,0,0)}
    for k in range(1,7):
        reachable = {tuple(x[i]+q[i] for i in range(4)) for x in reachable for q in PLUS}
        old = 0
        for v in product(range(-k,k+1,2),repeat=4):
            assert quad_sums_ok(v,k) == (v in reachable), (k,v)
            old += sum(v)%4 == 0
        print(f'k={k}: exact={len(reachable)} old_box_mod4={old} removed={old-len(reachable)} PASS')
    self_states = {(0,0)}
    options = {(q[0]+q[2],q[1]+q[3]) for q in PLUS}
    for k in range(1,7):
        self_states = {(x+a,y+b) for x,y in self_states for a,b in options}
        for v in product(range(-2*k,2*k+1),repeat=2):
            assert self_sums_ok(v,k) == (v in self_states), (k,v)
    print('Self-reflected residue component: exhaustive k=1..6 PASS')
    root_states = set(ROOT)
    for k in range(0,6):
        if k:
            root_states = {tuple(x[i]+q[i] for i in range(4)) for x in root_states for q in PLUS}
        for v in product(range(-k-1,k+2,2),repeat=4):
            pred = any(quad_sums_ok(tuple(x-y for x,y in zip(v,root)),k) for root in ROOT)
            assert pred == (v in root_states), (k,v)
    print('Special pinned A,B root + 0..5 ordinary quads: exhaustive PASS')
    files = sorted(Path('results/champions').glob('*.txt')) + sorted(Path('results/reference').glob('*.txt'))
    checks = 0
    for path in files:
        n, seqs = load(path)
        # Independent real NPAF check first: this probe does not trust filenames.
        for s in range(1,n+1):
            assert sum(sum(v[i]*v[i+s] for i in range(len(v)-s)) for v in seqs) == 0, (path,s)
        for side in (0,2):
            A,B = seqs[side:side+2]
            A = [x*A[0] for x in A]; B = [x*B[0] for x in B]
            for m in (3,6):
                for depth in range(len(A)//2+1):
                    assert residual_ok(A,B,m,depth,side==0), (path,m,side,depth)
                    checks += 1
    print(f'Known solutions: {len(files)}/{len(files)} pass NPAF and all {checks} residual profile checks (m=3,6)')
    sums = [(a,b,c,d) for a in range(1,14,2) for b in range(a,14,2)
            for c in range(0,14,2) for d in range(c,14,2) if a*a+b*b+c*c+d*d==178]
    print(f'Signature enumeration: {len(sums)} classes, {sums}')
    for L, side in ((45,'AB'),(44,'CD')):
        components = {}
        for j in range(L//2):
            key = tuple(sorted(set((j%6,(L-1-j)%6))))
            components.setdefault(key, []).append(j)
        print(f'n=44 {side} m=6 grouping: '+', '.join(f'{key}: {len(js)} quads'+(' (root)' if 0 in js else '') for key,js in sorted(components.items())) + (f'; center class {L//2%6}' if L%2 else ''))

if __name__ == '__main__': main()
