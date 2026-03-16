// TSFi Vulkan Plugin - Unity Build Wrapper
// This file aggregates all modular components into a single compilation unit
// for the hotloader, ensuring all symbols are resolved and logic is unified.

#include "tsfi_staging.c"
#include "vulkan_init.c"

#include "vulkan/vulkan_input.c"
#include "vulkan/vulkan_dmabuf.c"
#include "vulkan/vulkan_xdg.c"
#include "vulkan/vulkan_render.c"
#include "vulkan/vulkan_logic.c"
#include "vulkan/backend_display.c"
#include "vulkan/input_vty.c"
#include "vulkan/vulkan_main.c"