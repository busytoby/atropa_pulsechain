#include "tsfi_quasirandomsampler.h"

// Halton sequence generator for prime bases 2 and 3
static double halton(int index, int base) {
    double result = 0.0;
    double f = 1.0 / base;
    int i = index;
    while (i > 0) {
        result += f * (i % base);
        i /= base;
        f /= base;
    }
    return result;
}

void tsfi_quasirandomsampler_init(TSFiQuasiRandomSampler *qrs, int sample_count) {
    if (!qrs) return;
    if (sample_count > TSFI_MAX_QUASISAMPLES) {
        sample_count = TSFI_MAX_QUASISAMPLES;
    }
    qrs->count = sample_count;
    
    // Generate Halton low-discrepancy points for uniform spatial sampling
    for (int i = 0; i < sample_count; i++) {
        qrs->samples[i].x = halton(i + 1, 2);
        qrs->samples[i].y = halton(i + 1, 3);
    }
}

bool tsfi_quasirandomsampler_get(const TSFiQuasiRandomSampler *qrs, int index, double *out_x, double *out_y) {
    if (!qrs || index < 0 || index >= qrs->count || !out_x || !out_y) return false;
    
    // Constant-time O(1) retrieval of low-discrepancy coordinate displacements
    *out_x = qrs->samples[index].x;
    *out_y = qrs->samples[index].y;
    return true;
}
