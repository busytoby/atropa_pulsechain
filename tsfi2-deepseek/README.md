# TSFi2: RDNA 4 Simulation Engine (Bijective Helmholtz Architecture)

**TSFi2** is a high-performance simulation engine designed to emulate the behavior of next-generation GPU architectures (specifically RDNA 4 "Zhong" and "Zhao" compute models) on AVX-512 capable CPUs. It serves as the core runtime for the **Dysnomia** socio-economic simulation.

## 1. Architectural Overview

TSFi2 abandons traditional CPU-GPU synchronization (PCIe/DMA) in favor of a **Zero-Copy, Resident-State Architecture**.

### Key Technologies
*   **Zhong Thunking:** A JIT-compiled assembly layer (`src/lau_thunk.c`) that manages execution contexts ("Wavefronts") entirely within the CPU's vector register file (ZMM0-31). This eliminates memory R/W overhead for context switching.
*   **Bijective Helmholtz Wiring:** Geometry and Compute are unified.
    *   **Boltzmann Operators (SGPRs):** Scalar registers control topology, entropy, and branching (Structure).
    *   **Feynman Points (VGPRs):** Vector registers represent voxel energy, phase, and substance (Data).
    *   **Interaction:** A unified "Helmholtz List" drives both traversal and compute in lockstep.
*   **Zero-Copy Scheduler:** Wavefronts are "Resident" in registers. Switching active contexts costs **<3 nanoseconds** (verified).
*   **Helmholtz SVDAG:** Sparse Voxel Directed Acyclic Graphs are implemented as linear operator streams, not pointer trees, enabling **~370 Million Traversal Steps/Second** on a single thread.

## 2. Core Subsystems

### Font Engine (Zero-Copy Reservoir)
The font system (`src/tsfi_font_engine.c`) implements a **64MB Virtual Reservoir** for glyph segments.
*   **Strategy:** Allocates a massive virtual address range (via `lau_malloc_wired`) at startup but only commits physical pages as needed.
*   **Benefit:** Eliminates `realloc` and `memcpy` overhead during font atlas growth, ensuring strictly linear O(1) append performance.

### Crypto-Math Kernel (AVX-512 IFMA)
The mathematical core (`src/tsfi_math.c`) utilizes **AVX-512 Integer Fused Multiply-Add (IFMA)** instructions.
*   **Capability:** Performs 2048-bit Modular Exponentiation entirely within ZMM registers.
*   **Performance:** Saturates the host CPU's FMA units, serving as a stress test for thermal and power stability.

## 3. Simulation Realizations

### Project Ipomoea (Autonomous Evolution)
*   **Objective:** Evolution of the HelmK0Rn voxel substrate into recognized biological phenotypes (e.g., White Ipomoea).
*   **Mechanism:** A closed-loop "Gardener" system (`tests/test_auto_flower.c`) using AVX-512 Vision scoring to drive genetic selection.
*   **Status:** **ACTIVE** (Approved Gold Standard reached Gen 524).

### Unicode32 Discovery Mission (Autonomous Scribe)
*   **Objective:** Discover the precise resonance parameters ("Secrets") within the single HelmK0Rn root asset to manifest every character in Unicode32.
*   **Mechanism:** The "Autonomous Scribe" (`tests/test_auto_scribe.c`) iterates through the substrate's parameter space, guided by `tsfi_vision_analyze_glyph`.
*   **Status:** **ACTIVE** (Verified: Latin Capital 'A' approved at Corr: 0.961).
*   **Asset Registry:** `assets/universal_font/` stores discovered glyph checkpoints.

### Morphing Terminal (Font Fidelity)
*   **Objective:** Validation of resolution-independent K0Rn geometry and smooth interpolation logic.
*   **Mechanism:** `tests/test_font_pt` renders 'A' <-> 'B' transitions at 18pt, 36pt, 72pt, and 120pt simultaneously.
*   **Status:** **ACTIVE** (Verified smooth transitions with sub-pixel density accuracy).

## 4. Build Instructions

### Requirements
*   **CPU:** x86_64 with **AVX-512 Foundation** (AVX512F) and **FMA**. AVX512-IFMA is recommended for cryptographic workloads.
*   **OS:** Linux (Kernel 5.x+ recommended for `io_uring` and `userfaultfd` support, though standard POSIX is currently used).
*   **Compiler:** GCC 9+ or Clang 10+.

### Compilation
The project uses a standard `Makefile`.

