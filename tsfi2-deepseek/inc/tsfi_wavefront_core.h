#ifndef TSFI_WAVEFRONT_CORE_H
#define TSFI_WAVEFRONT_CORE_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define WAVEFRONT_HARMONICS 512
#define WAVEFRONT_SAMPLES 256

// Unified Wavefront State representing the Hilbert-Banach Singularity
typedef struct {
    // Hilbert Space (Fourier Coefficients)
    float coeffs_real[WAVEFRONT_HARMONICS] __attribute__((aligned(64)));
    float coeffs_imag[WAVEFRONT_HARMONICS] __attribute__((aligned(64)));

    // Banach Space (Spatial Voxel Coordinates)
    float coords_x[WAVEFRONT_SAMPLES] __attribute__((aligned(64)));
    float coords_y[WAVEFRONT_SAMPLES] __attribute__((aligned(64)));

    // Bessel Phase State
    float bessel_phase;
    uint32_t consensus_signature;
    bool verified;
} TSFiWavefront;

// Lifecycle & Math API
void tsfi_wavefront_init(TSFiWavefront *wf);
void tsfi_wavefront_evolve(TSFiWavefront *wf, float dt_ms);
void tsfi_wavefront_reconstruct_avx512(TSFiWavefront *wf, const float *basis_cos, const float *basis_sin);
bool tsfi_wavefront_verify_isomorphism(const TSFiWavefront *wf, uint32_t target_hash);

#endif // TSFI_WAVEFRONT_CORE_H
