# Computer Design Magazine (Issue #18 - September 1964) Architectural Analysis

This report reviews the core technical concepts of September 1964-era digital signal conditioning featured in **Computer Design Magazine (Vol. 3, No. 9)**, specifically focusing on **Discrete Transistor Emitter-Coupled Schmitt Triggers**, and documents its simulation mapping in **TSFi2**.

---

## 1. Core Historical Features

### Emitter-Coupled Schmitt Triggers
* **Signal Conditioning in 1964:** Early digital computers had to read inputs from slow-moving or noisy mechanical and magnetic peripherals (e.g., photoelectric paper tape readers, magnetic drum read heads). Slow rise times and noise would cause double-triggering or jitter in standard logic gates.
* **Hysteresis Loop:** Implementing a Schmitt trigger using emitter-coupled transistors provides two distinct threshold voltages:
  * **Upper Threshold Point (UTP):** The input voltage at which the output switches state on a rising signal.
  * **Lower Threshold Point (LTP):** The input voltage at which the output switches back on a falling signal.
* **Hysteresis Voltage ($V_{h}$):** $V_{h} = V_{utp} - V_{ltp}$. By matching the hysteresis margin to exceed the peak noise amplitude, designers completely eliminate false noise-triggered state transitions.

---

## 2. TSFi2 Integration Architecture

To model the physical regenerative switching behavior, hysteresis margins, and noise rejection of a classic emitter-coupled Schmitt trigger, we introduce:

1. **[tsfi_schmitt.h](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/inc/tsfi_schmitt.h):** Interface declaring the Schmitt trigger components (resistors, transistor parameters) and update step functions.
2. **[tsfi_schmitt.c](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/src/tsfi_schmitt.c):** Physical model simulating emitter current sharing, transistor switching, and output voltage propagation.
3. **[test_schmitt.c](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/tests/test_schmitt.c):** Test suite verifying UTP/LTP thresholds and demonstrating successful noise rejection on a slow, noisy analog input compared to a simple non-hysteresis comparator.

### Mathematical & Physical Model

#### 1. Common Emitter Current Sharing
The emitters of transistors $Q_1$ and $Q_2$ are tied to a common resistor $R_e$. Emitter voltage $V_e$ is determined by the active currents:
$$V_e = (I_{e1} + I_{e2}) \cdot R_e$$
Where:
* $I_{e1} \approx I_{c1} = \frac{V_{cc} - V_{c1}}{R_{c1}}$ (when $Q_1$ is ON).
* $I_{e2} \approx I_{c2} = \frac{V_{cc} - V_{out}}{R_{c2}}$ (when $Q_2$ is ON).

#### 2. Regenerative Threshold Switch
* **UTP Threshold:** When $Q_1$ is OFF and $Q_2$ is ON, $V_e$ is at its maximum because $R_{c2}$ is smaller than $R_{c1} + R_1$. The input must rise to:
  $$V_{utp} \approx V_{e,max} + V_{be,on}$$
* **LTP Threshold:** Once $Q_1$ turns ON, it draws current away from $Q_2$, turning $Q_2$ OFF. The emitter voltage drops to $V_{e,min}$. The input must drop to:
  $$V_{ltp} \approx V_{e,min} + V_{be,on}$$

---

## 3. Verification & Memory Sanitation
The Schmitt trigger simulator is fully verified under the standard memory check guidelines of `LauMemory` with zero unsealed leaks.
