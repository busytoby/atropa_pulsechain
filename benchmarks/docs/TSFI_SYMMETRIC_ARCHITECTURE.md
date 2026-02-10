# TSFi Symmetric Architecture: The Bijective Waveform

The TSFi platform realizes a "Supremacy Architecture" where compute logic is defined once (Bijective Operator) and executed on the optimal hardware path (CPU Thunk vs. GPU Shader) based on real-time resonance metrics.

## 1. The Quad-Path Execution Model

Every TSFi Operator supports four distinct execution paths, ensuring robustness and performance scaling across all workload sizes.

| Path | Description | Best Use Case | Performance (256MB) |
| :--- | :--- | :--- | :--- |
| **1. CPU Shader (C)** | Pure C reference implementation. | Debugging, Fallback. | ~4.2 GB/s |
| **2. CPU Thunk (ASM)** | JIT-compiled Pure WAVE Assembly (AVX-512). | Low-Latency, < 64MB Streams. | **~6.3 GB/s** |
| **3. GPU Shader** | Stateless Vulkan Compute Dispatch. | One-off large compute jobs. | ~3.1 GB/s |
| **4. GPU Thunk** | Stateful, Baked Command Buffers + ReBAR. | High-Throughput, > 64MB Streams. | **~4.8 - 17 GB/s** |

## 2. WAVE(Any) Data Structure

The `WaveStream` descriptor allows for variadic atom sizes (e.g., 15-byte packed structures) to be processed efficiently.

*   **CPU Thunk**: Handles packed data via `VMOVUPS` unaligned loads and masked stores for tail processing.
*   **GPU Thunk**: Prioritizes `aligned(16)` access but falls back to CPU for packed streams (`stride % 4 != 0`) to prevent alignment faults.

## 3. Bijective Integrity (Dysnomia Reactive CRC)

To guarantee that "Crossover Always Occurs" safely, the system implements a post-quantum checksum:
*   **Signature**: `State = (State * Data) + React(Data, Ichidai, Daiichi)`
*   **Ichidai (Forward)**: Additive modulation ($\eta$).
*   **Daiichi (Inverse)**: Multiplicative modulation ($\kappa$).
*   **Verification**: `WAVE_FLAG_VERIFY` triggers a dual-path execution (CPU + GPU) and compares memory bit-identity.

## 4. Fused Operator Pipeline

Operators can be chained into a `TSFiFusedOp` to maximize arithmetic intensity.
*   **CPU**: Function pointer chaining.
*   **GPU**: Command Buffer recording with `VkMemoryBarrier` injection between dispatches.

## 5. K0Rn Generative Codegen

The architecture is evolving to support a unified IR (`K0RnModule`) that compiles to both AVX-512 machine code and SPIR-V binaries at runtime, closing the "Source-Level Bijective Gap".
