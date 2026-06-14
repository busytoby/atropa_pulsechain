# MTAC Issue #11: Bickley, Mathieu Functions, and Parametric Waveguide Resonance

This document provides a detailed technical analysis of W. G. Bickley’s paper *"The Tabulation of Mathieu Functions"* (MTAC Volume 1, Number 11, July 1945) and its direct application to parametric FM synthesis and elliptical waveguide resonators.

---

## 1. Mathieu Functions and Parametric Resonance

Mathieu functions are solutions to Mathieu's differential equation, which describes wave propagation in systems with elliptical boundaries or periodic parameter variation:
$$\frac{d^2 y}{dx^2} + [a - 2q \cos(2x)] y = 0$$
*   **The Parameters**:
    *   $a$: The characteristic value (related to the natural frequency of the system).
    *   $q$: The parameter representing the strength of the periodic modulation.
*   **Physical Meaning**: Unlike linear wave equations (which yield standard sines and cosines), Mathieu's equation models **parametric resonance**—where the physical parameters of the oscillator (such as stiffness or tension) are modulated periodically over time.

---

## 2. DSP Application: Elliptical Waveguides and FM Synthesis

In our **Interpolation** synthesizer, Mathieu functions expand our physical modeling capabilities in two primary directions:

### A. Elliptical Percussion Waveguides
Standard cylindrical drums are modeled using Bessel functions ($J_n, Y_n$). Real cymbals, string soundboards, and acoustic chambers, however, are often non-circular (elliptical).
*   By solving Mathieu's equation in elliptical coordinates, the synthesizer can model the asymmetric, organic resonant decay of an elliptical plate:
    $$\text{Wave}(r, \theta) = \text{Ce}_n(r, q) \cdot \text{ce}_n(\theta, q)$$
    where $\text{Ce}_n$ is the modified (radial) Mathieu function and $\text{ce}_n$ is the angular Mathieu function. This produces rich, complex harmonic beatings and decays that are far more organic than circular Bessel approximations.

### B. Parametric FM Synthesis (Frequency Modulation)
Mathematically, Mathieu's equation represents an oscillator whose tuning is driven by an FM modulator:
*   The term $-2q \cos(2x)$ is exactly the modulator input in **Frequency Modulation (FM) Synthesis** (John Chowning).
*   By setting the modulation index $q$ dynamically via the **TUN** slider, the agent can transition the sound from a simple harmonic sine wave (low $q$) to a wide-spectrum, metallic bell-like sideband spectrum (high $q$), matching the parametric resonance profiles documented in Bickley's 1945 paper.
