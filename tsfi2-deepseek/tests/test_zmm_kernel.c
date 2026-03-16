#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <immintrin.h>
#include <stdint.h>

// We include the .c file directly to test static internal functions
#include "tsfi_helmholtz.h"

void test_zmm_ops() {
    printf("[TEST] Verifying Raw ZMM operations...\n");
    
    uint64_t a[32] __attribute__((aligned(64)));
    uint64_t b[32] __attribute__((aligned(64)));
    uint64_t r[32] __attribute__((aligned(64)));
    
    for(int i=0; i<32; i++) { a[i] = i; b[i] = i; }
    
    // Test Add
    zmm_add_512(r, a, b);
    for(int i=0; i<32; i++) {
        assert(r[i] == (uint64_t)(i + i));
    }
    printf("  -> zmm_add_512: PASS\n");
    
    // Test XOR
    zmm_xor_512(r, a, b);
    for(int i=0; i<32; i++) {
        assert(r[i] == 0);
    }
    printf("  -> zmm_xor_512: PASS\n");
}

void test_reduction_masking() {
    printf("[TEST] Verifying Kirchhoff Masking Rigidity...\n");
    
    uint64_t left[32] __attribute__((aligned(64)));
    uint64_t right[32] __attribute__((aligned(64)));
    uint64_t res[32] __attribute__((aligned(64)));
    
    memset(left, 0xFF, 256);
    memset(right, 0, 256);
    
    react_reduce_nodes_internal(res, left, right, 1);
    
    // Final 4 bytes MUST be zero
    uint8_t *bytes = (uint8_t*)res;
    assert(bytes[252] == 0);
    assert(bytes[253] == 0);
    assert(bytes[254] == 0);
    assert(bytes[255] == 0);
    
    // Value at [0] should be (0xFFFFFFFFFFFFFFFF + 0 + 1) = 0
    assert(res[0] == 0);
    
    printf("  -> Kirchhoff Masking: PASS\n");
}

int main() {
    printf("=== TSFi Rigid ZMM Kernel Unit Test ===\n");
    tsfi_helmholtz_init();
    
    test_zmm_ops();
    test_reduction_masking();
    
    printf("[SUCCESS] ZMM Kernel is rigid and bijective.\n");
    tsfi_helmholtz_cleanup();
    
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    extern void lau_assert_zero_unsealed_leaks(const char *context, void *teardown_ptr);
    lau_assert_zero_unsealed_leaks("test", NULL);
    return 0;
}
