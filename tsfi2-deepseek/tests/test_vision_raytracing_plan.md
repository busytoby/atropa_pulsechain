# TSFi Vision Ray Tracing & Glyph Integration Test Plan

This document outlines the test strategy for integrating AVX-512 rasterized fonts into the TSFi Vision system, enabling both production (rendering) and identification (OCR/Vision) of glyphs within the active window compositor.

## Objectives
1.  **Verify Rendering Pipeline:** Ensure the "Wave512" (AVX-512) rasterizer correctly produces glyphs that are uploaded to the GPU and displayed.
2.  **Verify Vision Recognition:** Ensure the `tsfi_vision` system can "see" and correctly identify these rendered glyphs on the screen.
3.  **Performance Verification:** Benchmark the end-to-end latency of "Alter-as-we-go" (Dynamic update -> Rasterize -> Upload -> Render -> Detect).

## Test Cases

### 1. Basic Glyph Visibility (Static)
*   **Goal:** Confirm a glyph rendered by the engine appears on the screen.
*   **Action:**
    *   Initialize `TSFiFontSystem` and `VulkanSystem`.
    *   Rasterize 'A' to the Texture Atlas using AVX-512.
    *   Render a quad with this texture in the center of the screen.
    *   Use `tsfi_vision_snapshot()` to grab the framebuffer.
*   **Verification:**
    *   Scan the center region of the snapshot.
    *   Assert that non-black pixels exist (Coverage Check).
    *   Assert `tsfi_vision_max_value` > Threshold.

### 2. Glyph Identification (Shape Matching)
*   **Goal:** Confirm the vision system can distinguish 'A' from 'B'.
*   **Action:**
    *   Render 'A' on the left side, 'B' on the right.
    *   Snapshot the screen.
*   **Verification:**
    *   Extract the ROI (Region of Interest) for 'A'.
    *   Compare against a "Golden Reference" of 'A' (generated offline or via the rasterizer directly).
    *   Assert `tsfi_vision_compare_raw(ROI_A, Reference_A)` is High Confidence.
    *   Assert `tsfi_vision_compare_raw(ROI_A, Reference_B)` is Low Confidence.

### 3. Dynamic "Alter-as-we-go" Update
*   **Goal:** Verify realtime updates to the font definition.
*   **Action:**
    *   Render 'C' (Default Shape).
    *   Snapshot & Verify.
    *   **Modify 'C':** Use `tsfi_font_set_glyph` to change 'C' into a solid box.
    *   Trigger Atlas Update (Rasterize -> Upload).
    *   Render next frame.
    *   Snapshot & Verify.
*   **Verification:**
    *   Assert Frame 1 matches 'C'.
    *   Assert Frame 2 matches Box.
    *   Assert Frame 1 != Frame 2.

### 4. Full Text Stream OCR
*   **Goal:** Render a string "HELLO" and read it back.
*   **Action:**
    *   Implement a simple text layout engine (advance cursor X).
    *   Render "HELLO".
    *   Snapshot.
*   **Verification:**
    *   Scan the line.
    *   Segment blobs (simple whitespace separation).
    *   Identify each blob: 'H', 'E', 'L', 'L', 'O'.

## Implementation Strategy
*   **Test Harness:** Create `tests/test_vision_raytracing.c`.
*   **Integration:**
    *   The test will act as a "Client" to `tsfi_vulkan.c` and `tsfi_font_engine.c`.
    *   It will leverage the `upload_staging_to_image` hook to push the atlas.

## Wave512 Considerations
*   The tests will implicitly verify the correctness of the AVX-512 rasterizer.
*   If `test_font_avx512` passed (it did), the data in the staging buffer is correct.
*   The primary failure mode here is the *upload path* (ReBAR/DMA) or the *rendering coordinates*.

