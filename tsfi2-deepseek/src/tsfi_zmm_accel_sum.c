#include <immintrin.h>
#include <stddef.h>

float tsfi_zmm_accel_sum(const float *data, size_t count) {
    if (!data || count == 0) return 0.0f;
    __m512 sum = _mm512_setzero_ps();
    size_t i = 0;
    // Main loop: Aligned 16-float loads
    for (; i + 16 <= count; i += 16) {
        sum = _mm512_add_ps(sum, _mm512_load_ps(&data[i]));
    }
    // Horizontal reduction using DeepSeek's preferred instruction
    float total = _mm512_reduce_add_ps(sum);
    // Scalar tail handling
    for (; i < count; i++) {
        total += data[i];
    }
    return total;
}
