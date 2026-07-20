#ifndef TSFI_PARC_ALFONT_H
#define TSFI_PARC_ALFONT_H

#include <stdint.h>

typedef struct {
    uint8_t width;
    uint8_t height;
    int8_t x_offset;
    int8_t y_offset;
    uint16_t bitmap[16];
} tsfi_parc_alfont_char_t;

typedef struct {
    tsfi_parc_alfont_char_t chars[128];
} tsfi_parc_alfont_t;

/* Initialize font layout structures */
void tsfi_parc_alfont_init(tsfi_parc_alfont_t *font);

/* Load a binary .AL font file from disk */
int tsfi_parc_alfont_load(tsfi_parc_alfont_t *font, const char *filepath);

#endif // TSFI_PARC_ALFONT_H
