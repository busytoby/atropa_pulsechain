#include "tsfi_parc_video.h"

int tsfi_parc_video_init(tsfi_parc_video_controller_t *vc) {
    if (!vc) return -1;
    vc->cursor_x = 0;
    vc->cursor_y = 0;
    vc->scan_line = 0;
    vc->vertical_interrupt_triggered = 0;
    return 0;
}

int tsfi_parc_video_step(tsfi_parc_video_controller_t *vc, int mx, int my) {
    if (!vc) return -1;

    // Simulate vertical scan lines up to classic Alto portrait resolution (808 lines)
    vc->scan_line = (vc->scan_line + 1) % 808;
    
    // Trigger vertical sync interrupt at start of blanking period (line 800)
    if (vc->scan_line == 800) {
        vc->vertical_interrupt_triggered = 1;
    } else {
        vc->vertical_interrupt_triggered = 0;
    }

    // Hardware cursor registers tracking (clamped to 606x808 resolution bounds)
    vc->cursor_x = (uint16_t)(mx < 0 ? 0 : (mx > 606 ? 606 : mx));
    vc->cursor_y = (uint16_t)(my < 0 ? 0 : (my > 808 ? 808 : my));
    return 0;
}

int tsfi_parc_video_render_frame(
    const tsfi_parc_video_controller_t *vc,
    const uint16_t *display_mem,
    uint32_t *dest_pixels,
    int dest_w, int dest_h,
    int offset_x, int offset_y,
    uint32_t phosphor_color,
    uint32_t cursor_color,
    const uint16_t *cursor_bitmap
) {
    if (!vc || !display_mem || !dest_pixels || dest_w <= 0 || dest_h <= 0) return -1;

    // Dimmed scanline color
    uint32_t r = (phosphor_color >> 16) & 0xFF;
    uint32_t g = (phosphor_color >> 8) & 0xFF;
    uint32_t b = phosphor_color & 0xFF;
    uint32_t scanline_color = (0xFF000000) |
                              (((r * 3) / 4) << 16) |
                              (((g * 3) / 4) << 8) |
                              ((b * 3) / 4);

    for (int y = 0; y < 808; y++) {
        int dy = offset_y + y;
        if (dy < 0 || dy >= dest_h) continue;

        int is_scanline = (y % 2 == 0);
        uint32_t base_color = is_scanline ? scanline_color : phosphor_color;

        for (int x = 0; x < 606; x++) {
            int dx = offset_x + x;
            if (dx < 0 || dx >= dest_w) continue;

            // Extract pixel bit from 16-bit word array (38 words per line)
            int word_idx = y * 38 + (x / 16);
            int bit_idx = 15 - (x % 16);
            int pixel_bit = (display_mem[word_idx] >> bit_idx) & 1;

            uint32_t pixel_val = pixel_bit ? base_color : 0xFF050403; // dark background

            // Hardware cursor plane mixin
            if (cursor_bitmap &&
                x >= vc->cursor_x && x < vc->cursor_x + 16 &&
                y >= vc->cursor_y && y < vc->cursor_y + 16) {
                int cx = x - vc->cursor_x;
                int cy = y - vc->cursor_y;
                int cursor_bit = (cursor_bitmap[cy] >> (15 - cx)) & 1;
                if (cursor_bit) {
                    pixel_val = cursor_color;
                }
            }

            dest_pixels[dy * dest_w + dx] = pixel_val;
        }
    }
    return 0;
}
