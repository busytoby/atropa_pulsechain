#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>

typedef enum {
    CUTOFF_STATE,
    CONDUC_STATE
} gate_state_t;

// PL/I FIXED BINARY(P,Q) simulation representation
typedef struct {
    uint64_t value; // Fixed-point integer value representation
    uint32_t p;     // Precision (maximum bit width allowed)
    uint32_t q;     // Scale (fractional bit width shift)
    gate_state_t write_gate;
} pli_fixed_bin_t;

// -------------------------------------------------------------
// PL/I FIXED BINARY Multiplication with Precision Bounds Check
// -------------------------------------------------------------
bool pli_fixed_bin_mul(const pli_fixed_bin_t *a, const pli_fixed_bin_t *b, 
                        pli_fixed_bin_t *res) {
    res->write_gate = CUTOFF_STATE;

    // Perform fixed-point multiplication: (a * b) >> q
    uint64_t raw_mul = a->value * b->value;
    uint64_t val_shifted = raw_mul >> a->q;

    // Wortman Bounds Check: Verify result fits within precision bit limit P
    uint64_t max_limit = (1ULL << res->p) - 1;
    if (val_shifted > max_limit) {
        return false; // Precision Overflow: write blocked
    }

    res->write_gate = CONDUC_STATE;
    res->value = val_shifted;
    return true;
}

// -------------------------------------------------------------
// Unit Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT PL/I FIXED-POINT PRECISION VALIDATION SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    // FIXED BINARY(8, 4) -> 8-bit precision, 4-bit fractional scale
    // Value representing 2.5: 2.5 * 2^4 = 40 (0x28)
    pli_fixed_bin_t a = { .value = 40, .p = 8, .q = 4, .write_gate = CUTOFF_STATE };
    
    // Value representing 3.0: 3.0 * 2^4 = 48 (0x30)
    pli_fixed_bin_t b = { .value = 48, .p = 8, .q = 4, .write_gate = CUTOFF_STATE };

    pli_fixed_bin_t res = { .value = 0, .p = 8, .q = 4, .write_gate = CUTOFF_STATE };

    // 1. Valid multiplication (2.5 * 3.0 = 7.5 -> Represented as 7.5 * 16 = 120 <= 255 limit)
    printf("[TEST] Multiplying valid fixed-point variables (2.5 * 3.0)...\n");
    fflush(stdout);
    bool ok = pli_fixed_bin_mul(&a, &b, &res);
    assert(ok == true);
    assert(res.write_gate == CONDUC_STATE);
    assert(res.value == 120); // 7.5
    printf("   ✓ Result calculated successfully within precision: %u (7.5)\n", (uint32_t)res.value);
    fflush(stdout);

    // 2. Precision Overflow (Multiplier set to large value, exceeding 8-bit limit)
    printf("[TEST] Triggering precision overflow multiplication...\n");
    fflush(stdout);
    // Value representing 10.0: 10.0 * 2^4 = 160
    pli_fixed_bin_t large_b = { .value = 160, .p = 8, .q = 4, .write_gate = CUTOFF_STATE };

    // 2.5 * 10.0 = 25.0 -> Represented as 25.0 * 16 = 400 > 255 limit
    ok = pli_fixed_bin_mul(&a, &large_b, &res);
    assert(ok == false);
    assert(res.write_gate == CUTOFF_STATE); // Cutoff active
    printf("   ✓ Precision overflow caught: operation blocked.\n");
    fflush(stdout);

    printf("=============================================================\n");
    printf("PL/I FIXED-POINT PRECISION TESTS PASSED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
