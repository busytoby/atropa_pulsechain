#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <immintrin.h>
#include <string.h>

#define WIDTH 1024
#define HEIGHT 1536

// Optimized ZMM Coordinate generation
static inline __m512 get_vpx(int x) {
    static const float base_x[16] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    __m512 vbase = _mm512_loadu_ps(base_x);
    __m512 vx = _mm512_add_ps(_mm512_set1_ps((float)x), vbase);
    return _mm512_mul_ps(vx, _mm512_set1_ps(1.0f / WIDTH));
}

static inline __m512 fast_hash_zmm(__m512 px, __m512 py, __m512 vtime) {
    __m512i ix = _mm512_cvtps_epi32(_mm512_mul_ps(px, _mm512_set1_ps(1000.0f)));
    __m512i iy = _mm512_cvtps_epi32(_mm512_mul_ps(py, _mm512_set1_ps(1000.0f)));
    __m512i it = _mm512_cvtps_epi32(_mm512_mul_ps(vtime, _mm512_set1_ps(100.0f)));
    
    __m512i h = _mm512_xor_si512(ix, iy);
    h = _mm512_xor_si512(h, it);
    h = _mm512_mullo_epi32(h, _mm512_set1_epi32(0x45d9f3b));
    
    __m512 res = _mm512_cvtepi32_ps(_mm512_and_si512(h, _mm512_set1_epi32(0x7FFFFF)));
    return _mm512_mul_ps(res, _mm512_set1_ps(1.0f / 8388608.0f));
}

void render_bear_avx512_fast(uint32_t *frame_buffer, int frame) {
    float pulse = sinf((float)frame * 0.1f);
    __m512 vtime = _mm512_set1_ps((float)frame * 0.1f);
    float pr = 0.02f + (pulse * 0.005f);
    float e2r = 0.045f + (pulse * 0.01f);
    __m512 v_pr2 = _mm512_set1_ps(pr * pr);
    __m512 v_e2r2 = _mm512_set1_ps(e2r * e2r);
    __m512 v_ear2 = _mm512_set1_ps(0.075f * 0.075f);
    __m512 v_cran2 = _mm512_set1_ps(0.2f * 0.2f);
    __m512 v_body2 = _mm512_set1_ps(0.25f * 0.25f);

    __m512i v_white = _mm512_set1_epi32(0xFFFFFFFF);
    __m512i v_green = _mm512_set1_epi32(0xFF00FF00);
    __m512i v_red   = _mm512_set1_epi32(0xFFFF0000);
    __m512i v_brown = _mm512_set1_epi32(0xFF5C4033);
    __m512i v_sick  = _mm512_set1_epi32(0xFF556B2F);
    __m512i v_void  = _mm512_set1_epi32(0xFF000000);

    for (int y = 0; y < HEIGHT; y++) {
        __m512 vpy = _mm512_set1_ps((float)y / HEIGHT);
        uint32_t *row = &frame_buffer[y * WIDTH];

        for (int x = 0; x < WIDTH; x += 16) {
            __m512 vpx = get_vpx(x);
            __m512i vout = v_void;

            // 1. Eyes
            __m512 dx = _mm512_sub_ps(vpx, _mm512_set1_ps(0.5f));
            __m512 dy1 = _mm512_sub_ps(vpy, _mm512_set1_ps(0.4f));
            __m512 dy2 = _mm512_sub_ps(vpy, _mm512_set1_ps(0.6f));
            __m512 dsq1 = _mm512_fmadd_ps(dx, dx, _mm512_mul_ps(dy1, dy1));
            __m512 dsq2 = _mm512_fmadd_ps(dx, dx, _mm512_mul_ps(dy2, dy2));
            __mmask16 m_eyes = _mm512_cmp_ps_mask(dsq1, v_pr2, _CMP_LT_OQ) | _mm512_cmp_ps_mask(dsq2, v_e2r2, _CMP_LT_OQ);
            vout = _mm512_mask_blend_epi32(m_eyes, vout, v_green);

            // 2. Ears
            __m512 ex1 = _mm512_sub_ps(vpx, _mm512_set1_ps(0.362f));
            __m512 ex2 = _mm512_sub_ps(vpx, _mm512_set1_ps(0.662f));
            __m512 ey = _mm512_sub_ps(vpy, _mm512_set1_ps(0.25f));
            __m512 esq1 = _mm512_fmadd_ps(ex1, ex1, _mm512_mul_ps(ey, ey));
            __m512 esq2 = _mm512_fmadd_ps(ex2, ex2, _mm512_mul_ps(ey, ey));
            __mmask16 m_ears = _mm512_cmp_ps_mask(esq1, v_ear2, _CMP_LT_OQ) | _mm512_cmp_ps_mask(esq2, v_ear2, _CMP_LT_OQ);
            vout = _mm512_mask_blend_epi32(_mm512_kandn(m_eyes, m_ears), vout, v_red);

            // 3. Cranium
            __m512 cy = _mm512_sub_ps(vpy, _mm512_set1_ps(0.5f));
            __m512 csq = _mm512_fmadd_ps(dx, dx, _mm512_mul_ps(cy, cy));
            __mmask16 m_cran = _mm512_cmp_ps_mask(csq, v_cran2, _CMP_LT_OQ);
            vout = _mm512_mask_blend_epi32(_mm512_kandn(m_eyes | m_ears, m_cran), vout, v_red);

            // 4. Body
            __m512 by = _mm512_sub_ps(vpy, _mm512_set1_ps(0.75f));
            __m512 bsq = _mm512_fmadd_ps(dx, dx, _mm512_mul_ps(by, by));
            __mmask16 m_body = _mm512_cmp_ps_mask(bsq, v_body2, _CMP_LT_OQ);
            __mmask16 body_only = _mm512_kandn(m_eyes | m_ears | m_cran, m_body);
            
            if (body_only) {
                __m512 noise = fast_hash_zmm(vpx, vpy, vtime);
                __mmask16 m_sick = _mm512_cmp_ps_mask(noise, _mm512_set1_ps(0.8f), _CMP_GT_OQ);
                vout = _mm512_mask_blend_epi32(_mm512_kand(body_only, m_sick), vout, v_sick);
                vout = _mm512_mask_blend_epi32(_mm512_kandn(m_sick, body_only), vout, v_brown);
            }

            _mm512_storeu_si512(&row[x], vout);
        }
    }
}

int main() {
    printf("=== TSFi ULTRA FAST AVX-512 Teddy Bear Rendering Benchmark ===\n");
    int w = WIDTH, h = HEIGHT;
    uint32_t *px = _mm_malloc(w * h * 4, 64);
    if (!px) return 1;

    int frames = 1000;
    for(int i=0; i<frames; i++) {
        render_bear_avx512_fast(px, i);
        if (i % 100 == 0) { printf("ZMM FAST Rendered frame %d\n", i); fflush(stdout); }
    }

    printf("[SUCCESS] AVX-512 Rendered %d frames locally.\n", frames);
    _mm_free(px);
    return 0;
}
