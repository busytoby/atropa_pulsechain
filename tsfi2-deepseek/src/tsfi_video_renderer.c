#include "tsfi_c_math.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <immintrin.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#define W 1024
#define H 1536
#define FPS 60
#define DURATION 4
#define TOTAL_FRAMES (FPS * DURATION)

// Optimized AVX-512 Procedural Noise
// Replicates: fract(sin(dot(p, vec2(12.9898, 78.233))) * 43758.5453)
static inline __m512 procedural_noise_avx512(__m512 px, __m512 py, float time_offset) {
    __m512 v12_9898 = _mm512_set1_ps(12.9898f);
    __m512 v78_233 = _mm512_set1_ps(78.233f);
    __m512 v43758_5453 = _mm512_set1_ps(43758.5453f);
    __m512 vTime = _mm512_set1_ps(time_offset);

    // dot = px*12.9898 + py*78.233 + time_offset
    __m512 dot = _mm512_add_ps(_mm512_add_ps(_mm512_mul_ps(px, v12_9898), _mm512_mul_ps(py, v78_233)), vTime);

    // sin approximation or use svml if available. 
    // For "Sick" noise, a fast Taylor or even a bit-shuffle is often enough, 
    // but let's use a basic sine-like wrap for visual parity.
    // sin(x) approx for [0..2pi]: x - x^3/6 + x^5/120
    // Actually, for this specific hash, just the fractional part of a large multiplication is the key.
    
    __m512 s = _mm512_mul_ps(dot, v43758_5453);
    // fract(s) = s - floor(s)
    __m512 fs = _mm512_sub_ps(s, _mm512_floor_ps(s));
    return fs;
}

