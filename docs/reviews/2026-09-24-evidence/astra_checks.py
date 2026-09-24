#!/usr/bin/env python3
"""Verify Astra's 2026-09-24 math-review claims on the six known solutions.

Pure arithmetic on banked sequences (no solver, no search). Run from the repo root:
    python3 docs/reviews/2026-09-24-evidence/astra_checks.py

Item 1: quad switch Q(C,D) = (U+RV, U-RV), U=(C+D)/2, V=(C-D)/2.
Item 2: one-sign extension BS(n+1,n) -> BS(n+2,n+1) over each seed's full 4,096 orbit.
Item 3: A,B exchange (only a sanity check that it is a symmetry; implementation later).
"""
from itertools import product
from pathlib import Path

ROOT = Path(__file__).resolve().parents[3]
FILES = {
    'ours-41': 'results/champions/champion_firsthit_bs42_41.txt',
    'ours-42': 'results/champions/champion_firsthit_bs43_42.txt',
    'ours-43': 'results/champions/champion_firsthit_bs44_43.txt',
    'WZ-41': 'results/reference/wz_table1_bs42_41.txt',
    'WZ-42': 'results/reference/wz_table1_bs43_42.txt',
    'WZ-43': 'results/reference/wz_table1_bs44_43.txt',
}


def load(path):
    rows = [l.split() for l in (ROOT / path).read_text().splitlines()
            if l.strip() and not l.startswith('#')]
    n = int(rows[0][0])
    A, B, C, D = (tuple(int(x) for x in r) for r in rows[2:6])
    assert len(A) == len(B) == n + 1 and len(C) == len(D) == n
    return A, B, C, D


def acf(X, s):
    return sum(X[i] * X[i + s] for i in range(len(X) - s)) if s < len(X) else 0


def is_bs(A, B, C, D):
    return all(acf(A, s) + acf(B, s) + acf(C, s) + acf(D, s) == 0 for s in range(1, len(A)))


def pair_acf(C, D):
    return tuple(acf(C, s) + acf(D, s) for s in range(1, len(C)))


def Q(C, D):
    L = len(C)
    out_c, out_d = [], []
    for i in range(L):
        u, v_r = (C[i] + D[i]) / 2, (C[L - 1 - i] - D[L - 1 - i]) / 2
        out_c.append(u + v_r)
        out_d.append(u - v_r)
    if not all(x in (1, -1) for x in out_c + out_d):
        return None
    return tuple(int(x) for x in out_c), tuple(int(x) for x in out_d)


def profile(X, m=6):
    p = [0] * m
    for i, x in enumerate(X):
        p[i % m] += x
    return p


def rev_profile(p, L, m=6):
    return [p[(L - 1 - r) % m] for r in range(m)]


def quad_products(C, D):
    L = len(C)
    return [C[i] * C[L - 1 - i] * D[i] * D[L - 1 - i] for i in range(L)]


neg = lambda X: tuple(-x for x in X)
rev = lambda X: tuple(reversed(X))
alt = lambda X: tuple(x if i % 2 == 0 else -x for i, x in enumerate(X))


def orbit(A, B, C, D):
    """Full group: A,B signs/reversals/swap (32) x C,D signs/reversals/swap/Q (64) x alternation."""
    seen = set()
    for e in range(2):
        a0, b0, c0, d0 = (alt(X) if e else X for X in (A, B, C, D))
        cd_forms = {(c0, d0)}
        q = Q(c0, d0)
        if q:
            cd_forms.add(q)
        for (cc, dd) in cd_forms:
            for na, nb, ra, rb, sw, nc, nd, rc, rd, sw2 in product(range(2), repeat=10):
                x, y = a0, b0
                x = neg(x) if na else x; y = neg(y) if nb else y
                x = rev(x) if ra else x; y = rev(y) if rb else y
                if sw: x, y = y, x
                u, v = cc, dd
                u = neg(u) if nc else u; v = neg(v) if nd else v
                u = rev(u) if rc else u; v = rev(v) if rd else v
                if sw2: u, v = v, u
                seen.add((x, y, u, v))
    return seen


