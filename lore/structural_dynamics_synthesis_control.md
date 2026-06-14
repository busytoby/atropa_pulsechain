# Implementing Structural Dynamics in Synthesizer Control Loops

This document details how we apply second-order structural dynamics (specifically, William Prager’s mass-spring-damper formulation) to optimize the active feedback loops, envelope detectors, and Q-damping protection networks in the synthesis studio.

---

## 1. Equivalency: Mechanical Structure vs. Electronic Control Loop

There is a direct mathematical isomorphism between a mechanical vibrating structure and an electronic resonant filter:

| Mechanical Property | Electrical Equivalent (Rlc Filter) | Control System Equivalent (Envelope / Damping Loop) |
| :--- | :--- | :--- |
| **Mass ($M$)** | Inductance ($L$) | Attack Time / Inertia Coefficient |
| **Damping ($C$)** | Resistance ($R$) | Decay Rate / Feedback Loop Gain |
| **Stiffness ($K$)** | Reciprocal Capacitance ($1/C$) | Cutoff Frequency / Target Setpoint Stiffness |
| **Forcing Function ($F$)** | Input Voltage ($V_{\text{in}}$) | Input Amplitude / Error Signal |

By treating the filter's control parameters (like Q-damping coefficients) as dynamic mechanical components, we can design control loops that react smoothly without sharp digital overshoots or metallic ringing.

---

## 2. Dynamic Q-Damping as a Viscous Damper

In our **Formant Filter**, high-Q settings cause resonant peaks that can lead to clipping. We model the auto-damping control system as a variable **Viscous Damper** ($C(t)$) in a mechanical suspension:
* **The Normal State**: Damping $C(t)$ is low, allowing the system to oscillate freely (high-Q resonance).
* **The Overload State**: When the input acceleration (slew rate of the signal) exceeds a threshold, the damper stiffness is instantly increased.
* **The Update Equation**: The dynamic resistance parameter $R_{\text{damping}}$ is adjusted using the discrete second-order update:
  $$R_{\text{damping}}(t) = 2(1 - \alpha) R_{\text{damping}}(t-1) - (1 - 2\alpha) R_{\text{damping}}(t-2) + G \cdot e_{\text{clipping}}(t)$$
  where $\alpha$ represents the damping inertia (mass equivalent) and $G$ is the loop gain.

By utilizing this second-order update, the damping control voltage converges on its target level along a critically damped trajectory, eliminating the harsh, unnatural "step" artifacts of first-order digital clamps.
