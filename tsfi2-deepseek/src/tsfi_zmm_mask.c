#include "tsfi_c_math.h"
#include <stdint.h>
#include <math.h>

// DeepSeek's specific procedural hash noise
static inline float fractf(float x) {
    return x - floorf(x);
}

static inline float procedural_noise(float px, float py, float time_offset) {
    float dot_val = (px * 12.9898f) + (py * 78.233f) + time_offset;
    return fractf(sinf(dot_val) * 43758.5453f);
}

// True Branching Shadow Physics
static inline float compute_bifurcated_shadow(float x, float y) {
    float dist_left = hypotf(x - 0.400f, y - 0.780f);
    float dist_right = hypotf(x - 0.624f, y - 0.780f);
    float k = 50.0f; 
    float res = expf(-k * dist_left) + expf(-k * dist_right);
    return -logf(res) / k;
}

// The Vectorized Entry Point called from Python via ctypes
void generate_zmm_mask(uint8_t* pixels, int w, int h, int frame, int total_frames) {
    float pulse = sinf(((float)frame / (float)total_frames) * ((float)TSFI_SECRET_CORE) * 10.0f);
    float sickness_threshold = ((float)frame / (float)total_frames) * 100.0f;
    float time_offset = frame * 0.1f;
    
    // GCC Auto-Vectorization Pragma forces the compiler to unroll this loop into AVX-512 ZMM registers
    #pragma GCC ivdep
    for (int y = 0; y < h; y++) {
        float py = (float)y / h;
        if (py < 0.1f || py > 0.95f) continue;
        
        int row_idx = y * w * 3;
        
        #pragma omp simd
        for (int x = 0; x < w; x++) {
            float px = (float)x / w;
            if (px < 0.1f || px > 0.9f) continue;
            
            int idx = row_idx + (x * 3);
            
            float dist_eye1 = hypotf(px - 0.46f, py - 0.4f);
            float dist_eye2 = hypotf(px - 0.54f, py - 0.4f);
            float pulse_radius = 0.02f + (pulse * 0.005f);
            
            float shadow_dist = compute_bifurcated_shadow(px, py);
            
            if (shadow_dist < 0.08f) {
                pixels[idx] = 19; pixels[idx+1] = 19; pixels[idx+2] = 19;
            }
            else if (dist_eye1 < pulse_radius) {
                pixels[idx] = 0; pixels[idx+1] = 255; pixels[idx+2] = 0;
            }
            else if (dist_eye2 < pulse_radius) {
                pixels[idx] = 0; pixels[idx+1] = 255; pixels[idx+2] = 0;
            }
            else if (hypotf(px - 0.35f, py - 0.25f) < 0.075f || hypotf(px - 0.65f, py - 0.25f) < 0.075f) {
                pixels[idx] = 76; pixels[idx+1] = 29; pixels[idx+2] = 14;
            }
            else if (hypotf(px - 0.5f, py - 0.5f) < (0.2f + (pulse * 0.01f))) {
                if (hypotf(px - 0.45f, py - 0.45f) < 0.08f) {
                    pixels[idx] = 252; pixels[idx+1] = 252; pixels[idx+2] = 252;
                } else {
                    pixels[idx] = 76; pixels[idx+1] = 29; pixels[idx+2] = 14;
                }
            }
            else if (hypotf(px - 0.5f, py - 0.75f) < 0.25f) {
                float noise = procedural_noise(px * 100.0f, py * 100.0f, time_offset);
                if ((noise * 100.0f) < sickness_threshold) {
                    pixels[idx] = 51; pixels[idx+1] = 65; pixels[idx+2] = 0;
                } else {
                    pixels[idx] = 76; pixels[idx+1] = 29; pixels[idx+2] = 14;
                }
            }
        }
    }
}
