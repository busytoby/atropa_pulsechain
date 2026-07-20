#include "tsfi_parc_bravo.h"
#include "tsfi_paint.h"

int tsfi_parc_bravo_init(tsfi_parc_bravo_layout_t *layout, int ml, int mr, int mt, int mb) {
    if (!layout) return -1;
    if (ml < 0 || mr < 0 || mt < 0 || mb < 0) return -2;
    layout->margin_left = ml;
    layout->margin_right = mr;
    layout->margin_top = mt;
    layout->margin_bottom = mb;
    layout->current_column = 0;
    return 0;
}

int tsfi_parc_bravo_draw_header(uint32_t *pixels, int w, int h, tsfi_parc_bravo_layout_t *layout, const char *title, uint32_t color) {
    if (!pixels || !layout || w <= 0 || h <= 0) return -1;
    if (layout->margin_left < 0 || layout->margin_left >= w || layout->margin_top < 12 || layout->margin_top >= h) return -2;
    // Draw WYSIWYG title right at the margin top position
    tsfi_quantel_paintbox_typographer(pixels, w, h, layout->margin_left, layout->margin_top - 12, title, color, 8.0f);
    return 0;
}

int tsfi_parc_bravo_draw_guides(uint32_t *pixels, int w, int h, tsfi_parc_bravo_layout_t *layout, uint32_t guide_color) {
    if (!pixels || !layout || w <= 0 || h <= 0) return -1;

    // Draw dashed layout lines on the margin boundaries
    // Left boundary
    for (int y = layout->margin_top; y < h - layout->margin_bottom; y++) {
        if ((y / 4) % 2 == 0) {
            if (layout->margin_left >= 0 && layout->margin_left < w && y >= 0 && y < h) {
                pixels[y * w + layout->margin_left] = guide_color;
            }
            int right_x = w - layout->margin_right;
            if (right_x >= 0 && right_x < w && y >= 0 && y < h) {
                pixels[y * w + right_x] = guide_color;
            }
        }
    }

    // Top and bottom boundaries
    for (int x = layout->margin_left; x < w - layout->margin_right; x++) {
        if ((x / 4) % 2 == 0) {
            if (x >= 0 && x < w && layout->margin_top >= 0 && layout->margin_top < h) {
                pixels[layout->margin_top * w + x] = guide_color;
            }
            int bottom_y = h - layout->margin_bottom;
            if (x >= 0 && x < w && bottom_y >= 0 && bottom_y < h) {
                pixels[bottom_y * w + x] = guide_color;
            }
        }
    }

    return 0;
}
