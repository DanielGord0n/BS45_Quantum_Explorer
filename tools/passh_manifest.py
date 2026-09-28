#!/usr/bin/env python3
"""Pass H ownership manifest (2026-09-28; Astra Pass H review, required change 2).

For each of the 12 n=44 classes, runs the solver in list-only mode (ORDER=3, canon + Q +
closure prune) to get the raw-list length, kept count, window count and the toolchain-
independent digests, then partitions the windows [0, ceil(N_raw/178)) into disjoint lane
ranges of S windows per class. Every raw index i belongs to exactly one (window = i // 178,
arm = i % 178), so a partition of windows gives every kept index exactly one owner per
direction; the script asserts that explicitly by simulation and writes:
  docs/plans/passh_manifest.json   (lanes, digests, counts)
  docs/plans/passh_submit_<cluster>.txt   (one sbatch line per lane, fwd + rev)
Usage: SDKROOT=... python3 tools/passh_manifest.py [--binary path] [--out docs/plans]
"""
import argparse
import json
import math
import os
from pathlib import Path
import re
import shutil
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
NARMS = 178
# class -> (sig, S windows per lane, cluster, letter); same assignment and S as Pass G
CLASSES = [
    ((3, 13, 0, 0), 1000, 'fir', 'g'),
    ((9, 9, 0, 4), 300, 'rorqual', 'G'), ((3, 5, 0, 12), 300, 'rorqual', 'H'),
    ((1, 7, 8, 8), 300, 'nibi', 'B'), ((5, 5, 8, 8), 300, 'nibi', 'C'), ((5, 11, 4, 4), 300, 'nibi', 'D'),
    ((3, 3, 4, 12), 150, 'nibi', 'E'), ((7, 7, 4, 8), 150, 'nibi', 'F'), ((5, 7, 2, 10), 150, 'nibi', 'I'),
    ((5, 9, 6, 6), 300, 'trillium', 'A'), ((7, 11, 2, 2), 300, 'trillium', 'K'), ((1, 13, 2, 2), 300, 'trillium', 'L'),
]
ENV_COMMON = 'WZ_FH_PROF_ORDER=3,WZ_FH_ORBIT_CANON=1,WZ_FH_ORBIT_Q=1,WZ_FH_ORBIT_QPRUNE=1,WZ_FH_DRAIN_TOP=50000,WZ_FH_AB_BUDGET=2000000,FH_NARMS=178'
ACCOUNT = {'fir': '--account=rrg-ikotsire_cpu', 'rorqual': '--account=rrg-ikotsire_cpu', 'nibi': '--account=rrg-ikotsire_cpu', 'trillium': ''}


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--binary')
    ap.add_argument('--out', default=str(ROOT / 'docs/plans'))
    a = ap.parse_args()
    tmp = tempfile.mkdtemp(prefix='bs45-manifest-')
    binary = a.binary
    if not binary:
        binary = os.path.join(tmp, 'solver')
        cc = shutil.which('clang++') or shutil.which('g++')
        subprocess.run([cc, '-O3', '-std=c++17', str(ROOT / 'src/solver/wz_match.cpp'), '-o', binary], check=True)
    env = {k: v for k, v in os.environ.items() if not k.startswith(('WZ_', 'FH_'))}
    env.update(dict(WZ_FIRSTHIT='1', WZ_FH_ORBIT_CANON='1', WZ_FH_ORBIT_Q='1', WZ_FH_ORBIT_QPRUNE='1',
                    WZ_FH_PROF_ORDER='3', WZ_FH_LIST_ONLY='1', WZ_FH_NSHARD=str(NARMS)))
    manifest = {'build': subprocess.run(['git', 'rev-parse', '--short', 'HEAD'], capture_output=True, text=True,
                                        cwd=ROOT).stdout.strip(), 'narms': NARMS, 'classes': []}
    submit = {}
    for sig, S, cluster, letter in CLASSES:
        p = subprocess.run([binary, '44', *map(str, sig)], env=env, capture_output=True, text=True, timeout=1800)
        m = re.search(r'^\[order\] ord3 cells=(\d+) kept=(\d+) windows=(\d+) digest=(\S+)', p.stdout, re.M)
        assert p.returncode == 0 and m, (sig, p.stdout[-400:])
        cells, kept, windows, digest = int(m[1]), int(m[2]), int(m[3]), m[4]
        assert windows == math.ceil(cells / NARMS)
        lanes = []
        for k in range(0, windows, S):
            lanes.append({'skip': k, 'end': min(k + S, windows)})
        # ownership assertion by simulation: every raw index has exactly one owner per direction
        owner = [0] * cells
        for ln in lanes:
            for i in range(ln['skip'] * NARMS, min(ln['end'] * NARMS, cells)):
                owner[i] += 1
        assert all(o == 1 for o in owner), (sig, 'ownership gap/duplicate')
        cls = {'sig': sig, 'letter': letter, 'cluster': cluster, 'cells': cells, 'kept': kept, 'windows': windows,
               'S': S, 'digest': digest, 'lanes': lanes}
        manifest['classes'].append(cls)
        lines = []
        for ln in lanes:
            for rev in (0, 1):
                name = f"H44{letter}{'r' if rev else ''}{ln['skip']}"
                envs = f"WZ_N=44,WZ_A={sig[0]},WZ_B={sig[1]},WZ_C={sig[2]},WZ_D={sig[3]},{ENV_COMMON}" \
                       f"{',WZ_FH_STREAM_REV=1' if rev else ''},WZ_FH_PROF_SKIP={ln['skip']},WZ_FH_PROF_END={ln['end']}," \
                       f"WZ_FH_EXPECT_DIGEST={digest}"
                lines.append(f"sbatch --requeue --mem=0 {ACCOUNT[cluster]} -J {name} -d singleton --export=ALL,{envs} ./cluster_firsthit_probe.sh")
        submit.setdefault(cluster, []).extend(lines)
        print(f"{sig} {letter}: cells={cells} kept={kept} windows={windows} S={S} lanes={len(lanes)}x2 digest={digest}", flush=True)
    out = Path(a.out)
    (out / 'passh_manifest.json').write_text(json.dumps(manifest, indent=1) + '\n')
    for cluster, lines in submit.items():
        (out / f'passh_submit_{cluster}.txt').write_text('\n'.join(lines) + '\n')
    tot = sum(len(c['lanes']) for c in manifest['classes'])
    print(f"manifest: {tot} lane ranges x 2 directions = {2 * tot} lane-units; kept orbits total "
          f"{sum(c['kept'] for c in manifest['classes'])}", flush=True)


if __name__ == '__main__':
    main()
