#include "tsfi_vae_firmware.h"
#include <immintrin.h>
#include <math.h>

/**
 * TSFi VAE Field Decoder (Upsampler)
 * 
 * Efficiently splats 1024 fibers onto a 512x512 density field.
 * Uses AVX-512 to process 16 pixels at a time.
 */
void tsfi_vae_decode_to_field(const LauVaeFirmware_State *state, float *out_field, int width, int height) {
    if (!state || !state->nand_source || !out_field) return;

    // 1. Clear Field
    for (int i = 0; i < width * height; i++) out_field[i] = 1e5f; // High value for SDF-like density

    const TsfiGran *fibers = state->nand_source->fibers;
    float half_w = (float)width * 0.5f;
    float half_h = (float)height * 0.5f;

    // 2. Splat Fibers
    // We process each fiber and update the field in its bounding box
    // To keep it fast, we treat each fiber as a smooth sphere of influence
    for (int i = 0; i < GRANS; i++) {
        if (fibers[i].weight < 0.01f) continue;

        float fx = (fibers[i].x + 1.0f) * half_w;
        float fy = (fibers[i].y + 1.0f) * half_h;
        float fw = fibers[i].weight;
        float fr = 12.0f; // Radius of influence

        int x0 = (int)(fx - fr); if (x0 < 0) x0 = 0;
        int x1 = (int)(fx + fr); if (x1 >= width) x1 = width - 1;
        int y0 = (int)(fy - fr); if (y0 < 0) y0 = 0;
        int y1 = (int)(fy + fr); if (y1 >= height) y1 = height - 1;

        __m512 vfx = _mm512_set1_ps(fx);
        __m512 vfy = _mm512_set1_ps(fy);
        __m512 vfr = _mm512_set1_ps(fr * fw); // Weight modulates radius

        for (int y = y0; y <= y1; y++) {
            __m512 vy = _mm512_set1_ps((float)y);
            for (int x = x0; x <= x1; x += 16) {
                int remaining = x1 - x + 1;
                if (remaining <= 0) break;
                __mmask16 mask = (remaining >= 16) ? 0xFFFF : (1U << remaining) - 1;
                
                __m512 vx = _mm512_add_ps(_mm512_set1_ps((float)x), _mm512_set_ps(15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0));
                
                __m512 dx = _mm512_sub_ps(vx, vfx);
                __m512 dy = _mm512_sub_ps(vy, vfy);
                __m512 dist = _mm512_sqrt_ps(_mm512_add_ps(_mm512_mul_ps(dx, dx), _mm512_mul_ps(dy, dy)));
                
                __m512 cur_d = _mm512_sub_ps(dist, vfr);
                
                // Smooth union (standard TSFi blend)
                __m512 old_d = _mm512_mask_loadu_ps(_mm512_set1_ps(1e5f), mask, &out_field[y * width + x]);
                __m512 h = _mm512_max_ps(_mm512_setzero_ps(), _mm512_sub_ps(_mm512_set1_ps(4.0f), _mm512_abs_ps(_mm512_sub_ps(old_d, cur_d))));
                __m512 new_d = _mm512_sub_ps(_mm512_min_ps(old_d, cur_d), _mm512_mul_ps(h, _mm512_mul_ps(h, _mm512_set1_ps(0.25f))));
                
                _mm512_mask_storeu_ps(&out_field[y * width + x], mask, new_d);
            }
        }
    }
}
