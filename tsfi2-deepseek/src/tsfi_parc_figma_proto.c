#include <stdio.h>
#include <string.h>
#include "tsfi_parc_figma_proto.h"

void tsfi_parc_figma_smart_animate(
    const tsfi_parc_layout_item_t *start,
    const tsfi_parc_layout_item_t *end,
    float progress,
    tsfi_parc_layout_item_t *out_interpolated
) {
    if (!start || !end || !out_interpolated) return;

    if (progress < 0.0f) progress = 0.0f;
    if (progress > 1.0f) progress = 1.0f;

    out_interpolated->x = start->x + (int)((end->x - start->x) * progress);
    out_interpolated->y = start->y + (int)((end->y - start->y) * progress);
    out_interpolated->w = start->w + (int)((end->w - start->w) * progress);
    out_interpolated->h = start->h + (int)((end->h - start->h) * progress);
}

void tsfi_parc_figma_to_css(
    const tsfi_parc_layout_item_t *item,
    float corner_radius,
    uint32_t color,
    char *out_css,
    int max_len
) {
    if (!item || !out_css || max_len <= 0) return;

    int r = (color >> 16) & 0xFF;
    int g = (color >> 8) & 0xFF;
    int b = color & 0xFF;
    float a = ((color >> 24) & 0xFF) / 255.0f;
    if (a == 0.0f && color <= 0x00FFFFFF) {
        a = 1.0f; // opaque fallback if alpha not set
    }

    snprintf(out_css, max_len,
             "/* Figma Code Export */\n"
             "position: absolute;\n"
             "width: %dpx;\n"
             "height: %dpx;\n"
             "left: %dpx;\n"
             "top: %dpx;\n"
             "border-radius: %.1fpx;\n"
             "background: rgba(%d, %d, %d, %.2f);\n",
             item->w, item->h, item->x, item->y, corner_radius, r, g, b, a);
}

void tsfi_parc_figma_drop_shadow(
    uint32_t *pixels, int width, int height,
    const tsfi_parc_layout_item_t *item,
    int offset_x, int offset_y,
    int blur_radius,
    uint32_t shadow_color
) {
    if (!pixels || !item || width <= 0 || height <= 0) return;
    (void)blur_radius;

    // Draw offset boundary shadow representation
    int sx = item->x + offset_x;
    int sy = item->y + offset_y;

    for (int y = sy; y < sy + item->h; y++) {
        for (int x = sx; x < sx + item->w; x++) {
            if (x >= 0 && x < width && y >= 0 && y < height) {
                // Alpha blend shadow color with existing base pixel
                uint32_t base_pix = pixels[y * width + x];
                
                int br = (base_pix >> 16) & 0xFF;
                int bg = (base_pix >> 8) & 0xFF;
                int bb = base_pix & 0xFF;

                int sr = (shadow_color >> 16) & 0xFF;
                int sg = (shadow_color >> 8) & 0xFF;
                int sb = shadow_color & 0xFF;

                // Simple 50% opacity blend
                int out_r = (br + sr) / 2;
                int out_g = (bg + sg) / 2;
                int out_b = (bb + sb) / 2;

                pixels[y * width + x] = (base_pix & 0xFF000000) | (out_r << 16) | (out_g << 8) | out_b;
            }
        }
    }
}
