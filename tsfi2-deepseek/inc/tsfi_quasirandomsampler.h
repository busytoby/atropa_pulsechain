#ifndef TSFI_QUASIRANDOMSAMPLER_H
#define TSFI_QUASIRANDOMSAMPLER_H

#include <stdint.h>
#include <stdbool.h>

#define TSFI_MAX_QUASISAMPLES 64

typedef struct {
    double x;
    double y;
} TSFiQuasiSample;

typedef struct {
    TSFiQuasiSample samples[TSFI_MAX_QUASISAMPLES];
    int count;
} TSFiQuasiRandomSampler;

// Initialize the QuasiRandomSampler context with Halton sequence coordinate points
void tsfi_quasirandomsampler_init(TSFiQuasiRandomSampler *qrs, int sample_count);

// Performs O(1) constant-time sample point retrieval for a given sample index
bool tsfi_quasirandomsampler_get(const TSFiQuasiRandomSampler *qrs, int index, double *out_x, double *out_y);

#endif // TSFI_QUASIRANDOMSAMPLER_H
