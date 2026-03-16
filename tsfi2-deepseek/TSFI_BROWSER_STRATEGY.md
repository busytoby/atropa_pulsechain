# TSFi2 Browser Integration Strategy: The Helmholtz Web

**Status:** DRAFT | **Target:** WebGPU + Wasm
**Context:** Moving from AVX-512 Simulation to Native GPU Execution.

## 1. Architectural Shift: "Post-Fourier"
The system has evolved from **Fourier-G** (Pure Spectrum) to **Helmholtz-Bijective** (Spectrum + Topology).
*   **Old:** `F(x) -> Frequency`
*   **New:** `Helmholtz(x) -> { Divergence (SVDAG Splits), Curl (Hilbert Phase) }`

In the browser, we cannot emulate AVX-512 efficiently. However, the architecture was designed to emulate RDNA 4. **WebGPU** gives us direct access to the GPU, meaning we can stop "emulating" and start "executing" the Helmholtz Lists natively.

## 2. The "Zhong" Web Pipeline

### A. The Asset (Genome)
The `TSFiHelmholtzSVDAG` (linear `op_stream`) serves as the immutable genome.
1.  **C-Engine:** Compiles Font/Glyph -> `op_stream` (Binary Blob).
2.  **Browser:** Fetches `blob`. Uploads to `storage_buffer` (Binding 0).

### B. The Compute (WebGPU)
We replace the C `kernel_helmholtz_svdag` with a WGSL Compute Shader.

**Mapping:**
*   **SGPR (Boltzmann):** `var<workgroup> shared_entropy: array<f32>;`
*   **VGPR (Feynman):** `var<private> local_energy: f32;`
*   **Dispatch:** `workgroup_size(64)` maps to our Wave64 simulation.

### C. The Vision (Render)
Instead of rasterizing to a buffer, we use **Ray-Marching** in a Fragment Shader.
1.  **Vertex Shader:** Full-screen Quad.
2.  **Fragment Shader:** Traverses the SVDAG (from the Storage Buffer) for each pixel ray.
3.  **Output:** Visualizes the "Energy" (Feynman Voxel) density directly.

## 3. Immediate Next Steps

### Step 1: Serialization Test
Create `tests/test_svdag_export.c`.
*   **Goal:** Compile a glyph (e.g., "A") into a Helmholtz SVDAG.
*   **Action:** Write the `op_stream` to `out_svdag.bin`.
*   **Verify:** Ensure the binary layout matches the `TSFiBoltzmannSplit` / `TSFiFeynmanVoxel` structs.

### Step 2: Headless Vision Test (CPU)
Create `tests/test_vision_render_cpu.c`.
*   **Goal:** "See" the SVDAG without a GPU.
*   **Action:** Write a simple C-based ray-caster that walks the SVDAG `op_stream` and outputs a `.ppm` image.
*   **Why:** If the CPU ray-caster produces garbage, the WebGPU one definitely will. We must verify the Bijective Wiring produces coherent geometry first.

### Step 3: WGSL Kernel
Create `shaders/helmholtz.wgsl`.
*   **Goal:** Port the logic from `src/tsfi_svdag.c` to WGSL.

## 4. Nomenclature Update
*   **Fourier-G** -> **Helmholtz Fields**.
*   **Spectrum** -> **Energy/Phase State**.
