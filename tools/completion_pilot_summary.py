#!/usr/bin/env python3
"""Read a CPILOT output (cluster_completion_pilot.sh) and apply its pre-registered rule.

Usage: python3 tools/completion_pilot_summary.py cpilot_<job>.txt
Pairs CAND lines by (shard, pi, drain sequence) across mode 0 and EACH nonzero mode: the k-th
CAND line of a (shard, cell) in mode 0 pairs with the k-th in the on-mode. Both modes drain the same
sorted buffer in the same order, so the sequence is the identity of the candidate. (2026-10-01: the
CAND line's idx= is the STREAM counter, which in the buffered path is already final when draining
starts, so idx is constant within a cell: keying on it collapsed CPILOT 62283881 to 4 pairs.)
Outcomes r: 0 hit, 2 clean-no, 3 budget-abort (1 = prefilter reject, never expected at n=44).
IDENTITY: 0->0 (nodes on <= nodes off), 2->2, 3->{0,2,3}; anything else is FAIL.
PASS: completion time (sum ns over paired candidates) falls >= 15% and resolved (r in {0,2})
count does not fall; CLOSE: saving < 5%; else INCONCLUSIVE. Requires >= 2000 paired candidates.
SHADOW modes (A2SHADOW lines present, search unchanged): identity must be exact with nodes equal;
the estimate is saved_nodes/total_nodes x time(off) minus the measured overhead time(on)-time(off);
PROCEED (to a pruning pilot) if the estimate exceeds 10% of time(off), else CLOSE (Astra 2026-10-01).
"""
import re
import sys

L = re.compile(r'cp_s(\d+)_m(\d)\.log CAND idx=(\d+)(?: ci=-?\d+)? pi=(\d+) r=(\d) nodes=(\d+) ns=(\d+)')  # ci= added 2026-10-01 (print-only)
SH = re.compile(r'cp_s(\d+)_m(\d)\.log A2SHADOW mode=(\d) rows_tested=(\d+) rows_rejected=(\d+) cuts=(\d+) saved_nodes=(\d+) total_nodes=(\d+) hit_under_cut=(\d+)')


def main():
    c = {}
    seq = {}
    shadow = {}
    for line in open(sys.argv[1], errors='replace'):
        m = L.search(line)
        if m:
            s, mode, idx, pi, r, nodes, ns = (int(x) for x in m.groups())
            k = seq[(s, pi, mode)] = seq.get((s, pi, mode), -1) + 1
            c[(s, pi, k, mode)] = (r, nodes, ns)
        m = SH.search(line)
        if m:
            s, mode = int(m[1]), int(m[2])
            shadow[(s, mode)] = tuple(int(x) for x in m.groups()[2:])
    modes = sorted({k[3] for k in c})
    ons = [m for m in modes if m != 0]
    assert ons, 'no "on" mode lines'
    cells0 = {(s, pi): n + 1 for (s, pi, mode), n in seq.items() if mode == 0}
    for on in ons:
        cells1 = {(s, pi): n + 1 for (s, pi, mode), n in seq.items() if mode == on}
        print(f'--- mode {on} vs 0: cells off {len(cells0)} on {len(cells1)}; '
              f'per-cell counts equal: {cells0 == cells1}' + ('' if cells0 == cells1 else f' (off {cells0} on {cells1})'))
        pairs = [(k[:3], c[k], c[(k[0], k[1], k[2], on)]) for k in c if k[3] == 0 and (k[0], k[1], k[2], on) in c]
        is_shadow = any(mode == on and v[0] == 1 for (_, mode), v in shadow.items())  # A2SHADOW mode=1 only; mode 2 prunes
        bad = []
        ns0 = ns1 = nd0 = nd1 = 0
        res0 = res1 = hit0 = hit1 = 0
        for key, a, b in pairs:
            if is_shadow:
                ok = a[0] == b[0] and a[1] == b[1]
            else:
                ok = (a[0] == 0 and b[0] == 0 and b[1] <= a[1]) or (a[0] == 2 and b[0] == 2) or (a[0] == 3 and b[0] in (0, 2, 3))
            if not ok:
                bad.append((key, a[0], b[0]))
            ns0 += a[2]; ns1 += b[2]; nd0 += a[1]; nd1 += b[1]
            res0 += a[0] in (0, 2); res1 += b[0] in (0, 2); hit0 += a[0] == 0; hit1 += b[0] == 0
        print(f'paired candidates: {len(pairs)}  unpaired off/on: {sum(cells0.values()) - len(pairs)}/{sum(cells1.values()) - len(pairs)}  identity violations: {len(bad)} {bad[:5]}')
        if pairs:
            print(f'completion time off/on: {ns0 / 1e9:.0f}s / {ns1 / 1e9:.0f}s = {ns0 / max(1, ns1):.3f}x  '
                  f'(saving {100 * (1 - ns1 / max(1, ns0)):.1f}%); nodes off/on {nd0 / max(1, nd1):.3f}x; '
                  f'resolved {res0} -> {res1}; hits {hit0} -> {hit1}')
        if is_shadow:
            rows_t = rows_r = cuts = saved = total = huc = 0
            for (s, mode), v in sorted(shadow.items()):
                if mode != on:
                    continue
                print(f'  shadow s{s}: rows_tested={v[1]} rows_rejected={v[2]} cuts={v[3]} saved_nodes={v[4]} total_nodes={v[5]} '
                      f'({100 * v[4] / max(1, v[5]):.1f}%) hit_under_cut={v[6]}')
                rows_t += v[1]; rows_r += v[2]; cuts += v[3]; saved += v[4]; total += v[5]; huc += v[6]
            frac = saved / max(1, total)
            overhead = ns1 - ns0
            est = frac * ns0 - overhead
            print(f'  shadow total: rows_rejected {rows_r}/{rows_t}, cuts {cuts}, saved nodes {saved}/{total} = {100 * frac:.1f}%, '
                  f'overhead {overhead / 1e9:.0f}s ({100 * overhead / max(1, ns0):.1f}%), estimated net saving {est / 1e9:.0f}s = {100 * est / max(1, ns0):.1f}% of off time, hit_under_cut={huc}')
            if huc:
                v = 'FAIL (hit found under a would-be cut: predicate unsound)'
            elif bad or (pairs and cells0 != cells1):
                v = 'FAIL (shadow changed the search)'
            elif len(pairs) < 2000:
                v = 'INCONCLUSIVE (fewer than 2000 paired candidates)'
            elif est > 0.10 * ns0:
                v = 'PROCEED (estimated net saving > 10%: run the pruning pilot)'
            else:
                v = 'CLOSE (estimated net saving <= 10%)'
            print(f'VERDICT[mode {on}, shadow]:', v)
            continue
        if bad:
            v = 'FAIL (identity violated: do not deploy)'
        elif len(pairs) < 2000:
            v = 'INCONCLUSIVE (fewer than 2000 paired candidates)'
        elif cells0 != cells1:
            v = 'FAIL (per-cell candidate counts differ between modes: drain order not shared)'
        elif res1 < res0:
            v = 'FAIL (resolved count fell)'
        elif 1 - ns1 / ns0 >= 0.15:
            v = 'PASS'
        elif 1 - ns1 / ns0 < 0.05:
            v = 'CLOSE'
        else:
            v = 'INCONCLUSIVE'
        print(f'VERDICT[mode {on}]:', v)


if __name__ == '__main__':
    main()
