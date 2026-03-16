#include "tsfi_opt_zmm.h"
#include "lau_memory.h"
#include <immintrin.h>
#include <stdint.h>
#include <string.h>

/**
 * @brief Performs a high-speed AVX-512 latent warp (e.g. translation).
 * 
 * Logic: Slides the latent tensor by (dx, dy) to maintain temporal flow.
 * Width/Height assumed to be latent space (e.g. 64x64 for 512x512).
 */
void tsfi_latent_warp_translate(float *latent, int w, int h, int dx, int dy) {
    if (!latent || (dx == 0 && dy == 0)) return;

    size_t latent_sz = w * h * 4 * sizeof(float);
    float *tmp = (float*)lau_malloc_wired(latent_sz);
    memset(tmp, 0, latent_sz);

    for (int y = 0; y < h; y++) {
        int ny = y + dy;
        if (ny < 0 || ny >= h) continue;

        for (int x = 0; x < w; x++) {
            int nx = x + dx;
            if (nx < 0 || nx >= w) continue;

            // 4-channels per latent pixel (Vectorized Copy)
            __m128 vLatent = _mm_loadu_ps(&latent[(y * w + x) * 4]);
            _mm_storeu_ps(&tmp[(ny * w + nx) * 4], vLatent);
        }
    }

    memcpy(latent, tmp, latent_sz);
    lau_free(tmp);
}

/**
 * @brief Zero-Copy Latent Noise Mixer
 * 
 * Mixes 'warped' latents with a fresh seed (e.g. 90% warp, 10% new noise).
 */
void tsfi_latent_mix(float *target, const float *source, float alpha, size_t count) {
    __m512 vAlpha = _mm512_set1_ps(alpha);
    __m512 vOneMinusAlpha = _mm512_set1_ps(1.0f - alpha);

    for (size_t i = 0; i <= count - 16; i += 16) {
        __m512 vS = _mm512_loadu_ps(&source[i]);
        __m512 vT = _mm512_loadu_ps(&target[i]);
        __m512 vR = _mm512_add_ps(_mm512_mul_ps(vS, vAlpha), _mm512_mul_ps(vT, vOneMinusAlpha));
        _mm512_storeu_ps(&target[i], vR);
    }
}
