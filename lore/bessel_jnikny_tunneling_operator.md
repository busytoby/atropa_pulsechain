# Parameterizing Bessel Families (Jn, In, Kn, Yn) with the TUN Operator

This document provides a mathematical formulation describing how the four primary families of Bessel functions—$J_n, I_n, K_n,$ and $Y_n$—are parameterized and modulated by the quantum tunneling coefficient (**TUN**) inside our synthesis engine.

---

## 1. Mathematical Definitions of the Four Bessel Families

For an integer order $n$ and radial variable $x$:

| Family | Function Name | Mathematical Behavior ($x > 0$) | Boundary Conditions |
| :--- | :--- | :--- | :--- |
| **$J_n(x)$** | Bessel of 1st Kind | Oscillatory decay (standing waves) | Finite at origin ($x \to 0$) |
| **$Y_n(x)$** | Bessel of 2nd Kind | Oscillatory decay (standing waves) | Singular at origin ($\lim_{x \to 0} Y_n(x) = -\infty$) |
| **$I_n(x)$** | Modified Bessel of 1st Kind | Hyperbolic growth ($e^x$ profile) | Finite at origin ($x \to 0$) |
| **$K_n(x)$** | Modified Bessel of 2nd Kind | Hyperbolic decay ($e^{-x}$ profile) | Singular at origin ($\lim_{x \to 0} K_n(x) = +\infty$) |

---

## 2. The TUN Operator and Boundary Matching

In the physical model of a cylindrical quantum well, the wave function $\psi(r)$ of a tunneling particle is solved by matching these four families across boundaries, governed by the tunneling probability coefficient **TUN** ($T \in [0, 1.0]$):

$$\psi(r) = \begin{cases} 
A \cdot J_n(k r) & r < R_1 \quad \text{(Region I: Well Core)} \\
B \cdot K_n(\kappa r) + \mathbf{TUN} \cdot C \cdot I_n(\kappa r) & R_1 \le r < R_2 \quad \text{(Region II: Barrier / Tunneling)} \\
\mathbf{TUN} \cdot D \cdot Y_n(k r) & r \ge R_2 \quad \text{(Region III: Free Propagation)}
\end{cases}$$

* **TUN = 0 (No Tunneling)**: The particle is perfectly confined. Inside the barrier, only the decaying state $K_n$ is active, and the wavefunction outside (Region III) is $0$. The sound is a pure, muted metallic resonance ($J_n$).
* **TUN > 0 (Quantum Leakage)**: The barrier becomes partially transparent. The growing state $I_n$ is mixed in, allowing energy to tunnel across the barrier. This excites the singular $Y_n$ wave outside, generating bright, noisy, propagating transients.

---

## 3. Synth DSP Parameter Mapping

In our **Interpolation** synthesizer control path:
* The user adjusts the **TUN** slider.
* The slider value directly interpolates the mixture coefficients of the Yul math coprocessor Bessel outputs:
  $$\text{Output}(x) = (1 - \text{TUN}) \cdot J_0(x) + \text{TUN} \cdot \left[ K_0(x) + \text{sinh}(x) \right] + \text{Transient}(\text{TUN}) \cdot Y_0(x)$$
* This allows the timbre to morph seamlessly from a standard, stable drumhead strike (high $J_0$) into a highly saturated, decaying quantum tunneling wave (high $K_0$) with sharp, explosive edge transients (high $Y_0$).
