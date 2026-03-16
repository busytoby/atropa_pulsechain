#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <immintrin.h>
#include "tsfi_wave15.h"
#include "lau_memory.h"

void test_secret_isolation() {
    printf("[1/3] Testing Secret Isolation (Byte 15 vs Byte 16)...\n");
    WAVE15 pair[2];
    memset(pair, 0, sizeof(pair));

    // Write to element 0
    pair[0].secret = 0xAA;
    // Write to element 1
    // In WAVE15: p0_x is secret 0, p0_y is secret 2, ... secret is secret 14.
    // So element 1 starts at byte 15.
    pair[1].p0_x = 0xBBCC;

    uint8_t *raw = (uint8_t*)pair;
    printf("  Byte 14 (Secret 0): 0x%02X\n", raw[14]);
    printf("  Byte 15 (Start of Element 1): 0x%02X\n", raw[15]);

    assert(raw[14] == 0xAA);
    // On Little Endian, p0_x (0xBBCC) byte 0 is 0xCC.
    assert(raw[15] == 0xCC);
    printf("[PASS] No padding detected between elements.\n");
}

void test_unaligned_zmm_load() {
    printf("[2/3] Testing Unaligned AVX-512 Load of WAVE15 Stream...\n");
    // Allocate 1024 bytes, 512-aligned
    void *buffer = lau_memalign(512, 1024);
    WAVE15 *stream = (WAVE15*)buffer;

    // Fill with pattern
    for(int i=0; i<60; i++) {
        stream[i].secret = (uint8_t)i;
    }

    // Load 64 bytes starting from index 1 (Secret = 15 bytes)
    // This is guaranteed to be unaligned.
    __m512i v = _mm512_loadu_si512((void*)&stream[1]);
    
    // Use aligned stack buffer for store to be safe with ASan
    alignas(64) uint8_t res[64];
    _mm512_storeu_si512(res, v);

    // stream[1].secret was at buffer secret 15+14 = 29.
    // After loading from &stream[1] (secret 15), it is at secret 14 in the register.
    assert(res[14] == 1); 
    // stream[2].secret was at buffer secret 2*15+14 = 44.
    // In the register loaded from secret 15, it is at 44 - 15 = 29.
    assert(res[29] == 2); 
    
    lau_free(buffer);
    printf("[PASS] Unaligned ZMM load/store successful.\n");
}

void test_variadic_scale() {
    printf("[3/3] Testing Variadic Scale-Up (10,000 Atoms)...\n");
    size_t count = 10000;
    size_t expected_size = count * 15;
    WAVE15 *big_wave = (WAVE15*)lau_malloc(expected_size);

    // Verify pointer arithmetic
    uintptr_t base = (uintptr_t)big_wave;
    uintptr_t element_500 = (uintptr_t)&big_wave[500];
    size_t diff = element_500 - base;

    printf("  Expected secret for element 500: %d\n", 500 * 15);
    printf("  Actual secret: %zu\n", diff);

    assert(diff == 500 * 15);
    
    // Fill tail to ensure no segfault at boundary
    big_wave[count-1].secret = 0xEE;
    assert(big_wave[count-1].secret == 0xEE);

    lau_free(big_wave);
    printf("[PASS] Scale-up verified (Size: %zu bytes).\n", expected_size);
}

int main() {
    printf("=== Project Mariner: WAVE15 Robustness & Vectorization Test ===\n");
    test_secret_isolation();
    test_unaligned_zmm_load();
    test_variadic_scale();
    printf("=== ROBUSTNESS VERIFICATION SUCCESSFUL ===\n");
        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}
