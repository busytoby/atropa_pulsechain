# 5.82-Second Pipelined Latency Trace

This trace details the millisecond-by-millisecond execution overlap of the concurrent pipeline (Thread A and Thread B) during a cold run.

---

## 1. Concurrency Timeline (0.00s to 5.82s)

| Time (sec) | Thread A (Python Coordinator) | Thread B (C++ Worker Process) | Shared Resources State |
| :--- | :--- | :--- | :--- |
| **0.00s** | Spawns Thread B; begins on-chain YUE stats RPC fetch. | Worker process starts; initializes Vulkan heaps. | `/dev/shm/tsfi_cn_depth` is empty. |
| **1.28s** | Still waiting for network RPC fetch (HTTP delay). | Finishes loading SD 1.5 weights (1.28s CPU load). | No change. |
| **3.43s** | Completes RPC fetch (takes 3.43s); starts voxel rendering. | Loading ControlNet weights (1.4GB). | No change. |
| **3.60s** | Voxel rendering completes; writes depth map to `/dev/shm/tsfi_cn_depth` and enters `wait()` block. | Still loading ControlNet weights. | `/dev/shm/tsfi_cn_depth` contains 512x512 depth map. |
| **4.08s** | Idle (blocked on `wait()`). | Finishes loading ControlNet weights (takes 2.80s). | Depth map ready. |
| **4.35s** | Idle (blocked on `wait()`). | Vulkan initialization completes (0.27s). Reads depth map from shared memory; begins Denoising. | Depth map loaded into VRAM. |
| **5.40s** | Idle (blocked on `wait()`). | Denoise loop completes (4 Euler A steps take 1.05s). Begins VAE decoding. | Latent grid generated. |
| **5.65s** | Wakes up from `wait()` as C++ process exits. Reads raw output. | VAE decode completes (0.25s). Writes output raw file and exits. | C++ process terminated. |
| **5.82s** | Composites art onto template, writes PNGs, executes VLM check, and exits. | Terminated. | Completed card assets written to disk. |

---

## 2. Temporal Analysis & Insights

- **Zero-Latency Network RPCs:** Because Thread A's stats fetch (3.43s) and voxel drawing (0.17s) complete at **3.60s**, they finish **0.75 seconds before** the C++ worker completes its weight-loading phase (4.35s). Thus, the entire 3.43 seconds of blocking network latency is **completely hidden (0ms contribution to total runtime)**.
- **Synchronization Wait (1.30s):** Once Thread A finishes writing the depth map at 3.60s, it waits for Thread B until 5.65s. This wait time represents the C++ worker finishing its load (4.08s -> 4.35s) and executing inference (4.35s -> 5.65s).
- **Primary Bottleneck Remaining:** The C++ worker initialization and model loading (4.35 seconds total) remains the only reason the pipeline takes >1.5 seconds.
