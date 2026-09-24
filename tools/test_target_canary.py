#!/usr/bin/env python3
"""WZ_FH_TARGET canary-planning gate (2026-09-24). Local, n <= 10 only.

For known solutions at n=8 and n=10 (brute force, see test_orbit_q_retention.py), with the
64-group canonicalization on: (1) LOCATE finds the kept cell holding an image; (2) the
count-only TARGET mode reports that image's (idx, batch, rank_in_batch) in that cell;
(3) a real search of only that cell with DRAIN_TOP=rank+1, DRAIN_BATCHES=batch+1 and exact
completion must report FOUND (NPAF-verified), at the predicted idx whenever the hit is
the target image itself, and never after it in drain order.
A tiny buffer (BUF_CAP=2) forces multi-batch cells so the batch arithmetic is exercised.
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

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / 'src/solver/wz_match.cpp'
ENV = {k: v for k, v in os.environ.items() if not k.startswith(('WZ_', 'FH_'))}
BUF = 2


def run(binary, n, sig, settings):
    env = {**ENV, **{k: str(v) for k, v in settings.items()}}
    p = subprocess.run([str(binary), str(n), *map(str, sig)], env=env, text=True,
                       capture_output=True, timeout=120)
    assert p.returncode in (0, 3), p.stderr[-400:]
    return p.stdout


def main():
    compiler = shutil.which('clang++') or shutil.which('g++')
    with tempfile.TemporaryDirectory(prefix='bs45-target-') as name:
        binary = Path(name) / 'current'
        subprocess.run([compiler, '-O3', '-std=c++17', str(SOURCE), '-o', str(binary)], check=True)
        base = dict(WZ_FIRSTHIT=1, WZ_FH_M6=1, WZ_FH_ORBIT_CANON=1, WZ_FH_ORBIT_Q=1,
                    WZ_FH_PROF_ORDER=1, WZ_FH_PROG_SEC=9999, WZ_FH_BUF_CAP=BUF)
        checked = exact = deeper_batches = 0
        for n in (8, 10):
            reps = {}
            for C, D, abs_ in solutions(n):
                for (a, b) in abs_:
                    reps.setdefault((a, b, canon64(C, D)), (C, D, a, b))
            for (C, D, a, b) in list(reps.values())[:60]:
                sig = (a, b, sum(C), sum(D))
                cs, ds = ','.join(map(str, C)), ','.join(map(str, D))
                loc = run(binary, n, sig, {**base, 'WZ_FH_LOCATE_C': cs, 'WZ_FH_LOCATE_D': ds})
                kept = sorted({int(m) for m in re.findall(r'cell_idx=(\d+) canon_kept=YES', loc)})
                assert kept, (n, sig, 'no kept cell')
                pi = kept[0]
                cell = {'WZ_FH_PROF_SKIP': pi, 'WZ_FH_PROF_END': pi + 1}
                tgt = run(binary, n, sig, {**base, **cell, 'WZ_FH_CELLSIZE': 10**9,
                                           'WZ_FH_TARGET_C': cs, 'WZ_FH_TARGET_D': ds})
                m = re.search(r'TARGET pi=(\d+) idx=(\d+) batch=(\d+) rank_in_batch=(\d+) score=(\d+)', tgt)
                assert m, (n, sig, pi, 'target not streamed in its kept cell', tgt[-600:])
                tpi, idx, batch, rank, score = map(int, m.groups())
                assert tpi == pi and (idx - 1) // BUF == batch and rank < BUF
                deeper_batches += batch > 0
                out = run(binary, n, sig, {**base, **cell, 'WZ_FH_AB_BUDGET': 0,
                                           'WZ_FH_DRAIN_TOP': rank + 1, 'WZ_FH_DRAIN_BATCHES': batch + 1})
                assert 'RESULT: FOUND' in out, (n, sig, pi, idx, batch, rank, out[-800:])
                ver = subprocess.run(['python3', str(ROOT / 'tools/verify_npaf.py')], input=out,
                                     text=True, capture_output=True)
                assert ver.returncode == 0, ver.stdout + ver.stderr
                hit = re.search(r'FIRSTHIT: idx=(\d+)', out)
                hidx = int(hit[1])
                hc = [int(x) for x in re.search(r'^C = \{([^}]*)\}', out, re.M)[1].split(',')]
                hd = [int(x) for x in re.search(r'^D = \{([^}]*)\}', out, re.M)[1].split(',')]
                if canon64(tuple(hc), tuple(hd)) == canon64(C, D) and hidx == idx:
                    exact += 1
                else:
                    # a different solution may come first, but only within the drained set
                    assert (hidx - 1) // BUF <= batch, (n, sig, 'hit after the target batch')
                checked += 1
        print(f'PASS: {checked} solution canaries (n=8,10, Q on): target located in its kept cell, '
              f're-found with DRAIN_TOP=rank+1/DRAIN_BATCHES=batch+1; {exact} at exactly the predicted '
              f'idx, the rest by an earlier solution in the drained set; {deeper_batches} targets beyond batch 0.',
              flush=True)


if __name__ == '__main__':
    main()
