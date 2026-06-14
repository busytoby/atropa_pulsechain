# Modeling Bessel Families (Jn, In, Kn, Yn) with DUG and DUS Conduction Curves

This document provides a physical-modeling mapping that translates the mathematical behaviors of the four Bessel families ($J_n, I_n, K_n, Y_n$) into the electrical conduction characteristics of Germanium (**DUG**) and Silicon (**DUS**) diodes.

---

## 1. DUG (Germanium Diode) Mapping: Soft-Saturation (In, Kn)

A universal Germanium diode (**DUG**) has a low forward-voltage threshold ($V_f \approx 0.15\text{V}$) and a gradual, soft exponential conduction knee. 
* **The Mathematical Equivalent**: **Modified Bessel Functions $I_n(x)$ and $K_n(x)$**.
* **Physical Modeling Alignment**:
  * The exponential growth of $I_n(x)$ and decay of $K_n(x)$ match the charge-carrier diffusion profile across the Germanium lattice.
  * When a signal is routed through the $(I_n, K_n)$ block, it experiences soft-clipping and compression, mimicking the warm, second-harmonic warmth of a point-contact Germanium DUG diode.

---

## 2. DUS (Silicon Diode) Mapping: Sharp-Clipping (Jn, Yn)

A universal Silicon diode (**DUS**) has a high forward-voltage threshold ($V_f \approx 0.6\text{V}$) and a sharp, steep conduction curve, acting as a virtual open circuit until the threshold is crossed.
* **The Mathematical Equivalent**: **Standard Bessel Functions $J_n(x)$ and $Y_n(x)$**.
* **Physical Modeling Alignment**:
  * **Discontinuity ($Y_n$)**: The singularity of the Weber function $Y_n(x)$ at $x \to 0$ models the sharp threshold-crossing barrier (the sudden jump from non-conduction to conduction).
  * **Oscillatory Clipping ($J_n$)**: The rapid, non-harmonic zero-crossings of $J_n(x)$ model the harsh, high-frequency odd-harmonic ripple that occurs when a Silicon diode array clips a wave sharply.
  * Routing a signal through the $(J_n, Y_n)$ block produces hard-clipping distortion with bright, metallic, sibilant harmonics.

---

## 3. The Unified Diode Wave-shaper Matrix

In the synthesizer's **Interpolation** panel, we map the diode selector to the Bessel parameter mix:

```
                  Input Signal (x)
                         |
           +-------------+-------------+
           |                           |
     [ DUG Selector ]            [ DUS Selector ]
     Soft Germanium              Sharp Silicon
   (In, Kn Modulators)        (Jn, Yn Modulators)
```
* **DUG Mode**: Smooth, warm saturation using hyperbolic modified Bessel curves.
* **DUS Mode**: Hard, bright clipping using oscillating and singular Bessel curves.
