# Computer Design Magazine (Issue #15 - June 1964) Architectural Analysis

This report reviews the core technical concepts of June 1964-era hardware engineering featured in **Computer Design Magazine (Vol. 3, No. 5)**, specifically focusing on **Plated-Wire Memory (Non-Destructive Readout - NDRO)**, and documents its simulation mapping in **TSFi2**.

---

## 1. Core Historical Features

### Plated-Wire Memory (NDRO)
* **High-Speed Thin-Film Storage:** Developed in the late 1950s/early 1960s and commercialized by 1964, plated-wire memory consists of a beryllium-copper wire electroplated with a thin layer of magnetic permalloy (nickel-iron).
* **Non-Destructive Readout (NDRO):** Unlike magnetic core memory (where reading a bit flips the core and requires a slow rewrite cycle), plated-wire memory utilizes magnetization vector rotation. Reading rotates the vector temporarily, inducing a sense signal, and then the vector returns to its easy axis automatically when the read field is removed.
* **Easy and Hard Axes:**
  * Magnetization normally lies along the **easy axis** (circumferential around the wire).
  * A word line (strap wrapped around the wire) applies a transverse field along the **hard axis** (longitudinal to the wire).

---

## 2. TSFi2 Integration Architecture

To simulate the electromagnetic and physical mechanics of Plated-Wire Memory cells (incorporating read/write fields, magnetization angle rotation, and induced sense voltage dynamics), we introduce:

1. **[tsfi_plated_wire.h](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/inc/tsfi_plated_wire.h):** Interface declaring the plated-wire memory cell structure, magnetic parameters, and read/write update functions.
2. **[tsfi_plated_wire.c](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/src/tsfi_plated_wire.c):** Physical model simulating coherent magnetization rotation under external fields and electromagnetic induction.
3. **[test_plated_wire.c](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/tests/test_plated_wire.c):** Test suite verifying NDRO read cycles (retaining states after read), write cycles (setting 0 and 1), and checking for zero memory leaks.

### Mathematical & Physical Model

#### 1. Magnetization Rotation (Stoner-Wohlfarth Model)
The magnetization vector angle $\theta$ (relative to the easy axis) under longitudinal field $H_L$ and transverse field $H_T$ is modeled by minimizing the magnetic energy:
$$E = K \sin^2(\theta) - M_s H_L \cos(\theta) - M_s H_T \sin(\theta)$$
Where:
* $K$ is the uniaxial anisotropy constant.
* $M_s$ is the saturation magnetization.
* Setting $\frac{\partial E}{\partial \theta} = 0$ yields the equilibrium angle.

#### 2. Induced Sense Voltage
The rotation of magnetization $\theta(t)$ changes the longitudinal flux, inducing a voltage in the central copper conductor:
$$V_{sense} = -\alpha \cdot M_s \cdot \frac{d(\cos \theta)}{dt}$$
* A positive $\theta$ rotation induces a positive voltage pulse if the initial state was '1' (clockwise magnetization).
* A negative voltage pulse is induced if the initial state was '0' (counter-clockwise).

---

## 3. Verification & Memory Sanitation
The plated-wire simulator is fully verified under the standard memory check guidelines of `LauMemory` with zero unsealed leaks.
