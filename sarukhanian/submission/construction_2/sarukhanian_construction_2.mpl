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

# Turyn Sequences (n=3)
n := 3;
A := Matrix(1,n,[1, 1, 1]);
B := Matrix(1,n,[1, 1,-1]);
C := Matrix(1,n-1,[1,-1]);
D := Matrix(1,n-1,[1,-1]);

# Golay Sequences (k=2)
k := 2;
F := Matrix(1,k,[1, 1]);
G := Matrix(1,k,[1, -1]);

# Vectors
x := Matrix(4,1,[1,1,0,0]);
y := Matrix(4,1,[1,-1,0,0]);
z := Matrix(4,1,[0,0,1,1]);
w := Matrix(4,1,[0,0,1,-1]);

# Helper to get element i (1-based)
getA := i -> A[1,i];
getB := i -> B[1,i];
getC := i -> C[1,i];
getD := i -> D[1,i];
getF := i -> F[1,i];
getG := i -> G[1,i];

# Construction 2
# We build the list of columns first
cols := [];

# 1. Loop j=1 to k
for j from 1 to k do
    # {a_i(x f_{k-j+1} + z g_{k-j+1})}_{i=1}^n
    f_val := getF(k-j+1);
    g_val := getG(k-j+1);
    my_coeff := x * f_val + z * g_val;
    for i from 1 to n do
        cols := [op(cols), my_coeff * getA(i)];
    od;
    
    # {c_i(x g_j + z f_{k-j+1})}_{i=1}^{n-1}
    g_val := getG(j);
    f_val := getF(k-j+1);
    my_coeff := x * g_val + z * f_val;
    for i from 1 to n-1 do
        cols := [op(cols), my_coeff * getC(i)];
    od;
od;

# 2. {x a_i - z b_i}_{i=1}^n
for i from 1 to n do
    val := x * getA(i) - z * getB(i);
    cols := [op(cols), val];
od;

# 3. {x d_i - z c_i}_{i=1}^{n-1}
for i from 1 to n-1 do
    val := x * getD(i) - z * getC(i);
    cols := [op(cols), val];
od;

# 4. Loop j=1 to k
for j from 1 to k do
    # {b_i(x g_j + z f_{k-j+1})}_{i=1}^n
    g_val := getG(j);
    f_val := getF(k-j+1);
    my_coeff := x * g_val + z * f_val;
    for i from 1 to n do
        cols := [op(cols), my_coeff * getB(i)];
    od;
    
    # {d_i(-x f_j + z g_{k-j+1})}_{i=1}^{n-1}
    f_val := getF(j);
    g_val := getG(k-j+1);
    my_coeff := -x * f_val + z * g_val;
    for i from 1 to n-1 do
        cols := [op(cols), my_coeff * getD(i)];
    od;
od;

# 5. Loop j=1 to k
for j from 1 to k do
    # {a_i(y f_{k-j+1} + w g_{k-j+1})}_{i=1}^n
    f_val := getF(k-j+1);
    g_val := getG(k-j+1);
    my_coeff := y * f_val + w * g_val;
    for i from 1 to n do
        cols := [op(cols), my_coeff * getA(i)];
    od;
    
    # {c_i(y g_j - w f_j)}_{i=1}^{n-1}
    g_val := getG(j);
    f_val := getF(j);
    my_coeff := y * g_val - w * f_val;
    for i from 1 to n-1 do
        cols := [op(cols), my_coeff * getC(i)];
    od;
od;

# 6. {-y a_i + w b_i}_{i=1}^n
for i from 1 to n do
    val := -y * getA(i) + w * getB(i);
    cols := [op(cols), val];
od;

# 7. {y d_i + w c_i}_{i=1}^{n-1}
for i from 1 to n-1 do
    val := y * getD(i) + w * getC(i);
    cols := [op(cols), val];
od;

# 8. Loop j=1 to k
for j from 1 to k do
    # {-b_i(y g_j + w f_{k-j+1})}_{i=1}^n
    g_val := getG(j);
    f_val := getF(k-j+1);
    my_coeff := y * g_val + w * f_val;
    for i from 1 to n do
        cols := [op(cols), -my_coeff * getB(i)];
    od;
    
    # {d_i(y f_j - w g_{k-j+1})}_{i=1}^{n-1}
    f_val := getF(j);
    g_val := getG(k-j+1);
    my_coeff := y * f_val - w * g_val;
    for i from 1 to n-1 do
        cols := [op(cols), my_coeff * getD(i)];
    od;
od;

# Concatenate all columns
# We need to form X, Y, Z, W lists
X := []; Y := []; Z := []; W := [];

for col in cols do
    X := [op(X), col[1,1]];
    Y := [op(Y), col[2,1]];
    Z := [op(Z), col[3,1]];
    W := [op(W), col[4,1]];
od;

nops(X);

for s from 1 to nops(X)-1 do
s, NPAF(X,s)+NPAF(Y,s)+NPAF(Z,s)+NPAF(W,s);
od;
