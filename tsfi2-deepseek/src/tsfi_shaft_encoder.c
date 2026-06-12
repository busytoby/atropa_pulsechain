#include "tsfi_shaft_encoder.h"
#include <math.h>

uint32_t tsfi_angle_to_binary(double theta, int bits) {
    double pi2 = 2.0 * 3.14159265358979323846;
    double normalized = fmod(theta, pi2);
    if (normalized < 0.0) {
        normalized += pi2;
    }
    uint64_t max_val = 1ULL << bits;
    uint32_t val = (uint32_t)((normalized / pi2) * (double)max_val);
    return val & (max_val - 1);
}

uint32_t tsfi_binary_to_gray(uint32_t binary) {
    return binary ^ (binary >> 1);
}

uint32_t tsfi_gray_to_binary(uint32_t gray, int bits) {
    uint32_t binary = 0;
    for (int i = bits - 1; i >= 0; i--) {
        uint32_t gray_bit = (gray >> i) & 1;
        uint32_t parent_bit = (i == bits - 1) ? 0 : ((binary >> (i + 1)) & 1);
        binary |= (gray_bit ^ parent_bit) << i;
    }
    return binary;
}
