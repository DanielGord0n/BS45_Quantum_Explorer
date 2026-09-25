#!/usr/bin/env python3
"""Front-only resume-boundary regression (2026-09-26, Astra red-team item 1.3). Local, n=13.

A checkpoint written after the last selected drain batch but before the cell advanced says
(resume_pi, batch = DRAIN_BATCHES, k = 0). Resuming it must complete NOTHING more in that cell:
total completions = uninterrupted total minus the completions already done in that cell's
front. The pre-fix solver completed one whole extra buffer there.
"""
import os
from pathlib import Path
import re
import shutil
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
ENV = {k: v for k, v in os.environ.items() if not k.startswith(('WZ_', 'FH_'))}


def main():
    compiler = shutil.which('clang++') or shutil.which('g++')
    with tempfile.TemporaryDirectory(prefix='bs45-resume-') as name:
        tmp = Path(name)
        binary = tmp / 'current'
        subprocess.run([compiler, '-O3', '-std=c++17', str(ROOT / 'src/solver/wz_match.cpp'), '-o', str(binary)],
                       check=True)
        ck = tmp / 'ck'
        checks = 0
        for buf, top, batches in ((4, 2, 1), (4, 2, 2), (6, 3, 1), (6, 3, 3), (8, 5, 2)):
            base = {**ENV, 'WZ_FIRSTHIT': '1', 'WZ_FH_M6': '1', 'WZ_FH_AB_PROF': '1', 'WZ_THM211B': '1',
                    'WZ_THM212': '1', 'WZ_FH_ORBIT_CANON': '1', 'WZ_FH_PROF_ORDER': '1', 'WZ_FH_BUF_CAP': str(buf),
                    'WZ_FH_DRAIN_TOP': str(top), 'WZ_FH_DRAIN_BATCHES': str(batches), 'WZ_FH_AB_BUDGET': '1',
                    'WZ_FH_PROG_SEC': '9999', 'WZ_FH_CKPT_DIR': str(ck)}

            def run(extra):
                o = subprocess.run([str(binary), '13', '2', '4', '3', '5'], env={**base, **extra},
                                   capture_output=True, text=True, timeout=60).stdout
                return int(re.search(r'^backtracks_entered=(\d+)', o, re.M)[1])

            shutil.rmtree(ck, ignore_errors=True); ck.mkdir()
            full = run({})
            shutil.rmtree(ck); ck.mkdir()
            run({'WZ_FH_TEST_STOP_AFTER': '1'})
            p = ck / 'arm_0.ckpt'
            s = p.read_text()
            # completions already done in the resume cell's front, from a fresh run of that cell alone
            pi = int(re.search(r'resume_pi=(\d+)', s)[1])
            shutil.copy(p, tmp / 'saved.ckpt')
            shutil.rmtree(ck); ck.mkdir()
            front = run({'WZ_FH_PROF_SKIP': str(pi), 'WZ_FH_PROF_END': str(pi + 1)})
            ck.mkdir(exist_ok=True)
            s = re.sub(r'resume_batch=\d+', f'resume_batch={batches}', s)
            s = re.sub(r'resume_k=\d+', 'resume_k=0', s)
            p.write_text(s)
            resumed = run({})
            assert resumed == full - front, (buf, top, batches, full, front, resumed, 'extra work at the boundary')
            checks += 1
        print(f'PASS: {checks} front-only configs: resuming at batch == DRAIN_BATCHES does no extra work '
              '(resumed = uninterrupted - completed front).', flush=True)


if __name__ == '__main__':
    main()
