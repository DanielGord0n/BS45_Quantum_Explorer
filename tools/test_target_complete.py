#!/usr/bin/env python3
"""WZ_FH_TARGET_COMPLETE gate (canary v3, 2026-09-26). Local, n = 8, 10.

For known solutions (brute force), with Q canonicalization on: LOCATE the kept cell, then run
count-only TARGET mode with TARGET_COMPLETE=1 on that cell. The run must print TARGET_COMPLETE,
then FOUND, the found C,D must lie in the target's 64-orbit, verify_npaf must PASS, and the
run must stop right there (candidates_streamed == the target's idx).
"""
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys
import tempfile

sys.path.insert(0, str(Path(__file__).resolve().parent))
from test_orbit_q_retention import canon64, solutions  # noqa: E402
from test_target_canary import run  # noqa: E402

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / 'src/solver/wz_match.cpp'


def main():
    compiler = shutil.which('clang++') or shutil.which('g++')
    with tempfile.TemporaryDirectory(prefix='bs45-tc-') as name:
        binary = Path(name) / 'current'
        subprocess.run([compiler, '-O3', '-std=c++17', str(SOURCE), '-o', str(binary)], check=True)
        base = dict(WZ_FIRSTHIT=1, WZ_FH_M6=1, WZ_FH_ORBIT_CANON=1, WZ_FH_ORBIT_Q=1, WZ_FH_PROF_ORDER=1,
                    WZ_FH_PROG_SEC=9999)
        checked = 0
        for n in (8, 10):
            reps = {}
            for C, D, abs_ in solutions(n):
                for (a, b) in abs_:
                    reps.setdefault((a, b, canon64(C, D)), (C, D, a, b))
            for (C, D, a, b) in list(reps.values())[:40]:
                sig = (a, b, sum(C), sum(D))
                cs, ds = ','.join(map(str, C)), ','.join(map(str, D))
                loc = run(binary, n, sig, {**base, 'WZ_FH_LOCATE_C': cs, 'WZ_FH_LOCATE_D': ds})
                pi = sorted({int(m) for m in re.findall(r'cell_idx=(\d+) canon_kept=YES', loc)})[0]
                out = run(binary, n, sig, {**base, 'WZ_FH_PROF_SKIP': pi, 'WZ_FH_PROF_END': pi + 1,
                                           'WZ_FH_CELLSIZE': 10**9, 'WZ_FH_TARGET_C': cs, 'WZ_FH_TARGET_D': ds,
                                           'WZ_FH_TARGET_COMPLETE': 1, 'WZ_FH_AB_BUDGET': 0})
                tc = re.search(r'^TARGET_COMPLETE pi=(\d+) idx=(\d+)', out, re.M)
                assert tc and int(tc[1]) == pi, (n, sig, pi, out[-500:])
                assert 'RESULT: FOUND' in out, (n, sig, pi, 'no FOUND', out[-500:])
                hc = tuple(int(x) for x in re.search(r'^C = \{([^}]*)\}', out, re.M)[1].split(','))
                hd = tuple(int(x) for x in re.search(r'^D = \{([^}]*)\}', out, re.M)[1].split(','))
                assert canon64(hc, hd) == canon64(C, D), (n, sig, 'completed a different orbit')
                streamed = int(re.search(r'^candidates_streamed=(\d+)', out, re.M)[1])
                assert streamed == int(tc[2]), (n, sig, 'did not stop at the target')
                ver = subprocess.run(['python3', str(ROOT / 'tools/verify_npaf.py')], input=out, text=True,
                                     capture_output=True)
                assert ver.returncode == 0, ver.stdout + ver.stderr
                checked += 1
        print(f'PASS: {checked} known solutions (n=8,10, Q on): TARGET_COMPLETE completes the located image '
              'in its kept cell, FOUND is in the target orbit and NPAF-verified, and the run stops at the target.',
              flush=True)


if __name__ == '__main__':
    main()
