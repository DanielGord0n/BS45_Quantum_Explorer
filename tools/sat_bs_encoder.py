#!/usr/bin/env python3
"""SAT encoding of BS(n+1,n) base sequences — the SAT+CAS lever's canary.

Lever 2 of docs/n44_search_narrowing_research.md (MathCheck-style, after
Bright-Kotsireas-Ganesh's Williamson-to-70 result). Encodes NPAF[s]=0 for
s=1..n as exact-cardinality constraints over XNOR product variables, plus the
signature sums (WLOG nonnegative: each sequence may be independently negated
without changing any NPAF). No reversal canon in v1 — this is a canary, not
the production engine.

Modes:
  --soundness <champion_file>   assign a banked solution as assumptions; the
                                encoding MUST be SAT under them (else the
                                encoding is wrong, not the math)
  --solve n a b c d [--conflicts N]   blind search; any model is re-verified
                                internally (NPAF check) before being printed
"""
import argparse
import sys
import time

from pysat.card import CardEnc, EncType
from pysat.formula import IDPool
from pysat.solvers import Cadical195


def npaf(x, s):
    return sum(x[i] * x[i + s] for i in range(len(x) - s))


class BSEncoder:
    def __init__(self, n, a, b, c, d):
        self.n = n
        self.sig = (a, b, c, d)
        self.pool = IDPool()
        self.cnf = []
        self.seqs = {}  # name -> [var ids], true == +1
        for name, ln in (("A", n + 1), ("B", n + 1), ("C", n), ("D", n)):
            self.seqs[name] = [self.pool.id((name, i)) for i in range(ln)]
        self._encode()

    def _xnor(self, u, v):
        t = self.pool.id(("t", u, v))
        self.cnf += [[-t, -u, v], [-t, u, -v], [t, u, v], [t, -u, -v]]
        return t

    def _exactly(self, lits, k):
        enc = CardEnc.equals(lits=lits, bound=k, vpool=self.pool,
                             encoding=EncType.totalizer)
        self.cnf += enc.clauses

    def _encode(self):
        n = self.n
        # Signature sums, WLOG nonnegative: #true = (len + sum) / 2.
        for name, s in zip("ABCD", self.sig):
            xs = self.seqs[name]
            tot = len(xs) + abs(s)
            assert tot % 2 == 0, f"parity violation in sig for {name}"
            self._exactly(xs, tot // 2)
        if self.sig[0] == 0:
            self.cnf.append([self.seqs["A"][0]])  # WLOG when sum(A)=0
        # NPAF[s] = 0: over all four sequences, at shift s the +1-products
        # must exactly balance the -1-products.
        for s in range(1, n + 1):
            terms = []
            for name in "ABCD":
                xs = self.seqs[name]
                for i in range(len(xs) - s):
                    terms.append(self._xnor(xs[i], xs[i + s]))
            assert len(terms) % 2 == 0
            self._exactly(terms, len(terms) // 2)

    def assumptions_from(self, A, B, C, D):
        out = []
        for name, vals in zip("ABCD", (A, B, C, D)):
            for var, v in zip(self.seqs[name], vals):
                out.append(var if v > 0 else -var)
        return out

    def model_to_seqs(self, model):
        pos = set(v for v in model if v > 0)
        return tuple([1 if v in pos else -1 for v in self.seqs[name]]
                     for name in "ABCD")


def load_champion(path):
    rows = open(path).read().splitlines()
    return [list(map(int, rows[i].split())) for i in (2, 3, 4, 5)]


def normalize_nonneg(seq):
    return [-v for v in seq] if sum(seq) < 0 else seq


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--soundness")
    ap.add_argument("--solve", nargs=5, type=int, metavar=("n", "a", "b", "c", "d"))
    ap.add_argument("--conflicts", type=int, default=0, help="conflict budget (0 = none)")
    args = ap.parse_args()

    if args.soundness:
        A, B, C, D = (normalize_nonneg(s) for s in load_champion(args.soundness))
        n = len(C)
        sig = tuple(abs(sum(s)) for s in (A, B, C, D))
        enc = BSEncoder(n, *sig)
        with Cadical195(bootstrap_with=enc.cnf) as slv:
            t0 = time.time()
            ok = slv.solve(assumptions=enc.assumptions_from(A, B, C, D))
            print(f"soundness n={n} sig={sig}: {'SAT (PASS)' if ok else 'UNSAT (ENCODING BUG)'}"
                  f"  vars={enc.pool.top} clauses={len(enc.cnf)} wall={time.time()-t0:.2f}s")
        sys.exit(0 if ok else 1)

    n, a, b, c, d = args.solve
    enc = BSEncoder(n, a, b, c, d)
    print(f"blind n={n} sig=({a},{b},{c},{d}): vars={enc.pool.top} clauses={len(enc.cnf)}", flush=True)
    with Cadical195(bootstrap_with=enc.cnf) as slv:
        t0 = time.time()
        ok = slv.solve_limited(expect_interrupt=False) if False else (
            slv.solve() if args.conflicts == 0 else
            (slv.conf_budget(args.conflicts) or slv.solve_limited()))
        wall = time.time() - t0
        if ok:
            A, B, C, D = enc.model_to_seqs(slv.get_model())
            bad = max(abs(npaf(A, s) + npaf(B, s) + npaf(C, s) + npaf(D, s))
                      for s in range(1, n + 1))
            print(f"SAT in {wall:.1f}s — internal NPAF re-check: max|sum|={bad} "
                  f"{'OK' if bad == 0 else 'BROKEN'}")
            for nm, x in zip("ABCD", (A, B, C, D)):
                print(f"{nm} = {{{','.join(map(str, x))}}};")
            sys.exit(0 if bad == 0 else 2)
        print(f"{'UNKNOWN (budget)' if ok is None else 'UNSAT'} in {wall:.1f}s")
        sys.exit(3)


if __name__ == "__main__":
    main()
