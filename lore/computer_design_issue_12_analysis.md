# Computer Design Magazine (Issue #12 - February/March 1964) Architectural Analysis

This report reviews the core technical concepts of February/March 1964-era hardware engineering featured in **Computer Design Magazine (Vol. 3, No. 2)**, and documents the mapping of the **Parametron** phase-locked subharmonic logic to the **TSFi2** simulation pipeline.

---

## 1. Core Historical Features

### The Parametron
* **Subharmonic Phase-Locked Logic:** Invented by Eiichi Goto in 1954 and widely used in early Japanese computers (e.g., PC-1, Musasino-1) into the early 1960s, the Parametron is a resonant circuit utilizing non-linear inductors (ferrite cores).
* **Operation & Excitation:**
  * The circuit is pumped by an excitation current at frequency $2f$ (typically $\sim 2\text{ MHz}$).
  * Through parametric resonance, subharmonic oscillations at frequency $f$ ($\sim 1\text{ MHz}$) are induced.
  * The resulting subharmonic signal has two stable phases, $0$ and $\pi$ (Logic '0' and '1').
* **Majority Logic Inputs:** Multiple weak input signals at frequency $f$ are fed to the circuit. During the rise of the pump excitation, the circuit locks its phase to the majority phase of the inputs.

---

## 2. TSFi2 Integration Architecture

To simulate a Parametron logic cell (factoring in parametric pumping, phase locking, and majority logic inputs), we introduce:
1. **[tsfi_parametron.h](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/inc/tsfi_parametron.h):** Interface declaring the resonant circuit states, excitation currents, inputs, and step functions.
2. **[tsfi_parametron.c](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/src/tsfi_parametron.c):** Numerical differential equation model of the subharmonic oscillator phase locking under parametric pumping.
3. **[test_parametron.c](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/tests/test_parametron.c):** Test suite verifying majority logic (AND/OR gates built from majority voters), phase locking, and zero memory leaks.

### Mathematical Oscillator Model
The subharmonic voltage $x(t)$ of the parametron is governed by a parametric Mathieu-type differential equation with non-linear damping:
$$\frac{d^2x}{dt^2} + \gamma(x) \frac{dx}{dt} + \omega_0^2 \left[1 - h \cos(2\omega_0 t)\right] x + \beta x^3 = F_{in}(t)$$
Where:
* $h$ is the pump excitation depth (at frequency $2\omega_0$).
* $\gamma(x) = \gamma_0 + \alpha x^2$ represents non-linear limiting.
* $F_{in}(t) = \sum A_i \cos(\omega_0 t + \theta_i)$ represents the sum of the input signals.
* When $h$ exceeds the threshold, the phase of $x(t)$ locks to $0$ or $\pi$ matching the sign of the majority input.

---

## 3. Verification & Memory Sanitation
The parametron simulator is verified under the standard memory check guidelines of `LauMemory`.
