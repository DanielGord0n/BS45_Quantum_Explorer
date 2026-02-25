# Delta-Code Construction from BS(20,19)
# CP493 - Directed Research - Daniel Gordon
# Verifies BS(20,19) and constructs a delta-code of length 39

restart;
with(LinearAlgebra):
unprotect(D):

# Input: BS(20,19) from Professor Kotsireas
A := Vector([-1,1,-1,1,-1,-1,1,1,1,-1,-1,-1,1,1,1,1,1,-1,-1,-1]);
B := Vector([1,-1,-1,1,-1,-1,1,-1,-1,1,1,1,-1,1,1,1,1,-1,1,-1]);
C := Vector([1,-1,1,1,1,1,1,-1,1,1,1,-1,-1,1,1,-1,1,-1,-1]);
D := Vector([1,1,1,1,1,1,-1,-1,1,-1,1,-1,1,1,1,-1,1,1,-1]);

n := Dimension(A);   # 20
n1 := Dimension(C);  # 19

printf("=== Verifying BS(%d,%d) ===\n", n, n1);

# Check NPAF = 0 for all shifts
bs_valid := true:
for s from 1 to n-1 do
    total := 0:
    for i from 1 to n-s do
        total := total + A[i]*A[i+s] + B[i]*B[i+s]:
    end do:
    for i from 1 to n1-s do
        total := total + C[i]*C[i+s] + D[i]*D[i+s]:
    end do:
    if total <> 0 then
        printf("  Shift %d: NPAF = %d (FAILED)\n", s, total);
        bs_valid := false:
    end if:
end do:
if bs_valid then
    printf("  Result: BS(%d,%d) is VALID. NPAF = 0 for all shifts.\n\n", n, n1);
end if:

# Construct delta-code of length 2n-1 = 39
m := 2*n - 1;
printf("=== Constructing delta-code of length %d ===\n", m);

T1 := Vector(m): T2 := Vector(m): T3 := Vector(m): T4 := Vector(m):

for i from 1 to n do
    T1[i] := A[i]: T2[i] := B[i]: T3[i] := A[i]: T4[i] := B[i]:
end do:
for i from 1 to n1 do
    T1[n+i] := C[i]:  T2[n+i] := D[i]:
    T3[n+i] := -C[i]: T4[n+i] := -D[i]:
end do:

printf("  T1 = [A,  C], length %d\n", m);
printf("  T2 = [B,  D], length %d\n", m);
printf("  T3 = [A, -C], length %d\n", m);
printf("  T4 = [B, -D], length %d\n\n", m);

# Verify delta-code NPAF = 0
dc_valid := true:
for s from 1 to m-1 do
    total := 0:
    for i from 1 to m-s do
        total := total + T1[i]*T1[i+s] + T2[i]*T2[i+s] + T3[i]*T3[i+s] + T4[i]*T4[i+s]:
    end do:
    if total <> 0 then
        printf("  Shift %d: NPAF = %d (FAILED)\n", s, total);
        dc_valid := false:
    end if:
end do:
if dc_valid then
    printf("  Result: Delta-code of length %d is VALID. NPAF = 0 for all %d shifts.\n", m, m-1);
end if:
