# TSFi RDNA4 Visual Verification Plan

This document outlines display-specific test cases to verify the integration of RDNA4 (GFX1201) memory hierarchies into `lau_memory`. Each test is designed to produce a distinct **Visual Outcome** that confirms the underlying architectural success (Bijection, Thunking, Coherency).

## 1. Test Case: "The Canvas" (L2/ReBAR + Bijection)
**Objective:** Verify basic **Bijection** (CPU Pointer <-> GPU Handle) and **ReBAR** access (L2).

*   **Logic:**
    1.  Allocates a 1080p buffer using `lau_malloc_gpu(..., LAU_GPU_REBAR)`.
    2.  CPU writes a "Test Pattern" (SMPTE bars or Gradient) directly to the returned pointer.
    3.  Thunk calls `lau_sync` (CPU->GPU).
    4.  Vulkan shader simply samples this buffer as a texture and displays it.
*   **Visual Outcome (Success):** The Test Pattern appears instantly and creates a "static" clear image.
*   **Visual Outcome (Failure):** Black screen (Bijection failed) or Garbled Noise (Coherency/Thunk failed to flush).

## 2. Test Case: "The Strobe" (L3/MALL + Bandwidth)
**Objective:** Verify **L3 (Infinity Cache)** priority and **High-Frequency Thunking**.

*   **Logic:**
    1.  Allocates a small (4MB) buffer using `lau_malloc_gpu(..., LAU_GPU_L3)`.
    2.  CPU fills this buffer with a solid color that changes every frame (60Hz or 144Hz).
    3.  Thunk executes `vkFlush` immediately after write.
    4.  GPU renders a full-screen quad using this color.
*   **Visual Outcome (Success):** A smooth, high-speed strobing effect (Red -> Green -> Blue) with zero tearing or latency.
*   **Visual Outcome (Failure):** "Stuttering" colors or tearing (indicates fallback to System RAM or missed V-Sync deadlines due to bus contention).

## 3. Test Case: "The Ripple" (L1/GL1 + Coherency)
**Objective:** Verify **L1 (Device Coherent)** behavior and fine-grained synchronization.

*   **Logic:**
    1.  Allocates a grid (e.g., 64x64) using `lau_malloc_gpu(..., LAU_GPU_L1)`.
    2.  CPU writes a single value ("The Drop") at the center.
    3.  A Vulkan Compute Shader reads this grid, propagates the value to neighbors (simulating a wave/ripple), and writes back.
    4.  GPU renders the height map.
*   **Visual Outcome (Success):** A smooth circular ripple expands from the center whenever the CPU "touches" memory.
*   **Visual Outcome (Failure):** Blocky artifacts (cache lines not flushing) or the ripple "freezes" (coherency loss).

## 4. Test Case: "The Swarm" (L0/LDS + Injection)
**Objective:** Verify **L0 (LDS)** simulation and complex Thunking (Command Buffer Injection).

*   **Logic:**
    1.  Allocates a particle state buffer.
    2.  Thunk constructs a Command Buffer that:
        *   Loads data from VRAM -> L0 (LDS).
        *   Runs a generic "Flocking" algorithm entirely in LDS.
        *   Stores L0 -> VRAM.
    3.  GPU renders the particles as points.
*   **Visual Outcome (Success):** Thousands of particles moving organically (boid-like behavior) with high frame rates.
*   **Visual Outcome (Failure):** Particles moving in straight lines (logic bypass), "exploding" (memory corruption), or extremely low FPS (LDS usage failed, fell back to VRAM).

## Execution Strategy
The tests will be implemented as a single interactive binary `tests/test_visual_hierarchy` controlled by the `lau_memory` system directives.

*   **Command:** `./test_visual_hierarchy [CASE_ID]`
*   **Verification:** User observes the window for the specific Visual Outcome described above.
