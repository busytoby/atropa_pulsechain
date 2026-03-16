#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <immintrin.h>
#include <string.h>

#define WIDTH 1024
#define HEIGHT 1536

// Alternative fast hash if libmvec is missing: bitwise XOR-shift style
static inline __m512 fast_hash_zmm(__m512 px, __m512 py, float time_offset) {
    __m512i ix = _mm512_castps_si512(_mm512_mul_ps(px, _mm512_set1_ps(1000.0f)));
    __m512i iy = _mm512_castps_si512(_mm512_mul_ps(py, _mm512_set1_ps(1000.0f)));
    __m512i it = _mm512_set1_epi32((int)(time_offset * 100.0f));
    
    __m512i h = _mm512_xor_si512(ix, iy);
    h = _mm512_xor_si512(h, it);
    h = _mm512_mullo_epi32(h, _mm512_set1_epi32(0x45d9f3b));
    h = _mm512_xor_si512(h, _mm512_srli_epi32(h, 16));
    
    __m512 res = _mm512_cvtepi32_ps(_mm512_and_si512(h, _mm512_set1_epi32(0x7FFFFF)));
    return _mm512_div_ps(res, _mm512_set1_ps(8388608.0f));
}

void render_bear_avx512(uint32_t *frame_buffer, int frame) {
    float pulse = sinf((float)frame * 0.1f);
    float time_offset = frame * 0.1f;
    float pulse_radius = 0.02f + (pulse * 0.005f);
    float eye2_radius = 0.045f + (pulse * 0.01f);

    // Standard colors
    __m512 brown_r = _mm512_set1_ps(92.0f);
    __m512 brown_g = _mm512_set1_ps(64.0f);
    __m512 brown_b = _mm512_set1_ps(51.0f);

    __m512 dark_brown_r = _mm512_set1_ps(62.0f);
    __m512 dark_brown_g = _mm512_set1_ps(39.0f);
    __m512 dark_brown_b = _mm512_set1_ps(35.0f);

    __m512 sick_green_r = _mm512_set1_ps(84.0f);
    __m512 sick_green_g = _mm512_set1_ps(107.0f);
    __m512 sick_green_b = _mm512_set1_ps(45.0f);

    for (int y = 0; y < HEIGHT; y++) {
        float py = (float)y / HEIGHT;
        __m512 vpy = _mm512_set1_ps(py);

        for (int x = 0; x < WIDTH; x += 16) {
            float nx_arr[16];
            for (int i=0; i<16; i++) nx_arr[i] = (float)(x+i) / WIDTH;
            __m512 vpx = _mm512_loadu_ps(nx_arr);

            __m512 out_r = _mm512_setzero_ps();
            __m512 out_g = _mm512_setzero_ps();
            __m512 out_b = _mm512_setzero_ps();
            __mmask16 final_mask = 0;

            // 1. Eyes (Top Layer)
            __m512 dx1 = _mm512_sub_ps(vpx, _mm512_set1_ps(0.5f));
            __m512 dy1 = _mm512_sub_ps(vpy, _mm512_set1_ps(0.4f));
            __m512 dsq1 = _mm512_add_ps(_mm512_mul_ps(dx1, dx1), _mm512_mul_ps(dy1, dy1));
            __mmask16 mask_eye1 = _mm512_cmp_ps_mask(dsq1, _mm512_set1_ps(pulse_radius * pulse_radius), _CMP_LT_OQ);

            __m512 dx2 = _mm512_sub_ps(vpx, _mm512_set1_ps(0.5f));
            __m512 dy2 = _mm512_sub_ps(vpy, _mm512_set1_ps(0.6f));
            __m512 dsq2 = _mm512_add_ps(_mm512_mul_ps(dx2, dx2), _mm512_mul_ps(dy2, dy2));
            __mmask16 mask_eye2 = _mm512_cmp_ps_mask(dsq2, _mm512_set1_ps(eye2_radius * eye2_radius), _CMP_LT_OQ);

            __mmask16 mask_eyes = mask_eye1 | mask_eye2;
            out_r = _mm512_mask_blend_ps(mask_eyes, out_r, _mm512_setzero_ps());
            out_g = _mm512_mask_blend_ps(mask_eyes, out_g, _mm512_set1_ps(255.0f));
            out_b = _mm512_mask_blend_ps(mask_eyes, out_b, _mm512_setzero_ps());
            final_mask |= mask_eyes;

            // 2. Ears
            __m512 ear1_x = _mm512_sub_ps(vpx, _mm512_set1_ps(0.362f));
            __m512 ear1_y = _mm512_sub_ps(vpy, _mm512_set1_ps(0.25f));
            __m512 ear1_dsq = _mm512_add_ps(_mm512_mul_ps(ear1_x, ear1_x), _mm512_mul_ps(ear1_y, ear1_y));
            
            __m512 ear2_x = _mm512_sub_ps(vpx, _mm512_set1_ps(0.662f));
            __m512 ear2_y = _mm512_sub_ps(vpy, _mm512_set1_ps(0.25f));
            __m512 ear2_dsq = _mm512_add_ps(_mm512_mul_ps(ear2_x, ear2_x), _mm512_mul_ps(ear2_y, ear2_y));
            
            __mmask16 mask_ears = _mm512_cmp_ps_mask(ear1_dsq, _mm512_set1_ps(0.075f * 0.075f), _CMP_LT_OQ) |
                                  _mm512_cmp_ps_mask(ear2_dsq, _mm512_set1_ps(0.075f * 0.075f), _CMP_LT_OQ);
            
            __mmask16 ears_layer = _mm512_kandn(final_mask, mask_ears);
            out_r = _mm512_mask_blend_ps(ears_layer, out_r, _mm512_set1_ps(255.0f)); // Red ears as per python
            out_g = _mm512_mask_blend_ps(ears_layer, out_g, _mm512_setzero_ps());
            out_b = _mm512_mask_blend_ps(ears_layer, out_b, _mm512_setzero_ps());
            final_mask |= mask_ears;

            // 3. Cranium
            __m512 cran_x = _mm512_sub_ps(vpx, _mm512_set1_ps(0.5f));
            __m512 cran_y = _mm512_sub_ps(vpy, _mm512_set1_ps(0.5f));
            __m512 cran_dsq = _mm512_add_ps(_mm512_mul_ps(cran_x, cran_x), _mm512_mul_ps(cran_y, cran_y));
            __mmask16 mask_cran = _mm512_cmp_ps_mask(cran_dsq, _mm512_set1_ps(0.2f * 0.2f), _CMP_LT_OQ);
            
            __mmask16 cran_layer = _mm512_kandn(final_mask, mask_cran);
            out_r = _mm512_mask_blend_ps(cran_layer, out_r, _mm512_set1_ps(255.0f)); // Red head
            out_g = _mm512_mask_blend_ps(cran_layer, out_g, _mm512_setzero_ps());
            out_b = _mm512_mask_blend_ps(cran_layer, out_b, _mm512_setzero_ps());
            final_mask |= mask_cran;

            // 4. Sick Body
            __m512 body_x = _mm512_sub_ps(vpx, _mm512_set1_ps(0.5f));
            __m512 body_y = _mm512_sub_ps(vpy, _mm512_set1_ps(0.75f));
            __m512 body_dsq = _mm512_add_ps(_mm512_mul_ps(body_x, body_x), _mm512_mul_ps(body_y, body_y));
            __mmask16 mask_body = _mm512_cmp_ps_mask(body_dsq, _mm512_set1_ps(0.25f * 0.25f), _CMP_LT_OQ);
            
            __mmask16 body_layer = _mm512_kandn(final_mask, mask_body);
            if (body_layer) {
                __m512 noise = fast_hash_zmm(_mm512_mul_ps(vpx, _mm512_set1_ps(100.0f)), 
                                             _mm512_mul_ps(vpy, _mm512_set1_ps(100.0f)), 
                                             time_offset);
                __mmask16 sick_mask = _mm512_cmp_ps_mask(noise, _mm512_set1_ps(0.8f), _CMP_GT_OQ);
                __mmask16 body_sick = _mm512_kand(body_layer, sick_mask);
                __mmask16 body_brown = _mm512_kandn(sick_mask, body_layer);
                
                out_r = _mm512_mask_blend_ps(body_sick, out_r, sick_green_r);
                out_g = _mm512_mask_blend_ps(body_sick, out_g, sick_green_g);
                out_b = _mm512_mask_blend_ps(body_sick, out_b, sick_green_b);
                
                out_r = _mm512_mask_blend_ps(body_brown, out_r, brown_r);
                out_g = _mm512_mask_blend_ps(body_brown, out_g, brown_g);
                out_b = _mm512_mask_blend_ps(body_brown, out_b, brown_b);
            }

            // Convert to 32-bit pixel (ARGB)
            __m512i vr = _mm512_cvtps_epi32(out_r);
            __m512i vg = _mm512_cvtps_epi32(out_g);
            __m512i vb = _mm512_cvtps_epi32(out_b);
            __m512i va = _mm512_set1_epi32(0xFF);

            // Pack components: (A << 24) | (R << 16) | (G << 8) | B
            __m512i vpix = _mm512_or_si512(_mm512_slli_epi32(va, 24),
                           _mm512_or_si512(_mm512_slli_epi32(vr, 16),
                           _mm512_or_si512(_mm512_slli_epi32(vg, 8), vb)));

            _mm512_storeu_si512(&frame_buffer[y * WIDTH + x], vpix);
        }
    }
}

int main() {
    printf("=== TSFi AVX-512 Teddy Bear Rendering Benchmark (TRAPPING RICCI) ===\n");
    int w = WIDTH, h = HEIGHT;
    uint32_t *px = _mm_malloc(w * h * 4, 64);
    if (!px) return 1;

    int frames = 1000;
    for(int i=0; i<frames; i++) {
        render_bear_avx512(px, i);
        if (i % 10 == 0) { printf("ZMM Rendered frame %d\n", i); fflush(stdout); }
    }

    printf("[SUCCESS] AVX-512 Rendered %d frames locally.\n", frames);
    _mm_free(px);
    return 0;
}
