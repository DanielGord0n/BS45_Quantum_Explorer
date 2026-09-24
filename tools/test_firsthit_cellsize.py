#!/usr/bin/env python3
"""WZ_FH_CELLSIZE gate (2026-09-24). Local searches use only n <= 13.

The measurement mode must (1) stream exactly the candidates a normal full-cell run
streams, in the same order, (2) attribute them to the right cell, (3) truncate each
cell at the cap and nothing else, and (4) never read or write a checkpoint.
Default-off identity against the pre-instrumentation baseline is covered separately by
tools/test_firsthit_telemetry.py (telemetry mode 0 == cellsize off).
"""
import os
from pathlib import Path
import re
import shutil
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / 'src/solver/wz_match.cpp'
ENV = {k: v for k, v in os.environ.items() if not k.startswith(('WZ_', 'FH_'))}
LINE = re.compile(r'^CELLSIZE pi=(\d+) cand=(\d+) capped=([01]) partial=([01]) sec=\S+ '
                  r'sec_at_buf=\S+ leaves=(\d+) hall_ok=(\d+)$')


def run(binary, args, settings, ckdir):
    ckdir.mkdir(exist_ok=True)
    dump = ckdir / 'stream.txt'
    env = {**ENV, **{k: str(v) for k, v in settings.items()},
           'WZ_FH_CKPT_DIR': str(ckdir), 'WZ_FH_DUMP': str(dump)}
    p = subprocess.run([str(binary), *map(str, args)], env=env, text=True,
                       capture_output=True, timeout=60)
    assert p.returncode in (0, 3), p.stderr
    cells = []
    for s in p.stdout.splitlines():
        if s.startswith('CELLSIZE'):
            m = LINE.match(s)
            assert m, f'malformed line: {s}'
            cells.append(tuple(int(g) for g in m.groups()))
    streamed = int(re.search(r'^candidates_streamed=(\d+)', p.stdout, re.M)[1])
    return p.stdout, cells, dump.read_text().splitlines(), streamed


def main():
    compiler = shutil.which('clang++') or shutil.which('g++')
    assert compiler
    with tempfile.TemporaryDirectory(prefix='bs45-cellsize-') as name:
        tmp = Path(name)
        binary = tmp / 'current'
        subprocess.run([compiler, '-O3', '-std=c++17', str(SOURCE), '-o', str(binary)], check=True)
        common = dict(WZ_FIRSTHIT=1, WZ_FH_M6=1, WZ_FH_AB_PROF=1, WZ_THM211B=1, WZ_THM212=1,
                      WZ_FH_ORBIT_CANON=1, WZ_FH_PROF_ORDER=1, WZ_FH_PROG_SEC=9999,
                      WZ_FH_AB_BUDGET=1, WZ_FH_BUF_CAP=10**9, WZ_FH_DRAIN_TOP=0)
        cases = [((11, 0, 6, 1, 3), {}), ((12, 1, 7, 0, 0), {}), ((13, 2, 4, 3, 5), {}),
                 ((13, 2, 4, 3, 5), {'WZ_FH_STREAM_REV': 1}),
                 ((13, 2, 4, 3, 5), {'WZ_FH_PROF_ORDER': 2}),
                 ((13, 2, 4, 3, 5), {'WZ_FH_NSHARD': 3, 'WZ_FH_SHARD': 1}),
                 ((13, 2, 4, 3, 5), {'WZ_FH_PROF_END': 8}),
                 ((13, 2, 4, 3, 5), {'WZ_FH_PROF_SKIP': 5})]
        checks = 0
        for ci, (args, extra) in enumerate(cases):
            cfg = {**common, **extra}
            base_out, _, base_stream, base_n = run(binary, args, cfg, tmp / f'base-{ci}')
            assert 'FOUND' not in base_out, 'budget-1 reference must stream every cell'
            assert base_n == len(base_stream) and base_n > 0
            base_ck = {f.name: f.read_bytes() for f in (tmp / f'base-{ci}').glob('*.ckpt')}
            assert base_ck, 'reference run should have written a checkpoint'

            # (1)+(4): uncapped count run, with the reference checkpoint planted in its CKDIR.
            d = tmp / f'full-{ci}'
            d.mkdir()
            for k, v in base_ck.items():
                (d / k).write_bytes(v)
            out, cells, stream, n_str = run(binary, args, {**cfg, 'WZ_FH_CELLSIZE': 10**12}, d)
            assert stream == base_stream, 'stream differs from the normal run'
            assert n_str == base_n == sum(c[1] for c in cells)
            assert all(c[2] == 0 and c[3] == 0 for c in cells)
            assert all(c[5] == c[1] for c in cells), 'every Hall-ok leaf reaches the sink'
            assert {f.name: f.read_bytes() for f in d.glob('*.ckpt*')} == base_ck, 'ckpt touched'
            assert 'backtracks_entered=0 ' in out and 'FOUND' not in out
            sizes = [c[1] for c in cells]

            # (2)+(3): capped run == per-cell prefixes of the reference stream.
            for cap in (1, 3, max(sizes) // 2 or 1, max(sizes)):
                d = tmp / f'cap-{ci}-{cap}'
                _, ccells, cstream, _ = run(binary, args, {**cfg, 'WZ_FH_CELLSIZE': cap}, d)
                assert [c[0] for c in ccells] == [c[0] for c in cells], 'cell set changed'
                expect, pos = [], 0
                for size in sizes:
                    expect += base_stream[pos:pos + min(size, cap)]
                    pos += size
                assert cstream == expect, (ci, cap, 'capped stream is not the per-cell prefix')
                for c, size in zip(ccells, sizes):
                    assert c[1] == min(size, cap) and c[2] == (1 if size >= cap else 0)
                assert not list(d.glob('*.ckpt*')), 'cellsize mode wrote a checkpoint'
                checks += 1
            checks += 1
        print(f'PASS: {len(cases)} fixtures, {checks} runs: CELLSIZE stream == normal stream, '
              'per-cell counts sum to candidates_streamed, caps give exact per-cell prefixes, '
              'no checkpoint read or written.', flush=True)


if __name__ == '__main__':
    main()
