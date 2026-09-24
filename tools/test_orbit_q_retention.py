#!/usr/bin/env python3
"""Per-solution retention under WZ_FH_ORBIT_Q (2026-09-24). Local, n <= 10 only.

Independent ground truth: brute-force EVERY BS(n+1,n) at n = 6, 8, 10 (numpy join of all
C,D pair correlations against all A,B pair correlations). For every solution C,D (one per
64-element orbit, per signature class) ask the solver's LOCATE instrument whether some
image of it lies in a cell that the canonicalization KEEPS, for the 32-group (baseline)
the 64-group (Q), and Q + the closure prune, with mod-3 and mod-6 cells. Soundness requires retained=YES always.
"""
import itertools
import os
from pathlib import Path
import shutil
import subprocess
import tempfile

import numpy as np

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / 'src/solver/wz_match.cpp'
ENV = {k: v for k, v in os.environ.items() if not k.startswith(('WZ_', 'FH_'))}


def all_seqs(L):
    idx = np.arange(1 << L, dtype=np.int64)[:, None]
    return (1 - 2 * ((idx >> np.arange(L)) & 1)).astype(np.int8)


def acfs(X, lags):
    L = X.shape[1]
    return np.stack([(X[:, :L - s].astype(np.int32) * X[:, s:]).sum(1) if s < L
                     else np.zeros(len(X), np.int32) for s in lags], 1).astype(np.int32)


def solutions(n):
    lags = range(1, n + 1)
    S = all_seqs(n)
    T = all_seqs(n + 1)
    aS, aT = acfs(S, lags), acfs(T, lags)
    # pair correlation for every ordered pair (i, j), i <= j is enough for C,D and A,B
    iu = np.triu_indices(len(S))
    cd = aS[iu[0]] + aS[iu[1]]
    ia = np.triu_indices(len(T))
    ab = aT[ia[0]] + aT[ia[1]]
    keys = {}
    for k, (i, j) in enumerate(zip(*ia)):
        keys.setdefault(ab[k].tobytes(), set()).add((int(T[i].sum()), int(T[j].sum())))
    out = []
    neg = (-cd).astype(np.int32)
    for k in range(len(cd)):
        hit = keys.get(neg[k].tobytes())
        if hit:
            i, j = iu[0][k], iu[1][k]
            out.append((tuple(int(x) for x in S[i]), tuple(int(x) for x in S[j]), hit))
    return out


def canon64(C, D):
    """Orbit key under the C,D 64-group at the sequence level (for dedup only)."""
    L = len(C)
    forms = {(C, D)}
    qc = tuple((C[i] + D[i] + C[L - 1 - i] - D[L - 1 - i]) // 2 for i in range(L))
    qd = tuple((C[i] + D[i] - C[L - 1 - i] + D[L - 1 - i]) // 2 for i in range(L))
    if all(x in (1, -1) for x in qc + qd):
        forms.add((qc, qd))
    best = None
    for (c, d) in forms:
        for nc, nd, rc, rd, sw in itertools.product(range(2), repeat=5):
            x = tuple(-v for v in c) if nc else c
            y = tuple(-v for v in d) if nd else d
            x = x[::-1] if rc else x
            y = y[::-1] if rd else y
            k = (y, x) if sw else (x, y)
            best = k if best is None or k < best else best
    return best


def main():
    compiler = shutil.which('clang++') or shutil.which('g++')
    with tempfile.TemporaryDirectory(prefix='bs45-orbitq-ret-') as name:
        binary = Path(name) / 'current'
        subprocess.run([compiler, '-O3', '-std=c++17', str(SOURCE), '-o', str(binary)], check=True)
        runs = 0
        for n in (6, 8, 10):
            sols = solutions(n)
            reps = {}
            for C, D, abs_ in sols:
                for (a, b) in abs_:
                    reps.setdefault((a, b, canon64(C, D)), (C, D, a, b))
            print(f'n={n}: {len(sols)} solution C,D pairs, {len(reps)} (class, 64-orbit) representatives', flush=True)
            for (C, D, a, b) in reps.values():
                c, d = sum(C), sum(D)
                for m6, q in itertools.product((0, 1), (0, 1, 2)):  # q=2: Q + closure prune
                    env = {**ENV, 'WZ_FIRSTHIT': '1', 'WZ_FH_ORBIT_CANON': '1', 'WZ_FH_M6': str(m6),
                           'WZ_FH_ORBIT_Q': str(min(q, 1)), 'WZ_FH_ORBIT_QPRUNE': str(int(q == 2)),
                           'WZ_THM211B': '1', 'WZ_THM212': '1', 'WZ_FH_LOCATE_C': ','.join(map(str, C)),
                           'WZ_FH_LOCATE_D': ','.join(map(str, D))}
                    p = subprocess.run([str(binary), str(n), str(a), str(b), str(c), str(d)], env=env,
                                       text=True, capture_output=True, timeout=120)
                    out = p.stdout
                    if 'LOCATE: cell NOT FOUND' in out:
                        raise AssertionError(('filter rejected a true solution', n, a, b, C, D, m6))
                    assert 'retained=YES' in out, (n, a, b, c, d, C, D, m6, q, out[-800:])
                    if q:
                        assert ('group=64' in out) == ((c + d) % 4 == 0), (n, c, d)
                    runs += 1
        print(f'PASS: {runs} LOCATE runs, every solution orbit retained under the 32-group, the 64-group and 64-group + Q-closure prune '
              '(mod-3 and mod-6 cells).', flush=True)


if __name__ == '__main__':
    main()
