#!/usr/bin/env python3
"""WZ_FH_CD_PRUNE identity gate (2026-09-25, Astra Pass H review items 2-3). Local, n <= 13.

Both prunes may only cut C,D DFS subtrees that contain no profile-matching leaf, so for every
signature class at n = 6..13 (odd n takes the unchanged path), forward and reversed streams,
flat and reversed cell order, orbit canon on and off, the emitted candidate stream (WZ_FH_DUMP
bytes), the per-cell CELLSIZE counts and the firsthit verdict/hit must be IDENTICAL for
CD_PRUNE = 0, 1, 2, 3. Reports the DFS-visit reduction (CDSTAT) as information only.
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


def run(binary, args, settings, dump):
    env = {**ENV, **{k: str(v) for k, v in settings.items()}, 'WZ_FH_DUMP': str(dump)}
    p = subprocess.run([str(binary), *map(str, args)], env=env, text=True, capture_output=True, timeout=300)
    assert p.returncode in (0, 3), p.stderr[-400:]
    return p.stdout, dump.read_bytes()


def semantic(out):
    return [re.sub(r' elapsed=.*', '', l) for l in out.splitlines()
            if l.startswith(('RESULT:', 'candidates_streamed=', 'backtracks_entered=', 'FIRSTHIT:',
                             'A =', 'B =', 'C =', 'D ='))]


def main():
    compiler = shutil.which('clang++') or shutil.which('g++')
    with tempfile.TemporaryDirectory(prefix='bs45-cdprune-') as name:
        tmp = Path(name)
        binary = tmp / 'current'
        subprocess.run([compiler, '-O3', '-std=c++17', str(SOURCE), '-o', str(binary)], check=True)
        base = dict(WZ_FIRSTHIT=1, WZ_FH_M6=1, WZ_THM211B=1, WZ_THM212=1, WZ_FH_PROG_SEC=9999)
        runs = 0
        nodes = {0: 0, 1: 0, 2: 0, 3: 0}
        for n in (6, 7, 8, 9, 10, 11, 12, 13):
            for sig in classes(n):
                for rev, order, canon in itertools.product((0, 1), (1, 2), (0, 1)):
                    cfg = {**base, 'WZ_FH_STREAM_REV': rev, 'WZ_FH_PROF_ORDER': order,
                           'WZ_FH_ORBIT_CANON': canon}
                    ref = None
                    for mode in (0, 1, 2, 3):
                        # count-only pass: exact stream + per-cell counts + DFS visits
                        out, dump = run(binary, (n, *sig), {**cfg, 'WZ_FH_CELLSIZE': 10**9,
                                                            'WZ_FH_CD_PRUNE': mode}, tmp / f'd{mode}')
                        cells = re.findall(r'^CELLSIZE pi=(\d+) cand=(\d+)', out, re.M)
                        nodes[mode] += sum(int(x) for x in re.findall(r'^CDSTAT pi=\d+ dfs_nodes=(\d+)', out, re.M))
                        # real search with exact completion: verdict and hit identical
                        full, _ = run(binary, (n, *sig), {**cfg, 'WZ_FH_AB_BUDGET': 0, 'WZ_FH_CD_PRUNE': mode},
                                      tmp / f'f{mode}')
                        key = (dump, cells, semantic(full))
                        if ref is None:
                            ref = key
                        assert key == ref, (n, sig, rev, order, canon, mode, 'stream/verdict differs')
                        runs += 1
        red = {m: 100 * (1 - nodes[m] / max(1, nodes[0])) for m in (1, 2, 3)}
        print(f'PASS: {runs} runs over n=6..13: emitted stream bytes, per-cell counts and exact verdicts '
              f'identical for CD_PRUNE 0/1/2/3. DFS visits vs off: root {red[1]:.1f}% fewer, '
              f'reach {red[2]:.1f}% fewer, both {red[3]:.1f}% fewer (small-n information only).', flush=True)


if __name__ == '__main__':
    main()
