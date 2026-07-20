#include "tsfi_parc_runlength.h"
#include "tsfi_paint.h"

int tsfi_parc_render_styled_text(uint32_t *pixels, int w, int h, int x, int y, const char *text, const tsfi_parc_style_run_t *style, float size) {
    if (!pixels || !text || !style || w <= 0 || h <= 0) return -1;

    // 1. Draw Shadow (offset by +1 pixel)
    if (style->is_shadow) {
        tsfi_quantel_paintbox_typographer(pixels, w, h, x + 1, y + 1, text, 0xFF050403, size);
    }

    // 2. Draw Base Text
    tsfi_quantel_paintbox_typographer(pixels, w, h, x, y, text, style->color, size);

    // 3. Draw Bold (offset by +1 pixel horizontally in same color)
    if (style->is_bold) {
        tsfi_quantel_paintbox_typographer(pixels, w, h, x + 1, y, text, style->color, size);
    }

    // 4. Draw Underline (horizontal line at the bottom of character bounds)
    if (style->is_underline) {
        int line_y = y + (int)(size * 0.2f);
        if (line_y >= 0 && line_y < h) {
            int line_w = (int)(size * 0.6f * (float)__builtin_strlen(text)); // approximate width
            for (int lx = x; lx < x + line_w; lx++) {
                if (lx >= 0 && lx < w) {
                    pixels[line_y * w + lx] = style->color;
                }
            }
        }
    }

    return 0;
}
