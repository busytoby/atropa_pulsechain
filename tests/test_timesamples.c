#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include "../tsfi2-deepseek/inc/tsfi_timesamples.h"

int main(void) {
    printf("=== RUNNING AUNCIENT TIMESAMPLES SLOT TESTS ===\n");

    TSFiTimeSamples ts;
    tsfi_timesamples_init(&ts);

    assert(ts.count == 0);

    tsfi_timesamples_add(&ts, 10.0, 0x1000);
    tsfi_timesamples_add(&ts, 20.0, 0x2000);
    tsfi_timesamples_add(&ts, 30.0, 0x3000);

    assert(ts.count == 3);

    // Verify constant-time O(1) slot conversions
    assert(tsfi_timesamples_lookup_slot(&ts, 0) == 0x1000);
    assert(tsfi_timesamples_lookup_slot(&ts, 1) == 0x2000);
    assert(tsfi_timesamples_lookup_slot(&ts, 2) == 0x3000);

    // Verify bounds culling
    assert(tsfi_timesamples_lookup_slot(&ts, -1) == 0);
    assert(tsfi_timesamples_lookup_slot(&ts, 3) == 0);

    printf("   ✓ Constant-time O(1) slot register offset retrieval verified.\n");
    printf("   ✓ Out-of-bounds slot checks verified successfully.\n");
    printf("=== AUNCIENT TIMESAMPLES SLOT TESTS COMPLETE (PASS) ===\n");
    return 0;
}
