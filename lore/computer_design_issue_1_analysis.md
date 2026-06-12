# Computer Design Magazine (Inaugural Issue - 1962) Architectural Analysis

This report reviews the core technical concepts of 1962-era hardware engineering featured in the inaugural issue of **Computer Design Magazine (1962)**, and documents the mapping of these systems to the **TSFi2** digital simulation pipeline.

---

## 1. Core Historical Features

### Diode-Transistor Logic (DTL)
* **Logic Paradigm:** The prevalent logic family preceding TTL. It uses input diodes to perform logical AND/OR gating, followed by a bipolar junction transistor (BJT) in common-emitter configuration for inversion (NAND/NOR) and gain amplification.
* **Level-Shifting Diodes:** Inserted between the input diode node and the transistor base to raise the input threshold voltage, improving noise immunity.
* **DTL NAND Circuit Schematic:**
  ```
  Vcc (+5V)
    |
   [Rd]
    |-----+------> Level-Shifting Diodes ------> Base of BJT (Inverter)
    |     |                                           |
   D_A   D_B                                         [R_base]
    |     |                                           |
   InA   InB                                         GND
  ```

### Coincident-Current Magnetic Core Memory
* **The Hysteresis Core:** Uses tiny toroidal ferrite cores with a square hysteresis loop (B-H curve) providing non-volatile binary states $+B_r$ (1) and $-B_r$ (0).
* **Coincident Selection:** Selection of a specific core is achieved by sending half the required switching current ($I_x = I_y = I_m / 2$) down the X and Y lines. Only the core at the intersection receives the full current $I_m > I_c$ (coercive threshold) and flips state.

---

## 2. TSFi2 Integration Architecture

To emulate DTL gates and Magnetic Core memory, we introduce:
1. **[tsfi_dtl_core.h](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/inc/tsfi_dtl_core.h):** Interface declaring DTL gates and Core state structure.
2. **[tsfi_dtl_core.c](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/src/tsfi_dtl_core.c):** Physics-based simulation of DTL gate transfer functions and core B-H hysteresis trajectories.
3. **[test_dtl_core.c](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/tests/test_dtl_core.c):** Verification test suite mapping inputs to saturated logic states and core memory read/write cycles.

---

## 3. Emulated Physical Models

### DTL Gate Voltage Transfer Function
Let input diodes have forward voltage drop $V_d \approx 0.7\text{V}$, and base level-shifters have drop $V_{shift} \approx 1.4\text{V}$.
* **Transistor cutoff/saturation boundary:**
  The transistor turns on when base node voltage $V_b \approx 0.7\text{V}$.
  The input node voltage threshold is:
  $$V_{thresh} = V_{b,on} + V_{shift} - V_d = 0.7\text{V} + 1.4\text{V} - 0.7\text{V} = 1.4\text{V}$$
* **State Equations:**
  For $V_{in} < V_{thresh}$, transistor is in cutoff: $V_{out} \approx V_{cc} = 5.0\text{V}$.
  For $V_{in} \ge V_{thresh}$, transistor saturates: $V_{out} \approx V_{ce,sat} = 0.2\text{V}$.

### Core Memory Hysteresis (B-H Curve Approximation)
We model the square hysteresis trajectory for magnetic flux density $B(H)$ based on current field strength $H$:
$$B(H) = \text{clamp}\left(B_{sat} \cdot \tanh\left(\frac{H - H_c \cdot \text{dir}}{W}\right), -B_{sat}, B_{sat}\right)$$
where $\text{dir} \in \{-1, +1\}$ tracks the direction of the magnetic field sweep to lock the remanent history ($+B_r$ or $-B_r$).

---

## 4. Verification & Memory Sanitation
The simulations are verified under the standard memory checks in the `LauMemory` suite.
