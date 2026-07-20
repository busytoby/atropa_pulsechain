#ifndef TSFI_PARC_BITBLT_H
#define TSFI_PARC_BITBLT_H

#include <stdint.h>

typedef enum {
    BITBLT_COPY = 0,
    BITBLT_OR   = 1,
    BITBLT_AND  = 2,
    BITBLT_XOR  = 3,
    BITBLT_PAINT = 4
} BitBltOp;

typedef struct {
    uint32_t *dest_bits;
    int dest_width;
    int dest_height;
    const uint32_t *src_bits;
    int src_width;
    int src_height;
    uint16_t halftone_pattern[16]; // 16x16 halftone pattern template
    int has_halftone;
} tsfi_parc_bitblt_t;

/* Execute the BitBlt transfer block operation */
int tsfi_parc_bitblt_transfer(tsfi_parc_bitblt_t *bb, int dest_x, int dest_y, int src_x, int src_y, int w, int h, BitBltOp op);

#endif // TSFI_PARC_BITBLT_H
