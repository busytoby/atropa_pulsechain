# Auncient Window Registry

This registry tracks the active windows initiated by the system and their respective execution states.

| Process / Window Name | Window Type | Visual State | Purpose |
| :--- | :--- | :--- | :--- |
| `rooted_frame_presenter` | Vulkan/Wayland Window | **Headed** (Visible) | The main Auncient Vulkan compositing container. Active instance running under task ID `task-10791`. |
| Lore Editor Browser | Google Chrome (Puppeteer) | **Headless** (Hidden) | Renders `lore_editor.html` in the background and pipes frames to the Vulkan presenter. |
| Dashboard Browser | Google Chrome (Puppeteer) | **Headless** (Hidden) | Renders `atropa_dashboard.html` in the background and pipes frames to the Vulkan presenter. |
| YouTube Sub-Browser | Google Chrome (Puppeteer) | **Headless** (Hidden) | Loads and controls YouTube videos dynamically inside the dashboard frame. |
| `tsfi2` Compositor | Vulkan/Wayland Window | **Headed** (Visible) | Custom display compositor socket `wayland-tsfi` for rendering experimental visual demos. |
| Demoscene Firefox | Firefox-ESR (Wayland client) | **Headed** (Visible) | Renders `teddy_bear_tournament_3d.html` showing the complex 4D Tesseract and 3D fighter bears. |
| `tsfi_vulkan_guide` | Vulkan/Wayland Window | **Headed** (Visible) | Standalone P. J. Brown Guide Lore & Docs app presenting formatted markdown documents on Wayland. |

