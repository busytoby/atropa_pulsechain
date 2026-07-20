#include <string.h>
#include "tsfi_parc_figma_vector.h"

// Helper: draw simple line segment
static void draw_line_internal(uint32_t *pixels, int width, int height, int x0, int y0, int x1, int y1, uint32_t color) {
    int dx = (x1 - x0) > 0 ? (x1 - x0) : -(x1 - x0);
    int dy = (y1 - y0) > 0 ? (y1 - y0) : -(y1 - y0);
    int sx = x0 < x1 ? 1 : -1;
    int sy = y0 < y1 ? 1 : -1;
    int err = dx - dy;

    while (1) {
        if (x0 >= 0 && x0 < width && y0 >= 0 && y0 < height) {
            pixels[y0 * width + x0] = color;
        }
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }
}

void tsfi_parc_figma_draw_bezier(
    uint32_t *pixels, int width, int height,
    float x0, float y0, float x1, float y1,
    float x2, float y2, float x3, float y3,
    uint32_t color
) {
    if (!pixels || width <= 0 || height <= 0) return;

    float prev_x = x0;
    float prev_y = y0;

    // Evaluate cubic Bezier curve at 32 step intervals
    for (int step = 1; step <= 32; step++) {
        float t = (float)step / 32.0f;
        float mt = 1.0f - t;

        float c0 = mt * mt * mt;
        float c1 = 3.0f * mt * mt * t;
        float c2 = 3.0f * mt * t * t;
        float c3 = t * t * t;

        float curr_x = c0 * x0 + c1 * x1 + c2 * x2 + c3 * x3;
        float curr_y = c0 * y0 + c1 * y1 + c2 * y2 + c3 * y3;

        draw_line_internal(pixels, width, height, (int)prev_x, (int)prev_y, (int)curr_x, (int)curr_y, color);
        
        prev_x = curr_x;
        prev_y = curr_y;
    }
}

void tsfi_parc_figma_apply_mask(
    uint32_t *pixels, int width, int height,
    const uint8_t *mask_stencil, int mask_w, int mask_h,
    int mask_x, int mask_y
) {
    if (!pixels || !mask_stencil || width <= 0 || height <= 0 || mask_w <= 0 || mask_h <= 0) return;

    for (int y = 0; y < mask_h; y++) {
        int cur_y = mask_y + y;
        if (cur_y < 0 || cur_y >= height) continue;

        for (int x = 0; x < mask_w; x++) {
            int cur_x = mask_x + x;
            if (cur_x < 0 || cur_x >= width) continue;

            uint8_t alpha = mask_stencil[y * mask_w + x];
            int idx = cur_y * width + cur_x;
            uint32_t pix = pixels[idx];

            // Blend color components with mask stencil alpha level
            int r = (int)(((pix >> 16) & 0xFF) * (alpha / 255.0f));
            int g = (int)(((pix >> 8) & 0xFF) * (alpha / 255.0f));
            int b = (int)((pix & 0xFF) * (alpha / 255.0f));

            pixels[idx] = (pix & 0xFF000000) | (r << 16) | (g << 8) | b;
        }
    }
}

void tsfi_parc_figma_apply_overrides(
    uint32_t *color, char *text,
    const tsfi_parc_figma_override_t *overrides
) {
    if (!overrides) return;

    if (overrides->has_color && color) {
        *color = overrides->color_override;
    }
    if (overrides->has_text && text) {
        strncpy(text, overrides->text_override, 31);
        text[31] = '\0';
    }
}
