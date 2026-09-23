#!/usr/bin/env python3
"""Deterministic telemetry identity gate. Local searches use only n <= 13.

Compiles both c2a3813 (pre-instrumentation baseline) and the working source in /tmp.
No production checkpoint, cluster access or production result mutation.

Archived n29 canary (all other search settings at defaults):
WZ_FIRSTHIT=1 WZ_FH_M6=1 OMP_NUM_THREADS=1 ./bin 29 0 6 9 1
"""
import argparse
import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / 'src/solver/wz_match.cpp'
ENV = {k: v for k, v in os.environ.items() if not k.startswith(('WZ_', 'FH_'))}


def semantic(out):
    lines = []
    for line in out.splitlines():
        if line.startswith(('RESULT:', 'candidates_streamed=', 'backtracks_entered=',
                            'A =', 'B =', 'C =', 'D =', 'VERIFY:', 'FIRSTHIT:', 'sig =')):
            lines.append(re.sub(r' elapsed=.*', '', line))
    return lines


def run(binary, args, settings, ckdir, timeout=30):
    ckdir.mkdir(exist_ok=True)
    env = {**ENV, **{k: str(v) for k, v in settings.items()},
           'WZ_FH_CKPT_DIR': str(ckdir), 'WZ_FH_DUMP': str(ckdir / 'stream.txt')}
    p = subprocess.run([str(binary), *map(str, args)], env=env, text=True,
                       capture_output=True, timeout=timeout)
    assert p.returncode in (0, 3), p.stderr
    checkpoints = {f.name: f.read_bytes() for f in ckdir.glob('*.ckpt')}
    telemetry = [json.loads(s.removeprefix('FH_TELEM ')) for s in p.stdout.splitlines()
                 if s.startswith('FH_TELEM ')]
    return semantic(p.stdout), checkpoints, (ckdir / 'stream.txt').read_bytes(), telemetry, p.stdout


