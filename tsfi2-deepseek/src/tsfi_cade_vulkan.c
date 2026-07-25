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

    // Viewport 3 (Right): MacWilliams Transistor Gating Matrix representation
    tsfi_quantel_paintbox_typographer(pixels, w, h, separator2_x + 10, start_y + 12, "GATING MATRIX ROUTER", 0xFFe6dfd3, 8.0f);
    
    // Draw 4x4 matrix grid of transistor junction indicators
    int matrix_start_x = separator2_x + 45;
    int matrix_start_y = start_y + 35;
    int cell_spacing = 15;
    
    // Label input/output lines
    tsfi_quantel_paintbox_typographer(pixels, w, h, separator2_x + 10, matrix_start_y, "IN", 0xFF8c7241, 6.0f);
    tsfi_quantel_paintbox_typographer(pixels, w, h, matrix_start_x, start_y + 24, "OUT: 0  1  2  3", 0xFF8c7241, 6.0f);

    for (int in_idx = 0; in_idx < 4; in_idx++) {
        char in_lbl[8];
        snprintf(in_lbl, sizeof(in_lbl), "%d", in_idx);
        tsfi_quantel_paintbox_typographer(pixels, w, h, separator2_x + 28, matrix_start_y + in_idx * cell_spacing, in_lbl, 0xFF8c7241, 6.0f);
        
        for (int out_idx = 0; out_idx < 4; out_idx++) {
            int cx = matrix_start_x + out_idx * cell_spacing;
            int cy = matrix_start_y + in_idx * cell_spacing;
            
            // Highlight active paths dynamically based on time t (simulate dynamic routes)
            bool gate_active = false;
            if (in_idx == 0 && out_idx == 1) gate_active = true; // Noise -> Filter
            if (in_idx == 2 && out_idx == 3) gate_active = true; // Excitation -> Envelope
            if (in_idx == 2 && out_idx == 1) gate_active = ((int)t % 2 == 0); // Dynamic path
            
            uint32_t cell_color = gate_active ? 0xFF00ffc8 : 0xFFff3c64; // Neon Cyan for open, Red for closed
            
            // Draw a solid 5x5 pixel block for the transistor junction
            for (int dy = -2; dy <= 2; dy++) {
                for (int dx = -2; dx <= 2; dx++) {
                    int px = cx + dx;
                    int py = cy + dy;
                    if (px >= 0 && px < w && py >= 0 && py < h) {
                        pixels[py * w + px] = cell_color;
                    }
                }
            }
        }
    }

    return 0;
}
