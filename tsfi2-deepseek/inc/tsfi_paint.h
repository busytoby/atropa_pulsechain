#ifndef TSFI_PAINT_H
#define TSFI_PAINT_H

#include "tsfi_layout.h"

// Paints the layout box tree onto the pixel buffer
void tsfi_paint_tree(const TsfiLayoutBox *box, uint32_t *pixels, int width, int height);

/* Quantel Paintbox: Pressure-sensitive Airbrushing with standard gaussian distribution */
int tsfi_quantel_paintbox_airbrush(uint32_t *pixels, int w, int h, int cx, int cy, int radius, float pressure, uint32_t color);

/* Quantel Paintbox: Dynamic stencil masking logic mapping vector stencil regions */
int tsfi_quantel_paintbox_apply_stencil(uint32_t *pixels, int w, int h, const uint8_t *stencil_mask, uint32_t color);

/* Quantel Mirage: 3D DVE transforms implementing page curls on image segments */
int tsfi_quantel_mirage_page_curl(const uint32_t *src_pixels, int src_w, int src_h, uint32_t *dst_pixels, int dst_w, int dst_h, float curl_radius, float roll_percent);

/* Quantel Harry: Multi-layer chroma key extraction */
int tsfi_quantel_harry_chroma_key(const uint32_t *src_pixels, int w, int h, uint8_t *out_mask, uint32_t key_color, float tolerance);

/* Quantel Harry: Dynamic timeline compositing blend modes */
int tsfi_quantel_harry_blend(const uint32_t *fg_pixels, const uint32_t *bg_pixels, uint32_t *dst_pixels, int w, int h, const uint8_t *mask, const char *blend_mode);

#endif // TSFI_PAINT_H

