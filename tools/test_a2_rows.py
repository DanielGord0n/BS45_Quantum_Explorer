#!/usr/bin/env python3
"""A2 exact profile-row reachability gate (Astra deep dive 2026-09-30 A2 + 2026-10-01 reply).

(1) FORMULAS vs BRUTE FORCE (python only, independent of the solver): for q = 0..4 remaining
    positive quads, enumerate every assignment (8 sign vectors per distinct-block quad; (+-2,+-2)
    or (0,0) per self-block quad; (+-1,+-1) for a free middle) and compare the reachable residual
    sets with Astra's closed forms (A2.1 distinct, A2.2 self, 4-choice middle).
(2) GEOMETRY + PREDICATE vs SOLVER (WZ_FH_A2_DUMP): for L in 45, 6, 7, 9, 12, 13 and m in 3, 6, the
    solver's blocks and per-depth remaining counts must equal a direct position enumeration under
    the reflection r -> (L-1-r) mod m, and its predicate must agree with the python formulas on
    1500 random residual rows per depth.
(3) IDENTITY (full stream, allowed rows on, canon on), n = 6..13, every class, budgets 0/1/30,
    A2 = 0 vs 1 (shadow) vs 2 (prune): shadow output identical to off with hit_under_cut = 0;
    prune uncapped identical verdict/hit/sequences, capped keeps every FOUND (NPAF-verified by the
    solver), nodes never increase.
(4) SIX CONTROLS (complete-only + WZ_FH_A2_ROWCHECK): each known solution's own profile row, as
    a one-row allowed set, re-completes to the same A,B under A2 = 0, 1, 2; shadow hit_under_cut=0.
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
H = [(1, 1, 1, 1), (1, -1, 1, -1), (1, 1, -1, -1), (1, -1, -1, 1)]
DISTINCT_STEPS = [tuple(sgn * x for x in col) for col in H for sgn in (1, -1)]  # the 8 positive quads
SELF_STEPS = [(2, 2), (2, -2), (-2, 2), (-2, -2), (0, 0)]


def distinct_ok(delta, q):
    t = [sum(h * d for h, d in zip(row, delta)) for row in H]
    if any(x % 4 for x in t):
        return False
    l1 = sum(abs(x) // 4 for x in t)
    return l1 <= q and (q - l1) % 2 == 0


def self_ok(u, v, q):
    return u % 2 == 0 and v % 2 == 0 and max(abs(u), abs(v)) // 2 <= q and (u // 2 - v // 2) % 2 == 0


def mid_ok(u, v, q):
    return any(self_ok(u - ea, v - eb, q) for ea in (-1, 1) for eb in (-1, 1))


def run(binary, n, sig, settings):
    env = {**ENV, **{k: str(v) for k, v in settings.items()}}
    p = subprocess.run([str(binary), str(n), *map(str, sig)], env=env, text=True, capture_output=True, timeout=900)
    return p.returncode, p.stdout


def sem(o):
    out = []
    for l in o.splitlines():
        if l.startswith(('RESULT:', 'FIRSTHIT:', 'A =', 'B =', 'C =', 'D =')):
            out.append(re.sub(r' nodes_this_cand=\d+| elapsed=.*', '', l))
    return out


def nodes_of(o):
    return int(re.search(r'total_AB_nodes=(\d+)', o.split('backtracks_entered=')[-1])[1])


def main():
    # (1) formulas vs brute force
    for q in range(0, 5):
        reach = set()
        for asg in itertools.product(DISTINCT_STEPS, repeat=q):
            reach.add(tuple(sum(s[i] for s in asg) for i in range(4)))
        box = range(-2 * q - 2, 2 * q + 3)
        form = {d for d in itertools.product(box, repeat=4) if distinct_ok(d, q)}
        assert reach == form, ('distinct', q, len(reach), len(form))
        reach_s = set()
        for asg in itertools.product(SELF_STEPS, repeat=q):
            reach_s.add((sum(s[0] for s in asg), sum(s[1] for s in asg)))
        box2 = range(-2 * q - 3, 2 * q + 4)
        form_s = {(u, v) for u in box2 for v in box2 if self_ok(u, v, q)}
        assert reach_s == form_s, ('self', q, len(reach_s), len(form_s))
        reach_m = {(u + ea, v + eb) for (u, v) in reach_s for ea in (-1, 1) for eb in (-1, 1)}
        form_m = {(u, v) for u in box2 for v in box2 if mid_ok(u, v, q)}
        assert reach_m == form_m, ('middle', q, len(reach_m), len(form_m))
    print('(1) formulas == brute force for q=0..4: distinct (8^q assignments), self (5^q), self + free middle', flush=True)

    compiler = shutil.which('clang++') or shutil.which('g++')
    with tempfile.TemporaryDirectory(prefix='bs45-a2-') as name:
        b = Path(name) / 'current'
        subprocess.run([compiler, '-O3', '-std=c++17', str(SOURCE), '-o', str(b)], check=True)
        # (2) geometry + predicate vs solver
        evals = 0
        for n in (44, 5, 6, 8, 11, 12):
            L = n + 1
            for m6 in (1, 0):
                m = 6 if m6 else 3
                sig = (1, 7, 8, 8) if n == 44 else classes(n)[0]
                rc, o = run(b, n, sig, {'WZ_FIRSTHIT': 1, 'WZ_FH_M6': m6, 'WZ_FH_A2_DUMP': 1})
                blk = {int(x[0]): (int(x[1]), int(x[2]), int(x[3])) for x in re.findall(r'^A2BLK (\d+) (\d+) (\d+) (\d+)', o, re.M)}
                half = L // 2
                refl = lambda c: (L - 1 - c) % m  # noqa: E731
                exp_blocks, seen = [], set()
                for c in range(m):
                    if c in seen:
                        continue
                    s = refl(c); seen.update({c, s})
                    exp_blocks.append((c, s, 1 if (L % 2 == 1 and (half % m) in (c, s)) else 0))
                assert [blk[i] for i in range(len(blk))] == exp_blocks, (L, m, blk, exp_blocks)
                blk_of = {}
                for i, (c, s, _) in enumerate(exp_blocks):
                    blk_of[c] = i; blk_of[s] = i
                for mm in re.finditer(r'^A2CNT (\d+)((?: \d+)+)', o, re.M):
                    p = int(mm[1]); cnt = [int(x) for x in mm[2].split()]
                    exp = [0] * len(exp_blocks)
                    for i in range(p, half):
                        exp[blk_of[i % m]] += 1
                        assert blk_of[i % m] == blk_of[(L - 1 - i) % m]
                    assert cnt == exp, (L, m, p, cnt, exp)
                cnt_at = {int(mm[1]): [int(x) for x in mm[2].split()] for mm in re.finditer(r'^A2CNT (\d+)((?: \d+)+)', o, re.M)}
                for mm in re.finditer(r'^A2EVAL (\d+)((?: -?\d+)+) ([01])$', o, re.M):
                    p = int(mm[1]); vals = [int(x) for x in mm[2].split()]; got = mm[3] == '1'
                    k, r = vals[:m], vals[m:]
                    ok = True
                    for i, (c, s, mid) in enumerate(exp_blocks):
                        q = cnt_at[p][i]
                        if c != s:
                            ok = ok and distinct_ok((k[c], r[c], k[s], r[s]), q)
                        elif mid:
                            ok = ok and mid_ok(k[c], r[c], q)
                        else:
                            ok = ok and self_ok(k[c], r[c], q)
                    assert ok == got, (L, m, p, vals, ok, got)
                    evals += 1
        print(f'(2) solver blocks + remaining counts == position enumeration (L=45,6,7,9,12,13; m=3,6); predicate agrees on {evals} sampled rows', flush=True)
        # (3) identity
        base = dict(WZ_FIRSTHIT=1, WZ_FH_M6=1, WZ_FH_AB_PROF=1, WZ_THM211B=1, WZ_THM212=1, WZ_FH_ORBIT_CANON=1,
                    WZ_FH_PROF_ORDER=1, WZ_FH_MAX_CAND=400, WZ_FH_PROG_SEC=9999)
        runs = kept = cuts = saved = tot = 0
        for n in range(6, 14):
            for sig in classes(n):
                for budget in (0, 1, 30):
                    outs = {}
                    for mode in (0, 1, 2):
                        rc, o = run(b, n, sig, {**base, 'WZ_FH_AB_BUDGET': budget, 'WZ_FH_A2': mode})
                        outs[mode] = o
                    nodes = {m_: nodes_of(o) for m_, o in outs.items()}
                    assert sem(outs[0]) == sem(outs[1]) and nodes[1] == nodes[0], (n, sig, budget, 'shadow changed the search')
                    sh = re.search(r'^A2SHADOW .*hit_under_cut=(\d+)', outs[1], re.M)
                    assert sh and sh[1] == '0', (n, sig, budget, 'hit under a would-be cut')
                    cuts += int(re.search(r'cuts=(\d+)', sh[0])[1]); saved += int(re.search(r'saved_nodes=(\d+)', sh[0])[1]); tot += nodes[0]
                    if budget == 0:
                        # uncapped: same traversal minus pruned subtrees => identical outcome, fewer nodes.
                        # Under a cap the budget charges quad trials, so a pruned candidate can run past
                        # the point where it used to abort and visit nodes it never reached before
                        # (n=9 (2,4,3,3) budget 30: 210 -> 212); total nodes are not monotone there.
                        assert nodes[2] <= nodes[0], (n, sig, budget, nodes)
                        assert sem(outs[0]) == sem(outs[2]), (n, sig, 'uncapped identity')
                    elif 'RESULT: FOUND' in outs[0]:
                        assert 'RESULT: FOUND' in outs[2] and 'NPAF==0 confirmed' in outs[2], (n, sig, budget, 'capped hit lost')
                        kept += 1
                    runs += 1
        print(f'(3) identity: {runs} class/budget runs; shadow identical to off, 0 hits under would-be cuts; prune uncapped identical, '
              f'{kept} capped hits retained, nodes never increase; shadow would cut {cuts} nodes saving {saved}/{tot} nodes ({100 * saved / max(1, tot):.1f}%)', flush=True)
        # (4) six controls
        for name_, f in SIX.items():
            rows = [l.split() for l in (ROOT / f).read_text().splitlines() if l.strip() and not l.startswith('#')]
            n = int(rows[0][0]); sig = tuple(int(x) for x in rows[1][:4])
            rc, o = run(b, n, sig, {'WZ_FIRSTHIT': 1, 'WZ_FH_M6': 1, 'WZ_FH_AB_BUDGET': 0, 'WZ_FH_A2_ROWCHECK': 1,
                                    'WZ_FH_COMPLETE_C': ','.join(rows[4]), 'WZ_FH_COMPLETE_D': ','.join(rows[5])})
            assert re.search(r'^COMPLETE_ONLY r=0', o, re.M), (name_, o[-300:])
            rc_ = {int(m_[1]): (int(m_[2]), int(m_[3]), int(m_[4]), int(m_[5])) for m_ in
                   re.finditer(r'^A2ROWCHECK mode=(\d) r=(\d) nodes=(\d+) same_AB=(\d) hit_under_cut=(\d+)', o, re.M)}
            assert sorted(rc_) == [0, 1, 2], (name_, o[-400:])
            for mode in (0, 1, 2):
                assert rc_[mode][0] == 0 and rc_[mode][2] == 1, (name_, mode, rc_)
            assert rc_[1][3] == 0 and rc_[1][1] == rc_[0][1] and rc_[2][1] <= rc_[0][1], (name_, rc_)
            print(f'(4) {name_}: own row re-completes to the same A,B under A2 off/shadow/prune; nodes {rc_[0][1]} -> {rc_[2][1]}', flush=True)
        print('PASS: A2 formulas exact, solver predicate and geometry match, identity holds, six controls keep their rows.')


if __name__ == '__main__':
    main()
