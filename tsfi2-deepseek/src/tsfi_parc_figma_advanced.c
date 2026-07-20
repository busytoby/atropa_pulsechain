#include <string.h>
#include "tsfi_parc_figma_advanced.h"

void tsfi_parc_figma_blend_layer(uint32_t *base, const uint32_t *blend, int width, int height, int mode) {
    if (!base || !blend || width <= 0 || height <= 0) return;

    for (int i = 0; i < width * height; i++) {
        uint32_t b_pix = base[i];
        uint32_t l_pix = blend[i];

        int br = (b_pix >> 16) & 0xFF;
        int bg = (b_pix >> 8) & 0xFF;
        int bb = b_pix & 0xFF;

        int lr = (l_pix >> 16) & 0xFF;
        int lg = (l_pix >> 8) & 0xFF;
        int lb = l_pix & 0xFF;

        int out_r = br;
        int out_g = bg;
        int out_b = bb;

        switch (mode) {
            case BLEND_MULTIPLY:
                out_r = (br * lr) / 255;
                out_g = (bg * lg) / 255;
                out_b = (bb * lb) / 255;
                break;
            case BLEND_SCREEN:
                out_r = 255 - ((255 - br) * (255 - lr)) / 255;
                out_g = 255 - ((255 - bg) * (255 - lg)) / 255;
                out_b = 255 - ((255 - bb) * (255 - lb)) / 255;
                break;
            case BLEND_OVERLAY:
                out_r = (br < 128) ? (2 * br * lr) / 255 : 255 - 2 * (255 - br) * (255 - lr) / 255;
                out_g = (bg < 128) ? (2 * bg * lg) / 255 : 255 - 2 * (255 - bg) * (255 - lg) / 255;
                out_b = (bb < 128) ? (2 * bb * lb) / 255 : 255 - 2 * (255 - bb) * (255 - lb) / 255;
                break;
            case BLEND_COLORDODGE:
                out_r = (lr == 255) ? 255 : (br * 255) / (255 - lr);
                out_g = (lg == 255) ? 255 : (bg * 255) / (255 - lg);
                out_b = (lb == 255) ? 255 : (bb * 255) / (255 - lb);
                break;
        }

        out_r = out_r > 255 ? 255 : out_r;
        out_g = out_g > 255 ? 255 : out_g;
        out_b = out_b > 255 ? 255 : out_b;

        base[i] = (b_pix & 0xFF000000) | (out_r << 16) | (out_g << 8) | out_b;
    }
}

void tsfi_parc_figma_variant_resolve(const char *variant_props, int *w_out, int *h_out, uint32_t *color_out) {
    if (!variant_props || !w_out || !h_out || !color_out) return;

    // Default variant options
    *w_out = 100;
    *h_out = 40;
    *color_out = 0xFFCCCCCC; // light gray

    if (strstr(variant_props, "state=hover")) {
        *color_out = 0xFF4A90E2; // blue highlight
        *w_out = 105;
        *h_out = 42;
    } else if (strstr(variant_props, "state=active")) {
        *color_out = 0xFF357ABD; // dark blue active
        *w_out = 100;
        *h_out = 40;
    }
}

void tsfi_parc_figma_vector_boolean_union(const tsfi_parc_layout_item_t *a, const tsfi_parc_layout_item_t *b, tsfi_parc_layout_item_t *out_result) {
    if (!a || !b || !out_result) return;

    int min_x = a->x < b->x ? a->x : b->x;
    int min_y = a->y < b->y ? a->y : b->y;
    int max_x = (a->x + a->w) > (b->x + b->w) ? (a->x + a->w) : (b->x + b->w);
    int max_y = (a->y + a->h) > (b->y + b->h) ? (a->y + a->h) : (b->y + b->h);

    out_result->x = min_x;
    out_result->y = min_y;
    out_result->w = max_x - min_x;
    out_result->h = max_y - min_y;
}
