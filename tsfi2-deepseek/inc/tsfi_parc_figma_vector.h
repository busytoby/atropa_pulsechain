#ifndef TSFI_PARC_FIGMA_VECTOR_H
#define TSFI_PARC_FIGMA_VECTOR_H

#include <stdint.h>

typedef struct {
    uint32_t color_override;
    char text_override[32];
    int has_color;
    int has_text;
} tsfi_parc_figma_override_t;

/* 1. Figma Cubic Bezier Vector Curve Rasterizer: Uses De Casteljau subdivision to draw smooth curves */
void tsfi_parc_figma_draw_bezier(
    uint32_t *pixels, int width, int height,
    float x0, float y0, float x1, float y1,
    float x2, float y2, float x3, float y3,
    uint32_t color
);

/* 2. Figma Layer Masking Engine: Clips canvas drawing region using an 8-bit alpha mask stencil */
void tsfi_parc_figma_apply_mask(
    uint32_t *pixels, int width, int height,
    const uint8_t *mask_stencil, int mask_w, int mask_h,
    int mask_x, int mask_y
);

/* 3. Figma Component Instance Override Tracker: Applies local design property overrides onto template assets */
void tsfi_parc_figma_apply_overrides(
    uint32_t *color, char *text,
    const tsfi_parc_figma_override_t *overrides
);

#endif // TSFI_PARC_FIGMA_VECTOR_H
