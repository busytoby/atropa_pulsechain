# MTAC Issue #14: Hammer's Duodecimal Tables, Base-12 Arithmetic, and Logarithmic Pitch Shifting

This document provides a detailed technical analysis of H. K. Hammer’s *"Tables of Reciprocals of Primes, Bases 2–12"* (MTAC Volume 2, Number 14, April 1946) and its application to base-12 fixed-point pitch representations and logarithmic octave division.

---

## 1. Hammer's Duodecimal Prime Reciprocals

In MTAC Issue 14, H. K. Hammer compiled calculation tables in non-decimal bases, specifically focusing on base-12 (duodecimal):
* **The Mathematics**: A number written in base-12 uses twelve digits ($0, 1, 2, 3, 4, 5, 6, 7, 8, 9, \text{A}, \text{B}$).
* **Prime Reciprocals**: Hammer calculated the fractional representation of $1/p$ (for primes $p = 2, 3, 5, \dots, 97$) across different bases. In base-12, fractions with denominators that are powers of 2 or 3 (like $1/2$, $1/3$, $1/4$, $1/6$, $1/8$, $1/12$) terminate cleanly:
  $$1/3 = 0.4_{12}, \quad 1/4 = 0.3_{12}, \quad 1/6 = 0.2_{12}$$

---

## 2. DSP Application: Duodecimal Fixed-Point pitch representation

In digital synthesizers, mapping MIDI note numbers (0 to 127) to frequencies requires calculating an exponential curve:
$$f = f_0 \times 2^{\frac{N - 69}{12}}$$
Executing exponents and divisions on the fly is extremely gas-heavy in Solidity or Yul.

### A. Duodecimal Octave-Semitone Packing
Because Western music is structured in octaves of 12 semitones, we can model our pitch coordinate system as a **duodecimal fixed-point number**:
* **The Format**: Let pitch $P$ be stored as a base-12 number:
  $$P = \text{Octave} . \text{Semitone} \quad (\text{Base-12})$$
  *   Note $60$ (C4): $5.0_{12}$ (5th octave, 0th semitone).
  *   Note $67$ (G4): $5.7_{12}$ (5th octave, 7th semitone).
  *   Note $72$ (C5): $6.0_{12}$ (6th octave, 0th semitone).

### B. Shift-Based Octave Scaling (Zero Multiplication)
Using this base-12 encoding:
1.  **Octave Shifts**: Raising a pitch by an octave is a simple integer addition of $1.0_{12}$ (or $+12$ in standard decimal integer math).
2.  **Fractional Tuning**: Hammer's base-12 tables provide the exact fraction coefficients for semitones. To calculate the target phase increment, instead of performing a slow transcendental exponent $2^{S/12}$, we perform a fast lookup in a 12-entry duodecimal table, followed by a simple bitwise shift (multiplication by powers of 2) for the octave component:
    $$\Delta \theta = \text{Wavetable}[S] \ll \text{Octave}$$
    This bypasses exponential division entirely, executing pitch changes in the ZMM VM with **zero multiplications** and **zero divisions**, yielding perfect 12-tone scaling.
