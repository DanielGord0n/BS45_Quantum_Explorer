#!/usr/bin/env python3
"""Summarize WZ_FH_CELLSIZE lines and apply the PRE-REGISTERED whole-cell top-K rule.

Usage: python3 tools/cellsize_summary.py [--prefix 500000] <files with CELLSIZE lines>...

R = full-cell candidate count / prefix (the 500k front-only buffer). Capped cells
(cand >= cap) and partial cells (walltime SIGTERM mid-cell) are RIGHT-CENSORED: their
R is only a lower bound. A censored cell counts toward "R >= x" when its lower bound
already reaches x, and is excluded as UNKNOWN from any fraction it cannot decide.

Rule (registered 2026-09-24, HANDOFF, BEFORE the job ran), over live cells (cand > 0):
  NO-OP  : P(R < 1) > 50%      -> most cells already fit the prefix, so the current policy
                                  is already whole-cell for them; do not build.
  BUILD  : P(R <= 1.5) >= 50% and P(R >= 4) <= 25%
                               -> whole-cell top-K costs <~30% throughput; build it, then
                                  gate it on the known-solution tests.
  KILL   : P(R >= 3) >= 50% or P(R >= 8) >= 50%
                               -> streaming whole cells is >=3x the prefix; prefix policy
                                  stands, cheaper stream enumeration is the lever.
  BETWEEN: otherwise           -> cheaper stream first, then re-measure.
  INSUFFICIENT: fewer than 100 live cells -> one repeat job at another window, then stop.
"""
import argparse
import re
import statistics
import sys

LINE = re.compile(r'CELLSIZE pi=(\d+) cand=(\d+) capped=([01]) partial=([01]) sec=(\S+) '
                  r'sec_at_buf=(\S+) leaves=(\d+) hall_ok=(\d+)')


def parse(paths):
    cells = []
    for path in paths:
        with open(path, errors='replace') as f:
            for s in f:
                m = LINE.search(s)
                if m:
                    pi, cand, capped, partial = (int(g) for g in m.groups()[:4])
                    sec, sec_buf = float(m[5]), float(m[6])
                    cells.append(dict(pi=pi, cand=cand, censored=bool(capped or partial),
                                      capped=bool(capped), partial=bool(partial), sec=sec,
                                      sec_buf=sec_buf, leaves=int(m[7])))
    return cells


def frac_ge(rs, x):
    """rs: list of (R, censored). Returns (fraction, n_decided)."""
    yes = sum(1 for r, c in rs if r >= x)
    decided = sum(1 for r, c in rs if r >= x or not c)
    return (yes / decided if decided else float('nan')), decided


def frac_le(rs, x, strict=False):
    """P(R <= x) (or R < x). A censored cell is decided only if its bound already exceeds x."""
    below = (lambda r: r < x) if strict else (lambda r: r <= x)
    yes = sum(1 for r, c in rs if not c and below(r))
    decided = sum(1 for r, c in rs if not c or not below(r))
    return (yes / decided if decided else float('nan')), decided


def verdict(cells, prefix):
    live = [c for c in cells if c['cand'] > 0]
    rs = [(c['cand'] / prefix, c['censored']) for c in live]
    out = {'cells': len(cells), 'live': len(live), 'empty': len(cells) - len(live),
           'capped': sum(c['capped'] for c in live), 'partial': sum(c['partial'] for c in live)}
    if len(live) < 100:
        out['verdict'] = 'INSUFFICIENT'
        return out, rs
    lt1, _ = frac_le(rs, 1.0, strict=True)
    le15, _ = frac_le(rs, 1.5)
    ge3, _ = frac_ge(rs, 3.0)
    ge4, _ = frac_ge(rs, 4.0)
    ge8, _ = frac_ge(rs, 8.0)
    out.update(P_R_lt_1=lt1, P_R_le_1_5=le15, P_R_ge_3=ge3, P_R_ge_4=ge4, P_R_ge_8=ge8)
    if lt1 > 0.5:
        out['verdict'] = 'NO-OP'
    elif le15 >= 0.5 and ge4 <= 0.25:
        out['verdict'] = 'BUILD'
    elif ge3 >= 0.5 or ge8 >= 0.5:
        out['verdict'] = 'KILL'
    else:
        out['verdict'] = 'BETWEEN'
    return out, rs


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('--prefix', type=int, default=500000)
    ap.add_argument('files', nargs='+')
    a = ap.parse_args()
    cells = parse(a.files)
    if not cells:
        sys.exit('no CELLSIZE lines found')
    out, rs = verdict(cells, a.prefix)
    live = [c for c in cells if c['cand'] > 0]
    exact = sorted(r for r, c in rs if not c)
    print(f"cells={out['cells']} live={out['live']} empty={out['empty']} "
          f"capped={out['capped']} partial={out['partial']} (censored = lower bounds)")
    if exact:
        q = statistics.quantiles(exact, n=10) if len(exact) >= 10 else exact
        print(f"R over uncensored cells: n={len(exact)} median={statistics.median(exact):.2f} "
              f"deciles={[round(v, 2) for v in q]}")
    for k in ('P_R_lt_1', 'P_R_le_1_5', 'P_R_ge_3', 'P_R_ge_4', 'P_R_ge_8'):
        if k in out:
            print(f"{k}={out[k]:.3f}")
    full = [c for c in live if not c['censored'] and c['sec'] > 0]
    if full:
        print(f"stream rate (uncensored): median {statistics.median(c['cand'] / c['sec'] for c in full):.1f} cand/s; "
              f"leaves per candidate median {statistics.median(c['leaves'] / c['cand'] for c in full):.1f}")
    buf = [c['sec_buf'] for c in live if c['sec_buf'] >= 0]
    if buf:
        print(f"time to stream the {a.prefix} prefix: median {statistics.median(buf) / 3600:.2f} h over {len(buf)} cells")
    print(f"VERDICT: {out['verdict']}")


if __name__ == '__main__':
    main()
