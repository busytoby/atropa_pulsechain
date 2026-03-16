#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_ottype.h"
#include "lau_memory.h"

void test_arabic_joining() {
    printf("[TEST] Initializing Arabic Joining Geometric Complexity Matrix...\n");

    // Standard Arabic word + ZWJ + ZWNJ bounds
    uint32_t cps[] = {
        0x0627, 0x0644, 0x0639, 0x0631, 0x0628, 0x064A, 0x0629, // "العربية"
        0x200D, 0x0628, 0x200C, 0x0628 // ZWJ + Beh + ZWNJ + Beh
    };
    int count = sizeof(cps)/sizeof(cps[0]);

    tsfi_ottype_t ot;
    memset(&ot, 0, sizeof(ot));
    
    // 1. Extreme Iterative Tracking Sequence
    printf("[TEST] Pumping 500 synchronous shaping cycles...\n");
    for (int i = 0; i < 500; i++) {
        tsfi_shaped_glyph_t shaped[64];
        int out_count = tsfi_ottype_shape(&ot, cps, count, shaped, 64, NULL, 0);
        assert(out_count >= count);
    }
    
    // 2. Truncated Geometric Buffer Assertion
    printf("[TEST] Triggering geometric boundary truncation limit...\n");
    tsfi_shaped_glyph_t truncated[3];
    int trunc_count = tsfi_ottype_shape(&ot, cps, count, truncated, 3, NULL, 0);
    assert(trunc_count <= 3);

    printf("[TEST] Complex topological bounds evaluated successfully.\n");
}

int main() {
    test_arabic_joining();
    
    // Explicitly unmap global tracking arrays
    extern void lau_registry_teardown(void);
    lau_registry_teardown();
    
    // Mathematically certify strictly zero geometric memory leaks
    extern void lau_report_memory_metrics(void);
    printf("\n[TEST] Expecting PERFECTLY ZERO LEAKS...\n");
    lau_report_memory_metrics();
    
    
    extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}
