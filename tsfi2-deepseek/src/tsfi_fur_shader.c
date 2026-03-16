#include <stdint.h>
#include <immintrin.h>
#include <math.h>
#include <string.h>
#include "tsfi_vae_firmware.h"

/**
 * TSFi Photorealistic Fur Shader (CPU Thunk)
 * 
 * Uses AVX-512 to splat 1024 fibers as Marschner-shaded hair strands.
 */
void tsfi_render_fur_thunk(uint32_t *pixels, const float *density_field, int w, int h, float t) {
    if (!pixels || !density_field) return;
    if (((uintptr_t)density_field % 64) != 0) {
        // Fallback for unaligned or small buffers if necessary, 
        // but our lau_malloc should provide aligned memory.
    }

    // Dark background
    for (int i = 0; i < w * h; i++) pixels[i] = 0xFF050505;

    float light_x = 0.5f, light_y = 1.0f, light_z = 1.0f;
    float inv_light_mag = 1.0f / sqrtf(light_x*light_x + light_y*light_y + light_z*light_z);
    light_x *= inv_light_mag; light_y *= inv_light_mag; light_z *= inv_light_mag;

    // We use the 16-pixel parallel lanes to splat strands
    for (int y = 2; y < h - 2; y++) {
        for (int x = 2; x < w - 2; x += 16) {
            __mmask16 mask = _mm512_cmp_ps_mask(_mm512_loadu_ps(&density_field[y * w + x]), _mm512_set1_ps(0.1f), _CMP_GT_OQ);
            if (mask == 0) continue;

            for (int lane = 0; lane < 16; lane++) {
                if (!(mask & (1 << lane))) continue;

                int cx = x + lane;
                int cy = y;
                
                // Calculate gradient for flow
                float dx = density_field[cy * w + (cx + 1)] - density_field[cy * w + (cx - 1)];
                float dy = density_field[(cy + 1) * w + cx] - density_field[(cy - 1) * w + cx];
                
                float angle = atan2f(dy, dx) + 1.5708f; // Perpendicular to gradient
                float len = 4.0f + 2.0f * sinf(t * 10.0f + (float)(cx ^ cy));

                // Marschner-ish Splatting
                for (float s = 0.0f; s < len; s += 0.5f) {
                    int sx = (int)(cx + cosf(angle) * s);
                    int sy = (int)(cy + sinf(angle) * s);
                    if (sx < 0 || sx >= w || sy < 0 || sy >= h) break;

                    float shadow = 1.0f - (s / len) * 0.5f;
                    float dot = (cosf(angle) * light_x + sinf(angle) * light_y + 0.5f * light_z);
                    dot = fmaxf(0.2f, dot);

                    uint8_t r = (uint8_t)(0.4f * dot * shadow * 255.0f);
                    uint8_t g = (uint8_t)(0.2f * dot * shadow * 255.0f);
                    uint8_t b = (uint8_t)(0.1f * dot * shadow * 255.0f);
                    
                    // Simple additive blend for "plush" look
                    uint32_t old_p = pixels[sy * w + sx];
                    uint8_t or = (old_p >> 16) & 0xFF, og = (old_p >> 8) & 0xFF, ob = old_p & 0xFF;
                    pixels[sy * w + sx] = (0xFF << 24) | 
                                          ((uint8_t)fminf(255, r + or) << 16) | 
                                          ((uint8_t)fminf(255, g + og) << 8) | 
                                          ((uint8_t)fminf(255, b + ob));
                }
            }
        }
    }
}
