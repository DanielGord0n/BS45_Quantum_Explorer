#!/usr/bin/env python3
"""WZ_FH_ORBIT_Q soundness gate (2026-09-24, Astra item 1). Local searches use only n <= 12.

With exact completion (budget 0) and no caps, a NO HIT verdict is a proof of absence in
that class, so if the 64-element canonicalization ever dropped a class's only solution
orbit, Q-on would say NO HIT where Q-off says FOUND. Checks, for every signature class
at n = 6, 8, 10, 12, mod-3 and mod-6 cells, forward and reversed streams:
  (1) Q-on verdict == Q-off verdict; every FOUND tuple passes tools/verify_npaf.py;
  (2) Q is refused outside its proven range (odd n, or c+d != 0 mod 4);
  (3) a Q-on checkpoint carries the ".oq1" CFGSIG suffix (new lane namespace).
Default-off identity is covered by tools/test_firsthit_telemetry.py.
"""
import itertools
import os
from pathlib import Path
import shutil
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / 'src/solver/wz_match.cpp'
ENV = {k: v for k, v in os.environ.items() if not k.startswith(('WZ_', 'FH_'))}


def classes(n):
    out = []
    for a, b, c, d in itertools.product(range(0, n + 2), range(0, n + 2), range(0, n + 1), range(0, n + 1)):
        if a > b or c > d:
            continue
        if (a - (n + 1)) % 2 or (b - (n + 1)) % 2 or (c - n) % 2 or (d - n) % 2:
            continue
        if a * a + b * b + c * c + d * d == 4 * n + 2:
            out.append((a, b, c, d))
    return out


def run(binary, args, settings, ckdir=None):
    env = {**ENV, **{k: str(v) for k, v in settings.items()}}
    if ckdir:
        env['WZ_FH_CKPT_DIR'] = str(ckdir)
    p = subprocess.run([str(binary), *map(str, args)], env=env, text=True,
                       capture_output=True, timeout=300)
    assert p.returncode in (0, 3), p.stderr[-500:]
    return p.stdout


def main():
    compiler = shutil.which('clang++') or shutil.which('g++')
    with tempfile.TemporaryDirectory(prefix='bs45-orbitq-') as name:
        tmp = Path(name)
        binary = tmp / 'current'
        subprocess.run([compiler, '-O3', '-std=c++17', str(SOURCE), '-o', str(binary)], check=True)
        base = dict(WZ_FIRSTHIT=1, WZ_FH_ORBIT_CANON=1, WZ_FH_AB_BUDGET=0, WZ_FH_PROG_SEC=9999)
        compared = found = refused = 0
        for n in (6, 8, 10, 12):
            for sig in classes(n):
                safe = (sig[2] + sig[3]) % 4 == 0
                for m6, rev in itertools.product((0, 1), (0, 1)):
                    cfg = {**base, 'WZ_FH_M6': m6, 'WZ_FH_STREAM_REV': rev}
                    off = run(binary, (n, *sig), cfg)
                    on = run(binary, (n, *sig), {**cfg, 'WZ_FH_ORBIT_Q': 1})
                    if not safe:
                        assert '[orbitq] DISABLED' in on, (n, sig)
                        refused += 1
                        continue
                    v_off, v_on = 'RESULT: FOUND' in off, 'RESULT: FOUND' in on
                    assert v_off == v_on, (n, sig, m6, rev, 'verdict differs', v_off, v_on)
                    if 'group=64' not in on and '[orbitcanon]' in on:
                        raise AssertionError((n, sig, 'Q not active in a safe class'))
                    if v_on:
                        found += 1
                        ver = subprocess.run(['python3', str(ROOT / 'tools/verify_npaf.py')], input=on,
                                             text=True, capture_output=True)
                        assert ver.returncode == 0, ver.stdout + ver.stderr
                    compared += 1
        # odd n is always refused
        out = run(binary, (11, 0, 6, 1, 3), {**base, 'WZ_FH_M6': 1, 'WZ_FH_ORBIT_Q': 1})
        assert '[orbitq] DISABLED' in out and 'group=64' not in out
        # CFGSIG namespace
        ck = tmp / 'ck'
        ck.mkdir()
        run(binary, (12, 1, 7, 0, 0), {**base, 'WZ_FH_M6': 1, 'WZ_FH_ORBIT_Q': 1,
                                        'WZ_FH_AB_BUDGET': 1, 'WZ_FH_MAX_CAND': 50}, ck)
        sig = next(ck.glob('*.ckpt')).read_text()
        assert '.oq1' in sig.splitlines()[0], sig.splitlines()[0]
        print(f'PASS: {compared} safe class/mode runs with identical Q-on/Q-off verdicts '
              f'({found} FOUND, all NPAF-verified); {refused} unsafe runs refused; odd n refused; '
              'CFGSIG carries .oq1.', flush=True)


if __name__ == '__main__':
    main()
