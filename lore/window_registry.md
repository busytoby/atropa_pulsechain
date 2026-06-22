# Auncient Window Registry

This registry tracks the active windows initiated by the system and their respective execution states.

| Process / Window Name | Window Type | Visual State | Purpose |
| :--- | :--- | :--- | :--- |
| `rooted_frame_presenter` | Vulkan/Wayland Window | **Headed** (Visible) | The main Auncient Vulkan compositing container. Renders the unified console workspace on the host display. |
| Lore Editor Browser | Headless Native Browser | **Headless** (Hidden) | Renders `lore_editor.html` in the background and pipes frames to the Vulkan presenter. |
| Dashboard Browser | Headless Native Browser | **Headless** (Hidden) | Renders `atropa_dashboard.html` in the background and pipes frames to the Vulkan presenter. |
| YouTube Sub-Browser | Headless Native Browser | **Headless** (Hidden) | Loads and controls YouTube videos dynamically inside the dashboard frame. |
