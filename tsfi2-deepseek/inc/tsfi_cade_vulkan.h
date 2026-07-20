#ifndef TSFI_CADE_VULKAN_H
#define TSFI_CADE_VULKAN_H

#include <stdint.h>
#include "tsfi_cade_imf.h"

typedef struct {
    uint32_t active_taxpayers;
    float current_fps;
    char live_terminal_log[2048];
} CadeVulkanDashboardState;

/* Draw the live CADE control panel dashboard onto the pixel buffer */
int tsfi_cade_draw_dashboard_panel(uint32_t *pixels, int w, int h, CadeVulkanDashboardState *state, float t);

#endif // TSFI_CADE_VULKAN_H
