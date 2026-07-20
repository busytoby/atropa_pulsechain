#ifndef TSFI_PARC_FIGMA_ADVANCED_H
#define TSFI_PARC_FIGMA_ADVANCED_H

#include <stdint.h>
#include "tsfi_parc_figma.h"

#define BLEND_MULTIPLY   1
#define BLEND_SCREEN     2
#define BLEND_OVERLAY    3
#define BLEND_COLORDODGE 4

/* 1. Figma WebGL Layer Blend Modes: Blends top layer into base canvas */
void tsfi_parc_figma_blend_layer(uint32_t *base, const uint32_t *blend, int width, int height, int mode);

/* 2. Figma Component Variant State Manager: Resolves layout attributes and color based on property state string */
void tsfi_parc_figma_variant_resolve(const char *variant_props, int *w_out, int *h_out, uint32_t *color_out);

/* 3. Figma Vector Boolean Solvers: Compute bounding box union results */
void tsfi_parc_figma_vector_boolean_union(const tsfi_parc_layout_item_t *a, const tsfi_parc_layout_item_t *b, tsfi_parc_layout_item_t *out_result);

#endif // TSFI_PARC_FIGMA_ADVANCED_H
