# MTAC Issue #1: Pre-computed Lookup Tables and Polynomial Interpolation

This document provides a detailed technical analysis of the computational mathematics presented in the inaugural issue of *Mathematical Tables and Other Aids to Computation* (MTAC) (January 1943). It explores the transition from hand-calculated tables to digital lookup tables (LUTs) and how it applies to low-level virtual machines like the ZMM.

---

## 1. The Role of Tables in Early Computation

Before electronic computers, mathematical calculation was bound to printed tables of logarithms, trigonometric ratios, Bessel functions, and probability integrals. 
* **The Errata Problem**: Because these tables were calculated by human "computers" using pen and paper, they were riddled with printing and calculation errors. Much of MTAC Issue #1 was dedicated to listing errata for widely used engineering tables.
* **Interpolation**: When a required value fell between two table entries, computers used **Differences (Method of Differences)** and polynomial interpolation (such as Newton's or Lagrange's formulas) to approximate the intermediate value.

---

## 2. Low-Level VM Optimization: Sine Lookup Tables (LUT)

In modern low-level virtual machines like the ZMM, calculating transcendental functions (like sines or cosines) using series approximations (e.g., Taylor series) is computationally expensive, consuming significant gas and CPU cycles.

### A. Pre-computed Sine Tables
Following the MTAC methodology, we can optimize our synthesis engines by replacing dynamic trigonometric approximations with a pre-computed **Sine Lookup Table (LUT)**:
1. Store a quarter-sine wave (0 to 90 degrees) in contract bytecode.
2. Translate the input phase angle to the first quadrant using symmetry.
3. Fetch the nearest entries and perform linear interpolation to determine the exact value.

### B. Application: Formant and Semafoon Oscillators
Replacing floating-point JS triggers and Taylor series approximations inside Yul with a fast, static-memory LUT dramatically reduces the simulation step latency and aligns our engines with the architectural patterns of early computers.
