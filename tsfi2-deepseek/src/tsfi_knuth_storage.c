#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "tsfi_knuth_storage.h"

uint64_t tsfi_knuth_pack_base2i(int32_t x_real, int32_t y_imag) {
    uint64_t packed = 0;
    int32_t r = x_real;
    int32_t i = y_imag;

    // Convert real part 'r' to base -4 digits, stored in even positions (bits 0, 2, 4, 6...)
    int pos = 0;
    while (r != 0 && pos < 16) {
        int rem = r % -4;
        r /= -4;
        if (rem < 0) {
            rem += 4;
            r += 1;
        }
        packed |= ((uint64_t)(rem & 3) << (pos * 4));
        pos++;
    }

    // Convert imag part 'i' to base -4 digits, stored in odd positions (bits 2, 6, 10...)
    pos = 0;
    while (i != 0 && pos < 16) {
        int rem = i % -4;
        i /= -4;
        if (rem < 0) {
            rem += 4;
            i += 1;
        }
        packed |= ((uint64_t)(rem & 3) << (pos * 4 + 2));
        pos++;
    }

    return packed;
}

int tsfi_knuth_unpack_base2i(uint64_t packed_base2i, tsfi_knuth_complex_pair_t *out_pair) {
    if (!out_pair) return -1;
    out_pair->x_real = 0;
    out_pair->y_imag = 0;

    int32_t mult_r = 1;
    int32_t mult_i = 1;

    for (int k = 0; k < 16; k++) {
        uint8_t digit_r = (packed_base2i >> (k * 4)) & 3;
        uint8_t digit_i = (packed_base2i >> (k * 4 + 2)) & 3;

        out_pair->x_real += (int32_t)digit_r * mult_r;
        out_pair->y_imag += (int32_t)digit_i * mult_i;

        mult_r *= -4;
        mult_i *= -4;
    }

    return 0;
}

uint32_t tsfi_knuth_calculate_gas_savings(uint32_t state_pair_count) {
    uint32_t unoptimized_gas = state_pair_count * 2 * 20000; // 2 SSTOREs per pair
    uint32_t optimized_gas   = state_pair_count * 1 * 20000; // 1 SSTORE per pair
    return unoptimized_gas - optimized_gas;
}
