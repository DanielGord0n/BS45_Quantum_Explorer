# Best Effort Construction 2 (Found by Genetic Algorithm)
# Score: 3032 (Approx 3 non-zero shifts)
restart;
with(LinearAlgebra):
n := 3; k := 2;
A := Vector([1, 1, 1]);
B := Vector([1, 1, -1]);
C := Vector([1, -1]);
D := Vector([1, -1]);
F := Vector([1, 1]);
G := Vector([1, -1]);
x := Vector([1, 1, 0, 0]);
y := Vector([1, -1, 0, 0]);
z := Vector([0, 0, 1, 1]);
w := Vector([0, 0, 1, -1]);
X := [];

# Group 1
for j from 1 to k do
    coeff := x * -F[k-j+1] + z * -G[k-j+1];
    for i from 1 to n do
        X := [op(X), coeff * -A[i]];
    end do;
end do;

# Group 2
for j from 1 to k do
    coeff := x * -G[j] + z * -F[j];
    for i from 1 to n-1 do
        X := [op(X), coeff * -C[i]];
    end do;
end do;

# Group 3
for i from 1 to n do
    val := x * A[i] + z * B[i];
    X := [op(X), val];
end do;

# Group 4
for i from 1 to n-1 do
    val := x * -D[i] + z * C[i];
    X := [op(X), val];
end do;

# Group 5
for j from 1 to k do
    coeff := x * G[k-j+1] + z * F[k-j+1];
    for i from 1 to n do
        X := [op(X), coeff * B[i]];
    end do;
end do;

# Group 6
for j from 1 to k do
    coeff := x * F[k-j+1] + z * G[k-j+1];
    for i from 1 to n-1 do
        X := [op(X), coeff * D[i]];
    end do;
end do;

# Group 7
for j from 1 to k do
    coeff := y * -F[k-j+1] + w * G[k-j+1];
    for i from 1 to n do
        X := [op(X), coeff * A[i]];
    end do;
end do;

# Group 8
for j from 1 to k do
    coeff := y * -G[j] + w * F[k-j+1];
    for i from 1 to n-1 do
        X := [op(X), coeff * C[i]];
    end do;
end do;

# Group 9
for i from 1 to n do
    val := y * A[i] + w * B[i];
    X := [op(X), val];
end do;

# Group 10
for i from 1 to n-1 do
    val := y * D[i] + w * -C[i];
    X := [op(X), val];
end do;

# Group 11
for j from 1 to k do
    coeff := y * G[j] + w * F[k-j+1];
    for i from 1 to n do
        X := [op(X), coeff * -B[i]];
    end do;
end do;

# Group 12
for j from 1 to k do
    coeff := y * F[k-j+1] + w * -G[k-j+1];
    for i from 1 to n-1 do
        X := [op(X), coeff * D[i]];
    end do;
end do;

# Verify NPAF
N := 50;
row1 := Vector[row](N); row2 := Vector[row](N); row3 := Vector[row](N); row4 := Vector[row](N);
for i from 1 to N do
    row1[i] := X[i][1]; row2[i] := X[i][2]; row3[i] := X[i][3]; row4[i] := X[i][4];
end do;
npaf := (s) -> add(row1[i]*row1[i+s] + row2[i]*row2[i+s] + row3[i]*row3[i+s] + row4[i]*row4[i+s], i=1..N-s);
shifts := [];
for s from 1 to N-1 do
    val := npaf(s);
    if val <> 0 then shifts := [op(shifts), [s, val]]; end if;
end do;
print(shifts);
