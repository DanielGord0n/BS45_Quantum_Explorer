#!/usr/bin/env python3
"""A1 outer-lag table gate (Astra deep dive 2026-09-30, item A1). Local, n <= 13 + six controls.

(1) INDEPENDENT ENUMERATION: rebuild both tables from the DEFINITION (place the boundary quads
    and the next k quads into real length-L arrays, compute the actual correlation contributions
    of the newly placed positions at lags L-1-d-j, collect reachable tuples and first-quad masks)
    and compare with the solver's dumped tables (WZ_FH_OUTER_DUMP). L=13 (d=3, k=3) and L=11 (k=2)
    exercise the k=3 guard that never fires for n <= 10.
(2) IDENTITY: every class n=6..13, budgets 0/1/30, canon on, OUTER_K = 0 vs 2 vs 3: uncapped runs
    give identical verdict, hit index and sequences; capped runs keep every old FOUND with the same
    sequences (aborts may resolve); charged nodes never increase.
(3) SIX CONTROLS: complete-only completion of the six known C,D with OUTER_K=0 and 3: all FOUND,
    identical A,B, nodes(3) <= nodes(0).
(4) C1: an injected verifier disagreement stops the arm with exit 5 and RESULT: INTERNAL ERROR.
"""
import itertools
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys
import tempfile

sys.path.insert(0, str(Path(__file__).resolve().parent))
from test_orbit_q import classes  # noqa: E402

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / 'src/solver/wz_match.cpp'
ENV = {k: v for k, v in os.environ.items() if not k.startswith(('WZ_', 'FH_'))}
SIX = {'ours-41': 'results/champions/champion_firsthit_bs42_41.txt', 'ours-42': 'results/champions/champion_firsthit_bs43_42.txt',
       'ours-43': 'results/champions/champion_firsthit_bs44_43.txt', 'WZ-41': 'results/reference/wz_table1_bs42_41.txt',
       'WZ-42': 'results/reference/wz_table1_bs43_42.txt', 'WZ-43': 'results/reference/wz_table1_bs44_43.txt'}


def run(binary, n, sig, settings):
    env = {**ENV, **{k: str(v) for k, v in settings.items()}}
    p = subprocess.run([str(binary), str(n), *map(str, sig)], env=env, text=True, capture_output=True, timeout=600)
    return p.returncode, p.stdout


def sem(o):
    out = []
    for l in o.splitlines():
        if l.startswith(('RESULT:', 'FIRSTHIT:', 'A =', 'B =', 'C =', 'D =')):
            out.append(re.sub(r' nodes_this_cand=\d+| elapsed=.*', '', l))
    return out


def independent_tables(POS, NEG, L, d, k):
    """Reachable (F_0..F_{k-1}) tuples and first-quad masks, from real arrays and real lags."""
    roots = [q for q in range(8) if NEG[q][0] == 1 and NEG[q][1] == 1]
    assert len(roots) == 2
    root_by_bit = {int(NEG[q][2] > 0): q for q in roots}
    tables = {}
    npat = 2 * 8 ** (k - 1)
    for pat in range(npat):
        A = [0] * L; B = [0] * L
        rb, rest = pat & 1, pat >> 1
        rq = NEG[root_by_bit[rb]]
        A[0], B[0], A[L - 1], B[L - 1] = rq
        for t in range(1, k):
            q = POS[rest % 8]; rest //= 8
            A[t], B[t], A[L - 1 - t], B[L - 1 - t] = q
        # known positions are [0,d) and [L-d, L); depths k..d-1 hold arbitrary positive quads,
        # which must NOT affect the selected lags: verify by trying two different fillings
        for filler in (0, 5):
            A2, B2 = A[:], B[:]
            for t in range(k, d):
                q = POS[filler]
                A2[t], B2[t], A2[L - 1 - t], B2[L - 1 - t] = q
            reach = {}
            for asg in itertools.product(range(8), repeat=k):
                A3, B3 = A2[:], B2[:]
                for r, qi in enumerate(asg):
                    q = POS[qi]
                    A3[d + r], B3[d + r], A3[L - 1 - d - r], B3[L - 1 - d - r] = q
                F = []
                for j in range(k):
                    s = L - 1 - d - j
                    # contribution of pairs (i, i+s) where at least one endpoint is newly placed
                    tot = 0
                    for i in range(L - s):
                        new_i = d <= i < d + k or L - d - k <= i < L - d
                        new_j = d <= i + s < d + k or L - d - k <= i + s < L - d
                        if new_i or new_j:
                            tot += A3[i] * A3[i + s] + B3[i] * B3[i + s]
                    F.append(tot)
                reach.setdefault(tuple(F), 0)
                reach[tuple(F)] |= 1 << asg[0]
            if filler == 0:
                first = reach
            else:
                assert reach == first, (pat, 'filler quads changed the outer tuples')
        tables[pat] = first
    return tables