**Debug Build (Simulated Latency & ASan):**
```bash
make debug
```
*   Enables AddressSanitizer and UndefinedBehaviorSanitizer.
*   Adds debug symbols (`-g`).

**Release Build (Maximum Throughput):**
```bash
make release
```
*   Enables `-O3`, `-march=native`, and Loop Unrolling optimizations.
*   Strips debug symbols.

**Clean:**
```bash
make clean
```

## 3. Benchmarking & Verification

The primary verification tool is `tests/benchmark_suite`. It validates performance against strict policy thresholds.

### Running the Suite
```bash
./tests/benchmark_suite
```

### Key Metrics & Policies

| Benchmark | Description | Target / Policy | Current Status |
| :--- | :--- | :--- | :--- |
| **Baseline Occupancy** | Dispatch overhead for a single Wavefront. | **< 100 Cycles/Wave** | **~51 Cycles** (PASS) |
| **Switch Latency** | Time to toggle active Resident Wavefronts. | **< 5.0 ns** | **~2.5 ns** (PASS) |
| **Wave512 Compute** | FP32 Throughput (Saturating all 32 ZMMs). | **> 300 GFLOPS** | **~350 GFLOPS** |
| **SVDAG Traversal** | Recursive Octree walk (8 levels). | **> 100 M Wave/s** | **~370 M Wave/s** |
| **VRAM Bandwidth** | Simulated Memory IO (Stream/Store). | **> 15 GB/s** | **~19 GB/s** |

### Interpretation
*   **[PASS] Policy Compliance:** The system is operating within RDNA 4 emulation parameters.
*   **[FAIL] Policy Violation:** A regression has occurred (e.g., memory allocation inside a hot loop, L1 cache miss storm). **Immediate revert required.**

## 4. Engineering Standards

### Memory Management (`lau_memory`)
*   **Wired Allocator:** All simulation objects must use `lau_malloc_wired`. This ensures **512-byte alignment** for ZMM compatibility and header protection.
*   **Seal & Scramble:** Critical objects are "Sealed" (checksummed) and "Scrambled" (obfuscated) when at rest to prevent tampering.

### Thunk Safety
*   **RX Pools:** Thunks are emitted into `PROT_READ | PROT_EXEC` memory pools.
*   **Indirect Branching:** Internal branching within thunks uses strictly permitted instruction sequences (e.g., `JNZ`, `JMP`, or `JA`) anchored to the physical context in **R15**.
*   **C/JIT Hybrid:** Complex coordination logic (Hardware Sync, Yielding) is implemented in C-level `logic_state` handlers, with JIT thunks performing minimal context-anchored transitions.

### Code Style
*   **C11 Standard.**
*   **AVX-512 Intrinsics:** Used extensively in `src/tsfi_opt_zmm.c` and `src/tsfi_svdag.c`.
*   **No C++:** Strictly C to maintain ABI compatibility and raw pointer control.

## 5. Usage Example (CLI)

Start the runtime:
```bash
./tsfi2
```

Commands:
*   `1.0 LOAD <plugin_name>`: Hot-load a logic plugin.
*   `0.0 MATH [BITS]`: Run modular exponentiation benchmark.
*   `0.0 PROVENANCE`: Verify system integrity.
*   `0.0 SCRAMBLE`: Trigger memory obfuscation event.
*   `0.0 EXIT`: Shutdown.

## 7. Theoretical Foundations (Documentation)

The architecture is derived from the following theoretical papers located in `docs/`:

*   **[FOURIER_G_BIJECTION.md](docs/FOURIER_G_BIJECTION.md):** The mathematical basis for mapping geometry to spectral fields.
*   **[HILBERT_SPACE_FONTS.md](docs/HILBERT_SPACE_FONTS.md):** Storing glyphs as infinite-resolution Hilbert curves.
*   **[SYMMETRY_AWARE_SVDAG.md](docs/SYMMETRY_AWARE_SVDAG.md):** Compression of voxel data using rotational symmetry.
*   **[WGP_PRIVATE_BIJECTIONS.md](docs/WGP_PRIVATE_BIJECTIONS.md):** Mapping AI workloads to RDNA 4 Workgroup Processors (WGP).
*   **[PLAN_TESSELLATION_WMMA.md](docs/PLAN_TESSELLATION_WMMA.md):** Using Matrix Cores for geometric tessellation.

---
**Status:** ACTIVE | **Version:** 4.1 | **License:** ATROPA/DYSNOMIA
