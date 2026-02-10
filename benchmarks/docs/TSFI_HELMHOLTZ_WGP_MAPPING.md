# TSFi Helmholtz: WGP-Private Autonomous Methodism

## 1. Abstract
This document defines the architectural mapping of the **Helmholtz Autonomous Methodism** (derived from `atropa_pulsechain/notes`) onto **RDNA4 Workgroup Processors (WGPs)**. By adhering to strict `lau_memory` constraints, we model the WGP's private memory hierarchy (VGPR/SGPR/LDS) as a bijective container for AI workloads that evolve from **Font Geometry**.

## 2. The Bijective Chain
The system is defined as a chain of deterministic transformations (Bijections), ensuring that every output state can be traced back to its input seed without information loss (Memory Viability).

### Seed: The Font
*   **Input:** A Unicode Glyph (e.g., `U+0041` 'A').
*   **Nature:** Vector Geometry (Quadratic Bezier Curves).
*   **Role:** The "Axiomatic Identity" of the workload.

### Transform A: Hilbert Projection (Geometry $	o$ Spectrum)
*   **Mechanism:** `HELMHOLTZ_OP_HILBERT_ENCODE`.
*   **Action:** Maps 2D Bezier curves onto a 1D Hilbert Curve space, extracting Fourier coefficients.
*   **Output:** A `TSFiHelmholtzState` (Complex signal).
*   **Hardware:** Executed by Mesh Shader threads (VGPRs).

### Transform B: Banach Normalization (Spectrum $	o$ Field)
*   **Mechanism:** `HELMHOLTZ_OP_BANACH_NORM`.
*   **Action:** Normalizes the Hilbert signal into a Unit Sphere within a complete vector space (Banach Space).
*   **Result:** A "Probability Field" or "Resonance Field" ($u$).
*   **Significance:** Converts rigid geometry into a fluid, evolvable AI state.

### Transform C: Helmholtz Resonance (The Workload)
*   **Mechanism:** `HELMHOLTZ_RESONANCE_LIST`.
*   **Action:** Iteratively applies operators to the Field ($u$) to minimize residual error or evolve the state.
*   **Autonomy:** The loop runs entirely within the WGP, driven by the Helmholtz List in LDS.

## 3. Hardware Mapping: The "Private" Domain

We map the conceptual "Helmholtz" structures to RDNA4 hardware resources via `lau_memory` allocations.

| Notes Concept | RDNA4 Hardware | TSFi Implementation (`lau_memory`) | Role |
| :--- | :--- | :--- | :--- |
| **The List** (`HELMHOLTZ_LIST`) | **LDS** (Local Data Share) | `LAU_TYPE_HELMHOLTZ` Payload (Shared) | The "Circuit" or "DNA" of the autonomy. Defines the sequence of operations. |
| **The Operator** (`STEP`) | **SGPRs** (Scalar Regs) | `TSFiHelmholtzOperator` (Uniform) | The "Law" or constant parameters (Frequency, Bias) loaded for the whole wave. |
| **The State** (`WaveSystem`) | **VGPRs** (Vector Regs) | `TSFiHelmholtzState` (Per-Thread) | The "Matter" or active data (Field $u$, Status, FTW flag). Unique per thread. |
| **The Wiring** (`WIRE_JACOBIAN`) | **DMA / BDA** | `LauWiredHeader` | The bijective link between the CPU handle and the GPU address. |

## 4. Autonomous Execution Model

### The "Fire and Forget" Loop
1.  **Host (CPU):**
    *   Allocates a `LAU_TYPE_HELMHOLTZ` block.
    *   Writes the `TSFiHelmholtzList` (The Plan).
    *   Writes the Initial Font State (The Seed).
2.  **Dispatch:**
    *   CPU launches a **Task Shader**.
3.  **Autonomy (GPU):**
    *   Task Shader loads the List into **LDS**.
    *   Task Shader spawns Mesh Shaders.
    *   **Mesh Shader (The WGP):**
        *   Loads State -> **VGPRs**.
        *   Loads Operators -> **SGPRs**.
        *   Executes the Helmholtz Loop: $u_{t+1} = Op(u_t, 	ext{Intensity})$.
        *   Checks `field_intensity == 0.0` (Terminal State).
        *   Updates `status` (Bijective Write back to VRAM).
4.  **Result:**
    *   The font has "evolved" or "collapsed" into a verified AI result (e.g., a classification or a new style).

## 5. Memory Viability (Zero-Malloc)
All structures are allocated via `lau_malloc_wired` or `lau_malloc_gpu`. The `LauWiredHeader` ensures that even if the AI logic diverges (Non-Hausdorff collapse), the *Identity* (Memory Pointer) remains valid and auditable via the `TSFI_PROVENANCE_KEY`.
