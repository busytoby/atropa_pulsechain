# MTAC Issue #5: D. H. Lehmer and the Computation of Modified Bessel Functions

This document provides a detailed technical analysis of Derrick Henry Lehmer’s paper *"Note on the Computation of the Bessel Function $I_n(x)$"* (MTAC Volume 1, Number 5, April 1944) and its application to calculating quantum barrier tunneling states.

---

## 1. D. H. Lehmer and Miller's Backward Recurrence

In 1944, computing Bessel functions of high order or large arguments was computationally difficult due to the accumulation of rounding errors.
* **The Problem with Forward Recurrence**: Running the standard recurrence relation:
  $$I_{n+1}(x) = I_{n-1}(x) - \frac{2n}{x} I_n(x)$$
  forwards is numerically unstable. Small errors in the initial terms $I_0$ and $I_1$ grow exponentially as $n$ increases, quickly ruining accuracy.
* **Lehmer's Solution (Backward Recurrence)**: Lehmer demonstrated that the recurrence is highly stable when run **backwards**:
  $$I_{n-1}(x) = \frac{2n}{x} I_n(x) + I_{n+1}(x)$$
  By choosing a sufficiently large starting order $N$, setting arbitrary initial values (e.g., $I_N = 0, I_{N-1} = 10^{-16}$), and running the recurrence backwards to $n=0$, the values converge to the true ratios of the functions. A final normalization step scaling by the identity:
  $$e^x = I_0(x) + 2 \sum_{k=1}^{\infty} I_k(x)$$
  recovers the absolute values of $I_n(x)$ with high precision.

---

## 2. DSP Application: Real-Time Tunneling Modulators

To compute the modified Bessel functions $I_0(x)$ and $I_1(x)$ (which govern the wave functions inside our circular tunneling barrier):

### A. The Yul Backward Recurrence Loop
Rather than hardcoding massive lookup tables for every order $n$, we can implement Lehmer's backward recurrence loop directly in the Math Coprocessor:
1. Select starting order $N = 12$ (sufficient for $x \le 5.0$).
2. Initialize $I_{12} = 0$, $I_{11} = 1$ (scaled).
3. Loop backwards from $n = 11$ down to $1$ to compute unnormalized terms.
4. Apply the exponential normalization sum to scale the results back to $1e18$ fixed-point coordinates.

This gives the synthesizer the ability to dynamically calculate quantum barrier wave equations for any arbitrary radial coordinate $x$ on the fly, producing mathematically perfect tunneling wave-shapes.