void render_frame(int frame, uint8_t *pixels) {
    float time_offset = frame * 0.1f;
    float pulse = sinf((frame / (float)TOTAL_FRAMES) * 2.0f * ((float)TSFI_SECRET_CORE));
    float pulse_radius = 0.02f + (pulse * 0.005f);
    float eye2_radius = 0.045f + (pulse * 0.01f);

    __m512 vInvW = _mm512_set1_ps(1.0f / (float)W);
    __m512 vInvH = _mm512_set1_ps(1.0f / (float)H);
    __m512 vHalf = _mm512_set1_ps(0.5f);
    
    for (int y = 0; y < H; y++) {
        float py_val = (float)y / H;
        __m512 vPy = _mm512_set1_ps(py_val);
        
        for (int x = 0; x < W; x += 16) {
            // px = [x, x+1, ..., x+15] / W
            __m512 vPx = _mm512_add_ps(_mm512_set1_ps((float)x), 
                         _mm512_set_ps(15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0));
            vPx = _mm512_mul_ps(vPx, vInvW);

            // Distance to Eye 1 (0.5, 0.4)
            __m512 dx1 = _mm512_sub_ps(vPx, vHalf);
            __m512 dy1 = _mm512_sub_ps(vPy, _mm512_set1_ps(0.4f));
            __m512 dist1 = _mm512_sqrt_ps(_mm512_add_ps(_mm512_mul_ps(dx1, dx1), _mm512_mul_ps(dy1, dy1)));

            // Distance to Eye 2 (0.5, 0.6)
            __m512 dx2 = _mm512_sub_ps(vPx, vHalf);
            __m512 dy2 = _mm512_sub_ps(vPy, _mm512_set1_ps(0.6f));
            __m512 dist2 = _mm512_sqrt_ps(_mm512_add_ps(_mm512_mul_ps(dx2, dx2), _mm512_mul_ps(dy2, dy2)));

            // Body (0.5, 0.75), Radius 0.25
            __m512 dxb = _mm512_sub_ps(vPx, vHalf);
            __m512 dyb = _mm512_sub_ps(vPy, _mm512_set1_ps(0.75f));
            __m512 distB = _mm512_sqrt_ps(_mm512_add_ps(_mm512_mul_ps(dxb, dxb), _mm512_mul_ps(dyb, dyb)));

            // Head (0.5, 0.5), Radius 0.2
            __m512 dxh = _mm512_sub_ps(vPx, vHalf);
            __m512 dyh = _mm512_sub_ps(vPy, vHalf);
            __m512 distH = _mm512_sqrt_ps(_mm512_add_ps(_mm512_mul_ps(dxh, dxh), _mm512_mul_ps(dyh, dyh)));

            // Ears
            __m512 dxe1 = _mm512_sub_ps(vPx, _mm512_set1_ps(0.362f));
            __m512 dye1 = _mm512_sub_ps(vPy, _mm512_set1_ps(0.25f));
            __m512 distE1 = _mm512_sqrt_ps(_mm512_add_ps(_mm512_mul_ps(dxe1, dxe1), _mm512_mul_ps(dye1, dye1)));

            __m512 dxe2 = _mm512_sub_ps(vPx, _mm512_set1_ps(0.662f));
            __m512 dye2 = _mm512_sub_ps(vPy, _mm512_set1_ps(0.25f));
            __m512 distE2 = _mm512_sqrt_ps(_mm512_add_ps(_mm512_mul_ps(dxe2, dxe2), _mm512_mul_ps(dye2, dye2)));

            // Masks
            __mmask16 mEye1 = _mm512_cmp_ps_mask(dist1, _mm512_set1_ps(pulse_radius), _CMP_LT_OS);
            __mmask16 mEye2 = _mm512_cmp_ps_mask(dist2, _mm512_set1_ps(eye2_radius), _CMP_LT_OS);
            __mmask16 mEars = _mm512_or_mask16(_mm512_cmp_ps_mask(distE1, _mm512_set1_ps(0.075f), _CMP_LT_OS),
                                               _mm512_cmp_ps_mask(distE2, _mm512_set1_ps(0.075f), _CMP_LT_OS));
            __mmask16 mHead = _mm512_cmp_ps_mask(distH, _mm512_set1_ps(0.2f), _CMP_LT_OS);
            __mmask16 mBody = _mm512_cmp_ps_mask(distB, _mm512_set1_ps(0.25f), _CMP_LT_OS);

            // Noise for body
            __m512 vNoise = procedural_noise_avx512(_mm512_mul_ps(vPx, _mm512_set1_ps(100.0f)), 
                                                     _mm512_mul_ps(vPy, _mm512_set1_ps(100.0f)), 
                                                     time_offset);
            __mmask16 mSick = _mm512_cmp_ps_mask(vNoise, _mm512_set1_ps(0.8f), _CMP_GT_OS);

            for (int i = 0; i < 16; i++) {
                int px_idx = (y * W + x + i) * 3;
                uint8_t r = 0, g = 0, b = 0;
                uint16_t bit = 1 << i;

                if (mEye1 & bit || mEye2 & bit) {
                    r = 0; g = 255; b = 0;
                } else if (mEars & bit || mHead & bit) {
                    r = 255; g = 0; b = 0;
                } else if (mBody & bit) {
                    if (mSick & bit) {
                        r = 84; g = 107; b = 45;
                    } else {
                        r = 91; g = 63; b = 51;
                    }
                }
                pixels[px_idx] = r;
                pixels[px_idx+1] = g;
                pixels[px_idx+2] = b;
            }
        }
    }
}

int main() {
    printf("[TSFI] Ultra-Fast C Renderer: Initiating FFmpeg Pipe...
");
    
    char ffmpeg_cmd[512];
    snprintf(ffmpeg_cmd, sizeof(ffmpeg_cmd), 
             "ffmpeg -y -f rawvideo -vcodec rawvideo -s %dx%d -pix_fmt rgb24 -r %d -i - "
             "-c:v libx264 -preset fast -pix_fmt yuv420p assets/atropa_glsl_phase3_fast.mp4",
             W, H, FPS);
    
    FILE *pipe = popen(ffmpeg_cmd, "w");
    if (!pipe) {
        perror("popen");
        return 1;
    }

    uint8_t *pixels = (uint8_t*)malloc(W * H * 3);
    
    for (int f = 0; f < TOTAL_FRAMES; f++) {
        memset(pixels, 0, W * H * 3);
        render_frame(f, pixels);
        fwrite(pixels, 1, W * H * 3, pipe);
        if (f % 20 == 0) printf("  -> Frame %d/%d rendered.
", f, TOTAL_FRAMES);
    }

    free(pixels);
    pclose(pipe);
    printf("[SUCCESS] Render Complete: assets/atropa_glsl_phase3_fast.mp4
");
    return 0;
}
