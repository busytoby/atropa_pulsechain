#ifndef TSFI_KNUTH_STORAGE_H
#define TSFI_KNUTH_STORAGE_H

#include <stdint.h>
#include <stddef.h>

typedef struct {
    int32_t x_real;
    int32_t y_imag;
} tsfi_knuth_complex_pair_t;

/* Pack 32-bit signed integer pair (x_real, y_imag) into a single 64-bit Knuth Base 2i storage word */
uint64_t tsfi_knuth_pack_base2i(int32_t x_real, int32_t y_imag);

/* Unpack 64-bit Knuth Base 2i storage word back into 32-bit signed integer pair (x_real, y_imag) */
int tsfi_knuth_unpack_base2i(uint64_t packed_base2i, tsfi_knuth_complex_pair_t *out_pair);

/* Calculate gas savings for EVM SSTORE operations using Knuth Base 2i packing */
uint32_t tsfi_knuth_calculate_gas_savings(uint32_t state_pair_count);

#endif // TSFI_KNUTH_STORAGE_H
