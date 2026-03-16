# Plan: Active Window Vision Integration

This plan details the next phase of verifying the TSFi Ray Traced Font System: moving from "Mock GPU" tests to **Active Window Verification**. The goal is to prove that the engine produces visible, identifiable glyphs within the actual Wayland/Vulkan compositor window (`tsfi_vulkan.c`).

## Objectives
1.  **Pipeline Integration:** Connect the `TSFiFontSystem` (AVX-512 Rasterizer) to the `VulkanSystem` render loop in `tsfi_vulkan.c`.
2.  **Live Verification:** Use `tsfi_vision` to snapshot the *actual* running window and identify the rendered text.
3.  **Dynamic Interaction:** Verify that "Alter-as-we-go" commands (e.g., changing font style) reflect immediately in the active window.

## Proposed Test Cases

### 1. `verify_window_render` (Integration Test)
*   **Goal:** Render "HELLO" in the live window and verify visibility.
*   **Mechanism:**
    *   Modify `tsfi_vulkan.c` to include `tsfi_font_engine.h`.
    *   In `create_vulkan_system`, initialize the font engine.
    *   In `draw_frame`, use `tsfi_font_rasterize_glyph_bda` to update the `paint_buffer` (which is uploaded to the swapchain).
    *   Launch the window (headless or visible).
    *   Use a "Screenshot Hook" (to be implemented) to dump the swapchain image to a PPM file.
    *   Analyze PPM with `tsfi_vision` tools.

### 2. `verify_dynamic_atlas` (Interactive Test)
*   **Goal:** Prove realtime font updates work.
*   **Action:**
    *   Render Frame 1 with Default Font. Snapshot.
    *   Send Directive: `MODIFY_GLYPH A box`.
    *   Render Frame 2. Snapshot.
    *   Compare Snapshots. Assert Difference.

### 3. `verify_full_ocr` (End-to-End)
*   **Goal:** Render a sentence and read it back.
*   **Action:**
    *   Render "TSFi System Active".
    *   Snapshot.
    *   Run full OCR segmentation and matching against the Font Engine's own vector definitions (using the rasterizer to generate Golden References).

## Implementation Steps

1.  **Vulkan Hooks:**
    *   Update `VulkanSystem` struct to hold a `TSFiFontSystem*`.
    *   Add logic in `draw_frame` to rasterize text strings into the `staging_vk_buffer` (or `paint_buffer`).

2.  **Readback Capability:**
    *   Implement `tsfi_vulkan_snapshot(VulkanSystem *s, const char *filename)` to dump the current swapchain image to disk (CPU Readback via Staging).

3.  **Test Harness:**
    *   Create `tests/test_active_window_ocr.c` which links the real `tsfi_vulkan.c`, spawns the window, triggers the render, snaps, and verifies.

## Rationale
This approach validates the *entire* stack:
*   **Storage:** `inc/tsfi_font_vectors.h` (BDA/Coherent)
*   **Compute:** `src/tsfi_font_rasterizer.c` (Wave512)
*   **Display:** `plugins/tsfi_vulkan.c` (Wayland/Surface)
*   **Vision:** `src/tsfi_vision.c` (Identification)

It moves beyond unit testing the components in isolation to verifying the *product*.
