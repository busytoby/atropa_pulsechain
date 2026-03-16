#include <math.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @brief Baseline Scalar Thetan Synthesis (Mind -> YANG)
 */
void tsfi_yang_synthesis_scalar(float *out, const float *in, size_t count) {
    for (size_t i = 0; i < count; i++) {
        // Simple phase modulation simulation
        out[i] = sinf(in[i]) * 0.5f + 0.5f;
    }
}

