#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <immintrin.h>
#include <sys/stat.h>
#include <sys/types.h>

#define WIDTH 512
#define HEIGHT 512
#define FRAMES 600

// ZMM Fast Inverse Square Root for distance calculations
static inline __m512 fast_inv_sqrt(__m512 x) {
    return _mm512_rsqrt14_ps(x);
}

void render_frame(uint8_t *pixels, int frame_idx) {
    float time_val = frame_idx * 0.05f;
    
    // ZMM Vectorized Pixel Math
    for (int y = 0; y < HEIGHT; y++) {
        float ny = (float)y / HEIGHT * 2.0f - 1.0f;
        __m512 vy = _mm512_set1_ps(ny);
        
        for (int x = 0; x < WIDTH; x += 16) {
            float nx[16];
            for (int i = 0; i < 16; i++) nx[i] = (float)(x + i) / WIDTH * 2.0f - 1.0f;
            __m512 vx = _mm512_loadu_ps(nx);
            
            // Base background (Wood/Floor texture)
            __m512 bg_r = _mm512_set1_ps(0.2f);
            __m512 bg_g = _mm512_set1_ps(0.1f);
            __m512 bg_b = _mm512_set1_ps(0.05f);
            
            // Table Center (Tea Table)
            __m512 tx = _mm512_set1_ps(0.0f);
            __m512 ty = _mm512_set1_ps(0.3f);
            __m512 dx = _mm512_sub_ps(vx, tx);
            __m512 dy = _mm512_sub_ps(vy, ty);
            __m512 dist_sq = _mm512_add_ps(_mm512_mul_ps(dx, dx), _mm512_mul_ps(dy, dy));
            __mmask16 table_mask = _mm512_cmp_ps_mask(dist_sq, _mm512_set1_ps(0.2f), _CMP_LT_OQ);
            
            // Table is white/gray
            bg_r = _mm512_mask_blend_ps(table_mask, bg_r, _mm512_set1_ps(0.9f));
            bg_g = _mm512_mask_blend_ps(table_mask, bg_g, _mm512_set1_ps(0.9f));
            bg_b = _mm512_mask_blend_ps(table_mask, bg_b, _mm512_set1_ps(0.9f));
            
            // 3 Teddy Bears at the table (Left, Right, Bottom)
            float bear_pos[3][2] = {
                {-0.5f, 0.3f},  // Left Bear
                {0.5f, 0.3f},   // Right Bear
                {0.0f, -0.4f}   // Bottom Bear
            };
            
            for (int b = 0; b < 3; b++) {
                // Bobbing animation for bears
                float bob = sinf(time_val + b * 2.0f) * 0.05f;
                __m512 bx = _mm512_set1_ps(bear_pos[b][0]);
                __m512 by = _mm512_set1_ps(bear_pos[b][1] + bob);
                
                __m512 bdx = _mm512_sub_ps(vx, bx);
                __m512 bdy = _mm512_sub_ps(vy, by);
                __m512 bdist_sq = _mm512_add_ps(_mm512_mul_ps(bdx, bdx), _mm512_mul_ps(bdy, bdy));
                __mmask16 bear_mask = _mm512_cmp_ps_mask(bdist_sq, _mm512_set1_ps(0.08f), _CMP_LT_OQ);
                
                // Bears are Brown
                bg_r = _mm512_mask_blend_ps(bear_mask, bg_r, _mm512_set1_ps(0.6f));
                bg_g = _mm512_mask_blend_ps(bear_mask, bg_g, _mm512_set1_ps(0.3f));
                bg_b = _mm512_mask_blend_ps(bear_mask, bg_b, _mm512_set1_ps(0.1f));
                
                // Bear Arms reaching for tea
                float arm_wave = sinf(time_val * 2.0f + b) * 0.1f;
                __m512 arm_x = _mm512_set1_ps(bear_pos[b][0] + (bear_pos[b][0] < 0 ? 0.2f : (bear_pos[b][0] > 0 ? -0.2f : 0.0f)));
                __m512 arm_y = _mm512_set1_ps(bear_pos[b][1] + arm_wave + 0.1f);
                
                __m512 adx = _mm512_sub_ps(vx, arm_x);
                __m512 ady = _mm512_sub_ps(vy, arm_y);
                __m512 adist_sq = _mm512_add_ps(_mm512_mul_ps(adx, adx), _mm512_mul_ps(ady, ady));
                __mmask16 arm_mask = _mm512_cmp_ps_mask(adist_sq, _mm512_set1_ps(0.02f), _CMP_LT_OQ);
                
                bg_r = _mm512_mask_blend_ps(arm_mask, bg_r, _mm512_set1_ps(0.7f));
                bg_g = _mm512_mask_blend_ps(arm_mask, bg_g, _mm512_set1_ps(0.4f));
                bg_b = _mm512_mask_blend_ps(arm_mask, bg_b, _mm512_set1_ps(0.2f));
            }
            
            // Teacups on the table
            for (int t = 0; t < 3; t++) {
                float cup_x = bear_pos[t][0] * 0.4f;
                float cup_y = 0.3f + (bear_pos[t][1] - 0.3f) * 0.4f;
                __m512 cx = _mm512_set1_ps(cup_x);
                __m512 cy = _mm512_set1_ps(cup_y);
                __m512 cdx = _mm512_sub_ps(vx, cx);
                __m512 cdy = _mm512_sub_ps(vy, cy);
                __m512 cdist_sq = _mm512_add_ps(_mm512_mul_ps(cdx, cdx), _mm512_mul_ps(cdy, cdy));
                __mmask16 cup_mask = _mm512_cmp_ps_mask(cdist_sq, _mm512_set1_ps(0.01f), _CMP_LT_OQ);
                
                bg_r = _mm512_mask_blend_ps(cup_mask, bg_r, _mm512_set1_ps(1.0f));
                bg_g = _mm512_mask_blend_ps(cup_mask, bg_g, _mm512_set1_ps(0.8f));
                bg_b = _mm512_mask_blend_ps(cup_mask, bg_b, _mm512_set1_ps(0.8f));
            }

            // Convert to 8-bit RGB
            __m512 scale = _mm512_set1_ps(255.0f);
            bg_r = _mm512_mul_ps(bg_r, scale);
            bg_g = _mm512_mul_ps(bg_g, scale);
            bg_b = _mm512_mul_ps(bg_b, scale);
            
            float rr[16], gg[16], bb[16];
            _mm512_storeu_ps(rr, bg_r);
            _mm512_storeu_ps(gg, bg_g);
            _mm512_storeu_ps(bb, bg_b);
            
            for (int i = 0; i < 16; i++) {
                int idx = (y * WIDTH + (x + i)) * 3;
                pixels[idx] = (uint8_t)rr[i];
                pixels[idx+1] = (uint8_t)gg[i];
                pixels[idx+2] = (uint8_t)bb[i];
            }
        }
    }
}

