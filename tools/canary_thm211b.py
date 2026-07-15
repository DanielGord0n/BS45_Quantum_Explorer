import sys, glob

def load(path):
    toks = [l for l in open(path) if not l.strip().startswith('#')]
    n = int(toks[0].split()[0])
    sig = [int(x) for x in toks[1].split()]
    seqs = []
    for l in toks[2:]:
        v = [int(x) for x in l.split()]
        if v: seqs.append(v)
        if len(seqs) == 4: break
    return n, sig, seqs

def class_sums(seq, m):
    """k_{i,m} = sum of seq[j] over j == i (mod m).  Paper is 1-indexed on j."""
    out = [0]*m
    for j, x in enumerate(seq, start=1):      # j = 1..len
        out[(j-1) % m] += x                   # class index i=1..m -> 0-based
    return out

def N(v, s):
    """Thm 2.3 eq 2.9: N_v(s) = sum_{i=1}^{m-s} v_i * v_{i+s}   (non-circular)"""
    m = len(v)
    return sum(v[i]*v[i+s] for i in range(m-s)) if s < m else 0

def check(path, m):
    n, sig, (A,B,C,D) = load(path)
    tgt = 4*n + 2
    K, R = class_sums(A, m), class_sums(B, m)
    P, Q = class_sums(C, m), class_sums(D, m)
    # --- 2.11a : the norm identity (this IS implemented in the code) ---
    norm = sum(x*x for x in K+R+P+Q)
    a_ok = (norm == tgt)
    # --- 2.11b : residue autocorrelation (NOT implemented anywhere) ---
    b_res = []
    for s in range(1, m//2 + 1):
        tot = (N(K,s)+N(R,s)+N(P,s)+N(Q,s)
             + N(K,m-s)+N(R,m-s)+N(P,m-s)+N(Q,m-s))
        b_res.append((s, tot))
    b_ok = all(t == 0 for _, t in b_res)
    name = path.split('/')[-1]
    print(f"  {name:28s} n={n:<3d} m={m}  2.11a: {'PASS' if a_ok else f'FAIL({norm}!={tgt})'}"
          f"   2.11b: {'PASS' if b_ok else 'FAIL'}  {b_res}")
    return a_ok, b_ok

files = sorted(glob.glob('results/champions/*.txt'))
for m in (3, 6):
    print(f"=== modulus m={m} ===")
    allb = []
    for f in files:
        try: allb.append(check(f, m)[1])
        except Exception as e: print(f"  {f}: parse skip ({e})")
    print(f"  --> 2.11b holds on {sum(allb)}/{len(allb)} banked solutions\n")
