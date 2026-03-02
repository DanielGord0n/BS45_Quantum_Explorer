# Code Comparison: Original vs. Corrected

This document highlights the specific change made to the Maple code to fix the Sarukhanian construction.

## 1. Summary of Changes
The original Maple code had the correct structure and length (110).
There was **exactly one sign error** found in the 26th block of the sequence.

## 2. Specific Change

### Block 26 (Line 26 in definition)
The term `zrD` (representing $z \cdot rD$) was incorrect. It should be `mzrD` (representing $-z \cdot rD$).

#### Original Code (Flawed)
```maple
X_saru := ArrayTools[Concatenate](2,
...
myD, zA, zC, mzA, zrD, mzA,  <-- ERROR: zrD is positive
...
);
```

#### Corrected Code (Fixed)
```maple
X_saru := ArrayTools[Concatenate](2,
...
myD, zA, zC, mzA, mzrD, mzA, <-- FIXED: Changed to mzrD (negative)
...
);
```

## 3. Verification
With this single change, the NPAF sum becomes zero for all shifts.
```maple
for s from 1 to 109 do
s, NPAF(X,s)+NPAF(Y,s)+NPAF(Z,s)+NPAF(W,s);
od;
```
**Result:** All values are `0`.
