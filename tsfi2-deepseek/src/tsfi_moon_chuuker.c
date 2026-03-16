#include <stdint.h>
#include <math.h>
#include <string.h>
#include "tsfi_vae_firmware.h"

// "Moon Chuuker" VLM-Guided Physical Thunk
// Directives from Moondream2:
// 1. [0.39, 0.41, 0.56, 0.52] - Micro-Venation & Light Bleed
// 2. [0.56, 0.42, 0.68, 0.52] - Directed Fur Clumping
// 3. [0.54, 0.42, 0.68, 0.52] - Deep Throat Specular Micro-Detail

static inline float chuuker_hash(int x, int y, uint32_t s) {
    uint32_t h = (uint32_t)x * 374761393U + (uint32_t)y * 668265263U + s;
    h = (h ^ (h >> 13)) * 1274126177U;
    return (float)(h & 0x7FFFFFFF) / 2147483648.0f;
}

void tsfi_moon_chuuker_render(uint32_t *pixels, const float *df, const float *db, int w, int h, float t) {
    if (!pixels || !df || !db) return;
    (void)t;

    for (int y = 0; y < h; y++) {
        float fy = (float)y / h;
        for (int x = 0; x < w; x++) {
            float fx = (float)x / w;
            int idx = y * w + x;
            
            float df_val = df[idx];
            float db_val = db[idx];
            
            // Base render logic (optimized from masterpiece_render)
            float r=0.01f, g=0.01f, b=0.015f;

            // 1. DIRECTED FUR CLUMPING (Box 2)
            bool in_clump_box = (fy > 0.56f && fy < 0.68f && fx > 0.42f && fx < 0.52f);
            float clump_mod = 1.0f;
            if (in_clump_box) {
                clump_mod = 0.7f + 0.6f * chuuker_hash(x/3, y/3, 0xBEEF);
            } else {
                clump_mod = 0.9f + 0.2f * chuuker_hash(x/5, y/5, 0xACE1);
            }

            if (db_val * clump_mod < 20.0f) {
                float dx = db[y*w+(x+1<w?x+1:x)] - db[y*w+(x>0?x-1:x)];
                float dy = db[(y+1<h?y+1:y)*w+x] - db[(y>0?y-1:y)*w+x];
                float mag = sqrtf(dx*dx+dy*dy)+1e-6f;
                dx/=mag; dy/=mag;
                float tx=-dy, ty=dx;
                float spec = powf(sqrtf(fmaxf(0, 1.0f - (tx*0.5f + ty*0.8f))), 25.0f);
                float diffuse = fmaxf(0.1f, dx*0.5f + dy*0.8f + 0.2f);
                r = 0.35f * diffuse + spec * 0.3f;
                g = 0.18f * diffuse + spec * 0.2f;
                b = 0.08f * diffuse + spec * 0.1f;
            }

            // 2. MICRO-VENATION & TRANSLUCENCY (Box 1)
            if (df_val < 15.0f) {
                bool in_petal_box = (fy > 0.39f && fy < 0.56f && fx > 0.41f && fx < 0.52f);
                
                // Venation via high-freq noise
                float v_noise = chuuker_hash(x, y, 0x1234);
                float venation = 1.0f;
                if (in_petal_box) {
                    venation = 0.92f + 0.15f * sinf(df_val * 2.0f + v_noise * 5.0f);
                }

                // Dual-Pass SSS
                float sss = fmaxf(0, 1.0f - df_val * 0.1f) * (in_petal_box ? 0.8f : 0.5f);
                
                // Wetness Specular (Box 3 logic integrated)
                bool in_throat = (fy > 0.54f && fy < 0.68f && fx > 0.42f && fx < 0.52f);
                float wet_spec = 0;
                if (in_throat || in_petal_box) {
                    float dx = df[y*w+(x+1<w?x+1:x)] - df[y*w+(x>0?x-1:x)];
                    float dy = df[(y+1<h?y+1:y)*w+x] - df[(y>0?y-1:y)*w+x];
                    float mag = sqrtf(dx*dx+dy*dy)+1e-6f;
                    wet_spec = powf(fmaxf(0, 1.0f - mag/10.0f), 64.0f) * 0.6f;
                }

                float fr = 1.0f * venation, fg = 1.0f * venation, fb = 0.95f;
                fr += sss * 0.3f; fg += sss * 0.2f;
                fr += wet_spec; fg += wet_spec; fb += wet_spec;

                float alpha = fminf(1.0f, fmaxf(0.0f, 3.0f - df_val * 0.25f));
                r = r*(1.0f-alpha) + fr*alpha;
                g = g*(1.0f-alpha) + fg*alpha;
                b = b*(1.0f-alpha) + fb*alpha;
            }

            r = powf(fminf(1.0f, r), 0.4545f);
            g = powf(fminf(1.0f, g), 0.4545f);
            b = powf(fminf(1.0f, b), 0.4545f);
            pixels[idx] = (0xFF << 24) | ((uint8_t)(r*255) << 16) | ((uint8_t)(g*255) << 8) | (uint8_t)(b*255);
        }
    }
}
