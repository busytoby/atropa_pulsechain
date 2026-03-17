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

## 5. Test Case: "Project Ipomoea" (Autonomous Vision-Guided Evolution)
**Objective:** Verify the **Autonomous Decision Engine** and **AVX-512 Vision Scoring**.

*   **Logic:**
    1.  Launches the "Gardener" loop via `./test_flower.sh`.
    2.  Observes real-time mutation of K0Rn genomes.
    3.  Vision Engine applies selection pressure based on rotational symmetry and radial density profiles.
*   **Visual Outcome (Success):** A recognizable flower structure emerges from chaotic voxel noise within < 500 generations.
*   **Visual Outcome (Failure):** Evolution remains stuck in noise (Score logic failure) or crashes (SIMD alignment/memory leak in thunk).

## 6. Test Case: "The Morphing Terminal" (Font Fidelity & Transition)
**Objective:** Verify **K0Rn Stream Interpolation** and **Resolution Independence** at multiple point sizes.

*   **Logic:**
    1.  Loads the 'A' (0x41) and 'B' (0x42) DNA streams.
    2.  Renders them simultaneously at four distinct point sizes (18pt, 36pt, 72pt, 120pt).
    3.  Modulates an interpolation factor `t` (0.0 to 1.0) using a sine wave.
    4.  Applies `tsfi_k0rn_interpolate` to generate intermediate glyphs.
*   **Visual Outcome (Success):** The letter 'A' smoothly morphs into 'B' and back across all sizes simultaneously, maintaining sharp edges (Density rendering) without pixelation.
*   **Visual Outcome (Failure):** Jittering geometry, "popping" between shapes, or blurry edges (Rasterizer fallback).

## 7. Test Case: "The Puppet Master" (OpenPose Interactivity)
**Objective:** Verify **Subjective Rigidity** during manual 16-joint skeletal manipulation via Port 9093.

*   **Logic:**
    1.  User "grabs" a joint (e.g., `beak_u` or `wing_tip`) on the World Serpent Dashboard.
    2.  Web UI sends `/puppet_move` RPCs to the realizer.
    3.  Realizer updates the manifold and applies **ICPC 414A (Speech)** jitter to the new pose.
    4.  Alligator audits the 1MB arena for unicyclic isomorphism consistency during the drag.
*   **Visual Outcome (Success):** The Crow follows the mouse with zero latency, maintains its skeletal structure, and continues its staccato clicking rhythm.
*   **Visual Outcome (Failure):** Limb detachment, joint "snapping," or manifold fracture (0,0 state).

## 8. Test Case: "The Iridescent Wing" (Anisotropic Shading)
**Objective:** Verify the mathematical distinction between **Feathers** and **Fur**.

*   **Logic:**
    1.  Renders the Crow and Teddy Bear side-by-side using the 720p realizer.
    2.  Exercises **`TASTE_CROW_FEATHERS`** (Anisotropic) and **`TASTE_TEDDY_FUR`** (Isotropic).
    3.  Path-tracer samples the 256 KiB **Packed Shading Manifold** (Normal, AO, Rim).
*   **Visual Outcome (Success):** The Crow shows a dynamic "oil-slick" sheen that shifts with the light, while the Teddy Bear exhibits soft, occluded fur depth.
*   **Visual Outcome (Failure):** Flat, non-material shading; both entities look like grey plastic.

## 9. Test Case: "The Neural Bridge" (720p Photorealism)
**Objective:** Verify **DeepSeek + Stable Diffusion** guidance at distribution resolution.

*   **Logic:**
    1.  Generates a 1280x720 Depth Map of the stabilized Crow.
    2.  DeepSeek-Coder-V2 generates a visual prompt based on the active Taste Atom.
    3.  Stable Diffusion (SD-CPP) synthesizes the final image using the depth map as a rigid structural guide.
*   **Visual Outcome (Success):** A hyper-photorealistic 12-inch Crow plush with detailed stitching and iridescent feathers, perfectly aligned with the underlying OpenPose skeleton.
*   **Visual Outcome (Failure):** Neural hallucinations (extra limbs), low resolution (512x512), or lack of subjective materiality.

## Execution Strategy
The tests will be implemented as a single interactive binary `tests/test_visual_hierarchy` controlled by the `lau_memory` system directives.


*   **Command:** `./test_visual_hierarchy [CASE_ID]`
*   **Verification:** User observes the window for the specific Visual Outcome described above.
