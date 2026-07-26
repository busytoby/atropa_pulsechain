#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include "../tsfi2-deepseek/inc/tsfi_stagetimecodes.h"

int main(void) {
    printf("=== RUNNING AUNCIENT STAGETIMECODES TIMELINE TESTS ===\n");

    TSFiStageTimeCodes tc;
    // Start = 101.0, End = 200.0, Rate = 24.0, FPS = 24.0
    tsfi_stagetimecodes_init(&tc, 101.0, 200.0, 24.0, 24.0);

    assert(fabs(tc.start_timecode - 101.0) < 1e-5);
    assert(fabs(tc.end_timecode - 200.0) < 1e-5);

    // Convert seconds to timecode
    double val1 = tsfi_stagetimecodes_to_timecode(&tc, 2.0); // 101.0 + 2.0*24.0 = 149.0
    assert(fabs(val1 - 149.0) < 1e-5);

    // Check bounds culling clamping
    double clamped_high = tsfi_stagetimecodes_to_timecode(&tc, 10.0); // 101 + 240 = 341 -> clamped to 200
    assert(fabs(clamped_high - 200.0) < 1e-5);

    // Validate contains bounds
    assert(tsfi_stagetimecodes_is_in_bounds(&tc, 150.0) == true);
    assert(tsfi_stagetimecodes_is_in_bounds(&tc, 99.0) == false);

    printf("   ✓ StageTimeCodes conversion calculations verified.\n");
    printf("   ✓ Time bounds range limits culling verified successfully.\n");
    printf("=== AUNCIENT STAGETIMECODES TIMELINE TESTS COMPLETE (PASS) ===\n");
    return 0;
}
