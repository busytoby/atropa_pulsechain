#include <immintrin.h>
#include <stddef.h>
#include <stdint.h>

void tsfi_helmholtz_reduce_8way_avx512(uint8_t *out, const uint8_t *leaves_bank) {
    __m512i sumA = _mm512_setzero_si512();
    __m512i sumB = _mm512_setzero_si512();

    // Process Chunks 0 and 1 in Bank A
    for (int l = 0; l < 8; l++) {
        sumA = _mm512_xor_si512(sumA, _mm512_loadu_si512((const __m512i*)(leaves_bank + l*256 + 0*64)));
        sumA = _mm512_xor_si512(sumA, _mm512_loadu_si512((const __m512i*)(leaves_bank + l*256 + 1*64)));
    }
    // Process Chunks 2 and 3 in Bank B
    for (int l = 0; l < 8; l++) {
        sumB = _mm512_xor_si512(sumB, _mm512_loadu_si512((const __m512i*)(leaves_bank + l*256 + 2*64)));
        sumB = _mm512_xor_si512(sumB, _mm512_loadu_si512((const __m512i*)(leaves_bank + l*256 + 3*64)));
    }

    __m512i sum = _mm512_xor_si512(sumA, sumB);
    __m256i lo = _mm512_extracti64x4_epi64(sum, 0);
    __m256i hi = _mm512_extracti64x4_epi64(sum, 1);
    __m256i res = _mm256_xor_si256(lo, hi);
    _mm256_storeu_si256((__m256i*)out, res);
}

