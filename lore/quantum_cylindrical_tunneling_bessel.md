# Quantum Tunneling through Cylindrical Barriers using Bessel Functions

This document explores the mathematical physics of quantum tunneling through a circular/cylindrical potential barrier, and how standard ($J_n$) and modified ($K_n$) Bessel functions can be synthesized as a dynamic wave-shaper modeling quantum well transitions.

---

## 1. Schrödinger Equation in Cylindrical Coordinates

For a quantum particle (like an electron) in a circular quantum dot of radius $R$ surrounded by a potential barrier of height $V_0$:
$$V(r) = \begin{cases} 0 & r < R \quad \text{(Classically Allowed Region I)} \\ V_0 & r \ge R \quad \text{(Classically Forbidden Region II)} \end{cases}$$

The time-independent Schrödinger equation in 2D cylindrical coordinates $(r, \theta)$ is:
$$-\frac{\hbar^2}{2m} \left( \frac{\partial^2 \psi}{\partial r^2} + \frac{1}{r} \frac{\partial \psi}{\partial r} + \frac{1}{r^2} \frac{\partial^2 \psi}{\partial \theta^2} \right) + V(r)\psi = E\psi$$

Using separation of variables $\psi(r, \theta) = R(r) \Theta(\theta)$ where $\Theta(\theta) = e^{im\theta}$, the radial equation becomes:
$$r^2 \frac{d^2 R}{dr^2} + r \frac{dR}{dr} + \left( k^2 r^2 - m^2 \right) R = 0$$

---

## 2. Solving with Bessel Functions

Depending on the energy state $E$ relative to the barrier height $V_0$, the radial solution splits into two classes:

### Region I: Classical Propagation ($E > 0$, inside the well $r < R$)
Since $V(r) = 0$, $k^2 = \frac{2mE}{\hbar^2} > 0$. The wave function oscillates:
$$R_{\text{in}}(r) = A \cdot J_m(k r)$$
where $J_m$ is the standard Bessel function of the first kind (representing standing waves inside the circular drumhead).

### Region II: Quantum Tunneling Decay ($E < V_0$, inside the barrier $r \ge R$)
Since $E < V_0$, the term $q^2 = \frac{2m(V_0 - E)}{\hbar^2} > 0$. The wave function cannot oscillate; it must decay exponentially. The solution is governed by the **Modified Bessel Function of the Second Kind ($K_m$)**:
$$R_{\text{out}}(r) = B \cdot K_m(q r)$$
$K_m(q r)$ decays exponentially as $r \to \infty$, modeling the decreasing probability density of the electron as it tunnels through the barrier.

---

## 3. Wave-shaper Synthesizer Implementation

We can model this boundary-matching quantum system as a **Cylindrical Tunneling Wave-shaper**:
1. **Classical Input ($x < \text{Threshold}$)**: The signal behaves classically, feeding into standard $J_0(x)$ metallic wave-shaping.
2. **Tunneling Leakage ($x \ge \text{Threshold}$)**: When the input signal level exceeds the barrier threshold (simulating an electron hitting the barrier wall), the output smoothly transitions from $J_0(x)$ to the modified decay curve $K_0(x)$.
3. **Acoustic Result**: Because the barrier transition is solved via boundary matching (requiring continuity of both the wave function $\psi$ and its derivative $\psi'$), the resulting soft-clipping is mathematically smooth and continuous, producing warm, organic saturation curves that mimic physical quantum well transitions.
