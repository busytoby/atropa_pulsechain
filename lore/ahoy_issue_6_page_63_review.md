# Ahoy! Magazine Issue 6 (June 1984) — Page 63 Code Review

Page 63 of the June 1984 issue of *Ahoy! Magazine* features Dale Rupert's recurring column **"Commodares,"** where reader-submitted solutions to previous programming challenges are showcased.

The code samples printed on this page present elegant, optimized, and creative implementations in C64 BASIC. Below is a technical analysis of the main listings.

---

## 1. Challenge #5-4: The Letter Triangle
This challenge required generating a triangle of alphabetical characters where each letter is repeated according to its alphabetical position (A once, B twice, C thrice, up to Z).

### BASIC Listing:
```basic
10 FOR L = 1 TO 26
20 FOR N = 1 TO L
30 PRINT CHR$(L + 64);
40 NEXT N
50 PRINT
60 NEXT L
```

### Analysis:
* **Nested Loops**: The outer loop cycles through the 26 letters of the alphabet (`L`). The inner loop repeats `L` times to print the character.
* **Character Mapping**: `CHR$(L + 64)` maps the loop index `L` (1 to 26) to the uppercase ASCII/PETSCII values for A–Z (`65` to `90`).
* **Formatting**: The semicolon `;` at the end of line 30 prevents printing carriage returns between matching letters, while line 50 forces a new line at the end of each row.

---

## 2. Problem #2: Reversals
The task was to take an input word or phrase and output it in reverse order. The shortest solutions were optimized down to a single BASIC line.

### Shortest One-Liner (Dino Russo):
```basic
1 INPUT "WORD OR PHRASE"; W$: FOR L = LEN(W$) TO 1 STEP -1: PRINT MID$(W$, L, 1);: NEXT
```

### Centered Screen Variation (S. Gaudet):
This variation centers the reversed string horizontally on the C64's 46-column screen:
```basic
1 INPUT N$: L = LEN(N$): FOR X = L TO 1 STEP -1: PRINT TAB((20 - LEN(N$)) / 2) MID$(N$, X, 1);: NEXT
```

---

## 3. Right-to-Left Typing (Ken Buskirk)
This creative solution prints characters onto the screen from right to left in real-time as the user types them.

### BASIC Listing (Paraphrased):
```basic
10 X = 39: B$ = CHR$(32): C$ = CHR$(13)
20 GET A$: IF A$ = "" THEN 20
30 X = X - 1: PRINT SPC(X) A$ "{CURSOR_UP_LEFT}"
40 IF A$ = B$ AND X <= 10 THEN X = 39: PRINT
50 IF X = 0 THEN X = 39: PRINT
60 IF A$ = C$ THEN X = 39: PRINT
70 GOTO 20
```

### Analysis:
* **Interactive Buffer**: Uses `GET` to capture single keypresses asynchronously instead of blocking on `INPUT`.
* **Dynamic Spacing**: Employs `SPC(X)` and terminal cursor controls to reposition the characters backwards, giving a reverse-typing visual effect.

---

## 4. Palindrome Solver (Mark Nekic)
A palindrome solver determines if a word is spelled the same forwards and backwards. Mark Nekic submitted the shortest program using a single line:

### BASIC Listing:
```basic
1 INPUT N$: L = LEN(N$): FOR X = 1 TO L: IF MID$(N$, X, 1) = MID$(N$, L + 1 - X, 1) THEN NEXT: PRINT "YES"
```

### Analysis:
* **Symmetry Check**: Compares character `X` from the beginning with character `L + 1 - X` from the end.
* **Implicit Loop Logic**: The loop continues using `THEN NEXT`. If any character mismatch occurs, the condition `IF ... THEN NEXT` fails, the loop halts, and "YES" is skipped, indicating it is not a palindrome.
