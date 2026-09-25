#!/usr/bin/env python3
"""WZ_FH_MID_SOLVE gate (2026-09-26, Astra red-team item 3). Local, n <= 12.

Skipping middle pairs with wrong final |sums| before placing must leave verdicts, hit index,
hit sequences, completion counts and budget aborts IDENTICAL; only the node statistics
(total_AB_nodes, nodes_this_cand) may drop.
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
ENV = {k: v for k, v in os.environ.items() if not k.startswith(('WZ_', 'FH_'))}


def sem(o):
    out = []
    for l in o.splitlines():
        if l.startswith(('RESULT:', 'candidates_streamed=', 'FIRSTHIT:', 'A =', 'B =', 'C =', 'D =', 'backtracks_entered=')):
            l = re.sub(r' elapsed=.*', '', l)
            l = re.sub(r'(total_AB_nodes|nodes_this_cand)=\d+', r'\1=*', l)
            out.append(l)
    return out


def main():
    compiler = shutil.which('clang++') or shutil.which('g++')
    with tempfile.TemporaryDirectory(prefix='bs45-mid-') as name:
        b = str(Path(name) / 'current')
        subprocess.run([compiler, '-O3', '-std=c++17', str(ROOT / 'src/solver/wz_match.cpp'), '-o', b], check=True)
        runs = n0 = n1 = 0
        tot = lambda o: int(re.search(r'total_AB_nodes=(\d+)', o.split('backtracks_entered=')[-1])[1])
        for n in (6, 8, 10, 12):
            for sig in classes(n):
                for budget, canon, rev in itertools.product((0, 1, 30), (0, 1), (0, 1)):
                    cfg = {**ENV, 'WZ_FIRSTHIT': '1', 'WZ_FH_M6': '1', 'WZ_FH_AB_PROF': '1', 'WZ_THM211B': '1',
                           'WZ_THM212': '1', 'WZ_FH_ORBIT_CANON': str(canon), 'WZ_FH_STREAM_REV': str(rev),
                           'WZ_FH_PROF_ORDER': '1', 'WZ_FH_AB_BUDGET': str(budget), 'WZ_FH_MAX_CAND': '400',
                           'WZ_FH_PROG_SEC': '9999'}
                    o0 = subprocess.run([b, str(n), *map(str, sig)], env=cfg, capture_output=True, text=True).stdout
                    o1 = subprocess.run([b, str(n), *map(str, sig)], env={**cfg, 'WZ_FH_MID_SOLVE': '1'},
                                        capture_output=True, text=True).stdout
                    assert sem(o0) == sem(o1), (n, sig, budget, canon, rev)
                    n0 += tot(o0); n1 += tot(o1); runs += 1
        print(f'PASS: {runs} runs (n=6..12, budgets 0/1/30, canon on/off, fwd/rev): verdicts, hits, sequences, '
              f'completions and aborts identical with MID_SOLVE on/off; total_AB_nodes {n0} -> {n1} '
              f'({100 * (1 - n1 / max(1, n0)):.1f}% fewer).', flush=True)


if __name__ == '__main__':
    main()
