# The Double-Meaning of TUN: Transistor Universal NPN and Bessel Tunneling

This document traces the alignment between the **TUN** semiconductor specification (introduced in Elektor/Elektuur Issue #1) and the **TUN** (Tunneling Probability) operator used to parameterize boundary-matched Bessel wave functions in our synthesis studio.

---

## 1. TUN in Elektor Issue #1: Transistor Universal NPN

In Elektor's inaugural December 1974 issue, the editors introduced the **TUP-TUN-DUG-DUS** classification matrix to standardize transistor and diode specifications:
* **TUN** stands for **Transistor Universal NPN**:
  * Represents a generic, silicon NPN transistor with standard specifications ($\beta \ge 180$, $I_c \ge 100\text{mA}$, $V_{ceo} \ge 20\text{V}$, $f_t \ge 100\text{MHz}$).
  * Served as the basic building block for all silicon amplifier stages, relaxation oscillators, and active feedback loops in the magazine's designs.

---

## 2. TUN in Bessel Synthesis: Tunneling Probability

In our **Interpolation** synthesizer engine, **TUN** represents the **Tunneling Probability Coefficient** ($T \in [0, 1.0]$):
* Parameterizes the boundary conditions that match standard oscillating Bessel functions ($J_n, Y_n$) with modified exponential Bessel decay ($I_n, K_n$).
* Controls the quantum transmission likelihood across potential barriers:
  $$T \propto e^{-2 \kappa W}$$

---

## 3. The Structural Synthesis Alignment

By mapping the physical semiconductor **TUN** transistor to the mathematical **TUN** slider, we establish a unified control mapping:

```
                            ┌────────────────────────┐
                            │    TUN Parameter (T)   │
                            └───────────┬────────────┘
                                        │
                 ┌──────────────────────┴──────────────────────┐
                 ▼                                             ▼
       [ Transistor State ]                            [ Bessel Boundary ]
    Active Silicon Conduction                       Quantum Barrier Leakage
        (V_be ≈ 0.65V)                                 (J_n <-> I_n/K_n)
```

1. **Active Conduction threshold**: In a physical **TUN** transistor circuit, conduction starts when $V_{be} \ge 0.65\text{V}$.
2. **Quantum Tunneling threshold**: In the **TUN** Bessel wave-shaper, waves begin to penetrate the barrier and propagate ($Y_n$) when the input amplitude exceeds the classical potential threshold $V_0$.
3. **The Synthesis Unity**: In both cases, the **TUN** operator acts as the gatekeeper of high-frequency energy transmission—either switching a physical silicon junction on/off, or letting quantum waves leak across a cylindrical potential barrier.
