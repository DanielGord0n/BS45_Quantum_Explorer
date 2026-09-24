#!/usr/bin/env python3
"""Unit tests for tools/cellsize_summary.py (synthetic CELLSIZE lines, known verdicts)."""
import os
import sys
import tempfile

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from cellsize_summary import frac_ge, frac_le, parse, verdict  # noqa: E402

P = 500000


def lines(sizes, cap=4000000, partial=()):
    out = []
    for i, s in enumerate(sizes):
        c = min(s, cap)
        out.append(f'CELLSIZE pi={i} cand={c} capped={int(c >= cap)} partial={int(i in partial)} '
                   f'sec={max(c, 1) / 100} sec_at_buf={P / 100 if c >= P else -1} '
                   f'leaves={3 * c} hall_ok={c}')
    return out


def run(sizes, **kw):
    with tempfile.NamedTemporaryFile('w', suffix='.log', delete=False) as f:
        f.write('noise line\n' + '\n'.join(lines(sizes, **kw)) + '\n')
    try:
        return verdict(parse([f.name]), P)[0]
    finally:
        os.unlink(f.name)


def main():
    assert run([P // 2] * 150)['verdict'] == 'NO-OP'
    assert run([int(1.2 * P)] * 150)['verdict'] == 'BUILD'
    assert run([int(1.2 * P)] * 100 + [5 * P] * 50)['verdict'] == 'BETWEEN'   # P(R>=4)=1/3
    assert run([10 * P] * 150)['verdict'] == 'KILL'                           # all capped at 8x
    assert run([int(3.5 * P)] * 150)['verdict'] == 'KILL'
    assert run([2 * P] * 150)['verdict'] == 'BETWEEN'
    assert run([P] * 99)['verdict'] == 'INSUFFICIENT'
    empty = run([0] * 50 + [int(1.2 * P)] * 120)
    assert empty['verdict'] == 'BUILD' and empty['empty'] == 50 and empty['live'] == 120
    # Censoring: a partial cell at 0.5x is unknown for "R<1" and "R<=1.5", decided for nothing.
    rs = [(0.5, True), (0.5, False), (5.0, True)]
    assert frac_le(rs, 1.0, strict=True) == (0.5, 2)   # (0.5,F) yes, (5,T) no, (0.5,T) unknown
    assert frac_ge(rs, 4.0) == (0.5, 2)                # (5,T) yes, (0.5,F) no, (0.5,T) unknown
    # A walltime-partial cell already past 8x counts toward KILL.
    k = run([10 * P] * 80 + [int(1.2 * P)] * 70, partial=set(range(80)))
    assert k['verdict'] == 'KILL' and k['partial'] == 80
    print('OK: 11 cellsize_summary checks')


if __name__ == '__main__':
    main()
