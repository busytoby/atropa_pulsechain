# TSFi Phi-Resolution Status (2026-02-11)

## Architecture: The Symmetric Clutch
The system solves for Helmholtz Potential ($\Phi$) to determine "Inevitability" in a manifold. This replaces stochastic sampling.

- **CPU Thunk:** `thunks/tsfi_phi_resolve.c` (AVX-512 Bit-Resonance)
- **GPU Shader:** `shaders/tsfi_phi_resolve.comp` (GLSL Reference)
- **Manifold:** `TSFiHelmholtzManifold` (Mapped Struct)

## Benchmark Results (10k Atoms)
- **CPU (Host RAM):** ~0.02ms per iteration (26-27x Speedup)
- **GPU (ReBAR):** ~0.54ms per iteration (Dispatch Overhead dominance)

## Decision
The CPU Thunk is the **Active Clutch**. 
It is wired in `src/tsfi_zmm_vm.c` to handling $\Phi$-resolution for manifolds <1M atoms.
For larger manifolds, the GPU shader can be compiled and deployed, but currently the overhead disqualifies it for micro-kernel usage.

## Implementation Details
- **Vectorization:** `_mm512_xor_si512` + `_mm512_test_epi32_mask` + `popcnt`.
- **Memory:** CPU requires Host RAM (Cached) for performance. ReBAR (WC) is too slow for CPU reads.
- **Safety:** Vulkan Dispatcher uses "Local Proc" loading to survive ABI/Struct mismatches.
- **Cleanup:** Full resource destruction verified (0 leaks).
