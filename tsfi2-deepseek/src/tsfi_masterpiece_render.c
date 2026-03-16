#include <stdint.h>
#include <math.h>
#include <immintrin.h>
#include <string.h>
#include "tsfi_vae_firmware.h"

// Masterpiece V4: VLM-Informed Pure C High-Fidelity Renderer
// - Fur: Clumping + Dual-Layer Marschner Anisotropy
// - Petals: Venation + Dual-Pass SSS + Fresnel Rim

static inline float masterpiece_hash(int x, int y, uint32_t seed_adj) {
    uint32_t seed = (uint32_t)x * 1103515245U + (uint32_t)y * 12345U + seed_adj;
    seed = (seed ^ (seed >> 16)) * 0x85ebca6b;
    return (float)(seed & 0x7FFFFFFF) / 2147483648.0f;
}

void tsfi_masterpiece_pure_c_render(uint32_t *pixels, const float *density_flower, const float *density_bear, int w, int h, float t) {
    if (!pixels || !density_flower || !density_bear) return;

    const float light_x = 0.5f, light_y = 1.0f, light_z = 1.0f;
    const float inv_l = 1.0f / sqrtf(light_x*light_x + light_y*light_y + light_z*light_z);
    const float lx = light_x * inv_l, ly = light_y * inv_l, lz = light_z * inv_l;

    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            int idx = y * w + x;
            float df = density_flower[idx];
            float db = density_bear[idx];
            
            float out_r = 0.01f, out_g = 0.01f, out_b = 0.015f; // Ambient Deep Garden

            // 1. IMPROVED FUR (Clumping + Multi-Highlight)
            if (db < 25.0f) {
                // Clumping Logic: High-frequency modulation based on spatial hash
                float clumping = 0.8f + 0.4f * masterpiece_hash(x/4, y/4, 0xACE1);
                float effective_db = db * clumping;
                
                if (effective_db < 15.0f) {
                    float dx = density_bear[y * w + (x + 1 < w ? x + 1 : x)] - density_bear[y * w + (x > 0 ? x - 1 : x)];
                    float dy = density_bear[(y + 1 < h ? y + 1 : y) * w + x] - density_bear[(y > 0 ? y - 1 : y) * w + x];
                    float mag = sqrtf(dx*dx + dy*dy) + 1e-6f;
                    dx /= mag; dy /= mag;

                    // Marschner Dual-Layer Anisotropy
                    float tx = -dy, ty = dx;
                    float dot_tl = tx * lx + ty * ly;
                    float spec1 = powf(sqrtf(1.0f - dot_tl * dot_tl), 30.0f); // Primary (R)
                    float spec2 = powf(sqrtf(1.0f - dot_tl * dot_tl), 8.0f) * 0.3f; // Secondary (TRT)

                    float jitter = masterpiece_hash(x, y, (uint32_t)(t * 100));
                    float br = 0.35f + 0.1f * jitter;
                    float bg = 0.18f + 0.05f * jitter;
                    float bb = 0.08f + 0.03f * jitter;

                    float diffuse = fmaxf(0.05f, dx * lx + dy * ly + 0.3f * lz);
                    out_r = br * diffuse + spec1 * 0.5f + spec2 * 0.2f;
                    out_g = bg * diffuse + spec1 * 0.4f + spec2 * 0.2f;
                    out_b = bb * diffuse + spec1 * 0.3f + spec2 * 0.2f;
                }
            }

            // 2. IMPROVED PETALS (Venation + Dual SSS)
            if (df < 12.0f) {
                // Venation: Fine lines following the flow
                float vx = density_flower[y * w + (x + 1 < w ? x + 1 : x)] - density_flower[y * w + (x > 0 ? x - 1 : x)];
                float vy = density_flower[(y + 1 < h ? y + 1 : y) * w + x] - density_flower[(y > 0 ? y - 1 : y) * w + x];
                float v_mag = sqrtf(vx*vx + vy*vy) + 1e-6f;
                vx /= v_mag; vy /= v_mag;
                
                // Project position onto "radial" venation lines
                float venation = sinf(sqrtf((float)x*x + (float)y*y) * 0.5f + masterpiece_hash(x,y,0xDEAD) * 0.2f);
                float vein_mult = 0.95f + 0.1f * fmaxf(0, venation);

                // Dual-Pass SSS
                float sss_through = fmaxf(0.0f, 1.0f - df * 0.08f); // Light passing through thin edges
                float sss_internal = fmaxf(0.0f, 1.0f - df * 0.2f) * 0.4f; // Light bouncing inside thick core

                // Fresnel Sky Rim
                float fresnel = powf(1.0f - (v_mag / 6.0f), 5.0f);

                float fr = 1.0f, fg = 1.0f, fb = 0.92f; // Creamy White
                fr *= vein_mult; fg *= vein_mult; fb *= vein_mult;
                
                // Add SSS Glow (warm bleed)
                fr += sss_through * 0.25f + sss_internal * 0.1f;
                fg += sss_through * 0.20f + sss_internal * 0.05f;
                fb += sss_through * 0.10f;

                // Add Fresnel Rim (sky blue refraction)
                fr = fr * (1.0f - fresnel) + 0.6f * fresnel;
                fg = fg * (1.0f - fresnel) + 0.8f * fresnel;
                fb = fb * (1.0f - fresnel) + 1.0f * fresnel;

                // Senescence browning (t > 0.8)
                if (t > 0.8f) {
                    float brown_t = (t - 0.8f) / 0.2f;
                    fr *= (1.0f - 0.6f * brown_t);
                    fg *= (1.0f - 0.8f * brown_t);
                    fb *= (1.0f - 0.9f * brown_t);
                }

                float alpha = fminf(1.0f, fmaxf(0.0f, 3.0f - df * 0.3f));
                out_r = out_r * (1.0f - alpha) + fr * alpha;
                out_g = out_g * (1.0f - alpha) + fg * alpha;
                out_b = out_b * (1.0f - alpha) + fb * alpha;
            }

            // Gamma + Tonemap
            out_r = powf(fminf(1.0f, out_r), 0.4545f);
            out_g = powf(fminf(1.0f, out_g), 0.4545f);
            out_b = powf(fminf(1.0f, out_b), 0.4545f);

            pixels[idx] = (0xFF << 24) | ((uint8_t)(out_r * 255) << 16) | ((uint8_t)(out_g * 255) << 8) | (uint8_t)(out_b * 255);
        }
    }
}
