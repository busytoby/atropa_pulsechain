#ifndef TSFI_PARC_KNOLL_H
#define TSFI_PARC_KNOLL_H

#include <stdint.h>

/* Generate a Knoll-style Lens Flare centered at (cx, cy) */
void tsfi_parc_knoll_lens_flare(uint32_t *pixels, int width, int height, int cx, int cy, float intensity);

/* Perform a 2D Gaussian blur convolution filter */
void tsfi_parc_knoll_gaussian_blur(const uint32_t *src, uint32_t *dst, int width, int height, float sigma);

/* Map 24-bit RGB pixel data to 8-bit grayscale intensity values */
void tsfi_parc_knoll_gray_lut(const uint32_t *src, uint32_t *dst, int width, int height);

#endif // TSFI_PARC_KNOLL_H
