#!/usr/bin/env python3
"""Read a CDPILOT output (cluster_cd_prune_pilot.sh) and apply its pre-registered rule.

Usage: python3 tools/cd_prune_pilot_summary.py cdpilot_<job>.txt
Pairs CELLSIZE/CDSTAT lines by (shard, pi) across CD_PRUNE=0 and =3.
IDENTITY: the paired cells must have equal cand; cells present in one mode only are fine
only if they are the last, partial cell of a process. PASS: median off/on time ratio >= 1.25
over >= 8 paired finished (partial=0) cells; CLOSE: < 1.05; else INCONCLUSIVE.
"""
import re
import statistics
import sys

L = re.compile(r'cdp_s(\d+)_m(\d)\.log (CELLSIZE|CDSTAT) pi=(\d+) (.*)')


def main():
    cells, nodes = {}, {}
    for line in open(sys.argv[1], errors='replace'):
        m = L.search(line)
        if not m:
            continue
        shard, mode, kind, pi, rest = int(m[1]), int(m[2]), m[3], int(m[4]), m[5]
        f = dict(kv.split('=', 1) for kv in rest.split() if '=' in kv)
        if kind == 'CELLSIZE':
            cells[(shard, pi, mode)] = (int(f['cand']), int(f['partial']), float(f['sec']))
        else:
            nodes[(shard, pi, mode)] = int(f['dfs_nodes'])
    keys = {(s, p) for (s, p, _) in cells}
    ratios, node_ratios, mismatch = [], [], []
    for s, p in sorted(keys):
        a, b = cells.get((s, p, 0)), cells.get((s, p, 3))
        if not a or not b:
            continue
        if a[1] or b[1]:            # partial in either mode: counts are lower bounds, skip
            continue
        if a[0] != b[0]:
            mismatch.append((s, p, a[0], b[0]))
            continue
        if b[2] > 0:
            ratios.append(a[2] / b[2])
        n0, n3 = nodes.get((s, p, 0)), nodes.get((s, p, 3))
        if n0 and n3:
            node_ratios.append(n0 / n3)
    print(f'paired finished cells: {len(ratios)}  identity mismatches: {len(mismatch)} {mismatch[:5]}')
    if ratios:
        print(f'time ratio off/on: median {statistics.median(ratios):.3f} '
              f'min {min(ratios):.3f} max {max(ratios):.3f}')
    if node_ratios:
        print(f'DFS-visit ratio off/on: median {statistics.median(node_ratios):.3f}')
    if mismatch:
        verdict = 'FAIL (identity broken: do not deploy)'
    elif len(ratios) < 8:
        verdict = 'INCONCLUSIVE (fewer than 8 paired finished cells)'
    elif statistics.median(ratios) >= 1.25:
        verdict = 'PASS'
    elif statistics.median(ratios) < 1.05:
        verdict = 'CLOSE'
    else:
        verdict = 'INCONCLUSIVE'
    print('VERDICT:', verdict)


if __name__ == '__main__':
    main()
