#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include "../tsfi2-deepseek/inc/tsfi_timesampleinterpolation.h"

int main(void) {
    printf("=== RUNNING AUNCIENT TIMESAMPLEINTERPOLATION TESTS ===\n");

    TSFiTimeSampleInterpolation tsi;
    tsfi_timesampleinterpolation_init(&tsi, TSFI_INTERPOLATE_LINEAR);

    assert(tsi.mode == TSFI_INTERPOLATE_LINEAR);

    // Verify linear interpolation values
    double val_lin = tsfi_timesampleinterpolation_eval(&tsi, 15.0, 10.0, 100.0, 20.0, 200.0);
    assert(fabs(val_lin - 150.0) < 1e-5);

    // Verify held mode values
    tsfi_timesampleinterpolation_init(&tsi, TSFI_INTERPOLATE_HELD);
    double val_held = tsfi_timesampleinterpolation_eval(&tsi, 15.0, 10.0, 100.0, 20.0, 200.0);
    assert(fabs(val_held - 100.0) < 1e-5);

    printf("   ✓ Linear keyframe coordinate interpolation verified.\n");
    printf("   ✓ Held step coordinate interpolation verified successfully.\n");
    printf("=== AUNCIENT TIMESAMPLEINTERPOLATION TESTS COMPLETE (PASS) ===\n");
    return 0;
}
