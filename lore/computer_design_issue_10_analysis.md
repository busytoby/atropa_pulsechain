# Computer Design Magazine (Issue #10 - November/December 1963) Architectural Analysis

This report reviews the core technical concepts of November/December 1963-era hardware engineering featured in **Computer Design Magazine (Vol. 2, No. 10)**, and documents the mapping of **Core-Rope Read-Only Memory (ROM)** to the **TSFi2** simulation pipeline.

---

## 1. Core Historical Features

### Core-Rope Memory (ROM)
* **High-Density Storage:** Unlike coincident-current read/write core memory (where each core stores one bit), core-rope memory stores an entire digital word per core. It was famously used in the Apollo Guidance Computer (AGC) designed around this period.
* **Structural Wiring Rules:**
  * To store a **'1'**, a sense wire (bit line) is threaded **through** the selected core.
  * To store a **'0'**, the sense wire bypasses (threads **around**) the core.
* **Operation:** Pulsing the set current through a selected core induces a current pulse in all sense wires threading *through* it due to electromagnetic induction, while wires passing *around* it remain silent.

---

## 2. TSFi2 Integration Architecture

To simulate a Core-Rope Memory plane (e.g., a 16-word ROM matrix), we introduce:
1. **[tsfi_core_rope.h](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/inc/tsfi_core_rope.h):** Interface declaring the core plane structure, rope matrix mappings, and read methods.
2. **[tsfi_core_rope.c](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/src/tsfi_core_rope.c):** Numerical simulation of induced magnetic flux linkage and sense voltage peaks during core switching.
3. **[test_core_rope.c](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/tests/test_core_rope.c):** Test suite verifying ROM contents, flux-linkage transitions, half-select margins, and clean memory.

### Physics-Based Induction Model
When a select current $I_{sel}$ transitions, the change in core flux $\frac{d\Phi}{dt}$ induces a EMF voltage $V_{sense}$ on the $k$-th sense line:
$$V_{sense, k} = -N_k \cdot \frac{d\Phi}{dt} \cdot \kappa_k$$
Where:
* $\kappa_k = 1.0$ if the sense wire threads **through** the core.
* $\kappa_k = 0.0$ if the sense wire passes **around** the core.
* $\frac{d\Phi}{dt}$ follows a Gaussian switching transient peak: $\approx V_{peak} \cdot e^{-\frac{(t - t_{peak})^2}{2\sigma^2}}$.

---

## 3. Verification & Memory Sanitation
The core-rope simulator is verified under the standard memory check guidelines of `LauMemory`.
