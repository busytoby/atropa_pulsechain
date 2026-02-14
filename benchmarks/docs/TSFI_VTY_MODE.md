# TSFi VTY Mode: Direct Vulkan Backend

**Status:** EXPERIMENTAL (Phase 1)
**Backend:** `VULKAN_BACKEND_DIRECT_DISPLAY` (KMS/VTY)
**Driver:** RADV (RDNA 4 Optimized)

## 1. Overview

The **TSFi VTY Mode** is a "Zero-Copy Display" implementation that bypasses the windowing system (Wayland/X11) entirely. It allows the simulation engine to talk directly to the GPU's display controller via the Vulkan `VK_KHR_display` extension.

This achieves the ultimate "Physical Reality" rendering path:
*   **Zero Compositor Overhead:** No buffer copies or composition passes.
*   **Zero Input Latency:** Raw kernel `evdev` events read directly from `/dev/input/`.
*   **Maximum Bandwidth:** Direct scanout from VRAM.

## 2. Architecture

### Backend (`plugins/vulkan/backend_display.c`)
*   Replaces the Wayland `backend_main.c`.
*   Initializes the `VulkanSystem` singleton with `backend_type = VULKAN_BACKEND_DIRECT_DISPLAY`.
*   Uses `init_vulkan_display()` to enumerate physical connectors (`VkDisplayKHR`) and create a direct surface (`VkSurfaceKHR`) on the hardware plane.

### Input Subsystem (`plugins/vulkan/input_vty.c`)
*   Scans `/dev/input/event*` for devices with `EV_KEY` capabilities.
*   Implements a non-blocking poll loop reading `struct input_event` directly.
*   Maps raw Linux keycodes (e.g., `KEY_LEFTSHIFT`) to the internal `TSFiInputState`.
*   Bypasses `libinput` for minimal dependency footprint.

### Initialization (`plugins/vulkan_init.c`)
*   **Extension:** `VK_KHR_display` (Instance Level).
*   **Logic:**
    1.  Enumerates all connected displays.
    2.  Selects the primary display (currently Index 0).
    3.  Selects the primary mode (resolution/refresh).
    4.  Creates a `VkDisplaySurfaceKHR` representing the physical screen.

## 3. Usage

### Prerequisites
*   **Permissions:** The user must have read/write access to:
    *   `/dev/dri/card0` (Group: `video` or `render`)
    *   `/dev/input/event*` (Group: `input`)
*   **DRM Master:** The GPU must not be locked by another compositor (GNOME, Sway, etc.) on the current VT.
    *   **Recommendation:** Switch to an unused TTY (e.g., `Ctrl+Alt+F3`) before launching.

### Launching
Use the dedicated launch script:
```bash
./start_vty.sh
```
This script checks permissions, builds the debug target, and pipes the `OPEN_WINDOW_VTY` directive to the engine.

## 4. Roadmap

### Phase 1: Foundation (Completed)
*   [x] `VK_KHR_display` verification (`experiments/check_vulkan_display.c`).
*   [x] Backend architecture refactor (`VulkanBackendType`).
*   [x] Basic surface creation (`init_vulkan_display`).
*   [x] Raw input scanning (`input_vty.c`).
*   [x] Build system integration (Hotloader, Makefile).

### Phase 2: Robustness (Current)
*   [ ] **Mode Selection:** Logic to pick the highest refresh rate or specific resolution instead of the first available mode.
*   [ ] **VT Switching:** Implement `ioctl(KDSETMODE)` to properly take over the console and handle `SIGUSR` signals to release/acquire the GPU when switching TTYs.
*   [ ] **Device Hotplug:** Handle input devices connecting/disconnecting at runtime.

### Phase 3: Advanced
*   [ ] **DRM Leasing:** Use `VK_EXT_acquire_drm_display` to lease a specific output (e.g., a VR headset or secondary monitor) while a Wayland compositor continues running on the primary screen.
*   [ ] **Multi-Head:** Support spanning the simulation across multiple physical displays.

## 5. Technical Notes

*   **Zero-Fill OOB:** The VTY backend initialization logic handles uninitialized surface extents by enforcing the requested window dimensions, preventing the validation errors seen in early Wayland tests.
*   **Dual-Command Routing:** The VTY mode is fully compatible with the Cockpit's dual-command architecture (JSON-RPC for control, Text for benchmarks).
