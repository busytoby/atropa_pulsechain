# Project Mariner: Compute Fabric Report (Final Phase 0)
**Date:** 2026-02-07
**Status:** PHASE 0 VERIFIED & CORRECTED

## 1. Distinct Architecture Map
We acknowledge two distinct, non-isomorphic instruction sets operating in parallel.

### A. The CPU Domain (x86-64 / Zen 5)
*   **Architecture:** **AVX-512** (512-bit SIMD)
*   **Register File:** 32 x 512-bit ZMM registers (`zmm0` - `zmm31`).
*   **Lane Width:** 16 x Float32 (per register).
*   **Key Extensions:** `AVX512_BF16` (Brain Float), `AVX512_VNNI` (Neural), `AVX512_IFMA` (Integer Math).
*   **Execution Model:** Out-of-Order, Superscalar. Best for complex branching and heavy serial math (Folding).

### B. The GPU Domain (RDNA 4 / GFX1201)
*   **Architecture:** **RDNA ISA** (Wavefront SIMT)
*   **Thread Widths:**
    *   **Wave32:** 32 Threads (Native RDNA).
    *   **Wave64:** 64 Threads (High-Throughput Mode).
*   **Matrix Widths (Cooperative Matrix):**
    *   **Matrix16:** Operates on 16x16 tiles. Effectively "Wave256" throughput per clock per CU.
*   **Key Features:** `meshShader`, `accelerationStructure`, `cooperativeMatrix`.

## 2. The "Zhong Bridge" Strategy (Data Alignment)
We align data payloads to maximize bus throughput between these disparate architectures.

*   **Wave32 Alignment:** One GPU Wave32 consumes 32 floats. This matches **2x CPU ZMM** registers.
*   **Wave64 Alignment:** One GPU Wave64 consumes 64 floats. This matches **4x CPU ZMM** registers.
*   **Matrix Alignment:** A 16x16 FP16 matrix (512 bytes) matches exactly **8x CPU ZMM** registers.

**Conclusion:**
*   **CPU:** Validated for AVX-512 (Zen 5).
*   **GPU:** Validated for Wave64 and Cooperative Matrix (RDNA 4).
*   **Protocol:** "Zhong Bridge" defines the data-packing standard (4:1 or 8:1) to link these architectures.