def check_record(t, out, stride):
    assert t['version'] == 2 and t['stride'] == stride
    nodes = int(re.search(r'total_AB_nodes=(\d+)', out)[1])
    tested = int(re.search(r'backtracks_entered=(\d+)', out)[1])
    assert t['completions'] == tested
    assert sum(t['rank_count']) + t['unranked_count'] == tested
    assert sum(t['rank_nodes']) + t['unranked_nodes'] == nodes
    assert t['total_nodes'] == nodes
    assert sum(t['rank_ns']) + t['unranked_ns'] == t['complete_ns']
    assert sum(t['depth_nodes']) == t['hist_nodes']
    if stride == 1:
        assert t['hist_nodes'] == nodes
        assert t['hist_candidates'] == tested
        assert t['score_samples'] == t['score_calls']
    assert t['worker_ns'] >= t['cell_ns'] >= t['sort_ns'] + t['complete_ns']
    assert 0 <= t['resume_replay_sort_ns'] <= t['sort_ns']
    assert t['resume_replay_sort_ns'] <= t['resume_replay_ns'] <= t['cell_ns']
    assert t['resume_replay_score_ns_est'] <= t['score_ns_est']
    assert t['cell_ns'] >= t['resume_replay_ns'] + t['sort_ns'] - t['resume_replay_sort_ns'] + t['complete_ns']
    if stride == 1:
        assert t['cell_ns'] >= t['sort_ns'] + t['complete_ns'] + t['score_ns_est']
        assert t['resume_replay_ns'] >= t['resume_replay_sort_ns'] + t['resume_replay_score_ns_est']
    assert len(t['depth_nodes']) == (t['n'] + 1) // 2 + 1
    assert all(len(t[key]) == 10 for key in ('rank_count', 'rank_nodes', 'rank_ns', 'rank_aborts'))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--n29-canary', action='store_true',
                        help='archived n29 re-find, modes 0/64; at most 300s per variant')
    args_cli = parser.parse_args()
    compiler = shutil.which('clang++') or shutil.which('g++')
    assert compiler
    with tempfile.TemporaryDirectory(prefix='bs45-telemetry-') as name:
        tmp = Path(name)
        base = tmp / 'baseline.cpp'
        base.write_bytes(subprocess.check_output(['git', 'show', 'c2a3813:src/solver/wz_match.cpp'], cwd=ROOT))
        binaries = [tmp / 'baseline', tmp / 'current']
        for src, binary in zip((base, SOURCE), binaries):
            subprocess.run([compiler, '-O3', '-std=c++17', str(src), '-o', str(binary)], check=True)
        common = dict(WZ_FIRSTHIT=1, WZ_FH_M6=1, WZ_FH_AB_PROF=1,
                      WZ_THM211B=1, WZ_THM212=1, WZ_FH_ORBIT_CANON=1,
                      WZ_FH_PROF_ORDER=1, WZ_FH_BUF_CAP=16, WZ_FH_DRAIN_TOP=5,
                      WZ_FH_MAX_CAND=256, WZ_FH_PROG_SEC=9999)
        cases = [((6,5,1,0,0), {}), ((11,0,6,1,3), {}),
                 ((12,1,7,0,0), {}), ((13,2,4,3,5), {}),
                 ((13,2,4,3,5), {'WZ_FH_CELL_ORDER': 0}),
                 ((11,0,6,1,3), {'WZ_FH_PROF_END': 2}),
                 ((13,2,4,3,5), {'WZ_FH_DRAIN_BATCHES': 2})]
        checks = 0
        for ci, (args, extra) in enumerate(cases):
            for budget in (0, 1, 30):
                config = {**common, **extra, 'WZ_FH_AB_BUDGET': budget,
                          'WZ_FH_EARLY_CHECK': ci % 2, 'WZ_FH_STREAM_REV': ci % 2}
                expected = run(binaries[0], args, config, tmp / f'base-{ci}-{budget}')
                for mode in (0, 1, 64):
                    current = run(binaries[1], args, {**config, 'WZ_FH_TELEMETRY': mode},
                                  tmp / f'new-{ci}-{budget}-{mode}')
                    assert current[:3] == expected[:3], (ci, budget, mode, 'identity mismatch')
                    assert len(current[3]) == (1 if mode else 0), 'missing opt-in FH_TELEM record'
                    if mode:
                        check_record(current[3][0], current[4], mode)
                    if 'RESULT: FOUND' in current[4]:
                        verified = subprocess.run(['python3', str(ROOT/'tools/verify_npaf.py')],
                                                  input=current[4], text=True, capture_output=True)
                        assert verified.returncode == 0, verified.stdout + verified.stderr
                    checks += 1
        # Mid-drain interruption, exact ckpt bytes, then resume with telemetry toggled.
        for reverse, buffered, stop_after in ((0,1,3),(1,1,3),(0,0,3),(1,0,7),(0,1,7),(1,1,10)):
            config = {**common, 'WZ_FH_AB_BUDGET': 1, 'WZ_FH_TEST_STOP_AFTER': 3,
                      'WZ_FH_STREAM_REV': reverse, 'WZ_FH_CELL_ORDER': buffered,
                      'WZ_FH_TEST_STOP_AFTER': stop_after}
            dirs = [tmp / f'resume-base-{reverse}-{buffered}-{stop_after}',
                    tmp / f'resume-new-{reverse}-{buffered}-{stop_after}']
            first = [run(b, (13,2,4,3,5), {**config, 'WZ_FH_TELEMETRY': mode}, d)
                     for b, mode, d in zip(binaries, (0,64), dirs)]
            assert first[0][:3] == first[1][:3]
            config.pop('WZ_FH_TEST_STOP_AFTER')
            resumed = [run(b, (13,2,4,3,5), {**config, 'WZ_FH_TELEMETRY': mode}, d)
                       for b, mode, d in zip(binaries, (0,1), dirs)]
            assert resumed[0][:3] == resumed[1][:3]
            check_record(resumed[1][3][0], resumed[1][4], 1)
            assert resumed[1][3][0]['resume_cells'] == 1
            assert resumed[1][3][0]['resume_replay_ns'] > 0
            if buffered:
                assert resumed[1][3][0]['resume_replay_sort_ns'] > 0
        print(f'PASS: {checks} baseline/off/full/sampled comparisons + 6 interrupt/resume pairs; '
              'identical verdicts, sequences, counters, streams and checkpoint bytes.', flush=True)
        if args_cli.n29_canary:
            # Do not inherit common: canon/order/THM flags select a DIFFERENT stream.
            config = dict(WZ_FIRSTHIT=1, WZ_FH_M6=1, OMP_NUM_THREADS=1)
            baseline = run(binaries[0], (29,0,6,9,1), config, tmp/'n29-base', timeout=300)
            assert 'RESULT: FOUND' in baseline[4], 'n29 baseline did not re-find'
            fingerprint = re.search(r'FIRSTHIT: idx=(\d+) profile_rank=(\d+) nodes_this_cand=(\d+)', baseline[4])
            assert fingerprint and fingerprint.groups() == ('26694','588','81320'), 'wrong archived stream'
            print('PASS: n29 archived baseline idx=26694 rank=588 nodes=81320', flush=True)
            for mode in (0,64):
                current = run(binaries[1], (29,0,6,9,1), {**config, 'WZ_FH_TELEMETRY': mode},
                              tmp/f'n29-{mode}', timeout=300)
                assert baseline[:3] == current[:3], ('n29 identity', mode)
                if mode:
                    check_record(current[3][0], current[4], mode)
                verified = subprocess.run(['python3', str(ROOT/'tools/verify_npaf.py')],
                                          input=current[4], text=True, capture_output=True)
                assert verified.returncode == 0, verified.stdout + verified.stderr
                print(f'PASS: n29 telemetry={mode}, baseline identity and independent NPAF.', flush=True)


if __name__ == '__main__':
    main()
