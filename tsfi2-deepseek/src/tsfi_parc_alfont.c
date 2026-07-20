#include <stdio.h>
#include <string.h>
#include "tsfi_parc_alfont.h"

void tsfi_parc_alfont_init(tsfi_parc_alfont_t *font) {
    if (!font) return;
    memset(font->chars, 0, sizeof(font->chars));

    // Populate a fallback classic standard monospace glyph map
    for (int i = 32; i < 127; i++) {
        font->chars[i].width = 8;
        font->chars[i].height = 8;
        font->chars[i].x_offset = 0;
        font->chars[i].y_offset = 0;
        // Populating standard crossbar lines
        font->chars[i].bitmap[0] = 0x3C00;
        font->chars[i].bitmap[1] = 0x6600;
        font->chars[i].bitmap[2] = 0x6600;
        font->chars[i].bitmap[3] = 0x7E00;
        font->chars[i].bitmap[4] = 0x6600;
        font->chars[i].bitmap[5] = 0x6600;
        font->chars[i].bitmap[6] = 0x6600;
        font->chars[i].bitmap[7] = 0x0000;
    }
}

int tsfi_parc_alfont_load(tsfi_parc_alfont_t *font, const char *filepath) {
    if (!font || !filepath) return -1;

    // Enforce only .dat.bin extension check to satisfy database format rules
    if (!strstr(filepath, ".dat.bin")) return -4;

    FILE *f = fopen(filepath, "rb");
    if (!f) return -2;

    // Read characters block
    for (int i = 32; i < 127; i++) {
        uint8_t header[4];
        if (fread(header, 1, 4, f) != 4) {
            fclose(f);
            return -3; // EOF or read failure
        }
        font->chars[i].width = header[0];
        font->chars[i].height = header[1];
        font->chars[i].x_offset = (int8_t)header[2];
        font->chars[i].y_offset = (int8_t)header[3];

        if (fread(font->chars[i].bitmap, sizeof(uint16_t), 16, f) != 16) {
            fclose(f);
            return -3;
        }
    }

    fclose(f);
    return 0;
}
