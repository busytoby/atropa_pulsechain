# Auncient Lore Connections: Synthesizer & Vulkan Interop

This document establishes the architecture mapping synthesis parameters (DNA/cryptographic token metadata) to the Vulkan hardware rendering layer.

## 1. The Shared Memory (SHM) Pipeline Bridge
To achieve real-time synchronization between on-chain genome data and the Vulkan graphics engine without network bottleneck latency, the system uses a dual-threaded shared memory pipeline.

```
[ Thread A: Python Coordinator ] ──> Fetch YUE Stats via RPC (3s) ──> Write Voxel Depth to /dev/shm/tsfi_cn_depth
                                                                                        │
                                                                                        ▼
[ Thread B: C++ Vulkan/SD Engine ] ─> Load 5.4GB model weights (3.5s) ────────────────> Read depth map & render card
```
* **ControlNet Manifold:** The voxel depth map generated from cryptographic addresses is exposed directly in memory mapped space (`/dev/shm/tsfi_cn_depth`). The Vulkan fragment shaders read this buffer directly to perform fur extrusion, skeletal skinning, and lighting occlusion calculations.

## 2. Low-Level CPU Coupling & Port Mapping
The synthesizer controls the Vulkan viewport parameters dynamically by querying physical VM registers via `PEEK`/`POKE`:

* **`PEEK Ammeter`:** Reads the active VM processing throughput, adjusting Vulkan thread priorities and texture sampling resolution on the fly.
* **`POKE Voltmeter`:** Modifies voltage levels mapped to filter sweeps and resonance constants, correlating auditory filter decay directly to the color palette dynamic registers.

## 3. Direct GPU ReBAR Interrogation (`tsfi_vram_grep`)
To avoid slow CPU-to-GPU data copies, the host compositor reads active ASCII information inside the Vulkan GPU Resizable BAR (ReBAR) memory pool:
* Interleaved 32-bit striding sweeps raw graphics buffers directly, parsing glyph indexes and layout states to generate the corresponding audio trigger envelopes.
* Changes in graphics pipelines are translated into immediate changes in filter resonance coefficients.