int main() {
    printf("[INFO] Executing AVX-512 ZMM Teddy Bear Tea Party Rendering Kernel...\n");
    mkdir("frames", 0777);
    
    uint8_t *pixels = malloc(WIDTH * HEIGHT * 3);
    
    for (int f = 0; f < FRAMES; f++) {
        render_frame(pixels, f);
        
        char filename[256];
        snprintf(filename, sizeof(filename), "frames/tea_%04d.ppm", f);
        FILE *fp = fopen(filename, "wb");
        if (fp) {
            fprintf(fp, "P6\n%d %d\n255\n", WIDTH, HEIGHT);
            fwrite(pixels, 1, WIDTH * HEIGHT * 3, fp);
            fclose(fp);
        }
        
        if (f % 60 == 0) {
            printf("  -> Rendered %d/%d frames (%.1f seconds)...\n", f, FRAMES, f / 60.0f);
        }
    }
    
    free(pixels);
    
    printf("\n[INFO] ZMM Rendering complete. Compiling 10-second MP4 via FFmpeg...\n");
    int ret = system("ffmpeg -y -framerate 60 -i frames/tea_%04d.ppm -c:v libx264 -pix_fmt yuv420p -preset fast tsfi_tea_party.mp4");
    if (ret != 0) {
        printf("[FRACTURE] FFmpeg failed to compile the video.\n");
        return 1;
    }
    
    printf("[SUCCESS] 10-Second Teddy Bear Tea Party Video generated natively: tsfi_tea_party.mp4\n");
    return 0;
}
