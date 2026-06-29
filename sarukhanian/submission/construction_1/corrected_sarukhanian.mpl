restart;
unprotect('D');
NPAF := proc(a,s)
local i,j,n,npaf;
npaf := 0;
n := nops(a);
for i from 1 to n-s do
npaf := npaf + a[i]*a[i+s];
od;
RETURN(npaf);
end proc;

n := 3;
A := Matrix(1,n,[1, 1, 1]);
B := Matrix(1,n,[1, 1,-1]);
C := Matrix(1,n-1,[1,-1]);
D := Matrix(1,n-1,[1,-1]);
rA := Matrix(1,n,ListTools[Reverse]([1, 1, 1]));
rB := Matrix(1,n,ListTools[Reverse]([1, 1,-1]));
rC := Matrix(1,n-1,ListTools[Reverse]([1,-1]));
rD := Matrix(1,n-1,ListTools[Reverse]([1,-1]));

x := Matrix(4,1,[1,1,1,1]);
y := Matrix(4,1,[1,1,-1,-1]);
z := Matrix(4,1,[-1,1,-1,1]);
w := Matrix(4,1,[-1,1,1,-1]);

# --- CORRECTED DEFINITIONS ---
mwB := -w.B;
mwC := -w.C;
mwD := -w.D;
mxA := -x.A;
mxC := -x.C;
mxrB := -x.rB;
myB := -y.B;
myD := -y.D;
mzA := -z.A;
mzB := -z.B;
mzC := -z.C;
mzrD := -z.rD;
wB := w.B;
wD := w.D;
wrA := w.rA;
xA := x.A;
xC := x.C;
xD := x.D;
yA := y.A;
yB := y.B;
yD := y.D;
yrC := y.rC;
zA := z.A;
zC := z.C;

# --- CORRECTED SEQUENCE CONSTRUCTION ---
# This sequence has length 110 and zero NPAF.
X_saru := ArrayTools[Concatenate](2,
xA, xC, mxA, mxC, mxrB, mxC, mxA, xC, yA, xD, yA, xD, yA, xD, yB, yD, myB, yrC, myB, yD, yB, myD, zA, zC, mzA, mzrD, mzA, zC, zA, mzC, mzB, mwC, mzB, mwC, mzB, mwC, wB, wD, mwB, mwD, wrA, mwD, mwB, wD
);

X := convert(LinearAlgebra[Row](X_saru,1),list);
Y := convert(LinearAlgebra[Row](X_saru,2),list);
Z := convert(LinearAlgebra[Row](X_saru,3),list);
W := convert(LinearAlgebra[Row](X_saru,4),list);

map(nops,[X,Y,Z,W]);

for s from 1 to 109 do
s, NPAF(X,s)+NPAF(Y,s)+NPAF(Z,s)+NPAF(W,s);
od;
