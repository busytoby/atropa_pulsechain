# Verlet Synthesis Plan: Physical Modeling & Ordered Oscillators

This document details the mathematical framework and implementation structure for Verlet-integrated physical modeling and ordered non-linear oscillators within the speech synthesis environment.

---

## 1. Mass-Spring Vocal Fold Simulation (Model 1)

### Mathematical Formulation
The vocal folds are modeled as a 1D coupled mass-spring-damper system driven by subglottal pressure $P_s$. The displacement $x$ of the vocal fold mass $m$ represents opening size.

$$\ddot{x} + 2\gamma\omega_0\dot{x} + \omega_0^2 x = \frac{F_{\text{aerodynamic}}(x)}{m}$$

In Verlet integrated form, with damping represented using backward difference velocity approximation $v_t \approx \frac{x_t - x_{t-\Delta t}}{\Delta t}$:

$$x_{t+\Delta t} = 2x_t - x_{t-\Delta t} + \left( \frac{F_{\text{aero}}(x_t) - k x_t - c \frac{x_t - x_{t-\Delta t}}{\Delta t}}{m} \right) \Delta t^2$$

Where:
*   $m$: Mass coefficient.
*   $k$: Spring stiffness.
*   $c$: Damping coefficient.
*   $F_{\text{aero}}(x)$: Aerodynamic force proportional to subglottal pressure $P_s$ and bernoulli negative pressure during closure:
    $$F_{\text{aero}}(x) = P_s \cdot A_{\text{fold}} \cdot \Theta(x)$$

This yields natural glottal flow pulses $U_g \propto x^2 \sqrt{P_s}$ for speech excitation.

---

## 2. Ordered Duffing Oscillator (Model 2)

### Mathematical Formulation
To act as a stable, periodic generator instead of a chaotic source, the Duffing oscillator is constrained to its stable limit cycle phase space by tuning the damping $\delta$, linear stiffness $\beta$, non-linear cubic parameter $\alpha$, and driving force parameters:

$$\ddot{y} + \delta\dot{y} + \beta y + \alpha y^3 = \gamma \cos(\omega t)$$

Verlet integration steps:

$$y_{t+\Delta t} = 2y_t - y_{t-\Delta t} + \left( -\beta y_t - \alpha y_t^3 - \delta \frac{y_t - y_{t-\Delta t}}{\Delta t} + \gamma \cos(\omega t) \right) \Delta t^2$$

### Parametric Bounds for Periodic Regimes (Non-Chaotic)
To guarantee clean, harmonic oscillations:
*   $\alpha \ge 0$ (Hardening spring regime).
*   $\delta \ge 0.15$ (Sufficient damping to suppress chaotic attractor bifurcations).
*   $\gamma \le 0.35$ (Controlled driving amplitude to stay within periodic orbit boundaries).
