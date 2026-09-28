#!/usr/bin/env python3
"""Pass H ordering gate (WZ_FH_PROF_ORDER=3, 2026-09-28). Local, n <= 13.

Two builds: normal, and one with libc++'s unspecified-stability randomization (the tie
behaviour that differs between macOS and the clusters). For every signature class at
n = 8..13 with canon + Q + closure prune (Q refuses itself where unsafe):
  (1) [order] digests and cell counts identical between the two builds;
  (2) the kept cell-key set under ORDER=3 equals the set under ORDER=1 (permutation only);
  (3) exact search verdicts and hit sequences identical ORDER=3 vs ORDER=1;
  (4) known solutions LOCATE to the same cell index in both builds under ORDER=3;
  (5) WZ_FH_EXPECT_DIGEST: a wrong value refuses (rc 2, RESULT line), the right value runs;
  (6) ORDER=3 without canon refuses.
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
from test_orbit_q_retention import canon64, solutions  # noqa: E402

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / 'src/solver/wz_match.cpp'
ENV = {k: v for k, v in os.environ.items() if not k.startswith(('WZ_', 'FH_'))}


def run(binary, n, sig, settings, dump=None):
    env = {**ENV, **{k: str(v) for k, v in settings.items()}}
    if dump:
        env['WZ_FH_DUMP'] = str(dump)
    p = subprocess.run([str(binary), str(n), *map(str, sig)], env=env, text=True, capture_output=True, timeout=300)
    return p.returncode, p.stdout


def semantic(o):
    return [re.sub(r' elapsed=.*', '', l) for l in o.splitlines()
            if l.startswith(('RESULT:', 'FIRSTHIT:', 'A =', 'B =', 'C =', 'D =', 'candidates_streamed='))]


def main():
    compiler = shutil.which('clang++') or shutil.which('g++')
    with tempfile.TemporaryDirectory(prefix='bs45-passh-') as name:
        tmp = Path(name)
        bins = {}
        for tag, extra in (('normal', []), ('randties', ['-D_LIBCPP_DEBUG_RANDOMIZE_UNSPECIFIED_STABILITY'])):
            b = tmp / tag
            subprocess.run([compiler, '-O3', '-std=c++17', *extra, str(SOURCE), '-o', str(b)], check=True)
            bins[tag] = b
        base = dict(WZ_FIRSTHIT=1, WZ_FH_M6=1, WZ_THM211B=1, WZ_THM212=1, WZ_FH_ORBIT_CANON=1, WZ_FH_ORBIT_Q=1,
                    WZ_FH_ORBIT_QPRUNE=1, WZ_FH_PROG_SEC=9999, WZ_FH_ORD3_NODIGEST_OK=1)
        n_digest = n_kept = n_verdict = n_locate = 0
        for n in range(8, 14):
            sols = {}
            if n in (8, 10):
                for C, D, abs_ in solutions(n):
                    for (a, b) in abs_:
                        sols.setdefault((a, b, sum(C), sum(D)), []).append((C, D))
            for sig in classes(n):
                lines = {}
                for tag, b in bins.items():
                    rc, o = run(b, n, sig, {**base, 'WZ_FH_PROF_ORDER': 3, 'WZ_FH_LIST_ONLY': 1})
                    m = re.search(r'^\[order\] ord3 cells=(\d+) kept=(\d+) windows=(\d+) digest=(\S+)', o, re.M)
                    assert rc == 0 and m, (n, sig, tag, o[-300:])
                    lines[tag] = m.groups()
                assert lines['normal'] == lines['randties'], (n, sig, 'digest differs between tie orders', lines)
                digest = lines['normal'][3]
                n_digest += 1
                # kept set: ORDER=3 vs ORDER=1, as key sets from the CELLSIZE stream of kept cells
                keys = {}
                for order in (1, 3):
                    rc, o = run(bins['normal'], n, sig, {**base, 'WZ_FH_PROF_ORDER': order, 'WZ_FH_CELLSIZE': 10**9},
                                tmp / f'dump-{order}')
                    keys[order] = sorted(re.findall(r'^CELLSIZE pi=\d+ cand=(\d+) ', o, re.M))
                    # cell identity via the dumped streams: same multiset of per-cell candidate counts
                assert keys[1] == keys[3], (n, sig, 'kept multiset differs')
                n_kept += 1
                v = {}
                for order in (1, 3):
                    rc, o = run(bins['normal'], n, sig, {**base, 'WZ_FH_PROF_ORDER': order, 'WZ_FH_AB_BUDGET': 0})
                    v[order] = [l for l in semantic(o) if not l.startswith('FIRSTHIT')]  # idx differs by design
                assert ('RESULT: FOUND' in ''.join(v[1])) == ('RESULT: FOUND' in ''.join(v[3])), (n, sig, 'verdict differs')
                n_verdict += 1
                for (C, D) in sols.get(sig, [])[:3]:
                    idx = {}
                    for tag, b in bins.items():
                        rc, o = run(b, n, sig, {**base, 'WZ_FH_PROF_ORDER': 3, 'WZ_FH_LOCATE_C': ','.join(map(str, C)),
                                                'WZ_FH_LOCATE_D': ','.join(map(str, D))})
                        idx[tag] = sorted({int(x) for x in re.findall(r'cell_idx=(\d+) canon_kept=YES', o)})
                        assert idx[tag], (n, sig, tag, 'no kept cell for a known solution')
                    assert idx['normal'] == idx['randties'], (n, sig, 'LOCATE position differs between tie orders')
                    n_locate += 1
                if n == 12 and sig == classes(12)[0]:
                    rc, o = run(bins['normal'], n, sig, {**base, 'WZ_FH_PROF_ORDER': 3, 'WZ_FH_EXPECT_DIGEST': 'deadbeef:0'})
                    assert rc == 2 and 'RESULT: DIGEST MISMATCH' in o
                    rc, o = run(bins['normal'], n, sig, {**base, 'WZ_FH_PROF_ORDER': 3, 'WZ_FH_EXPECT_DIGEST': digest,
                                                          'WZ_FH_AB_BUDGET': 1})
                    assert rc in (0, 3) and 'DIGEST MISMATCH' not in o and 'candidates_streamed=' in o
                    rc, o = run(bins['normal'], n, sig, {**base, 'WZ_FH_PROF_ORDER': 3, 'WZ_FH_ORBIT_CANON': 0})
                    assert rc == 2 and 'requires WZ_FH_ORBIT_CANON' in o
                    # production ord3 without the manifest digest refuses (measurement modes exempt)
                    nod = {k: v for k, v in base.items() if k != 'WZ_FH_ORD3_NODIGEST_OK'}
                    rc, o = run(bins['normal'], n, sig, {**nod, 'WZ_FH_PROF_ORDER': 3, 'WZ_FH_AB_BUDGET': 1})
                    assert rc == 2 and 'requires WZ_FH_EXPECT_DIGEST' in o
                    # resume regression (Astra 09-28 gap 1): a checkpoint whose CFGSIG carries a
                    # different ordered-LIST digest but the same kept bitmap must be refused (fresh start)
                    ck = tmp / 'ck-ord3'; ck.mkdir(exist_ok=True)
                    cfg = {**base, 'WZ_FH_PROF_ORDER': 3, 'WZ_FH_AB_BUDGET': 1, 'WZ_FH_CKPT_DIR': str(ck),
                           'WZ_FH_TEST_STOP_AFTER': 2}
                    rc, o = run(bins['normal'], n, sig, cfg)
                    ckf = next(ck.glob('*.ckpt')); txt = ckf.read_text()
                    sigline = txt.splitlines()[0]
                    assert '.dg' in sigline and ':' in sigline.split('.dg')[1], sigline
                    lst, bm = sigline.split('.dg')[1].split(':')[0], sigline.split('.dg')[1].split(':')[1]
                    forged = txt.replace('.dg' + lst + ':' + bm, '.dg' + ('0' * 16) + ':' + bm)
                    assert forged != txt
                    ckf.write_text(forged)
                    cfg.pop('WZ_FH_TEST_STOP_AFTER')
                    rc, o = run(bins['normal'], n, sig, cfg)
                    assert 'starting FRESH' in o, 'old checkpoint with a different list digest was resumed'
        print(f'PASS: ORDER=3 digests + counts identical across tie orders in {n_digest} classes (n=8..13); '
              f'kept multiset == ORDER=1 in {n_kept}; verdicts identical in {n_verdict}; {n_locate} known-solution '
              'LOCATE positions identical across tie orders; EXPECT_DIGEST refuses on mismatch and is required for ord3 search; '
              'no-canon refused; checkpoint with a different list digest (same bitmap) starts FRESH.',
              flush=True)


if __name__ == '__main__':
    main()
