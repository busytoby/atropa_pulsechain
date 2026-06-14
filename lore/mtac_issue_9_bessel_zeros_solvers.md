# MTAC Issue #9: Algebraic Solvers, Bessel Zeros, and Dynamic Resonance Tuning

This document provides a detailed technical analysis of J. S. Frame’s *"Machines for Solving Algebraic Equations"* and Milton Abramowitz’s *"Zeros of Certain Bessel Functions of Fractional Order"* (MTAC Volume 1, Number 9, January 1945) and their combined application to dynamic root-finding for percussion synthesis.

---

## 1. Frame's Solvers and Abramowitz's Bessel Zeros

MTAC Issue 9 bridged numerical root-finding machines and special function wave boundaries:
*   **Frame's Algebraic Solvers**: Reviewed mechanical and analog electrical calculators designed to find the roots (zeros) of high-degree polynomials and transcendental equations.
*   **Abramowitz's Zeros of Fractional Bessel Functions**: Compiled the precise coordinates of the zero-crossings ($x$ where $J_v(x) = 0$) for fractional orders $v = \pm 1/3, \pm 2/3$. These fractional orders are critical in solving wave equations on curved structural shells and membranes.

---

## 2. DSP Application: Dynamic Percussion Modal Tuning

In physical modeling percussion synthesis (e.g. cymbals, drums, or bell plates), the frequencies of the resonant modes are not simple integer harmonics ($1f, 2f, 3f$). Instead, they are directly proportional to the **zeros (roots) of the Bessel functions**:
$$f_{m,n} = f_0 \times \frac{\alpha_{m,n}}{\alpha_{0,1}}$$
where $\alpha_{m,n}$ is the $n$-th zero of the Bessel function of order $m$.

### A. Dynamic Root-Finding in the VM
Instead of storing massive hardcoded tables of zeros for every possible fractional order (which consumes significant VM memory), we can implement a **Newton-Raphson root-finding loop** inspired by Frame's algebraic solvers:
1.  **Initial Guess**: Use a rational approximation for the $n$-th zero of $J_v(x)$.
2.  **Newton-Raphson Step**: Since the derivative of a Bessel function can be written using lower-order functions:
    $$\frac{d}{dx} J_v(x) = \frac{v}{x} J_v(x) - J_{v+1}(x)$$
    The update rule to locate the zero converges rapidly:
    $$x_{k+1} = x_k - \frac{J_v(x_k)}{J'_v(x_k)} = x_k - \frac{J_v(x_k)}{\frac{v}{x_k} J_v(x_k) - J_{v+1}(x_k)}$$
3.  **Result**: The loop converges in $3-4$ iterations, generating the exact resonance coordinates dynamically to retune percussion heads instantly when tension or material coefficients change.
