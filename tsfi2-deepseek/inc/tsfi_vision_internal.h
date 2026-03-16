#include "tsfi_vision.h"
#include "lau_memory.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tsfi_c_math.h"
#include <immintrin.h>
#include <dirent.h>

#define LOG(...) fprintf(stderr, __VA_ARGS__)

#ifndef sqrtf
#define sqrtf tsfi_sqrtf
#endif
#ifndef fminf
#define fminf tsfi_fminf
#endif
#ifndef fmaxf
#define fmaxf tsfi_fmaxf
#endif


#ifndef TSFI_VISION_INTERNAL_H
#define TSFI_VISION_INTERNAL_H

bool tsfi_classification_load(const char *name, LauClassificationAsset *out);
static inline __m512 get_lum_avx512(__m512i v_px, __m512 v_inv255) {
    __m512 v_r = _mm512_mul_ps(_mm512_cvtepi32_ps(_mm512_and_si512(_mm512_srli_epi32(v_px, 16), _mm512_set1_epi32(0xFF))), v_inv255);
    __m512 v_g = _mm512_mul_ps(_mm512_cvtepi32_ps(_mm512_and_si512(_mm512_srli_epi32(v_px, 8), _mm512_set1_epi32(0xFF))), v_inv255);
    __m512 v_b = _mm512_mul_ps(_mm512_cvtepi32_ps(_mm512_and_si512(v_px, _mm512_set1_epi32(0xFF))), v_inv255);
    return _mm512_mul_ps(_mm512_add_ps(v_r, _mm512_add_ps(v_g, v_b)), _mm512_set1_ps(0.333f));
}

static inline __m512i safe_load_si512(const uint32_t *data, int idx, int limit) {
    if (idx < 0 || idx + 16 > limit) return _mm512_setzero_si512();
    return _mm512_loadu_si512((void*)&data[idx]);
}

#endif
