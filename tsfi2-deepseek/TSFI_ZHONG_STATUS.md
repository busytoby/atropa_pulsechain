# TSFi Zhong: ReBAR/SAM Unified Memory & Concurrency

## Current Status: [INTEGRATED / VERIFIED]
**Date**: February 23, 2026
**Performance**: 26.70 GB/s Write Bandwidth (Peak)

### Core Mandate
Enable direct, zero-copy CPU-to-GPU memory mapping (Memory Type 3: Device Local + Host Visible) to support the "Zhong Threading" concept for unified CPU/GPU concurrency.

### Implementation Details
Zhong realizes the hardware-mapped memory layer for the **Resonance Manifold**:

1.  **ReBAR Mapping**: Successfully identifies and maps the 16GB VRAM heap on RX 9070 XT as Host Visible.
2.  **Zhong Threading**: Synchronizes CPU workers (via **Zhao**) and GPU compute kernels exclusively through Vulkan Timeline Semaphores, eliminating expensive CPU-side wait-for-idle cycles.
3.  **Above 4G Allocator**: Implements a specialized allocator within the mapped ReBAR space, allowing JIT thunks to write directly to GPU-resident buffers without intermediate staging.

### Benchmarks
- **Write Bandwidth**: 26.70 GB/s (Burst) / 20.67 GB/s (Sustained via Multi-Ring Zhao).
- **Latency**: Sub-microsecond command submission via shared memory command channels.

### Verification
Status verified via `tests/tsfi_zhong_overlay.c` and `tests/native_rdna4_rebar.c`. All "Zhong" operations are now dispatched through the **Zhao Ring Scheduler** to ensure maximum bus saturation.
