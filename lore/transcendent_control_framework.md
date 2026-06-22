# Transcendent Control Framework Architecture

The **Transcendent Control Framework** is a unified execution architecture designed to interface low-overhead hardware interfaces, virtualized display compositors, and on-chain state transitions. It facilitates CPU input injection straight to the query fields and filters of nested sub-sessions.

```mermaid
graph TD
    UserInterface["Dashboard SVG Overlay (Browser-in-Browser)"] -->|Event: click / move / scroll| ControlEndpoint["HTTP Server POST /control"]
    ControlEndpoint -->|Encode as hex transaction| WinchesterMQ["WinchesterMQ Yul Contract (Anvil EVM)"]
    WinchesterMQ -->|LogPut / Fast Log Event| BrowserController["Rooted Browser Controller (Node.js/Puppeteer)"]
    BrowserController -->|Input Injection| ChromeBrowser["System Google Chrome (Headless)"]
    ChromeBrowser -->|Screenshot Frame Capture| ImageFrame["latest_frame.jpg"]
    ImageFrame -->|Reload Frame Loop| UserInterface
    
    subgraph Presentation Pipeline
        BrowserController -->|STDIN JPEG Stream| VulkanPresenter["Vulkan / Wayland Presenter (C executable)"]
        VulkanPresenter -->|Decode JPEG to p72 plane| VulkanCompositor["Vulkan Queue Present (tsfi_vulkan_cells)"]
        VulkanCompositor -->|Alpha blend to scanout_buffer| WaylandCompositor["Wayland Surface Attach / Commit"]
    end
```

---

## 1. Yul On-Chain Logic
The underlying communication backbone relies on contract components written in **Yul** (strict assembly) to bypass standard Solidity compiler overhead. Yul allows precise memory offsets and zero-overhead execution for event emission.

* **Target Selector (`0xccb077a0`)**: Fast-path transactions write directly to the hardware-mapped data port.
* **Low Gas Dispatches**: By avoiding high-level EVM abstractions, input routing transactions are executed with minimal gas, bypassing typical SCSI handshakes and allowing high-frequency input updates (up to 60 events/second).

---

## 2. WinchesterMQ (MQ) Messaging Queue
WinchesterMQ acts as the on-chain message broker. Input commands (e.g., mouse movement coordinates, keycodes, clicks, and page scrolls) are dispatched as transaction data payloads targeting the WinchesterMQ deployment address:

* **Handshake Mode**: Larger command buffers are stored in contract storage slots starting at offset `0x1000 + blockId` and emitted via `LogPut` events.
* **Fast Log Mode**: Emits events using a dedicated log topic hash (`0xe1dae1da...`) containing the exact ASCII representations of the input command in the transaction body.
* **Listener Module**: The `rooted_browser_controller.js` listens to logs matching these topics, extracts the transaction inputs, and dynamically routes them to the corresponding interactive sub-sessions.

---

## 3. Browser-in-Browser (Picture-in-Picture) Routing
To achieve seamless nesting without layout conflicts, the dashboard uses a **Browser-in-Browser (BiB)** overlay mechanism:

* **Frame Capture**: Headless Puppeteer browser sub-sessions capture screenshot frames at 75% JPEG quality.
* **Layout Streaming**: The frames are written to a shared directory (`frontend/latest_frame.jpg`) and reloaded dynamically inside the SVG viewport of the parent dashboard page.
* **SVG Interactive Overlay**: An SVG overlay intercepts mouse clicks and movements on the client interface and maps local coordinate boxes to Puppeteer viewport coordinates. The mapped events are forwarded back as WinchesterMQ messages.

---

## 4. Vulkan Virtual Planes
For low-level hardware rendering, the presentation pipeline uses **Auncient** DRM/ZMM virtual planes:

* **Virtual Plane Allocation**: Registers a virtual display plane (ID `72`) using `tsfi_drmModeAddPlane`.
* **JPEG Decoder Routing**: The STDIN-piped screenshots are decoded into the raw virtual plane buffer `p72` (`tsfi_drmModeGetVirtualPlaneBuffer(72)`).
* **Mock Presentation**: Calling `vkQueuePresentKHR` initiates a mock rendering queue. The Vulkan cells compositor performs alpha blending of all registered active planes onto the final `g_scanout_buffer` background.

---

## 5. Wayland Client Interface
The compositor windowing uses Wayland protocols to remain lightweight and platform-independent:

* **Configure Resizing**: The presenter handles `xdg_toplevel` configure events. When the compositor triggers a resize, the presenter dynamically destroys and reconstructs the shared memory (`wl_shm_pool`) buffer (`wl_buffer`) to match the new dimensions.
* **Stability Safeguard**: Re-creation only occurs if the configure dimensions differ from the current layout (`width != g_w || height != g_h`). This prevents compositor crashes due to race conditions during configure acknowledge (`xdg_surface.ack_configure`) sequences.
* **Buffer Allocation Limits**: The shared memory pool is dynamically sized up to `3840 * 2160 * 4` bytes, ensuring safe buffer allocation on displays up to 4K.
