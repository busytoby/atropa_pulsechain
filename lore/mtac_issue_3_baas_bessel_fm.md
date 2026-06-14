# MTAC Issue #3: B.A.A.S. Mathematical Tables and FM Synthesis

This document provides a detailed technical analysis of the B.A.A.S. (British Association for the Advancement of Science) mathematical tables discussed in *Mathematical Tables and Other Aids to Computation* (MTAC Volume 1, Number 3, July 1943) and their direct relation to Bessel functions in Frequency Modulation (FM) synthesis.

---

## 1. B.A.A.S. and Bessel Function Tabulation

From 1873 to 1948, the B.A.A.S. mathematical tables committee published high-precision tables of transcendental functions, most notably **Bessel Functions of the First Kind ($J_n(x)$)**. 
* Bessel functions solve differential equations for radial wave propagation, such as acoustic vibrations in circular drumheads and electromagnetic waves in cylinders.
* These hand-calculated tables represented the highest level of numerical precision available prior to electronic computers.

---

## 2. DSP Application: Frequency Modulation (FM) Sidebands

In 1973, John Chowning developed **FM Synthesis**. The mathematical definition of a simple frequency-modulated carrier wave is:
$$x(t) = A \cdot \sin(\omega_c t + \beta \cdot \sin(\omega_m t))$$
where:
* $\omega_c$ is the carrier frequency.
* $\omega_m$ is the modulator frequency.
* $\beta$ is the **Modulation Index**.

Using the Jacobi-Anger expansion, this trigonometric equation can be expanded into an infinite sum of sidebands:
$$x(t) = A \sum_{n=-\infty}^{\infty} J_n(\beta) \cdot \sin((\omega_c + n \omega_m) t)$$

### A. The Role of the Bessel Tables
The amplitude of each sideband frequency ($f_c \pm n f_m$) is governed exactly by the $n$-th order Bessel function evaluated at the modulation index, $J_n(\beta)$.
* As the modulation index $\beta$ increases, the energy spreads out to higher sidebands, making the sound brighter.
* The B.A.A.S. Bessel tables provide the exact values needed to calculate sideband energy distribution, allowing synthesizers to accurately predict and balance harmonic timbres.
