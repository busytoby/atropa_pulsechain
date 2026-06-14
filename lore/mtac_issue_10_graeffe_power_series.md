# MTAC Issue #10: D. H. Lehmer, the Graeffe Process, and Filter Coefficient Solvers

This document provides a detailed technical analysis of Derrick Henry Lehmer’s paper *"The Graeffe process as applied to power series"* (MTAC Volume 1, Number 10, April 1945) and its application to solving recursive digital filter coefficients.

---

## 1. The Graeffe Root-Squaring Process for Power Series

The **Graeffe Process** (or root-squaring method) is an iterative numerical algorithm used to find all the roots of an algebraic polynomial:
$$P(x) = a_n x^n + a_{n-1} x^{n-1} + \dots + a_0 = 0$$
* **The Mechanism**: By repeatedly constructing new polynomials whose roots are the squares of the roots of the previous polynomial:
  $$P_{k+1}(x) = (-1)^n P_k(\sqrt{x}) P_k(-\sqrt{x})$$
  the magnitudes of the roots become widely separated (e.g. if roots are $r_1$ and $r_2$ with $r_1 > r_2$, then after $k$ steps $r_1^{2^k} \gg r_2^{2^k}$).
* **Lehmer's Contribution**: In 1945, Lehmer extended this technique to **infinite power series** (transcendental functions), showing how to calculate the poles and zeros of functions like Bessel or elliptic series by finding the dominant roots of their infinite expansion series.

---

## 2. DSP Application: Auto-Tuning Recursive IIR Filters

In digital synthesizer design, the poles of a filter's transfer function $H(z)$ must lie strictly inside the unit circle ($|z_p| < 1.0$) to guarantee stability:
$$H(z) = \frac{B(z)}{A(z)} = \frac{b_0 + b_1 z^{-1} + b_2 z^{-2}}{1 + a_1 z^{-1} + a_2 z^{-2}}$$

### A. Dynamic Pole Separation in the ZMM VM
When designing high-order filters (like the formant resonant paths):
1.  **Extract the Denominator**: The denominator polynomial $A(z)$ represents the poles.
2.  **Run Lehmer's Graeffe Loop**: By executing 3 iterations of the Graeffe root-squaring algorithm in Yul bytecode, we square the pole coordinates:
    $$A_{k+1}(z) = (-1)^2 A_k(\sqrt{z}) A_k(-\sqrt{z})$$
3.  **Stability Detection**: Since the root magnitudes are now separated exponentially ($|z_p|^8$), we can instantly check if the dominant pole magnitude is approaching $1.0$.
4.  **Auto-Q-Damping Trigger**: If the Graeffe step detects $|z_{\text{dominant}}|^8 \ge 0.95$, the system triggers the Q-damping protection loop, automatically adjusting $a_1$ and $a_2$ to pull the poles back toward the origin, preventing the filter from breaking into unstable self-oscillation.
