# Computer Design Magazine (Issue #9 - October 1963) Architectural Analysis

This report reviews the core technical concepts of October 1963-era integrated circuits featured in **Computer Design Magazine (Vol. 2, No. 9)**, and documents the mapping of early monolithic silicon microcircuits to the **TSFi2** simulation pipeline.

---

## 1. Core Historical Features

### The Impact of Integrated Circuits (Fairchild Micrologic & Signetics Utilogic)
* **The Planar Epitaxial Process:** By late 1963, planar diffusion was used to integrate multiple transistors, diodes, and resistors onto a single monolithic silicon die.
* **Junction Isolation & Parasitic Effects:** Early ICs isolated components using back-to-back p-n junctions. This isolation introduced significant parasitic substrate capacitance ($C_{sub}$) and leakage currents ($I_{leak}$) to the substrate, which:
  1. Increased propagation delays ($t_{pd}$).
  2. Degraded noise margins under temperature variations.
  3. Created latch-up risks if parasitic four-layer (SCR) paths were triggered.

---

## 2. TSFi2 Integration Architecture

To simulate an early monolithic IC gate (Fairchild-style RTL NOR gate with parasitic substrate effects), we introduce:
1. **[tsfi_integrated_circuit.h](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/inc/tsfi_integrated_circuit.h):** Interface declaring the gate inputs, parasitic parameters, substrate potential, temperature, and update functions.
2. **[tsfi_integrated_circuit.c](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/src/tsfi_integrated_circuit.c):** Physics-based model of a monolithic RTL NOR gate factoring in temperature-dependent isolation junction leakage and parasitic capacitive charging delays.
3. **[test_integrated_circuit.c](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/tests/test_integrated_circuit.c):** Test suite verifying logic correctness, degradation of switching times under elevated temperature/parasitic loads, and zero memory leaks.

### Parasitic Propagation Delay Model
The charging current available to switch the gate is degraded by isolation junction leakage $I_{leak}$ and must charge the parasitic substrate capacitance $C_{sub}$:
$$I_{charge} = I_{drive} - I_{leak}(T)$$
$$V_{out}(t) = V_{out}(0) + \int \frac{I_{charge}}{C_{load} + C_{sub}} dt$$
Where:
* $I_{leak}(T) = I_0 \cdot 2^{(T - 25)/10}$ (doubles every $10^\circ\text{C}$).
* If $I_{charge} \le 0$, the gate fails to switch (thermal latch-up/stall).

---

## 3. Verification & Memory Sanitation
The integrated circuit simulator is verified under the standard memory check guidelines of `LauMemory`.
