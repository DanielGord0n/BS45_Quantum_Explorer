#!/usr/bin/env python3
"""WZ_FH_ENDPOS + odd-n Q soundness gate (2026-10-02, Astra n=45 note A). Local, n <= 13.

ENDPOS emits only C,D whose endpoint quad is positive-product, which makes the quad switch Q
a bijection of the stream at ANY n (odd n included). Checks:
  (1) EXHAUSTIVE: every class at n = 5..13 (odd and even), mod-6 cells, canon on, uncapped
      exact completion, forward and reversed: the FOUND/none verdict is identical with
      ENDPOS off and on (a NO HIT is a proof of absence in that class, so ENDPOS never
      removes a class's only solutions); every FOUND passes tools/verify_npaf.py; with ENDPOS
      on, Q on == Q off verdict, group=64 is active, and Q + closure prune keeps the verdict.
  (2) BRUTE-FORCE RETENTION at odd n = 5, 7, 9: every true solution C,D (numpy join), one
      per (class, 64-orbit), is LOCATEd in a KEPT cell under ENDPOS + Q and ENDPOS + Q + prune.
  (3) BANKED ODD-n SOLUTIONS n = 33, 35, 37, 41, 43 (champions + WZ references): LOCATE says
      retained=YES under ENDPOS + Q + prune with the production ORDER=3 list (cell level, no
      streaming), and each has a positive endpoint quad.
  (4) Every cell the closure prune removes at odd n streams ZERO candidates under ENDPOS.
  (5) CFGSIG carries ".ep1" (fresh checkpoint namespace).
"""
import glob
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


def run(binary, args, settings, ckdir=None, timeout=600):
    env = {**ENV, **{k: str(v) for k, v in settings.items()}}
    if ckdir:
        env['WZ_FH_CKPT_DIR'] = str(ckdir)
    p = subprocess.run([str(binary), *map(str, args)], env=env, text=True, capture_output=True, timeout=timeout)
    assert p.returncode in (0, 3), p.stderr[-500:]
    return p.stdout


