#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <immintrin.h>
#include <string.h>

#define WIDTH 512
#define HEIGHT 512

void zmm_texture_map(uint8_t* output_buffer, const uint8_t* master_frame, int frame_idx) {
    float time_val = frame_idx * (1.0f / 30.0f);
    
    float arm_wave = sinf(time_val * 3.0f); 
    float body_bob = sinf(time_val * 1.5f) * 0.05f; 
    
    for (int y = 0; y < HEIGHT; y++) {
        float ny = (float)y / HEIGHT * 2.0f - 1.0f;
        __m512 vy = _mm512_set1_ps(ny);
        
        for (int x = 0; x < WIDTH; x += 16) {
            float nx_arr[16];
            for (int i=0; i<16; i++) nx_arr[i] = (float)(x+i) / WIDTH * 2.0f - 1.0f;
            __m512 vx = _mm512_loadu_ps(nx_arr);
            
            __m512 out_r = _mm512_setzero_ps();
            __m512 out_g = _mm512_setzero_ps();
            __m512 out_b = _mm512_setzero_ps();
            __m512 out_z = _mm512_set1_ps(100.0f); 
            
            #define MAP_SPHERE(cx, cy, radius, base_z, tx_offset, ty_offset) do { \
                __m512 dx = _mm512_sub_ps(vx, _mm512_set1_ps(cx)); \
                __m512 dy = _mm512_sub_ps(vy, _mm512_set1_ps(cy)); \
                __m512 dsq = _mm512_add_ps(_mm512_mul_ps(dx, dx), _mm512_mul_ps(dy, dy)); \
                __m512 rad_sq = _mm512_set1_ps((radius) * (radius)); \
                __mmask16 mask = _mm512_cmp_ps_mask(dsq, rad_sq, _CMP_LT_OQ); \
                __m512 z_local = _mm512_sub_ps(rad_sq, dsq); \
                __m512 intensity = _mm512_div_ps(z_local, rad_sq); \
                __m512 z_global = _mm512_sub_ps(_mm512_set1_ps(base_z), intensity); \
                __mmask16 z_mask = _mm512_kand(mask, _mm512_cmp_ps_mask(z_global, out_z, _CMP_LT_OQ)); \
                if (z_mask) { \
                    out_z = _mm512_mask_blend_ps(z_mask, out_z, z_global); \
                    for (int i = 0; i < 16; i++) { \
                        if ((z_mask >> i) & 1) { \
                            float u = (nx_arr[i] - (cx) + (radius)) / (2.0f * (radius)); \
                            float v = (ny - (cy) + (radius)) / (2.0f * (radius)); \
                            int src_x = (int)(((tx_offset) + u * 0.4f) * WIDTH); \
                            int src_y = (int)(((ty_offset) + v * 0.4f) * HEIGHT); \
                            if (src_x >= 0 && src_x < WIDTH && src_y >= 0 && src_y < HEIGHT) { \
                                int src_idx = (src_y * WIDTH + src_x) * 3; \
                                float shade = 0.5f + 0.5f * (z_local[i] / ((radius)*(radius))); \
                                out_r[i] = (float)master_frame[src_idx] * shade; \
                                out_g[i] = (float)master_frame[src_idx+1] * shade; \
                                out_b[i] = (float)master_frame[src_idx+2] * shade; \
                            } \
                        } \
                    } \
                } \
            } while(0)
            
            MAP_SPHERE(0.0f, 0.4f + body_bob, 0.45f, 5.0f, 0.3f, 0.4f); 
            MAP_SPHERE(0.0f, -0.1f + body_bob, 0.35f, 3.0f, 0.32f, 0.1f); 
            
            float l_arm_y = 0.3f + arm_wave * 0.10f + body_bob;
            float r_arm_y = 0.3f - arm_wave * 0.10f + body_bob;
            float l_arm_x = -0.35f + arm_wave * 0.05f;
            float r_arm_x = 0.35f + arm_wave * 0.05f;
            MAP_SPHERE(l_arm_x, l_arm_y, 0.15f, 4.0f, 0.1f, 0.4f); 
            MAP_SPHERE(r_arm_x, r_arm_y, 0.15f, 4.0f, 0.7f, 0.4f); 
            
            // Render the Hawk Moth Pet geometry
            float moth_time = time_val * 8.0f;
            float moth_x = 0.6f + sinf(time_val * 2.0f) * 0.2f;
            float moth_y = -0.4f + cosf(time_val * 3.0f) * 0.1f;
            MAP_SPHERE(moth_x, moth_y, 0.08f, 2.0f, 0.8f, 0.1f); // Moth Body
            float wing_span = 0.1f + sinf(moth_time) * 0.05f;
            MAP_SPHERE(moth_x - wing_span, moth_y, 0.05f, 2.1f, 0.8f, 0.2f); // L Wing
            MAP_SPHERE(moth_x + wing_span, moth_y, 0.05f, 2.1f, 0.8f, 0.2f); // R Wing
            
            float rr[16], gg[16], bb[16];
            _mm512_storeu_ps(rr, out_r);
            _mm512_storeu_ps(gg, out_g);
            _mm512_storeu_ps(bb, out_b);
            
            for (int i = 0; i < 16; i++) {
                int idx = (y * WIDTH + (x + i)) * 3;
                output_buffer[idx]   = (uint8_t)(rr[i] < 0.0f ? 0.0f : (rr[i] > 255.0f ? 255.0f : rr[i]));
                output_buffer[idx+1] = (uint8_t)(gg[i] < 0.0f ? 0.0f : (gg[i] > 255.0f ? 255.0f : gg[i]));
                output_buffer[idx+2] = (uint8_t)(bb[i] < 0.0f ? 0.0f : (bb[i] > 255.0f ? 255.0f : bb[i]));
            }
        }
    }
}
