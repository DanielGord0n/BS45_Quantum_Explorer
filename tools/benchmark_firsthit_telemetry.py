#!/usr/bin/env python3
"""Paired same-host telemetry overhead screen, n=13 only (no production searches).

Example: python3 tools/benchmark_firsthit_telemetry.py --baseline /tmp/old --current /tmp/new
The 2% production overhead gate still needs representative same-node cluster timing.
"""
import argparse
import hashlib
import json
import os
from pathlib import Path
import random
import re
import statistics
import subprocess

from test_firsthit_telemetry import semantic


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--baseline', type=Path, required=True)
    parser.add_argument('--current', type=Path, required=True)
    parser.add_argument('--repeats', type=int, default=41)
    args = parser.parse_args()
    assert args.repeats >= 5
    env = {k: v for k, v in os.environ.items() if not k.startswith(('WZ_', 'FH_'))}
    env.update(WZ_FIRSTHIT='1', WZ_FH_M6='1', WZ_FH_AB_PROF='1', WZ_FH_ORBIT_CANON='1',
               WZ_FH_PROF_ORDER='1', WZ_FH_AB_BUDGET='1', WZ_FH_BUF_CAP='500000',
               WZ_FH_DRAIN_TOP='50000', WZ_THM211B='1', WZ_THM212='1')
    variants = [('baseline', args.baseline, 0), ('off', args.current, 0),
                ('full', args.current, 1), ('sampled', args.current, 64)]
    rows, reference = [], None
    for i in range(args.repeats + 1):
        row = {}
        for label, binary, mode in variants[i % 4:] + variants[:i % 4]:
            p = subprocess.run([str(binary.resolve()), '13','2','4','3','5'],
                               env={**env, 'WZ_FH_TELEMETRY': str(mode)},
                               capture_output=True, text=True, timeout=15)
            assert p.returncode == 3, p.stdout + p.stderr
            result = semantic(p.stdout)
            if reference is None:
                reference = result
            assert result == reference, ('identity mismatch', label)
            row[label] = float(re.search(r'^Time: ([0-9.e+-]+)s$', p.stdout, re.M)[1])
        if i:
            rows.append(row)
    rng = random.Random(9321)
    report = {'fixture': 'n13 signature 2,4,3,5; cap1; full stream; K50000',
              'repeats': args.repeats, 'clock': 'solver steady_clock Time (not CPU time)',
              'sha256': {key: hashlib.sha256(path.read_bytes()).hexdigest()
                         for key, path in [('baseline', args.baseline), ('current', args.current)]},
              'raw_seconds': rows, 'overhead': {}}
    for label, denominator in [('off','baseline'), ('full','off'), ('sampled','off')]:
        ratios = [row[label] / row[denominator] - 1 for row in rows]
        boots = sorted(statistics.median(rng.choices(ratios, k=len(rows))) for _ in range(3000))
        lo, hi = boots[75], boots[2924]
        report['overhead'][label] = {'relative_to': denominator,
            'median_fraction': statistics.median(ratios), 'bootstrap95': [lo, hi],
            'local_under_2pct_supported': hi < .02}
    report['production_gate'] = 'OPEN: small-n timer-heavy fixture is not n44 workload'
    print(json.dumps(report, indent=2))


if __name__ == '__main__':
    main()
