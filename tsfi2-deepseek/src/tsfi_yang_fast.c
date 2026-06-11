#include <immintrin.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @brief Fast AVX-512 Yang Synthesis using FMA and Vectorization.
 * Processes 16 floats per iteration using super-scalar execution.
 */
void tsfi_yang_synthesis_avx512(float *out, const float *in, size_t count) {
    size_t i = 0;
    
    // Constants for 5th-order minimax polynomial approximation of sin(x) on [-PI, PI]:
    // sin(x) ~ x * (c1 + x^2 * (c3 + x^2 * c5))
    __m512 c5 = _mm512_set1_ps(0.00564311785f);
    __m512 c3 = _mm512_set1_ps(-0.155271411f);
    __m512 c1 = _mm512_set1_ps(0.987862168f);
    __m512 v_half = _mm512_set1_ps(0.5f);
    
    // Process 16 floats per iteration
    for (; i + 16 <= count; i += 16) {
        __m512 v_in = _mm512_loadu_ps(&in[i]);
        
        // x^2
        __m512 v_in2 = _mm512_mul_ps(v_in, v_in);
        
        // c5 * x^2 + c3
        __m512 v_poly = _mm512_fmadd_ps(c5, v_in2, c3);
        
        // (c5 * x^2 + c3) * x^2 + c1
        v_poly = _mm512_fmadd_ps(v_poly, v_in2, c1);
        
        // ((c5 * x^2 + c3) * x^2 + c1) * x -> sin(x)
        __m512 v_sin = _mm512_mul_ps(v_poly, v_in);
        
        // sin(x) * 0.5 + 0.5
        __m512 v_out = _mm512_fmadd_ps(v_sin, v_half, v_half);
        
        _mm512_storeu_ps(&out[i], v_out);
    }
    
    // Tail handling
    for (; i < count; i++) {
        float x = in[i];
        float x2 = x * x;
        float poly = (0.00564311785f * x2 - 0.155271411f) * x2 + 0.987862168f;
        float s = poly * x;
        out[i] = s * 0.5f + 0.5f;
    }
}
