from itertools import product
from functools import lru_cache

def class_count(L,c,m): return len(range(c,L,m))

def enum_class_sums(L,target,m):
    cnt=[class_count(L,c,m) for c in range(m)]
    out=[]; cur=[0]*m
    def rec(c,acc):
        if c==m-1:
            last=target-acc
            if -cnt[c]<=last<=cnt[c] and (last-(-cnt[c]))%2==0:
                cur[c]=last; out.append(tuple(cur))
            return
        for v in range(-cnt[c],cnt[c]+1,2):
            cur[c]=v; rec(c+1,acc+v)
    rec(0,0); return out

def norm(v): return sum(x*x for x in v)
def N(v,s):
    m=len(v)
    return sum(v[i]*v[i+s] for i in range(m-s)) if s<m else 0
def pair_auto(x,y,s):
    m=len(x); return N(x,s)+N(y,s)+N(x,m-s)+N(y,m-s)

def measure(n, sig, m):
    a,b,c,d = sig
    tgt = 4*n+2; half = m//2
    LAB, LCD = n+1, n
    K = enum_class_sums(LAB,a,m); R = enum_class_sums(LAB,b,m)
    P = enum_class_sums(LCD,c,m); Q = enum_class_sums(LCD,d,m)
    # AB side: achievable norms (what the code does) and (tuple,norm) (what WZ do)
    normset=set(); autoset=set()
    for kv in K:
        kn=norm(kv)
        if kn>tgt: continue
        for rv in R:
            rn=norm(rv)
            if kn+rn>tgt: continue
            normset.add(kn+rn)
            autoset.add((tuple(pair_auto(kv,rv,s) for s in range(1,half+1)), kn+rn))
    # CD profiles surviving each filter
    cur=0; wz=0
    for pv in P:
        pn=norm(pv)
        if pn>tgt: continue
        for qv in Q:
            need=tgt-pn-norm(qv)
            if need<0: continue
            if need in normset:
                cur+=1
                needT=tuple(-pair_auto(pv,qv,s) for s in range(1,half+1))
                if (needT,need) in autoset: wz+=1
    return cur, wz

for (n,sig) in [(7,(2,4,3,-1)), (11,(2,4,-5,1))]:
    for m in (3,6):
        cur,wz = measure(n,sig,m)
        r = cur/wz if wz else float('inf')
        print(f"n={n:<3d} m={m}  C,D profiles: norm-only={cur:<8d}  +2.11b={wz:<8d}  reduction={r:6.1f}x")

print()
print("=== scaling of the 2.11b profile cut at m=6 (the number that decides the build) ===")
pts=[]
for (n,sig) in [(7,(2,4,3,-1)),(11,(2,4,-5,1)),(15,(6,4,3,1)),(19,(6,4,5,1)),(23,(8,2,5,1))]:
    assert sum(x*x for x in sig)==4*n+2, (n,sig)
    cur,wz = measure(n,sig,6)
    r = cur/wz if wz else float('inf')
    pts.append((n,r))
    print(f"  n={n:<3d} norm-only={cur:<9d} +2.11b={wz:<8d}  reduction={r:7.1f}x")
import math
if len(pts)>=2:
    (n0,r0),(n1,r1)=pts[0],pts[-1]
    g=(r1/r0)**(1/(n1-n0))
    print(f"\n  growth of the reduction factor: {g:.3f}x per +1 in n")
    for n in (29,31,36):
        print(f"  projected 2.11b cut at n={n}: {r1*g**(n-n1):,.0f}x")
