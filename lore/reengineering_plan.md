# Atropa & Z-Machine Re-Engineering Plan

This document outlines the updates, alignments, and integrations completed to connect the rooted browser controller, Atropa Vulkan Dashboard, and the Z-Machine Console under a unified theme and functional framework.

---

## Completed Implementations

### Phase 1: Interactive SVG Video Streaming
- [x] Migrated the Atropa Console's stream element from a static HTML `<img>` tag to a responsive `#browserStreamSvg` canvas.
- [x] Integrated a real-time double helix DNA path animation running along the margins of the video frame.
- [x] Added digital corner brackets, cybernetic grids, and a live telemetry readout displaying the active WinchesterMQ port and fluctuating DNA sequences.
- [x] Unified coordinate mappings in the mouse event handlers to calculate pointer movements relative to the responsive SVG bounds.

### Phase 2: Z-Machine Picture-in-Picture Stream
- [x] Integrated a togglable floating `#pipContainer` overlay in the bottom-right corner of the Z-Machine fiction terminal card.
- [x] Styled the PiP frame with the sepia-gold Vaesen palette to blend with the text adventure layout.
- [x] Implemented a background image refresh loop polling `latest_frame.jpg` every 300ms.
- [x] Added a header control button to toggle the stream frame visibility dynamically.

### Phase 3: Atropa Console Theme Alignment (Vaesen Ink & Cream)
- [x] Upgraded [console.html](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/frontend/console.html) with a custom style sheet implementing the Nordic gothic Vaesen theme.
- [x] Set backgrounds to `vaesen-ink` (`#0d0c0a`) and body text to `vaesen-cream` (`#e6dfd3`).
- [x] Styled input blocks, dropdown menus, and cards with `vaesen-parchment` (`#25201b`) backgrounds, dashed inner borders, and tarnished gold accents.
- [x] Swapped font assets to **Alice** for headers and **Lora** for body copy.

### Phase 4: Headless Sub-Browser Mode
- [x] Modified [rooted_browser_controller.js](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/scripts/rooted_browser_controller.js) to detect YouTube URLs and run in headless frame-dumper mode, bypassing wayland surface presentation.
- [x] Increased screenshot writing frequency in headless mode to 50ms for smooth stream output.
- [x] Configured [server.py](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/server.py) to target only YouTube child browser processes during cleanup, preserving the parent presenter window.
- [x] Programmed the controller to always auto-start the background Python server on boot.

---

## Future Roadmap & Milestones

### Milestone 1: WinchesterMQ Command Relaying
* **Goal**: Enable direct command triggers from the Z-Machine text terminal to control background browser navigation.
* **Scope**:
  * Parse Z-Machine transaction logs to identify player inputs.
  * Map input strings (e.g. `USE TERMINAL`) to Puppeteer browser navigation actions.
  * Verify keystroke injection over WinchesterMQ Yul contract ports.

### Milestone 2: Unified Design System Audit
* **Goal**: Audit the entire suite of 20+ dashboards to enforce terminology constraints (`ancient` $\rightarrow$ `Auncient`) and color system consistency.
* **Scope**:
  * Run a forensic search on files for terminology formatting.
  * Establish a global stylesheet variable file containing the Vaesen color palette.
