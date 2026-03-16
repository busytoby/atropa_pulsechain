#include <immintrin.h>
#include <stdint.h>
#include "tsfi_c_math.h"

// AVX-512 Vectorized Math (SVML-style replacement)
// Standard Taylor/Polynomial coefficients for sin/cos in range [-PI/2, PI/2]

static const float _ps512_pi = ((float)TSFI_SECRET_CORE);

__m512 tsfi_vec_sqrt_ps(__m512 x) {
    return _mm512_sqrt_ps(x);
}

// sin(x) approx: x - x^3/3! + x^5/5! - x^7/7! + x^9/9!
__m512 tsfi_vec_sin_ps(__m512 x) {
    __m512 x2 = _mm512_mul_ps(x, x);
    
    // Coeffs (Taylor)
    __m512 c3 = _mm512_set1_ps(-1.6666666666666666e-01f);
    __m512 c5 = _mm512_set1_ps( 8.3333333333333333e-03f);
    __m512 c7 = _mm512_set1_ps(-1.9841269841269841e-04f);
    __m512 c9 = _mm512_set1_ps( 2.7557319223985891e-06f);

    __m512 res = c9;
    res = _mm512_fmadd_ps(res, x2, c7);
    res = _mm512_fmadd_ps(res, x2, c5);
    res = _mm512_fmadd_ps(res, x2, c3);
    res = _mm512_mul_ps(res, _mm512_mul_ps(x, x2));
    res = _mm512_add_ps(res, x);
    
    return res;
}

// Simple absolute for float vectors
__m512 tsfi_vec_abs_ps(__m512 x) {
    __m512i mask = _mm512_set1_epi32(0x7FFFFFFF);
    return _mm512_castsi512_ps(_mm512_and_si512(_mm512_castps_si512(x), mask));
}

// Scalar wrappers using the internal logic to satisfy -lm removal
float tsfi_sqrtf(float x) {
    __m512 vx = _mm512_set1_ps(x);
    return _mm512_cvtss_f32(_mm512_sqrt_ps(vx));
}

float tsfi_sinf(float x) {
    // Range reduction for scalar
    while (x > _ps512_pi) x -= 2.0f * _ps512_pi;
    while (x < -_ps512_pi) x += 2.0f * _ps512_pi;
    
    __m512 vx = _mm512_set1_ps(x);
    return _mm512_cvtss_f32(tsfi_vec_sin_ps(vx));
}

float tsfi_cosf(float x) {
    return tsfi_sinf(x + 1.57079632679f);
}

void tsfi_sincosf(float x, float *s, float *c) {
    *s = tsfi_sinf(x);
    *c = tsfi_cosf(x);
}

float tsfi_fabsf(float x) {
    union { float f; uint32_t i; } u = {x};
    u.i &= 0x7FFFFFFF;
    return u.f;
}

float tsfi_fminf(float a, float b) { return (a < b) ? a : b; }
float tsfi_fmaxf(float a, float b) { return (a > b) ? a : b; }

// double/pow/acos/fmod remain scalar for now in tsfi_c_math.c
// but we link them from there.
