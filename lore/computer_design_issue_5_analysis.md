# Computer Design Magazine (Issue #5 - April/May 1963) Architectural Analysis

This report reviews the core technical concepts of April/May 1963-era hardware engineering featured in **Computer Design Magazine (Vol. 2, No. 4/5)**, and documents the mapping of these systems to the **TSFi2** digital simulation pipeline.

---

## 1. Core Historical Features

### Ferrite Core Memory Planes
* **The Stacking Paradigm:** Rather than single discrete memory cells, 1963 systems organized ferrite cores into 2D planar matrices (e.g., $8 \times 8$).
* **Wiring Geometry:**
  * **X & Y Selection Wires:** Run horizontally and vertically, carrying half-select currents ($I_x, I_y$).
  * **Inhibit Wire:** Threaded through all cores in a plane to block writing a '1' during write cycles if necessary.
  * **Sense Wire:** A diagonal wire threading through all cores in the plane to detect state changes.
* **Destructive Read-Out:** Reading a core is done by forcing a write-0 current. If the core held a '1', it flips to '0', creating a magnetic flux change ($\Delta \Phi$) that induces a voltage pulse on the shared Sense Wire. If it held '0', no flip occurs, and the Sense Wire remains silent.

---

## 2. TSFi2 Integration Architecture

To emulate the 2D coincident-current memory plane, we introduce:
1. **[tsfi_core_plane.h](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/inc/tsfi_core_plane.h):** Interface declaring the memory matrix grid size, X-Y currents, and sense signal detection.
2. **[tsfi_core_plane.c](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/src/tsfi_core_plane.c):** Numerical simulation of 2D grid updates, half-current disturbs, and inductive sense responses.
3. **[test_core_plane.c](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/tests/test_core_plane.c):** Verification test suite mapping coordinate writes to destructive readout detections.

### Mathematical Sense Pulse Equation
The induced voltage $V_{sense}$ on the sense wire is proportional to the rate of change of total magnetic flux density $B$:
$$V_{sense} = -\sum_{x,y} \frac{dB_{x,y}}{dt}$$
Specifically, if core $(x,y)$ flips state from $+1.0 \to -1.0$, the discrete flux change is $\Delta B = -2.0$, generating a sharp positive sense spike.

---

## 3. Verification & Memory Sanitation
The core plane simulator is verified under the standard memory tracking suite of `LauMemory`.
