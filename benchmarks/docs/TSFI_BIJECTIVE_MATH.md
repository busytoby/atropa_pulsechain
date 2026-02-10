# TSFi Bijective Math: Thunk-Managed Precision

## Problem: Static Magic Numbers
The current AVX-512 Hilbert implementation relies on hardcoded "Magic Number" floats for trigonometric approximations (Minimax polynomials).
```c
__m512 c6 = _mm512_set1_ps(-60.18511444f); // Compile-time constant
```
This is **non-bijective**: The logic is frozen in the binary, detached from the `LauWiredHeader` identity. It prevents a font from "carrying" its own mathematical rules.

## Solution: Assembly Thunk Injection
We can leverage the `ThunkProxy` system to treat mathematical constants as **Dynamic Context** loaded into the WGP's private state (SGPRs/ZMMs) at runtime.

### 1. The Math Profile (SGPR State)
We define a structure within the `LauRegisterBank` (simulated SGPRs) to hold the coefficients.

```c
typedef struct {
    float domain_scaler; // 1/(2*PI)
    float cos_coeffs[4]; // c0, c2, c4, c6
    float sin_coeffs[4]; // s1, s3, s5, s7
    float range_limit;   // 0.5f
} TSFiMathProfile; // Fits in 10 SGPR slots (40 bytes)
```

### 2. The Thunk Preamble (JIT Assembly)
Instead of the kernel loading constants from memory inside the loop, the **Thunk** performs a "Register Setup" preamble.

**Assembly Logic (`ThunkProxy_emit_math_setup`):**
1.  **Load Pointer:** `MOV RAX, [Context + Offset_to_SGPRs]`
2.  **Broadcast Constants (AVX-512):**
    *   `VBROADCASTSS ZMM31, [RAX + 0x00]` (Load Domain Scaler)
    *   `VBROADCASTSS ZMM30, [RAX + 0x04]` (Load Cos C0)
    *   `VBROADCASTSS ZMM29, [RAX + 0x08]` (Load Cos C2)
    *   ...
3.  **Jump to Kernel:** `JMP Kernel_Entry`

### 3. The Kernel (Pure Compute)
The Hilbert Kernel is rewritten to assume that **ZMM20-ZMM31** are *already populated* with the correct constants.
*   It effectively becomes a "Pure Function" $f(	ext{Data}, 	ext{Registers})$.
*   It no longer touches memory for constants (reducing L1 pressure).

## Benefits
1.  **Inheritance of Precision:** A "Child" font can inherit the geometry of a parent but override the `TSFiMathProfile` in its SGPRs to use a faster (lower order) or more precise (higher order) approximation.
2.  **Bijective Integrity:** The "Identity" (Header) now fully describes the result. The output is a function of `(Geometry + MathState)`.
3.  **Performance:** Loading constants into ZMMs *once* via the Thunk (outside the loop) saves decode/issue bandwidth inside the tight Hilbert evaluation loop.

## Implementation Path
1.  Extend `LauRegisterBank` in `lau_memory.h` to include `TSFiMathProfile`.
2.  Implement `ThunkProxy_emit_avx_load` in `lau_thunk.c` to generate `vbroadcastss` instructions.
3.  Refactor `tsfi_hilbert_avx512.c` to use `register __m512 c0 asm("zmm30")` variable pinning (GCC extension) or inline assembly to alias the pre-loaded registers.
