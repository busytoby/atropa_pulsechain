#include "tsfi_parc_superpaint.h"

int tsfi_parc_superpaint_init_lut(tsfi_parc_superpaint_lut_t *lut) {
    if (!lut) return -1;
    // Retro 8-bit color palette initialization (warm golds, sepias, parchment, and charcoal)
    for (int i = 0; i < 256; i++) {
        if (i < 64) {
            // Sepia gradient
            lut->lut[i] = 0xFF000000 | ((i * 4) << 16) | ((i * 3) << 8) | (i * 2);
        } else if (i < 128) {
            // Gold gradient
            int r = 160 + (i - 64) * 1;
            int g = 120 + (i - 64) * 1;
            int b = 60 + (i - 64) * 1;
            lut->lut[i] = 0xFF000000 | (r << 16) | (g << 8) | b;
        } else if (i < 192) {
            // Charcoal/cream
            lut->lut[i] = 0xFF12100e;
        } else {
            lut->lut[i] = 0xFFe6dfd3; // Parchment
        }
    }
    return 0;
}

int tsfi_parc_superpaint_cycle_lut(tsfi_parc_superpaint_lut_t *lut, int shift) {
    if (!lut) return -1;
    uint32_t temp[256];
    for (int i = 0; i < 256; i++) {
        temp[i] = lut->lut[i];
    }
    // Shift colors within the gradient blocks
    for (int i = 0; i < 256; i++) {
        lut->lut[i] = temp[(i + shift) % 256];
    }
    return 0;
}

int tsfi_parc_superpaint_chroma_key(const uint32_t *fg, const uint32_t *bg, uint32_t *dst, int w, int h, uint32_t key_color) {
    if (!fg || !bg || !dst || w <= 0 || h <= 0) return -1;

    for (int i = 0; i < w * h; i++) {
        // If foreground matches key color, substitute background
        if ((fg[i] & 0x00FFFFFF) == (key_color & 0x00FFFFFF)) {
            dst[i] = bg[i];
        } else {
            dst[i] = fg[i];
        }
    }
    return 0;
}
