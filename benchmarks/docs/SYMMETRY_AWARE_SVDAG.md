# Fourier-G as Symmetry-Aware SVDAGs: A Systemic Synthesis

This document defines the ultimate architectural form of the TSFi Font System, integrating the **Banach-Hilbert Bijection** with **`lau_memory`** persistence and **SVDAG** (Sparse Voxel DAG) spatial representation.

## The Triad Architecture

We define the glyph lifecycle as a transformation across three domains:

1.  **Design (Banach Space $\mathcal{B}$):** The geometric contour (Bezier). Human-readable.
2.  **State (Hilbert Space $\mathcal{H}$):** The logical coefficients. Machine-learnable, Morphable.
    *   *System Integration:* Stored in **`lau_memory`**.
3.  **Representation (SVDAG $\mathcal{S}$):** The spatial index. Ray-traceable.
    *   *Hardware Integration:* RDNA4 Ray Accelerator.

$$ \mathcal{B} \xrightarrow{	ext{Analysis}} \mathcal{H} \xrightarrow{	ext{Synthesis (WMMA)}} \mathcal{S} $$

## 1. The Hilbert State in `lau_memory`

In the TSFi Wave System, the "True Name" of a glyph is its Hilbert Vector (Spectrum).

*   **Structure:** `TSFiHilbertGlyph` (from `tsfi_hilbert.h`).
*   **Storage:** Allocated via `lau_malloc_wired`.
    *   **Provenance:** The logical coefficients are signed and tracked. Any mutation (evolution) updates the `lau_metadata` version.
    *   **Scrambling:** `lau_mem_scramble` ensures the logical data remains resilient in memory, protecting the "DNA" of the font.

## 2. The Bijection: Spectrum $	o$ SVDAG

The critical innovation is mapping the **Logical Symmetries** of the Hilbert Space directly to the **Topological Symmetries** of an SVDAG.

### The Problem
Rasterizing a high-resolution 3D glyph (Voxel grid) consumes massive memory ($N^3$).

### The Solution: SVDAG
An SVDAG (Sparse Voxel Directed Acyclic Graph) replaces a voxel tree with a DAG. Identical branches point to the same child node.

### Symmetry-Aware Generation
Instead of checking for identical voxels *after* generation, we predict them from the Spectrum *before* generation.

*   **Even/Odd Harmonics:**
    *   If coefficients $C_n$ are only odd terms ($n=1, 3, 5...$), the function is **Anti-Symmetric** ($f(-x) = -f(x)$).
    *   **SVDAG Implication:** The Left Child is the geometric inverse of the Right Child. We generate *one* child and point both parent pointers to it (with a "Flip Bit" flag).
*   **Real/Imaginary Relations:**
    *   Symmetries in the Complex Plane map to rotation/reflection symmetries in Euclidean space.
*   **Fractal Self-Similarity:**
    *   If high-frequency harmonics are scaled copies of low-frequency harmonics, the glyph is fractal. The SVDAG can be recursive (pointers to ancestors).

## 3. The Hardware Pipeline (WMMA + RT)

### Step A: The "Wave Expansion" (WMMA)
A Compute Shader uses WMMA to evaluate the Hilbert State $|\Psiangle$ into a local dense voxel leaf (e.g., $4 	imes 4 	imes 4$ block).
*   **Input:** `lau_memory` Coefficient Matrix.
*   **Operation:** Matrix Multiply (Basis $	imes$ Coeffs).
*   **Output:** Bitmask of occupied voxels.

### Step B: The DAG Builder (Compute)
The builder links these leaves. It uses the **Logical Signatures** to deduplicate nodes.
*   Instead of hashing 64 bits of voxel data to find duplicates...
*   It hashes the **Logical Subset** responsible for that octant.
*   If the local spectrum matches, the geometry matches. **Perfect caching.**

### Step C: Traversal (RDNA4)
The RDNA4 Ray Accelerator traces the SVDAG.
*   Because the SVDAG is "Symmetry-Aware" (highly compressed via logical deduplication), the entire font library fits in L2/Infinity Cache.
*   **Infinite Resolution:** We can generate deeper DAG levels on-the-fly using the fractal properties of the Hilbert vector.

## Systemic Conclusion

This architecture resolves the "Infinite Resolution" requirement:
1.  **Storage:** Tiny (`lau_memory` holds only coefficients).
2.  **Render:** Fast (SVDAG fits in cache).
3.  **Evolution:** Trivial (Linearly interpolate coefficients in `lau_memory`; the SVDAG rebuilds automatically via Compute).

We have effectively turned Font Rendering into **Holographic Projection** from a Hilbert Space seed.
