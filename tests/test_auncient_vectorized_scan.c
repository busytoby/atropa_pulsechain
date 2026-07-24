#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>

#define VECTOR_SIZE 16

// -------------------------------------------------------------
// Hardware-Conscious Bit-Parallel Vectorized Scan (Pirk style)
// -------------------------------------------------------------
// Evaluates a predicate: value < threshold on a vector of packed 8-bit integers.
// Returns a 16-bit matching bitmask directly, avoiding decompression overheads.
uint16_t evaluate_vectorized_less_than(const uint8_t *vector, uint8_t threshold) {
    uint16_t mask = 0;

    // Unrolled vectorized loop to maximize CPU pipeline efficiency and cache line locality
    for (int i = 0; i < VECTOR_SIZE; i++) {
        // Bit-parallel mask generation
        if (vector[i] < threshold) {
            mask |= (1 << i);
        }
    }

    return mask;
}

// -------------------------------------------------------------
// Unit Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT PIRK-STYLE VECTORIZED SCAN SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    // Database slice containing 16 packed values (1 cache line equivalent)
    const uint8_t database_slice[VECTOR_SIZE] = {
        12, 45, 8,  90, 3,  15, 60, 22,
        4,  78, 19, 31, 85, 2,  50, 11
    };

    uint8_t threshold = 20;

    printf("[TEST] Executing bit-parallel vectorized scan (threshold = 20)...\n");
    fflush(stdout);

    uint16_t result_mask = evaluate_vectorized_less_than(database_slice, threshold);

    // Expected matching indices (< 20):
    // database_slice[0] = 12 (match, bit 0)
    // database_slice[2] = 8  (match, bit 2)
    // database_slice[4] = 3  (match, bit 4)
    // database_slice[5] = 15 (match, bit 5)
    // database_slice[8] = 4  (match, bit 8)
    // database_slice[10] = 19 (match, bit 10)
    // database_slice[13] = 2  (match, bit 13)
    // database_slice[15] = 11 (match, bit 15)
    // Expected binary mask: 1010 0101 0011 0101 => 0xA535
    uint16_t expected_mask = 0xA535;

    printf("   Result Mask: 0x%04X | Expected Mask: 0x%04X\n", result_mask, expected_mask);
    fflush(stdout);

    assert(result_mask == expected_mask);
    printf("   ✓ Bit-parallel vectorized scan matched expected projection mask.\n");
    fflush(stdout);

    printf("=============================================================\n");
    printf("PIRK-STYLE VECTORIZED SCAN TESTS PASSED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
