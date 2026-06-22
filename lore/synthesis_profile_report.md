# Stable Diffusion Synthesis Profiling & Performance Report

## 1. Timing Breakdown
We profiled the `render_vlm_synthesizer.py` execution pipeline under full voxel rendering settings. Below is the latency breakdown of the synthesis run:

| Pipeline Step | Duration (Seconds) | % of Total Time | Description |
| :--- | :--- | :--- | :--- |
| **Model Weight Loading** | 1.32s | 14.1% | Reading and loading `sd15.safetensors` (UNet + CLIP + VAE) |
| **ControlNet Model Loading** | 2.80s | 30.0% | Reading and loading `control_depth.safetensors` |
| **Stable Diffusion Sampling** | 1.06s | 11.3% | Core UNet denoising loop (4 Euler A steps) |
| **VAE Decoding** | 0.25s | 2.7% | Translating latent space representation to raw image pixels |
| **Vulkan/GGML Overhead** | ~2.41s | 25.8% | Initializing Vulkan devices, configuring memory heaps, mapping ReBAR buffers |
| **Python / I/O / Node Bridge** | ~1.50s | 16.1% | Spawning python/C++ processes, YUE on-chain stats HTTP requests, file composite writing |
| **Total Real-Time Latency** | **9.347s** | **100%** | **Total end-to-end time for the user** |

---

## 2. Root Cause Analysis
The primary reason the "Synthesize Art" button takes a long time is **on-demand weight loading and backend initialization**:
- On every button click, `tsfi_sd_worker` is run from scratch as a one-shot process.
- The process must load **5.4 GB of weights** (`sd15.safetensors` at 4.0 GB + `control_depth.safetensors` at 1.4 GB) from disk into system RAM/VRAM.
- It must initialize the Vulkan device (`Vulkan: Using device 0`) twice if fallbacks occur, causing GPU/CPU overhead.

### Dreamshaper Loading Fix
Previously, `LCM_Dreamshaper_v7.safetensors` was failing to load because it is a **UNet-only** checkpoint (missing CLIP text encoder and VAE weights). The worker was failing to determine the SD version, printing a warning, and falling back to a fresh load of `sd15.safetensors`.
- **Fix Implemented:** We modified [tsfi_native_sd.cpp](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/src/tsfi_native_sd.cpp) to explicitly load CLIP and VAE from `sd15.safetensors` when using the Dreamshaper profile.
- **Result:** The Dreamshaper model now loads successfully without triggering load failures or warnings, ensuring the correct aesthetic style is applied.

---

## 3. Recommended Resolution

To make the art generation near-instant (~1.3 seconds end-to-end instead of ~9.3 seconds), we should run the C++ worker as a **persistent daemon** that keeps the models loaded in VRAM.

### Implementation Steps
1. **Create a socket or stdin loop in C++ worker**: Update the C++ main loop to keep the model loaded and listen on a Unix domain socket or read prompts continuously from stdin.
2. **Hook Erlang worker to the daemon**: Update `sd_worker.erl` to keep the port open and feed incoming requests directly to the daemon's socket or stdin without restarting the process.
3. **Dispatch RPC calls from Node server**: Update `scripts/server.js` to dispatch rendering jobs directly to the Erlang cluster client instead of spawning a new Python process.
