#ifndef TSFI_PARC_SUPERPAINT_H
#define TSFI_PARC_SUPERPAINT_H

#include <stdint.h>

typedef struct {
    uint32_t lut[256];
} tsfi_parc_superpaint_lut_t;

/* Initialize SuperPaint LUT with a classic retro 8-bit palette */
int tsfi_parc_superpaint_init_lut(tsfi_parc_superpaint_lut_t *lut);

/* Cycle the colors in the LUT to produce retro animation effects */
int tsfi_parc_superpaint_cycle_lut(tsfi_parc_superpaint_lut_t *lut, int shift);

/* Key/Blend a foreground layer onto a background layer using chroma keying */
int tsfi_parc_superpaint_chroma_key(const uint32_t *fg, const uint32_t *bg, uint32_t *dst, int w, int h, uint32_t key_color);

#endif // TSFI_PARC_SUPERPAINT_H
