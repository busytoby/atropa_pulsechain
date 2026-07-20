#ifndef TSFI_PARC_TKNOLL_H
#define TSFI_PARC_TKNOLL_H

#include <stdint.h>

/* Grayscale dithering: Floyd-Steinberg error diffusion from 32-bit source to 1-bit Alto memory layout */
void tsfi_parc_tknoll_dither_floyd_steinberg(const uint32_t *src, uint16_t *dest_alto, int width, int height);

/* Contrast Adjustments: Input/output levels clipping and gamma curve adjustments using linear lookup tables */
void tsfi_parc_tknoll_levels_adjustment(uint32_t *pixels, int width, int height, int input_min, int input_max, float gamma);

/* Calculate a 256-channel grayscale histogram frequency counter */
void tsfi_parc_tknoll_histogram(const uint32_t *pixels, int width, int height, uint32_t *histogram_out);

#endif // TSFI_PARC_TKNOLL_H
