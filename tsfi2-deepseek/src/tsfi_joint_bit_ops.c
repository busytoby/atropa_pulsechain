#include <stdint.h>
uint64_t tsfi_bit_mask_collect(uint64_t value, uint64_t mask) {
    return value & mask;
}
