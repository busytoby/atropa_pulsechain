# Auncient Window Registry

This registry tracks the active windows initiated by the system and their respective execution states.

| Process / Window Name | Window Type | Visual State | Purpose |
| :--- | :--- | :--- | :--- |
| `tsfi2` | Vulkan/Wayland Window | **Headed** (Visible) | The Auncient Vulkan compositing container displaying on socket `wayland-tsfi`. |
| Lore Editor Browser (Firefox) | Headed Native Browser | **Headed** (Visible) | Renders `lore_editor.html` displaying the Tessarant MQ document (`tessarant_combat_physics.md`). |
| Dashboard Browser | Headless Native Browser | **Headless** (Hidden) | Renders `atropa_dashboard.html` in the background and pipes frames to the Vulkan presenter. |
| YouTube Sub-Browser | Headless Native Browser | **Headless** (Hidden) | Loads and controls YouTube videos dynamically inside the dashboard frame. |
| Witches' Sabbath Emulation Console (Vaesen) | Vulkan/Wayland Window | **Headed** (Visible) | Runs the B128 console with the Witches' Sabbath (Vaesen) game visualizer loop in the native Firefox/Wayland browser. |
