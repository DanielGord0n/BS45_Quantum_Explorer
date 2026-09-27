#!/usr/bin/env python3
"""WZ_FH_HALL_FAST identity gate (2026-09-26). Local, n <= 13.

The pair-only, adaptively ordered leaf test must accept exactly the leaves the original
three-test conjunction accepts: for every signature class at n = 6..13, forward and reversed
streams, flat and reversed cell order, canon on/off, the emitted stream bytes (WZ_FH_DUMP),
per-cell CELLSIZE counts and exact search verdicts must be IDENTICAL for HALL_FAST = 0 and 1.
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
from test_cd_prune import run, semantic  # noqa: E402

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / 'src/solver/wz_match.cpp'


def main():
    compiler = shutil.which('clang++') or shutil.which('g++')
    with tempfile.TemporaryDirectory(prefix='bs45-hallfast-') as name:
        tmp = Path(name)
        binary = tmp / 'current'
        subprocess.run([compiler, '-O3', '-std=c++17', str(SOURCE), '-o', str(binary)], check=True)
        base = dict(WZ_FIRSTHIT=1, WZ_FH_M6=1, WZ_THM211B=1, WZ_THM212=1, WZ_FH_PROG_SEC=9999)
        runs = leaves = 0
        for n in range(6, 14):
            for sig in classes(n):
                for rev, order, canon in itertools.product((0, 1), (1, 2), (0, 1)):
                    cfg = {**base, 'WZ_FH_STREAM_REV': rev, 'WZ_FH_PROF_ORDER': order, 'WZ_FH_ORBIT_CANON': canon}
                    ref = None
                    for fast in (0, 1):
                        out, dump = run(binary, (n, *sig), {**cfg, 'WZ_FH_CELLSIZE': 10**9, 'WZ_FH_HALL_FAST': fast},
                                        tmp / f'd{fast}')
                        cells = re.findall(r'^CELLSIZE pi=(\d+) cand=(\d+) .* leaves=(\d+) hall_ok=(\d+)', out, re.M)
                        full, _ = run(binary, (n, *sig), {**cfg, 'WZ_FH_AB_BUDGET': 0, 'WZ_FH_HALL_FAST': fast},
                                      tmp / f'f{fast}')
                        key = (dump, cells, semantic(full))
                        if ref is None:
                            ref = key
                            leaves += sum(int(c[2]) for c in cells)
                        assert key == ref, (n, sig, rev, order, canon, 'HALL_FAST changed the stream or verdict')
                        runs += 1
        print(f'PASS: {runs} runs over n=6..13 ({leaves} leaves): stream bytes, per-cell counts, leaf/accept '
              'counts and exact verdicts identical for HALL_FAST 0/1.', flush=True)


if __name__ == '__main__':
    main()
