#ifndef TSFI_FBM_LANDSCAPE_H
#define TSFI_FBM_LANDSCAPE_H

#include <stdint.h>

typedef struct {
    int octaves;
    double lacunarity;
    double persistence;
    uint32_t seed;
} TSFiFbmLandscape;

// Initializes the fBm landscape parameters
void tsfi_fbm_landscape_init(TSFiFbmLandscape *land, int octaves, double lacunarity, double persistence, uint32_t seed);

// Evaluates fractional Brownian motion (fBm) height coordinates at specific 2D coordinates
double tsfi_fbm_landscape_eval(const TSFiFbmLandscape *land, double x, double y);

#endif // TSFI_FBM_LANDSCAPE_H
