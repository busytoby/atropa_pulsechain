# Computer Design Magazine (Issue #19 - October 1964) Architectural Analysis

This report reviews the core technical concepts of October 1964-era digital memory addressing featured in **Computer Design Magazine (Vol. 3, No. 10)**, specifically focusing on **Diode Matrix Address Decoders**, and documents its simulation mapping in **TSFi2**.

---

## 1. Core Historical Features

### Diode Matrix Decoders
* **Cost-Effective Memory Addressing:** In early coincident-current core memory systems, address decoders were required to translate binary addresses (e.g., 3 bits) into unique select lines (e.g., 8 lines) to drive the core plane's X and Y windings. Because transistors were expensive in 1964, designers used large grids of semiconductor diodes (a "diode matrix") to perform the AND-logic decoding.
* **Electrical Behavior:**
  * Output lines are pulled up to $+V_{cc}$ via resistors.
  * Diode anodes connect to the output lines, and cathodes connect to the address lines.
  * If any input address line is LOW, it conducts current, pulling the output line LOW.
  * Only when all address lines connected to a select line are HIGH, the diodes are reverse-biased, allowing the output line to rise to $+V_{cc}$ (Logic '1').
* **Historical Non-Idealities:**
  * **Diode Forward Voltage Drop ($V_f$):** Non-zero $V_f$ prevents unselected lines from dropping to $0\text{V}$ (they stay at $V_{in} + V_f \approx 0.9\text{V}$), reducing the noise margin of the subsequent core driver transistors.
  * **Reverse Leakage Current ($I_r$):** Reverse leakage current through multiple diodes degrades the HIGH output voltage level.

---

## 2. TSFi2 Integration Architecture

To model the physical and circuit behavior of a 3-to-8 Diode Matrix Decoder (incorporating diode junction capacitances, forward drops, and temperature-dependent reverse leakage currents), we introduce:

1. **[tsfi_diode_matrix.h](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi_diode_matrix.h):** Interface declaring the decoder structure, diode electrical specifications, and step functions.
2. **[tsfi_diode_matrix.c](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi_diode_matrix.c):** Physical circuit model calculating output voltages under pull-up current, diode conduction, and leakage.
3. **[test_diode_matrix.c](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi_diode_matrix.c):** Test suite verifying binary address decoding, noise margins under elevated temperature, and checking for zero memory leaks.

### Physics-Based Circuit Model

#### 1. Output Select Line Voltage
For the $j$-th select line, the node voltage $V_j$ is determined by the pull-up current through $R_{pullup}$ and the current through the $n$ connected diodes:
$$I_{pullup} = \frac{V_{cc} - V_j}{R_{pullup}}$$
$$V_j = V_{cc} - R_{pullup} \cdot \left( \sum_{i \in \text{LOW}} I_{diode, i} - \sum_{k \in \text{HIGH}} I_{leak, k}(T) \right)$$
Where:
* $I_{diode, i}$ is the forward current of conducting diodes: $\approx \frac{V_j - (V_{in, i} + V_f)}{R_{diode}}$.
* $I_{leak}(T) = I_{leak,0} \cdot e^{\gamma \cdot (T - 25)}$ (doubles every $10^\circ\text{C}$ for typical semiconductor junctions).

---

## 3. Verification & Memory Sanitation
The diode matrix simulator is fully verified under the standard memory check guidelines of `LauMemory` with zero unsealed leaks.
