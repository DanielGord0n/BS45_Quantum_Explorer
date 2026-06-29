#!/usr/bin/env python3
"""
Independent NPAF verifier for Balonin-Seberry δ-codes.

Verifies that a (A,B,C,D) tuple satisfies NPAF[s] = 0 for all s=1..n+1.
Independent from wz_sa_v8.cpp's npaf_at() — different language, different
code path. Use this to confirm any solver-reported solution before
claiming a reproduction or world-record discovery.

Usage:
    # 1) Paste solver output (between A = {...}; B = {...}; C = ...; D = ...;)
    python3 verify_npaf.py < solver_output.txt

    # 2) Or run with default self-test against known BS(43,42) and BS(44,43)
    python3 verify_npaf.py --self-test

    # 3) Or pass A,B,C,D inline (comma-separated ±1 values)
    python3 verify_npaf.py --A "1,-1,1,..." --B "..." --C "..." --D "..."
"""

import argparse
import re
import sys


def parse_array(text):
    """Extract comma-separated ±1 ints from a string like '{1,-1,1,...}'."""
    return [int(x) for x in re.findall(r"-?1", text)]


def parse_solver_block(text):
    """Find 'A = {...};' / B / C / D blocks (in any order) and return as dict."""
    out = {}
    for name in ("A", "B", "C", "D"):
        m = re.search(rf"{name}\s*=\s*\{{([^}}]*)\}}", text)
        if not m:
            raise ValueError(f"Could not find '{name} = {{...}};' in input.")
        out[name] = parse_array(m.group(1))
    return out["A"], out["B"], out["C"], out["D"]


def npaf_at(A, B, C, D, s):
    """NPAF at shift s — must equal 0 for all s in [1, n+1] for a valid BS quad."""
    n1, n = len(A), len(C)
    c = 0
    if s < n1:
        for i in range(n1 - s):
            c += A[i] * A[i + s] + B[i] * B[i + s]
    if s < n:
        for i in range(n - s):
            c += C[i] * C[i + s] + D[i] * D[i + s]
    return c


def check_pair_encoding(A, B, C, D):
    """Verify the Wang-Zhu pair-product encoding the solver uses."""
    n1, n = len(A), len(C)
    issues = []
    # AB d=0 should have product -1 (comb8_neg)
    p = A[0] * B[0] * A[n1 - 1] * B[n1 - 1]
    if p != -1:
        issues.append(f"AB pair d=0: A[0]*B[0]*A[{n1-1}]*B[{n1-1}] = {p}, expected -1")
    # AB d>=1 (excluding middle if odd) should have product +1 (comb8_pos)
    for d in range(1, n1 // 2):
        p = A[d] * B[d] * A[n1 - 1 - d] * B[n1 - 1 - d]
        if p != 1:
            issues.append(f"AB pair d={d}: product = {p}, expected +1")
    # CD d>=1 should have product +1 (comb8_pos)
    for d in range(1, n // 2):
        p = C[d] * D[d] * C[n - 1 - d] * D[n - 1 - d]
        if p != 1:
            issues.append(f"CD pair d={d}: product = {p}, expected +1")
    return issues


def verify(A, B, C, D, name="(unnamed)"):
    n1, n = len(A), len(C)
    print(f"\n========== {name} ==========")
    print(f"Lengths: A={n1}, B={len(B)}, C={n}, D={len(D)}")

    # Sanity: lengths
    if len(B) != n1 or len(D) != n or n1 != n + 1:
        print("FAIL: length sanity check (need |A|=|B|=n+1, |C|=|D|=n)")
        return False
    # Sanity: ±1 entries
    if any(v not in (1, -1) for v in A + B + C + D):
        print("FAIL: non-±1 entry detected")
        return False

    # Signature
    sa, sb, sc, sd = sum(A), sum(B), sum(C), sum(D)
    ssq = sa * sa + sb * sb + sc * sc + sd * sd
    print(f"Signature: (a={sa}, b={sb}, c={sc}, d={sd})  a²+b²+c²+d²={ssq} (expect {4*n+2})")

    # NPAF check (the actual BS condition)
    vio = []
    for s in range(1, n1 + 1):
        v = npaf_at(A, B, C, D, s)
        if v != 0:
            vio.append((s, v))
    if vio:
        print(f"FAIL: NPAF non-zero at {len(vio)} shifts:")
        for s, v in vio[:10]:
            print(f"    s={s}: NPAF={v}")
        if len(vio) > 10:
            print(f"    ... and {len(vio)-10} more")
        return False
    print(f"PASS: NPAF[s]=0 for all s=1..{n1}")

    # Wang-Zhu pair encoding check (a real solution should fit our search space)
    enc_issues = check_pair_encoding(A, B, C, D)
    if enc_issues:
        print(f"WARN: pair encoding violated ({len(enc_issues)} issues) — this tuple")
        print(f"      is a valid BS quad but lives OUTSIDE wz_sa_v8.cpp's search space:")
        for issue in enc_issues[:5]:
            print(f"    {issue}")
    else:
        print(f"PASS: fits Wang-Zhu pair encoding (comb8_neg for d=0, comb8_pos otherwise)")
    return True


def self_test():
    # Same tuples as src/verifier/verify_bs43.cpp
    def parse(s1, s2):
        return [1 if c == "+" else -1 for c in (s1 + s2) if c in "+-"]

    A43 = parse("++--++--+-+++-+--+-++-", "-+----+++-+-+-+++++++")
    B43 = parse("+++++++++---+-+-+++--+", "+-+-+-++++--++-++--+-")
    C43 = parse("+++++----+++--+++-++-+", "--+----+--+--+++--+-")
    D43 = parse("++------+---++++---+-+", "-+++--+-++-+-++-+++-")
    ok43 = verify(A43, B43, C43, D43, "BS(43,42) Wang-Zhu (self-test)")

    A44 = parse("+++-+++++---+--+-++-++", "---+++-++-+-+-+-+++--+")
    B44 = parse("+++-++--++-+-+--+--+--", "+++---++-+-----++++---")
    C44 = parse("++---+--++++-+-+-+----", "+--+-+-++-+++++-+-+++")
    D44 = parse("--++++---++--+++++-++-", "-+-+++++++++-++--+---")
    ok44 = verify(A44, B44, C44, D44, "BS(44,43) Wang-Zhu (self-test)")
    return ok43 and ok44


def main():
    p = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument("--self-test", action="store_true",
                   help="Verify against the Wang-Zhu BS(43) and BS(44) tuples")
    p.add_argument("--A", help="Comma-separated ±1 values for sequence A")
    p.add_argument("--B", help="Sequence B")
    p.add_argument("--C", help="Sequence C")
    p.add_argument("--D", help="Sequence D")
    p.add_argument("--name", default="(input)", help="Display name for the tuple")
    args = p.parse_args()

    if args.self_test:
        sys.exit(0 if self_test() else 1)

    if args.A and args.B and args.C and args.D:
        A = [int(x) for x in args.A.split(",")]
        B = [int(x) for x in args.B.split(",")]
        C = [int(x) for x in args.C.split(",")]
        D = [int(x) for x in args.D.split(",")]
    else:
        text = sys.stdin.read()
        A, B, C, D = parse_solver_block(text)

    sys.exit(0 if verify(A, B, C, D, args.name) else 1)


if __name__ == "__main__":
    main()
