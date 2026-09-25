#!/usr/bin/env python3
"""Review-only probes. Builds temporary source copies; searches only n=6,11,13.

Run from any directory: python3 docs/reviews/evidence/2026-09-22/reproduce.py
No cluster access, production source changes, or production checkpoints.
"""
import hashlib
import itertools
import os
from pathlib import Path
import re
import shutil
import subprocess
import tempfile

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[3]
SOURCE = ROOT / "src/solver/wz_match.cpp"
BASE = SOURCE.read_text()
ENV = {k: v for k, v in os.environ.items() if not k.startswith("WZ_")}
COMPILER = shutil.which("clang++") or shutil.which("g++")
assert COMPILER, "Need clang++ or g++"
PIN = "  bool pinC = (G_SIG_C == 0), pinD = (G_SIG_D == 0);"
PIN_PROBE = PIN + '\n  if (getenv("WZ_FIRSTHIT") && getenv("WZ_FH_ORBIT_CANON") && atoi(getenv("WZ_FH_ORBIT_CANON"))) pinC = pinD = false;\n'
DISPATCH = "      auto complete_one = [&](const int *Ci, const int *Di) {\n"
TRACE = '''        printf("AUDIT pi=%d batch=%lld C=", pi, cur_batch);
        for(int ai=0;ai<n;ai++) putchar(Ci[ai]>0?'+':'-');
        printf(" D=");
        for(int ai=0;ai<n;ai++) putchar(Di[ai]>0?'+':'-');
        putchar('\\n'); return; // review-only dispatch observation
'''
TAIL = "    fh_place(i1, a1, b1, A, B, Dab, Kab, L);"
GATE = "    if(d>0 && A[0]*a2+B[0]*b2+A[n]*a1+B[n]*b1 != FH_CD_target[n-d]-Dab[n-d]) continue;\n"
for needle in (PIN, DISPATCH, TAIL):
    assert BASE.count(needle) == 1, "Source changed; re-audit probe insertion"


def build(directory, name, source):
    cpp = directory / (name + ".cpp")
    binary = directory / name
    cpp.write_text(source)
    subprocess.run([COMPILER, "-O3", "-std=c++17", str(cpp), "-o", str(binary)], check=True)
    return binary


def run(binary, args, settings):
    env = {**ENV, **{k: str(v) for k, v in settings.items()}}
    result = subprocess.run([str(binary), *map(str, args)], env=env,
                            text=True, capture_output=True, timeout=20, cwd=ROOT)
    assert result.returncode in (0, 3), result.stderr
    return result.stdout


def cd_orbit(c, d):
    def variants(x):
        neg = x.translate(str.maketrans("+-", "-+"))
        return x, x[::-1], neg, neg[::-1]
    return min(min(a + "|" + b, b + "|" + a)
               for a in variants(c) for b in variants(d))


def reference_pins(path):
    rows = [x for x in path.read_text().splitlines() if x and not x.startswith("#")]
    C, D = [tuple(map(int, x.split())) for x in rows[4:6]]
    def key(c, d):
        return "|".join("".join(str(sum(x[i::6])) + "," for i in range(6)) for x in (c, d))
    variants = []
    for nc, nd, rc, rd, swap in itertools.product(range(2), repeat=5):
        c = tuple((-1 if nc else 1) * x for x in (C[::-1] if rc else C))
        d = tuple((-1 if nd else 1) * x for x in (D[::-1] if rd else D))
        if swap:
            c, d = d, c
        variants.append((key(c, d), c, d))
    least = min(v[0] for v in variants)
    pins = [(c[0], d[0]) for k, c, d in variants if k == least]
    print(path.name, "retained_cell=", least, "endpoint_pairs=", pins,
          "survives_pins=", (1, 1) in pins)


