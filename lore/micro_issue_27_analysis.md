# MICRO: The 6502 Journal - Issue 27 (August 1980) & Arbitrary Precision Math

In August 1980, **Issue 27** of *MICRO: The 6502 Journal* published a landmark math optimization article for the 6502 architecture: **"Nth Precision Add & Subtract With Adjusted Processor Status" by Lawrence R. Golla**.

---

## 1. The Multi-Precision Challenge on 8-bit CPUs

The 6502 is a strict 8-bit processor. Its accumulator (`A`), index registers (`X`, `Y`), and Arithmetic Logic Unit (ALU) handle only 8-bit operands at a time. To add or subtract larger numbers (e.g. 16-bit, 24-bit, or 32-bit values), programmers had to chain arithmetic operations together using carry propagation:

```assembly
; Standard 16-Bit Addition Example
CLC             ; Clear Carry flag
LDA Num1L       ; Load low byte of number 1
ADC Num2L       ; Add low byte of number 2
STA ResultL     ; Store low byte of result
LDA Num1H       ; Load high byte of number 1
ADC Num2H       ; Add high byte of number 2 with carry
STA ResultH     ; Store high byte of result
```

However, writing custom routines for every required precision size (16-bit, 24-bit, 32-bit, etc.) was tedious and bloated code space. 

---

## 2. Golla's "Nth Precision" Zero-Page Engine

Golla solved this by creating a single, highly optimized subroutine that could handle **arbitrary byte lengths (N-precision)**. 

To use it, the programmer set up five zero-page pointer variables:
*   **Pointer A** (2 bytes): Base address of Operand A.
*   **Pointer B** (2 bytes): Base address of Operand B.
*   **Precision Parameter N** (1 byte): The number of bytes to process.

The routine looped through each byte, loading values indirectly through zero-page pointers using indexing (`LDA (PointerA), Y`), performing `ADC` or `SBC`, and writing the results back.

### The Status Flag Correction Hack
A major bug in simple multi-byte loops is that the CPU's status register flags (specifically **Zero (Z)** and **Negative (N)**) reflect only the result of the *very last byte* computed in the loop. 

Golla introduced a clever masking technique to adjust the processor status register (`P`) on the stack so that when the subroutine returned (`RTS`), standard conditional branches (`BEQ`, `BNE`, `BMI`, `BPL`) behaved as if the entire N-byte number had been computed in a single step.

---

## 3. Structural Parallel: ZMM Multi-Word Bridging

This zero-page pointer indirect mapping directly mirrors how the **ZMM VM to EVM Bridge** performs arbitrary-precision conversions between 16-bit local memories and 256-bit EVM structures:

1.  **Zero-Page Indirects = ZMM MMIO Pointers**: Rather than loading data directly, we pass offset registers (e.g., target RAM addresses) to a unified loop decoder.
2.  **State Flags Adjustment**: When converting 256-bit EVM numbers to 16-bit words, we must mask and adjust execution state flags to prevent truncation errors, ensuring the virtual Z-Machine correctly processes bounds checks across the transaction boundaries.
