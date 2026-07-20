#include "tsfi_parc_cursor.h"

// 16x16 cursor bitmaps
static const uint16_t arrow_bits[16] = {
    0xC000, 0xE000, 0xF000, 0xF800, 0xFC00, 0xFE00, 0xFF00, 0xFF80,
    0xFFC0, 0xFE00, 0xEF00, 0xC780, 0x0780, 0x03C0, 0x03C0, 0x0000
};

static const uint16_t ibeam_bits[16] = {
    0xFC3F, 0x1818, 0x1818, 0x1818, 0x1818, 0x1818, 0x1818, 0x1818,
    0x1818, 0x1818, 0x1818, 0x1818, 0x1818, 0x1818, 0x1818, 0xFC3F
};

static const uint16_t cross_bits[16] = {
    0x0180, 0x0180, 0x0180, 0x0180, 0x0180, 0x0180, 0x0180, 0xFFFF,
    0xFFFF, 0x0180, 0x0180, 0x0180, 0x0180, 0x0180, 0x0180, 0x0180
};

static const uint16_t wait_bits[16] = {
    0x7FFE, 0x4002, 0x2004, 0x1008, 0x0810, 0x0420, 0x0240, 0x0180,
    0x0180, 0x0240, 0x0420, 0x0810, 0x1008, 0x2004, 0x4002, 0x7FFE
};

int tsfi_parc_draw_cursor(uint32_t *pixels, int w, int h, int cx, int cy, AltoCursorType type, uint32_t color) {
    if (!pixels || w <= 0 || h <= 0) return -1;

    const uint16_t *bits = arrow_bits;
    switch (type) {
        case CURSOR_ARROW: bits = arrow_bits; break;
        case CURSOR_IBEAM: bits = ibeam_bits; break;
        case CURSOR_CROSSHAIR: bits = cross_bits; break;
        case CURSOR_WAIT: bits = wait_bits; break;
    }

    for (int y = 0; y < 16; y++) {
        int py = cy + y;
        if (py < 0 || py >= h) continue;
        uint16_t row = bits[y];

        for (int x = 0; x < 16; x++) {
            int px = cx + x;
            if (px < 0 || px >= w) continue;
            int bit = (row >> (15 - x)) & 1;
            if (bit) {
                pixels[py * w + px] = color;
            }
        }
    }

    return 0;
}
