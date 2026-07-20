#include <stdio.h>
#include <string.h>
#include <math.h>
#include "tsfi_cade_vulkan.h"
#include "tsfi_paint.h"

int tsfi_cade_draw_dashboard_panel(uint32_t *pixels, int w, int h, CadeVulkanDashboardState *state, float t) {
    if (!pixels || !state || w <= 0 || h <= 0) return -1;

    // Define dashboard viewport dimensions: positioned in the bottom telemetry area
    int start_y = 398;
    int end_y = 508;
    int start_x = 35;
    int end_x = w - 35;

    // Clear bottom area with dark phosphor background
    for (int y = start_y; y <= end_y; y++) {
        for (int x = start_x; x <= end_x; x++) {
            pixels[y * w + x] = 0xFF0d0c0a; // Phosphor dark background
        }
    }

    // Outer screen bezel
    for (int x = start_x; x <= end_x; x++) {
        pixels[start_y * w + x] = 0xFF8c7241;
        pixels[end_y * w + x] = 0xFF8c7241;
    }
    for (int y = start_y; y <= end_y; y++) {
        pixels[y * w + start_x] = 0xFF8c7241;
        pixels[y * w + end_x] = 0xFF8c7241;
    }

    // Draw viewport borders separating Left, Center, and Right panels
    int separator1_x = start_x + 140;
    int separator2_x = start_x + 300;
    for (int y = start_y; y <= end_y; y++) {
        pixels[y * w + separator1_x] = 0xFF8c7241;
        pixels[y * w + separator2_x] = 0xFF8c7241;
    }

    // Viewport 1 (Left): Taxpayer Registry & DEFCON Stats
    tsfi_quantel_paintbox_typographer(pixels, w, h, start_x + 5, start_y + 12, "TAXPAYER REGISTRY", 0xFFe6dfd3, 8.0f);
    char buf[128];
    snprintf(buf, sizeof(buf), "ACTIVE: %u", state->active_taxpayers);
    tsfi_quantel_paintbox_typographer(pixels, w, h, start_x + 5, start_y + 24, buf, 0xFFc5a059, 7.5f);
    
    char reg_data[256];
    tsfi_mf_cade_register_taxpayer("999-12-3456", 104000.50, 1, reg_data, sizeof(reg_data));
    snprintf(buf, sizeof(buf), "SSN: 999-12-3456");
    tsfi_quantel_paintbox_typographer(pixels, w, h, start_x + 5, start_y + 36, buf, 0xFFc5a059, 7.0f);
    
    double val_agi = 50000.0 + 1000.0 * sinf(t);
    int agi_match = 0;
    tsfi_mf_cade_verify_prior_year_agi(val_agi, 50000.0, &agi_match);
    snprintf(buf, sizeof(buf), "AGI: %.1f | VER:%s", val_agi, agi_match ? "OK" : "NO");
    tsfi_quantel_paintbox_typographer(pixels, w, h, start_x + 5, start_y + 48, buf, 0xFFc5a059, 7.0f);

    snprintf(buf, sizeof(buf), "FPS: %.1f | DEFCON 1", state->current_fps);
    tsfi_quantel_paintbox_typographer(pixels, w, h, start_x + 5, start_y + 60, buf, 0xFF8c7241, 7.0f);

    // Viewport 2 (Center): CICS Terminal 42 Session Log
    tsfi_quantel_paintbox_typographer(pixels, w, h, separator1_x + 10, start_y + 12, "CICS TERM42 SESSION LOG", 0xFFe6dfd3, 8.0f);
    
    // Split the terminal log into separate lines
    char log_copy[2048];
    strncpy(log_copy, state->live_terminal_log, sizeof(log_copy) - 1);
    log_copy[sizeof(log_copy) - 1] = '\0';
    
    char *line = strtok(log_copy, "\n");
    int line_y = start_y + 24;
    while (line && line_y < end_y - 10) {
        tsfi_quantel_paintbox_typographer(pixels, w, h, separator1_x + 10, line_y, line, 0xFFc5a059, 6.5f);
        line_y += 10;
        line = strtok(NULL, "\n");
    }

    // Viewport 3 (Right): Real-time Transaction Graphs
    tsfi_quantel_paintbox_typographer(pixels, w, h, separator2_x + 10, start_y + 12, "DAILY BATCH GRAPH", 0xFFe6dfd3, 8.0f);

    // Draw dynamic bezier curve representing transaction daily batch volumes
    float p0_x = separator2_x + 15;
    float p0_y = end_y - 15;
    float p1_x = separator2_x + 45;
    float p1_y = start_y + 20 + 10.0f * sinf(t * 2.0f);
    float p2_x = separator2_x + 85;
    float p2_y = start_y + 20 + 10.0f * cosf(t * 3.0f);
    float p3_x = end_x - 15;
    float p3_y = end_y - 15;

    // Draw a smooth gold spline using the paintbox bezier stroke pipeline
    tsfi_quantel_paintbox_bezier_stroke(pixels, w, h, p0_x, p0_y, p1_x, p1_y, p2_x, p2_y, p3_x, p3_y, 2, 0.8f, 0xFFc5a059);

    return 0;
}