def extend_ok(A, B, C, D, e):
    """Astra eq (2): e_A A_0 + e_B B_0 = 0 and e_A A_j + e_B B_j + e_C C_(j-1) + e_D D_(j-1) = 0."""
    eA, eB, eC, eD = e
    if eA * A[0] + eB * B[0] != 0:
        return False
    return all(eA * A[j] + eB * B[j] + eC * C[j - 1] + eD * D[j - 1] == 0 for j in range(1, len(A)))


def main():
    sols = {k: load(v) for k, v in FILES.items()}
    print('== sanity: all six are BS =', all(is_bs(*s) for s in sols.values()))

    print('\n== ITEM 1: quad switch Q')
    for name, (A, B, C, D) in sols.items():
        L = len(C)
        prods = quad_products(C, D)
        interior_neg = [i for i in range(1, L - 1) if prods[i] < 0]
        q = Q(C, D)
        line = (f'{name}: L={L} sig=({sum(A)},{sum(B)},{sum(C)},{sum(D)}) endpoint quad product={prods[0]:+d} '
                f'interior negative quads={len(interior_neg)}')
        if q is None:
            print(line + ' -> Q NOT BINARY')
            continue
        C2, D2 = q
        same_sums = (sum(C2), sum(D2)) == (sum(C), sum(D))
        same_pair = pair_acf(C2, D2) == pair_acf(C, D)
        still_bs = is_bs(A, B, C2, D2)
        p, qq = profile(C), profile(D)
        Rp, Rq = rev_profile(p, L), rev_profile(qq, L)
        pf = [(p[r] + qq[r] + Rp[r] - Rq[r]) / 2 for r in range(6)]
        qf = [(p[r] + qq[r] - Rp[r] + Rq[r]) / 2 for r in range(6)]
        formula_ok = pf == profile(C2) and qf == profile(D2)
        fixed = (C2, D2) == (C, D)
        # is Q's image already reachable by the OLD 32-element C,D group?
        old = set()
        for nc, nd, rc, rd, sw in product(range(2), repeat=5):
            u = neg(C) if nc else C; v = neg(D) if nd else D
            u = rev(u) if rc else u; v = rev(v) if rd else v
            old.add((v, u) if sw else (u, v))
        new_orbit = (C2, D2) not in old
        print(line + f' -> binary, sums kept={same_sums}, pair NPAF kept={same_pair}, '
              f'same A,B completes={still_bs}, formula(1)={formula_ok}, fixed point={fixed}, '
              f'outside old 32-group={new_orbit}')

    print('\n== ITEM 3: A,B exchange keeps BS (all six) =',
          all(is_bs(B, A, C, D) for (A, B, C, D) in sols.values()))

    print('\n== ITEM 2: one-sign extension over full orbits (all 16 sign vectors)')
    signs = list(product((1, -1), repeat=4))
    for name, s in sols.items():
        orb = orbit(*s)
        assert all(is_bs(*t) for t in orb), f'{name}: orbit member is not BS'
        tried = hits = disagree = 0
        prefilter = 0
        for (A, B, C, D) in orb:
            for e in signs:
                tried += 1
                if e[0] * sum(A) + e[1] * sum(B) + e[2] * sum(C) + e[3] * sum(D) == 0:
                    prefilter += 1
                crit = extend_ok(A, B, C, D, e)
                direct = is_bs(A + (e[0],), B + (e[1],), C + (e[2],), D + (e[3],))
                disagree += crit != direct
                hits += direct
                if direct:
                    print(f'   !!! {name}: extension FOUND with signs {e} — verify independently')
        print(f'{name}: orbit={len(orb)} distinct BS tuples, attempts={tried}, pass eq(4)={prefilter}, '
              f'extensions={hits}, criterion-vs-direct disagreements={disagree}')


if __name__ == '__main__':
    main()
