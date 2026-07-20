#ifndef TSFI_PARC_FIGMA_PROTO_H
#define TSFI_PARC_FIGMA_PROTO_H

#include <stdint.h>
#include "tsfi_parc_figma.h"

/* 1. Figma Smart Animate Engine: Interpolates coordinates between start and end states based on progress [0.0, 1.0] */
void tsfi_parc_figma_smart_animate(
    const tsfi_parc_layout_item_t *start,
    const tsfi_parc_layout_item_t *end,
    float progress,
    tsfi_parc_layout_item_t *out_interpolated
);

/* 2. Figma CSS Exporter: Generates copy-pasteable CSS string for a design element */
void tsfi_parc_figma_to_css(
    const tsfi_parc_layout_item_t *item,
    float corner_radius,
    uint32_t color,
    char *out_css,
    int max_len
);

/* 3. Figma Drop Shadow Filter: Draws a blurred alpha offset shadow under a layout box */
void tsfi_parc_figma_drop_shadow(
    uint32_t *pixels, int width, int height,
    const tsfi_parc_layout_item_t *item,
    int offset_x, int offset_y,
    int blur_radius,
    uint32_t shadow_color
);

#endif // TSFI_PARC_FIGMA_PROTO_H
