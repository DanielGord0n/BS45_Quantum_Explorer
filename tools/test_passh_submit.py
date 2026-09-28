#!/usr/bin/env python3
"""Lint the Pass H submit files against the manifest (Astra 09-28 launch review, item 1).

Every line must: name a lane H44<letter>[r]<skip>; carry explicit WZ_FH_STREAM_REV=0|1 matching
the name, WZ_FH_DRAIN_BATCHES=1, WZ_FH_PROF_ORDER=3, WZ_FH_ORBIT_CANON=1, WZ_FH_ORBIT_Q=1,
WZ_FH_ORBIT_QPRUNE=1, WZ_FH_DRAIN_TOP=50000, WZ_FH_AB_BUDGET=2000000, FH_NARMS=178, the class
signature, PROF_SKIP/END equal to a manifest range, and WZ_FH_EXPECT_DIGEST equal to the class
digest; -d singleton and --requeue --mem=0. Names unique; every manifest range appears exactly
once per direction; no measurement variables.
"""
import json
from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[1]


def main():
    man = json.load(open(ROOT / 'docs/plans/passh_manifest.json'))
    by_sig = {tuple(c['sig']): c for c in man['classes']}
    seen = {}
    total = 0
    for f in sorted((ROOT / 'docs/plans').glob('passh_submit_*.txt')):
        cluster = f.stem.split('_')[-1]
        for line in f.read_text().splitlines():
            assert line.startswith('sbatch --requeue --mem=0 '), line[:60]
            assert ' -d singleton ' in line and line.endswith(' ./cluster_firsthit_probe.sh'), line[:80]
            name = re.search(r' -J (H44[A-Za-z]+\d+) ', line)[1]
            env = dict(kv.split('=', 1) for kv in re.search(r'--export=ALL,(\S+)', line)[1].split(','))
            sig = tuple(int(env['WZ_' + k]) for k in ('A', 'B', 'C', 'D'))
            c = by_sig[sig]
            assert c['cluster'] == cluster, (name, cluster)
            rev = 1 if re.match(r'H44[A-Za-z]r\d+$', name) else 0
            letter = re.match(r'H44([A-Za-z])r?(\d+)$', name)
            assert letter[1] == c['letter'] and int(letter[2]) == int(env['WZ_FH_PROF_SKIP']), name
            want = {'WZ_N': '44', 'WZ_FH_PROF_ORDER': '3', 'WZ_FH_ORBIT_CANON': '1', 'WZ_FH_ORBIT_Q': '1',
                    'WZ_FH_ORBIT_QPRUNE': '1', 'WZ_FH_DRAIN_TOP': '50000', 'WZ_FH_AB_BUDGET': '2000000',
                    'FH_NARMS': '178', 'WZ_FH_STREAM_REV': str(rev), 'WZ_FH_DRAIN_BATCHES': '1',
                    'WZ_FH_EXPECT_DIGEST': c['digest']}
            for k, v in want.items():
                assert env.get(k) == v, (name, k, env.get(k), v)
            assert not any(k in env for k in ('WZ_FH_LIST_ONLY', 'WZ_FH_CELLSIZE', 'WZ_FH_TARGET_C', 'WZ_FH_LOCATE_C',
                                                'WZ_FH_TELEMETRY', 'WZ_FH_MAX_CAND')), name
            rng = (int(env['WZ_FH_PROF_SKIP']), int(env['WZ_FH_PROF_END']))
            assert {'skip': rng[0], 'end': rng[1]} in c['lanes'], (name, rng)
            key = (sig, rev, rng)
            assert key not in seen, ('duplicate owner', name, seen[key])
            seen[key] = name
            total += 1
    expected = {(tuple(c['sig']), rev, (l['skip'], l['end'])) for c in man['classes'] for l in c['lanes'] for rev in (0, 1)}
    assert set(seen) == expected, ('missing/extra units', expected ^ set(seen))
    print(f'PASS: {total} submit lines = {len(expected)} manifest units, each owned exactly once per direction; '
          'explicit REV/DRAIN_BATCHES, digest per class, no measurement variables.')


if __name__ == '__main__':
    main()
