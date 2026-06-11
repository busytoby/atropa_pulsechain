#include "tsfi_wavefront_core.h"
#include <math.h>
#include <string.h>
#include <immintrin.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

void tsfi_wavefront_init(TSFiWavefront *wf) {
    if (!wf) return;
    memset(wf, 0, sizeof(TSFiWavefront));
    
    // Seed initial circular wavefront geometry
    wf->coeffs_real[0] = 512.0f; // Origin X
    wf->coeffs_imag[0] = 875.0f; // Origin Y
    wf->coeffs_real[1] = 200.0f; // Radius
    wf->coeffs_imag[1] = 200.0f;
    
    wf->bessel_phase = 0.0f;
    wf->verified = false;
}

void tsfi_wavefront_evolve(TSFiWavefront *wf, float dt_ms) {
    if (!wf) return;
    
    // Update Bessel Phase (Simulating wave surfing along the temporal domain)
    wf->bessel_phase += (0.001f * dt_ms);
    if (wf->bessel_phase > 2.0f * M_PI) {
        wf->bessel_phase -= (2.0f * M_PI);
    }
    
    // Apply phase modulation to coefficients
    float cos_p = cosf(wf->bessel_phase);
    float sin_p = sinf(wf->bessel_phase);
    
    // Mutate coefficients locally in cache
    for (int i = 1; i < WAVEFRONT_HARMONICS; i++) {
        float r = wf->coeffs_real[i];
        float im = wf->coeffs_imag[i];
        wf->coeffs_real[i] = r * cos_p - im * sin_p;
        wf->coeffs_imag[i] = r * sin_p + im * cos_p;
    }
}

void tsfi_wavefront_reconstruct_avx512(TSFiWavefront *wf, const float *basis_cos, const float *basis_sin) {
    if (!wf || !basis_cos || !basis_sin) return;

    // Clear target spatial coordinates
    memset(wf->coords_x, 0, sizeof(wf->coords_x));
    memset(wf->coords_y, 0, sizeof(wf->coords_y));

    // DC offsets (Origin base)
    __m512 v_dc_x = _mm512_set1_ps(wf->coeffs_real[0]);
    __m512 v_dc_y = _mm512_set1_ps(wf->coeffs_imag[0]);

    for (int k = 0; k < WAVEFRONT_SAMPLES; k += 16) {
        _mm512_store_ps(&wf->coords_x[k], v_dc_x);
        _mm512_store_ps(&wf->coords_y[k], v_dc_y);
    }

    // Vectorized evaluation of N harmonics using AVX-512 FMA
    for (int n = 1; n < WAVEFRONT_HARMONICS; n++) {
        __m512 v_r = _mm512_set1_ps(wf->coeffs_real[n]);
        __m512 v_im = _mm512_set1_ps(wf->coeffs_imag[n]);

        const float *cos_base = &basis_cos[n * WAVEFRONT_SAMPLES];
        const float *sin_base = &basis_sin[n * WAVEFRONT_SAMPLES];

        for (int k = 0; k < WAVEFRONT_SAMPLES; k += 16) {
            __m512 v_cos = _mm512_load_ps(&cos_base[k]);
            __m512 v_sin = _mm512_load_ps(&sin_base[k]);

            __m512 v_cx = _mm512_load_ps(&wf->coords_x[k]);
            __m512 v_cy = _mm512_load_ps(&wf->coords_y[k]);

            // Real reconstruction: x = x_base + r * cos - im * sin
            v_cx = _mm512_fmadd_ps(v_r, v_cos, v_cx);
            v_cx = _mm512_fnmadd_ps(v_im, v_sin, v_cx);

            // Imaginary reconstruction: y = y_base + r * sin + im * cos
            v_cy = _mm512_fmadd_ps(v_r, v_sin, v_cy);
            v_cy = _mm512_fmadd_ps(v_im, v_cos, v_cy);

            _mm512_store_ps(&wf->coords_x[k], v_cx);
            _mm512_store_ps(&wf->coords_y[k], v_cy);
        }
    }
}

bool tsfi_wavefront_verify_isomorphism(const TSFiWavefront *wf, uint32_t target_hash) {
    if (!wf) return false;

    // Compute a fast spatial hash of the coordinate boundaries
    uint32_t hash = 0;
    for (int i = 0; i < WAVEFRONT_SAMPLES; i++) {
        uint32_t val_x, val_y;
        memcpy(&val_x, &wf->coords_x[i], sizeof(uint32_t));
        memcpy(&val_y, &wf->coords_y[i], sizeof(uint32_t));
        hash = hash * 33 + val_x;
        hash = hash * 33 + val_y;
    }

    // Verify against mandated target
    return (hash == target_hash);
}
