#!/usr/bin/env python3
"""Read a CPILOT output (cluster_completion_pilot.sh) and apply its pre-registered rule.

Usage: python3 tools/completion_pilot_summary.py cpilot_<job>.txt
Pairs CAND lines by (shard, pi, idx) across mode 0 and the nonzero mode. Outcomes r: 0 hit,
2 clean-no, 3 budget-abort (1 = prefilter reject, never expected at n=44).
IDENTITY: 0->0 (nodes on <= nodes off), 2->2, 3->{0,2,3}; anything else is FAIL.
PASS: completion time (sum ns over paired candidates) falls >= 15% and resolved (r in {0,2})
count does not fall; CLOSE: saving < 5%; else INCONCLUSIVE. Requires >= 2000 paired candidates.
"""
import re
import sys

L = re.compile(r'cp_s(\d+)_m(\d)\.log CAND idx=(\d+) pi=(\d+) r=(\d) nodes=(\d+) ns=(\d+)')


def main():
    c = {}
    for line in open(sys.argv[1], errors='replace'):
        m = L.search(line)
        if m:
            s, mode, idx, pi, r, nodes, ns = (int(x) for x in m.groups())
            c[(s, pi, idx, mode)] = (r, nodes, ns)
    modes = sorted({k[3] for k in c})
    on = [m for m in modes if m != 0]
    assert on, 'no "on" mode lines'
    on = on[0]
    pairs = [(k[:3], c[k], c[(k[0], k[1], k[2], on)]) for k in c if k[3] == 0 and (k[0], k[1], k[2], on) in c]
    bad = []
    ns0 = ns1 = nd0 = nd1 = 0
    res0 = res1 = hit0 = hit1 = 0
    for key, a, b in pairs:
        ok = (a[0] == 0 and b[0] == 0 and b[1] <= a[1]) or (a[0] == 2 and b[0] == 2) or (a[0] == 3 and b[0] in (0, 2, 3))
        if not ok:
            bad.append((key, a[0], b[0]))
        ns0 += a[2]; ns1 += b[2]; nd0 += a[1]; nd1 += b[1]
        res0 += a[0] in (0, 2); res1 += b[0] in (0, 2); hit0 += a[0] == 0; hit1 += b[0] == 0
    print(f'paired candidates: {len(pairs)}  identity violations: {len(bad)} {bad[:5]}')
    if pairs:
        print(f'completion time off/on: {ns0 / 1e9:.0f}s / {ns1 / 1e9:.0f}s = {ns0 / max(1, ns1):.3f}x  '
              f'(saving {100 * (1 - ns1 / max(1, ns0)):.1f}%); nodes off/on {nd0 / max(1, nd1):.3f}x; '
              f'resolved {res0} -> {res1}; hits {hit0} -> {hit1}')
    if bad:
        v = 'FAIL (identity violated: do not deploy)'
    elif len(pairs) < 2000:
        v = 'INCONCLUSIVE (fewer than 2000 paired candidates)'
    elif res1 < res0:
        v = 'FAIL (resolved count fell)'
    elif 1 - ns1 / ns0 >= 0.15:
        v = 'PASS'
    elif 1 - ns1 / ns0 < 0.05:
        v = 'CLOSE'
    else:
        v = 'INCONCLUSIVE'
    print('VERDICT:', v)


if __name__ == '__main__':
    main()
