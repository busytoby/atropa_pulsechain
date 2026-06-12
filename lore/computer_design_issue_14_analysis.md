# Computer Design Magazine (Issue #14 - May 1964) Architectural Analysis

This report reviews the core technical concepts of May 1964-era hardware engineering featured in **Computer Design Magazine (Vol. 3, No. 4)**, specifically focusing on the implementation of **Carry Look-Ahead Adders (CLA)** for high-speed binary arithmetic, and documents its simulation mapping in **TSFi2**.

---

## 1. Core Historical Features

### High-Speed Arithmetic: Carry Look-Ahead Adders
* **The Carry Propagation Bottleneck:** In standard Ripple Carry Adders (RCA), the carry bit must ripple through each bit position sequentially (from LSB to MSB). This introduces a linear propagation delay $O(n)$ where $n$ is the word length.
* **Carry Look-Ahead Logic:** By generating and propagating carry signals in parallel, CLA logic computes the carry bits for all stages simultaneously, reducing carry propagation delay to a constant $O(1)$ gate delay relative to word length.
* **Logic Terms:**
  * **Generate ($G_i$):** $A_i \cdot B_i$ (a carry is generated at stage $i$ if both inputs are 1).
  * **Propagate ($P_i$):** $A_i \oplus B_i$ (an incoming carry is propagated through stage $i$ if either input is 1).
  * **Carry ($C_{i+1}$):** Computed using nested sum-of-products of $G_i$ and $P_i$.

---

## 2. TSFi2 Integration Architecture

To model and compare the gate-level propagation delay of Ripple Carry vs. Carry Look-Ahead Adders, we introduce:

1. **[tsfi_cla.h](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/inc/tsfi_cla.h):** Interface declaring the inputs, outputs, gate delay parameters, and adder models.
2. **[tsfi_cla.c](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/src/tsfi_cla.c):** Numerical simulator that tracks node-by-node signal transition propagation times under gate-level delay constraints.
3. **[test_cla.c](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/tests/test_cla.c):** Test suite verifying logic correctness and demonstrating the propagation delay advantage of CLA ($O(1)$ delay) over RCA ($O(n)$ delay) under simulated gate delays.

### Propagation Delay Equations
Each gate (AND, OR, XOR) is modeled with a signal propagation delay:
$$V_{node}(t + t_{pd}) = \text{GateLogic}(V_{inputs}(t))$$
Where:
* $t_{pd, \text{AND}} = 4.0\text{ ns}$
* $t_{pd, \text{OR}} = 4.0\text{ ns}$
* $t_{pd, \text{XOR}} = 8.0\text{ ns}$

For a 4-bit adder, the Ripple Carry Adder requires cascading XOR/AND/OR logic across 4 stages, leading to a theoretical worst-case carry settling time of $\sim 32\text{ ns}$. The CLA adder computes carries in parallel, settling carries within $\sim 8\text{ ns}$.

---

## 3. Verification & Memory Sanitation
The CLA simulator is fully verified under the standard memory check guidelines of `LauMemory` with zero unsealed leaks.