def main():
    compiler = shutil.which('clang++') or shutil.which('g++')
    with tempfile.TemporaryDirectory(prefix='bs45-endpos-') as name:
        tmp = Path(name)
        b = tmp / 'current'
        subprocess.run([compiler, '-O3', '-std=c++17', str(SOURCE), '-o', str(b)], check=True)
        base = dict(WZ_FIRSTHIT=1, WZ_FH_M6=1, WZ_FH_AB_PROF=1, WZ_THM211B=1, WZ_THM212=1, WZ_FH_ORBIT_CANON=1,
                    WZ_FH_PROF_ORDER=1, WZ_FH_AB_BUDGET=0, WZ_FH_PROG_SEC=9999)
        compared = found = q64 = 0
        for n in range(5, 14):
            for sig in classes(n):
                for rev in (0, 1):
                    cfg = {**base, 'WZ_FH_STREAM_REV': rev}
                    off = run(b, (n, *sig), cfg)
                    on = run(b, (n, *sig), {**cfg, 'WZ_FH_ENDPOS': 1})
                    v_off, v_on = 'RESULT: FOUND' in off, 'RESULT: FOUND' in on
                    assert v_off == v_on, (n, sig, rev, 'ENDPOS changed the verdict', v_off, v_on)
                    qon = run(b, (n, *sig), {**cfg, 'WZ_FH_ENDPOS': 1, 'WZ_FH_ORBIT_Q': 1})
                    qpr = run(b, (n, *sig), {**cfg, 'WZ_FH_ENDPOS': 1, 'WZ_FH_ORBIT_Q': 1, 'WZ_FH_ORBIT_QPRUNE': 1})
                    assert '[orbitq] DISABLED' not in qon, (n, sig, 'Q refused under ENDPOS')
                    if '[orbitcanon]' in qon:
                        assert 'group=64' in qon, (n, sig, 'Q not active')
                        q64 += 1
                    assert ('RESULT: FOUND' in qon) == v_off, (n, sig, rev, 'Q verdict')
                    assert ('RESULT: FOUND' in qpr) == v_off, (n, sig, rev, 'prune verdict')
                    assert 'unknown=0 ' in qpr or '[qprune]' not in qpr, (n, sig, 'unknown absences')
                    for o in (on, qon, qpr):
                        if 'RESULT: FOUND' in o:
                            ver = subprocess.run(['python3', str(ROOT / 'tools/verify_npaf.py')], input=o, text=True, capture_output=True)
                            assert ver.returncode == 0, ver.stdout + ver.stderr
                            found += 1
                    compared += 1
        print(f'(1) {compared} class/direction runs n=5..13: ENDPOS keeps every verdict; Q active (64-group) in {q64} runs with '
              f'identical verdicts, closure prune identical, {found} FOUNDs NPAF-verified', flush=True)
        runs = 0
        for n in (5, 7, 9):
            sols = solutions(n)
            reps = {}
            for C, D, abs_ in sols:
                for (a, bb) in abs_:
                    reps.setdefault((a, bb, canon64(C, D)), (C, D, a, bb))
            for (C, D, a, bb) in reps.values():
                assert C[0] * D[0] * C[-1] * D[-1] == 1, (n, C, D, 'a true solution with a negative endpoint quad')
                c, d = sum(C), sum(D)
                for q in (1, 2):
                    out = run(b, (n, a, bb, c, d), {'WZ_FIRSTHIT': 1, 'WZ_FH_ORBIT_CANON': 1, 'WZ_FH_M6': 1, 'WZ_FH_ENDPOS': 1,
                                                     'WZ_FH_ORBIT_Q': 1, 'WZ_FH_ORBIT_QPRUNE': int(q == 2), 'WZ_THM211B': 1,
                                                     'WZ_THM212': 1, 'WZ_FH_LOCATE_C': ','.join(map(str, C)),
                                                     'WZ_FH_LOCATE_D': ','.join(map(str, D))})
                    assert 'LOCATE: cell NOT FOUND' not in out and 'retained=YES' in out, (n, a, bb, C, D, q, out[-600:])
                    assert 'group=64' in out, (n, a, bb)
                    runs += 1
            print(f'(2) n={n}: {len(sols)} brute-force solution pairs, {len(reps)} (class, 64-orbit) reps, all retained under ENDPOS+Q and +prune', flush=True)
        banked = 0
        for f in sorted(glob.glob(str(ROOT / 'results/champions/champion_firsthit_bs*.txt')) + glob.glob(str(ROOT / 'results/reference/wz_table1_bs*.txt'))):
            rows = [l.split() for l in open(f) if l.strip() and not l.startswith('#')]
            n = int(rows[0][0])
            if n % 2 == 0:
                continue
            sig = tuple(int(x) for x in rows[1][:4]); C = [int(x) for x in rows[4]]; D = [int(x) for x in rows[5]]
            assert C[0] * D[0] * C[-1] * D[-1] == 1, (f, 'negative endpoint quad')
            out = run(b, (n, *sig), {'WZ_FIRSTHIT': 1, 'WZ_FH_ORBIT_CANON': 1, 'WZ_FH_M6': 1, 'WZ_FH_ENDPOS': 1, 'WZ_FH_ORBIT_Q': 1,
                                     'WZ_FH_ORBIT_QPRUNE': 1, 'WZ_FH_PROF_ORDER': 3, 'WZ_FH_ORD3_NODIGEST_OK': 1, 'WZ_THM211B': 1, 'WZ_THM212': 1,
                                     'WZ_FH_LOCATE_C': ','.join(map(str, C)), 'WZ_FH_LOCATE_D': ','.join(map(str, D))}, timeout=1800)
            assert 'LOCATE: cell NOT FOUND' not in out and 'retained=YES' in out and 'group=64' in out, (f, out[-600:])
            banked += 1
            print(f'(3) {Path(f).name}: n={n} retained=YES under ENDPOS + Q + prune (ORDER=3 list)', flush=True)
        pruned_cells = 0
        for n in (7, 9, 11):
            for sig in classes(n):
                cfg = {**base, 'WZ_FH_ENDPOS': 1, 'WZ_FH_ORBIT_Q': 1, 'WZ_FH_CELLSIZE': 10 ** 9}
                sizes = [dict((int(a), int(bb)) for a, bb in re.findall(r'^CELLSIZE pi=(\d+) cand=(\d+)', o, re.M))
                         for o in (run(b, (n, *sig), cfg), run(b, (n, *sig), {**cfg, 'WZ_FH_ORBIT_QPRUNE': 1}))]
                assert set(sizes[1]) <= set(sizes[0])
                for pi in set(sizes[0]) - set(sizes[1]):
                    assert sizes[0][pi] == 0, (n, sig, pi, 'pruned a non-empty cell')
                    pruned_cells += 1
        print(f'(4) odd n=7,9,11: every one of the {pruned_cells} closure-pruned cells streams empty under ENDPOS', flush=True)
        # (6) the ENDPOS cell filter removes only cells that stream EMPTY under ENDPOS: with the
        # filter disabled by the test hook (list order 0, canon off => CELLSIZE pi == list index),
        # every cell flagged ok=0 must report cand=0.
        removed = kept_cells = 0
        for n in (7, 8, 9, 10, 11):
            for sig in classes(n):
                o = run(b, (n, *sig), {**base, 'WZ_FH_ORBIT_CANON': 0, 'WZ_FH_PROF_ORDER': 0, 'WZ_FH_ENDPOS': 1,
                                       'WZ_FH_ENDPOS_NOCELL': 1, 'WZ_FH_CELLSIZE': 10 ** 9})
                flags = {int(a): int(bb) for a, bb in re.findall(r'^ENDPOS_CELL idx=(\d+) ok=([01])', o, re.M)}
                sizes = {int(a): int(bb) for a, bb in re.findall(r'^CELLSIZE pi=(\d+) cand=(\d+)', o, re.M)}
                assert flags and set(sizes) <= set(flags), (n, sig, len(flags), len(sizes))
                for pi, cand in sizes.items():
                    if flags[pi] == 0:
                        assert cand == 0, (n, sig, pi, cand, 'cell filter removed a non-empty cell')
                removed += sum(1 for v in flags.values() if v == 0); kept_cells += sum(flags.values())
        print(f'(6) ENDPOS cell filter n=7..11: {removed} cells removed, every one streams empty; {kept_cells} kept', flush=True)
        ck = tmp / 'ck'; ck.mkdir()
        run(b, (11, 0, 6, 1, 3), {**base, 'WZ_FH_ENDPOS': 1, 'WZ_FH_ORBIT_Q': 1, 'WZ_FH_AB_BUDGET': 1, 'WZ_FH_MAX_CAND': 50}, ck)
        sigl = next(ck.glob('*.ckpt')).read_text().splitlines()[0]
        assert '.ep1' in sigl and '.oq1' in sigl, sigl
        print('(5) CFGSIG carries .ep1 and .oq1 at odd n', flush=True)
        print(f'PASS: ENDPOS keeps every verdict, Q is sound at odd n (brute-force n=5,7,9 + {banked} banked odd-n solutions retained), prune exact.')


if __name__ == '__main__':
    main()
