# MTAC Issue #2: Non-Sexagesimal Trigonometric Arguments

This document provides a detailed technical analysis of Raymond Clare Archibald’s paper *"Tables of Trigonometric Functions in Non-Sexagesimal Arguments"* (MTAC Volume 1, Number 2, April 1943) and its relevance to normalized phase digital oscillators.

---

## 1. Sexagesimal vs. Non-Sexagesimal Angle Arguments

The standard system of angle measurement is sexagesimal (base-60):
$$\text{Circle} = 360^\circ, \quad 1^\circ = 60', \quad 1' = 60''$$

While convenient for historical navigation and astronomy, sexagesimal units are highly inefficient for computational division and calculus.
* **Non-Sexagesimal Alternatives**: Archibald reviews tables that use decimal fractions of a degree, radians (which relate directly to arc length), or **gradians (centesimal degrees)**, where a right angle is divided into $100$ parts.

---

## 2. DSP Application: Normalized Phase Oscillators

In digital signal processing (DSP) and synthesis, the most efficient "non-sexagesimal" argument is **Normalized Phase** ($[0, 1]$):
* A complete circle/oscillation is represented by a floating or fixed-point value ranging from $0.0$ (start of cycle) to $1.0$ (end of cycle).
* This eliminates the need to continuously multiply by $2\pi$ or $360$ during phase accumulation.

### A. Phase Accumulation
Accumulating phase over time is a simple addition and modulo operation:
$$\phi_{t+1} = \text{mod}(\phi_t + f \cdot dt, 1.0)$$
In fixed-point arithmetic (e.g. 18-decimal scaling in the ZMM VM), modulo $1.0$ is equivalent to subtracting $10^{18}$ whenever the phase exceeds $10^{18}$.

### B. Index Lookup Optimization
When querying a quarter-sine lookup table, the index mapping becomes trivial:
1. Divide the $[0, 1]$ phase into quadrants by checking the range:
   * $[0, 0.25]$: Quadrant 1 (direct lookup).
   * $[0.25, 0.5]$: Quadrant 2 (reverse lookup).
   * $[0.5, 0.75]$: Quadrant 3 (negative direct lookup).
   * $[0.75, 1.0]$: Quadrant 4 (negative reverse lookup).
2. Performs lookup index calculation using simple shifts and masks, avoiding any floating-point or radian division.
