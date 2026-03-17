#include "tsfi_wave512.h"
#include <stdint.h>
#include <stdalign.h>
#include <string.h>
#include <immintrin.h>
#include <stdlib.h>

/**
 * TSFi Shimmer-Resonant Bloom Thunk (AVX-512)
 * 
 * Replaces stochastic jitter with deterministic resonance.
 * threshold_secret: A value that modulates the bloom barrier based on the simulation state.
 */

__attribute__((force_align_arg_pointer))
void tsfi_bloom_thunk(uint32_t* pixels, int width, int height, float threshold_secret, float intensity) {
    if (!pixels || width < 32) return;

    uint32_t* temp = (uint32_t*)calloc(width * height, sizeof(uint32_t));
    if (!temp) return;

    // The "Secret" modulates the global threshold
    __m512 v_base_thresh = _mm512_set1_ps(threshold_secret * 255.0f);
    __m512 v_intense = _mm512_set1_ps(intensity);
    __m512i v_x_ramp = _mm512_set_epi32(15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0);

    for (int y = 0; y < height; y++) {
        __m512i v_y = _mm512_set1_epi32(y);
        for (int x = 1; x < width - 17; x += 16) {
            // Resonance calculation: (x ^ y) as a spatial secret
            __m512i v_x = _mm512_add_epi32(_mm512_set1_epi32(x), v_x_ramp);
            __m512i v_res_key = _mm512_xor_si512(v_x, v_y);
            
            // Map resonance to a small float fluctuation [-0.05, 0.05]
            __m512 v_res_mod = _mm512_mul_ps(_mm512_cvtepi32_ps(_mm512_and_si512(v_res_key, _mm512_set1_epi32(0xFF))), _mm512_set1_ps(0.0004f));
            __m512 v_local_thresh = _mm512_add_ps(v_base_thresh, v_res_mod);

            __m512i v_p0 = _mm512_loadu_si512((void*)&pixels[y * width + x - 1]);
            __m512i v_p1 = _mm512_loadu_si512((void*)&pixels[y * width + x]);
            __m512i v_p2 = _mm512_loadu_si512((void*)&pixels[y * width + x + 1]);

            __m512 v_g0 = _mm512_cvtepi32_ps(_mm512_and_si512(_mm512_srli_epi32(v_p0, 8), _mm512_set1_epi32(0xFF)));
            __m512 v_g1 = _mm512_cvtepi32_ps(_mm512_and_si512(_mm512_srli_epi32(v_p1, 8), _mm512_set1_epi32(0xFF)));
            __m512 v_g2 = _mm512_cvtepi32_ps(_mm512_and_si512(_mm512_srli_epi32(v_p2, 8), _mm512_set1_epi32(0xFF)));

            __m512 v_avg = _mm512_mul_ps(_mm512_add_ps(v_g0, _mm512_add_ps(v_g1, v_g2)), _mm512_set1_ps(0.333f));
            
            // Shimmer Barrier: The threshold itself resonates spatially
            __mmask16 mask = _mm512_cmp_ps_mask(v_avg, v_local_thresh, _CMP_GT_OS);
            __m512 v_bloom = _mm512_maskz_mul_ps(mask, v_avg, v_intense);

            _mm512_storeu_si512((void*)&temp[y * width + x], _mm512_cvtps_epi32(v_bloom));
        }
    }

    for (int y = 1; y < height - 1; y++) {
        for (int x = 0; x < width - 16; x += 16) {
            __m512 v_b0 = _mm512_cvtepi32_ps(_mm512_loadu_si512((void*)&temp[(y-1) * width + x]));
            __m512 v_b1 = _mm512_cvtepi32_ps(_mm512_loadu_si512((void*)&temp[y * width + x]));
            __m512 v_b2 = _mm512_cvtepi32_ps(_mm512_loadu_si512((void*)&temp[(y+1) * width + x]));

            __m512 v_bloom_final = _mm512_mul_ps(_mm512_add_ps(v_b0, _mm512_add_ps(v_b1, v_b2)), _mm512_set1_ps(0.333f));

            __m512i v_orig = _mm512_loadu_si512((void*)&pixels[y * width + x]);
            __m512i v_bloom_int = _mm512_cvtps_epi32(v_bloom_final);
            
            __m512i v_res = _mm512_adds_epu8(v_orig, _mm512_or_si512(_mm512_slli_epi32(v_bloom_int, 16), 
                                                   _mm512_or_si512(_mm512_slli_epi32(v_bloom_int, 8), v_bloom_int)));
            
            _mm512_storeu_si512((void*)&pixels[y * width + x], v_res);
        }
    }

    free(temp);
}

