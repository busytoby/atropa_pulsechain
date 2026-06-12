# Computer Design Magazine (Issue #7 - June/July 1963) Architectural Analysis

This report reviews the core technical concepts of June/July 1963-era hardware engineering featured in **Computer Design Magazine (Vol. 2, No. 6/7)**, and documents the mapping of these systems to the **TSFi2** digital simulation pipeline.

---

## 1. Core Historical Features

### Leo Esaki's Tunnel Diode Logic & Memory
* **The High-Speed Frontier:** In 1963, standard silicon transistors were speed-limited. Tunnel diodes (relying on quantum tunneling) offered nanosecond switching speeds, making them ideal for high-speed logic and registers.
* **Negative Differential Resistance (NDR):** The hallmark of the tunnel diode. Its I-V curve rises to a peak current $I_p$, drops to a valley current $I_v$, and rises again.
* **Bistable Latch Operation:**
  By biasing the tunnel diode with a constant current $I_{bias}$ between $I_v$ and $I_p$, the load line intersects the I-V curve at two stable operating points:
  * **Low-Voltage State (Logic '0'):** $V \approx 0.1\text{V}$ (before the peak).
  * **High-Voltage State (Logic '1'):** $V \approx 0.5\text{V}$ (after the valley).
* **Switching Mechanics:**
  * A positive current pulse pushes the total current above $I_p$, forcing a transition to the high-voltage state.
  * A negative current pulse drops the total current below $I_v$, forcing a transition back to the low-voltage state.

---

## 2. TSFi2 Integration Architecture

To emulate the Tunnel Diode bistable latch, we introduce:
1. **[tsfi_tunnel_latch.h](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/inc/tsfi_tunnel_latch.h):** Interface declaring peak/valley currents and bias configuration.
2. **[tsfi_tunnel_latch.c](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/src/tsfi_tunnel_latch.c):** Numerical simulation of NDR switching dynamics.
3. **[test_tunnel_latch.c](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/tests/test_tunnel_latch.c):** Verification test suite mapping current pulses to bistable state transitions.

### Mathematical State Machine
Let $I_p = 1.0\text{ mA}$ and $I_v = 0.2\text{ mA}$, with bias current $I_{bias} = 0.6\text{ mA}$.
* **State Update:**
  Given input trigger current $I_{trig}(t)$:
  $$I_{\text{total}}(t) = I_{bias} + I_{trig}(t)$$
  $$\text{State}(t) = \begin{cases} 1 & \text{if } I_{\text{total}}(t) > I_p \\ 0 & \text{if } I_{\text{total}}(t) < I_v \\ \text{State}(t-\Delta t) & \text{otherwise} \end{cases}$$
* **Output Voltage:**
  $$V(t) = \begin{cases} 0.5\text{V} & \text{if } \text{State}(t) == 1 \\ 0.1\text{V} & \text{if } \text{State}(t) == 0 \end{cases}$$

---

## 3. Verification & Memory Sanitation
The tunnel diode simulator is verified under the standard memory check guidelines of `LauMemory`.