def main():
    print("Source SHA256:", hashlib.sha256(BASE.encode()).hexdigest(), flush=True)
    common = dict(WZ_FIRSTHIT=1, WZ_FH_M6=1, WZ_FH_AB_PROF=1,
                  WZ_FH_AB_BUDGET=0, WZ_THM211B=1, WZ_THM212=1)
    with tempfile.TemporaryDirectory(prefix="bs45-review-") as tmp:
        p = Path(tmp)
        stock = build(p, "stock", BASE)
        pin = build(p, "pin_probe", BASE.replace(PIN, PIN_PROBE))
        for binary, oc in ((stock, 0), (stock, 1), (pin, 1)):
            out = run(binary, (6, 5, 1, 0, 0), {**common, "WZ_FH_ORBIT_CANON": oc})
            print(binary.name, "oc=", oc)
            print("\n".join(x for x in out.splitlines() if x.startswith(("RESULT:", "candidates_streamed=", "VERIFY:"))))
            if "*** BS(7,6) FOUND" in out:
                result = subprocess.run(["python3", str(ROOT / "tools/verify_npaf.py")],
                                        input=out, text=True, capture_output=True, check=True)
                print("\n".join(x for x in result.stdout.splitlines() if x.startswith("PASS:")))
        audit_source = BASE.replace(DISPATCH, DISPATCH + TRACE)
        audit = build(p, "dispatch", audit_source)
        audit_pin = build(p, "dispatch_pin", audit_source.replace(PIN, PIN_PROBE))
        orbit_sets = []
        for binary, oc in ((audit, 0), (audit, 1), (audit_pin, 1)):
            out = run(binary, (6, 5, 1, 0, 0), {**common, "WZ_FH_CELL_ORDER": 0, "WZ_FH_ORBIT_CANON": oc})
            pairs = re.findall(r"AUDIT .* C=([+-]+) D=([+-]+)", out)
            orbit_sets.append({cd_orbit(c, d) for c, d in pairs})
            print(binary.name, "oc=", oc, "emitted=", len(pairs), "orbits=", len(orbit_sets[-1]))
        assert orbit_sets[0] == orbit_sets[2] and not orbit_sets[1]
        sets = []
        for skip in (0, 8):
            settings = {**common, "WZ_FH_ORBIT_CANON": 1, "WZ_FH_CELL_ORDER": 1,
                        "WZ_FH_PROF_ORDER": 1, "WZ_FH_BUF_CAP": 20, "WZ_FH_DRAIN_TOP": 2,
                        "WZ_FH_SHARD": 0, "WZ_FH_NSHARD": 2, "WZ_FH_PROF_SKIP": skip}
            out = run(audit, (11, 0, 6, 1, 3), settings)
            sets.append({x for x in out.splitlines() if x.startswith("AUDIT ")})
        print("Dispatch skip0 / skip8 / intersection / later-only:",
              len(sets[0]), len(sets[1]), len(sets[0] & sets[1]), len(sets[1] - sets[0]))
        assert sets[1] and sets[1] <= sets[0]
        renamed = BASE.replace("int main(int argc, char **argv)", "int campaign_main(int argc, char **argv)")
        bench = renamed + "\n" + (HERE / "tail_benchmark.cpp.inc").read_text()
        before = build(p, "tail_baseline", bench)
        after = build(p, "tail_gate", bench.replace(TAIL, GATE + TAIL))
        for n in (11, 13):
            outputs = [run(b, (n,), {}) for b in (before, after)]
            for label, output in zip(("tail_baseline", "tail_gate"), outputs):
                print(label, output.strip())
            assert re.search(r"hits=(\d+)", outputs[0])[1] == re.search(r"hits=(\d+)", outputs[1])[1]
            assert re.search(r"digest=(\d+)", outputs[0])[1] == re.search(r"digest=(\d+)", outputs[1])[1]
    reference_pins(ROOT / "results/champions/champion_firsthit_bs43_42.txt")
    reference_pins(ROOT / "results/reference/wz_table1_bs43_42.txt")
    subprocess.run(["python3", str(HERE / "quad_reachability.py")], cwd=ROOT, check=True)
    print("All review assertions PASS. No production source modified.")


if __name__ == "__main__":
    main()