def main():
    compiler = shutil.which('clang++') or shutil.which('g++')
    with tempfile.TemporaryDirectory(prefix='bs45-outer-') as name:
        b = Path(name) / 'current'
        subprocess.run([compiler, '-O3', '-std=c++17', str(SOURCE), '-o', str(b)], check=True)
        rc, dump = run(b, 12, (1, 7, 0, 0), {'WZ_FIRSTHIT': 1, 'WZ_FH_OUTER_DUMP': 1})
        POS = {int(m[1]): tuple(int(m.group(i)) for i in range(2, 6)) for m in re.finditer(r'^POS (\d) (-?1) (-?1) (-?1) (-?1)', dump, re.M)}
        NEG = {int(m[1]): tuple(int(m.group(i)) for i in range(2, 6)) for m in re.finditer(r'^NEG (\d) (-?1) (-?1) (-?1) (-?1)', dump, re.M)}
        K2 = {(int(m[1]), (int(m[2]), int(m[3]))): int(m[4]) for m in re.finditer(r'^K2 (\d+) (-?\d+) (-?\d+) (\d+)', dump, re.M)}
        K3 = {(int(m[1]), (int(m[2]), int(m[3]), int(m[4]))): int(m[5]) for m in re.finditer(r'^K3 (\d+) (-?\d+) (-?\d+) (-?\d+) (\d+)', dump, re.M)}
        for L, d, k, tab in ((11, 3, 2, K2), (13, 3, 3, K3), (13, 4, 2, K2)):
            ind = independent_tables(POS, NEG, L, d, k)
            got = {}
            for (pat, F), mask in tab.items():
                got.setdefault(pat, {})[F] = mask
            assert got == ind, (L, d, k, 'table differs from independent enumeration')
        print(f'(1) tables match the independent enumeration: K2 {len(K2)} entries, K3 {len(K3)} entries', flush=True)
        base = dict(WZ_FIRSTHIT=1, WZ_FH_M6=1, WZ_FH_AB_PROF=1, WZ_THM211B=1, WZ_THM212=1, WZ_FH_ORBIT_CANON=1,
                    WZ_FH_PROF_ORDER=1, WZ_FH_MAX_CAND=400, WZ_FH_PROG_SEC=9999)
        runs = kept = moved = 0
        for n in range(6, 14):
            for sig in classes(n):
                for budget in (0, 1, 30):
                    outs = {}
                    for K in (0, 2, 3):
                        rc, o = run(b, n, sig, {**base, 'WZ_FH_AB_BUDGET': budget, 'WZ_FH_OUTER_K': K})
                        outs[K] = o
                    nodes = {K: int(re.search(r'total_AB_nodes=(\d+)', o.split('backtracks_entered=')[-1])[1]) for K, o in outs.items()}
                    assert nodes[2] <= nodes[0] and nodes[3] <= nodes[0], (n, sig, budget, nodes)
                    if budget == 0:
                        assert sem(outs[0]) == sem(outs[2]) == sem(outs[3]), (n, sig, 'uncapped identity')
                    else:
                        if 'RESULT: FOUND' in outs[0]:
                            # Under a cap an earlier candidate that used to abort may now hit (allowed), so
                            # the reported hit may move. Retention rule: the OLD hit candidate is still
                            # completable under the new setting (uncapped complete-only), with <= nodes.
                            c0 = re.search(r'^C = \{([^}]*)\}', outs[0], re.M)[1]
                            d0 = re.search(r'^D = \{([^}]*)\}', outs[0], re.M)[1]
                            for K in (2, 3):
                                if sem(outs[0]) == sem(outs[K]):
                                    continue
                                assert 'RESULT: FOUND' in outs[K], (n, sig, budget, K, 'hit lost under cap')
                                co = {}
                                for KK in (0, K):  # complete-only (unconstrained rows) in both settings
                                    rc, o = run(b, n, sig, {'WZ_FIRSTHIT': 1, 'WZ_FH_M6': 1, 'WZ_FH_AB_BUDGET': 0,
                                                            'WZ_FH_OUTER_K': KK, 'WZ_FH_COMPLETE_C': c0, 'WZ_FH_COMPLETE_D': d0})
                                    m = re.search(r'^COMPLETE_ONLY r=(\d) nodes=(\d+)', o, re.M)
                                    co[KK] = (int(m[1]), int(m[2]))
                                assert co[K][0] == 0 and co[K][1] <= co[0][1], (n, sig, budget, K, 'old hit candidate no longer completes', co)
                                moved += 1
                            kept += 1
                    runs += 1
        print(f'(2) identity: {runs} class/budget runs, nodes never increase; {kept} capped hits retained ({moved} moved to an earlier resolving candidate, old hit still completes)', flush=True)
        for name_, f in SIX.items():
            rows = [l.split() for l in (ROOT / f).read_text().splitlines() if l.strip() and not l.startswith('#')]
            n = int(rows[0][0]); sig = tuple(int(x) for x in rows[1][:4])
            res = {}
            for K in (0, 3):
                rc, o = run(b, n, sig, {'WZ_FIRSTHIT': 1, 'WZ_FH_M6': 1, 'WZ_FH_AB_BUDGET': 0, 'WZ_FH_OUTER_K': K,
                                        'WZ_FH_COMPLETE_C': ','.join(rows[4]), 'WZ_FH_COMPLETE_D': ','.join(rows[5])})
                m = re.search(r'^COMPLETE_ONLY r=(\d) nodes=(\d+)', o, re.M)
                assert m and m[1] == '0', (name_, K, o[-300:])
                res[K] = (int(m[2]), re.search(r'^A = .*', o, re.M)[0], re.search(r'^B = .*', o, re.M)[0])
            assert res[0][1:] == res[3][1:] and res[3][0] <= res[0][0], (name_, res)
            print(f'(3) {name_}: FOUND, identical A,B; nodes {res[0][0]} -> {res[3][0]}', flush=True)
        rc, o = run(b, 6, (5, 1, 0, 0), {**base, 'WZ_FH_AB_BUDGET': 0, 'WZ_FH_TEST_INTERNAL_ERROR': 1})
        assert rc == 5 and 'RESULT: INTERNAL ERROR' in o and 'FH_INTERNAL_ERROR' in o, (rc, o[-300:])
        print('(4) injected verifier disagreement: exit 5, RESULT: INTERNAL ERROR, no ordinary outcome', flush=True)
        print('PASS: A1 outer tables exact, identity holds, six controls complete identically, C1 fatal path works.')


if __name__ == '__main__':
    main()
