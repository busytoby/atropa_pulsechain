#ifndef TSFI_WAVE15_H
#define TSFI_WAVE15_H

#include <stdint.h>

/**
 * WAVE15: 15-byte Geometric Wavelet Element.
 * 
 * DESIGN RATIONALE:
 * A single WAVE15 structure is guaranteed to fit appropriately with default alignment
 * in any area of GPU memory when used as a singular atom. This enables building 
 * capacity for WAVE structures of ANY length by treating them as non-homogeneous 
 * byte-aligned streams.
 * 
 * LAYOUT:
 * [0-1]  p0_x (int16)
 * [2-3]  p0_y (int16)
 * [4-5]  p1_x (int16)
 * [6-7]  p1_y (int16)
 * [8-9]  p2_x (int16)
 * [10-11] p2_y (int16)
 * [12-13] flags (uint16)
 * [14]    secret (uint8) - Reserved for Dysnomia Secret Injection
 * 
 * TOTAL: 15 bytes.
 */
typedef struct {
    int16_t p0_x, p0_y;
    int16_t p1_x, p1_y;
    int16_t p2_x, p2_y;
    uint16_t flags;
    uint8_t  secret;
} __attribute__((packed)) WAVE15;

#endif // TSFI_WAVE15_H