__attribute__((force_align_arg_pointer))
void tsfi_bloom_thunk_ab4h(uint16_t* pixels, int width, int height, float threshold_secret, float intensity) {
    if (!pixels || width < 32) return;

    float* temp = (float*)calloc(width * height, sizeof(float));
    if (!temp) return;

    __m512 v_base_thresh = _mm512_set1_ps(threshold_secret);
    __m512 v_intense = _mm512_set1_ps(intensity);
    __m512i v_x_ramp = _mm512_set_epi32(15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0);

    for (int y = 0; y < height; y++) {
        __m512i v_y = _mm512_set1_epi32(y);
        for (int x = 1; x < width - 17; x += 16) {
            __m512i v_x = _mm512_add_epi32(_mm512_set1_epi32(x), v_x_ramp);
            __m512i v_res_key = _mm512_xor_si512(v_x, v_y);
            __m512 v_res_mod = _mm512_mul_ps(_mm512_cvtepi32_ps(_mm512_and_si512(v_res_key, _mm512_set1_epi32(0xFF))), _mm512_set1_ps(0.0004f));
            __m512 v_local_thresh = _mm512_add_ps(v_base_thresh, v_res_mod);

            // Extract G channel for luminance (simplified)
            __m512 v_g1 = _mm512_cvtph_ps(_mm256_set_epi16(
                pixels[(y*width+x+15)*4+1], pixels[(y*width+x+14)*4+1], pixels[(y*width+x+13)*4+1], pixels[(y*width+x+12)*4+1],
                pixels[(y*width+x+11)*4+1], pixels[(y*width+x+10)*4+1], pixels[(y*width+x+9)*4+1],  pixels[(y*width+x+8)*4+1],
                pixels[(y*width+x+7)*4+1],  pixels[(y*width+x+6)*4+1],  pixels[(y*width+x+5)*4+1],  pixels[(y*width+x+4)*4+1],
                pixels[(y*width+x+3)*4+1],  pixels[(y*width+x+2)*4+1],  pixels[(y*width+x+1)*4+1],  pixels[(y*width+x+0)*4+1]
            ));

            __mmask16 mask = _mm512_cmp_ps_mask(v_g1, v_local_thresh, _CMP_GT_OS);
            __m512 v_bloom = _mm512_maskz_mul_ps(mask, v_g1, v_intense);

            _mm512_storeu_ps(&temp[y * width + x], v_bloom);
        }
    }

    for (int y = 1; y < height - 1; y++) {
        for (int x = 0; x < width - 16; x += 16) {
            __m512 v_b0 = _mm512_loadu_ps(&temp[(y-1) * width + x]);
            __m512 v_b1 = _mm512_loadu_ps(&temp[y * width + x]);
            __m512 v_b2 = _mm512_loadu_ps(&temp[(y+1) * width + x]);

            __m512 v_bloom_final = _mm512_mul_ps(_mm512_add_ps(v_b0, _mm512_add_ps(v_b1, v_b2)), _mm512_set1_ps(0.333f));

            alignas(64) float bloom_results[16];
            _mm512_store_ps(bloom_results, v_bloom_final);

            for(int i=0; i<16; i++) {
                float bloom_val = bloom_results[i];
                if (bloom_val > 0) {
                    for(int c=0; c<3; c++) {
                        uint16_t h_val = pixels[(y*width+x+i)*4+c];
                        __m128 f_vec = _mm_cvtph_ps(_mm_set1_epi16(h_val));
                        float f_val = _mm_cvtss_f32(f_vec);
                        f_val += bloom_val;
                        if (f_val > 1.0f) f_val = 1.0f;
                        __m128i h_res_vec = _mm_cvtps_ph(_mm_set_ss(f_val), 0);
                        uint16_t h_res = (uint16_t)_mm_extract_epi16(h_res_vec, 0);
                        pixels[(y*width+x+i)*4+c] = h_res;
                    }
                }
            }
        }
    }

    free(temp);
}
