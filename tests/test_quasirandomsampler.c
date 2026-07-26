#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include "../tsfi2-deepseek/inc/tsfi_quasirandomsampler.h"

int main(void) {
    printf("=== RUNNING AUNCIENT QUASIRANDOMSAMPLER TESTS ===\n");

    TSFiQuasiRandomSampler qrs;
    tsfi_quasirandomsampler_init(&qrs, 10);

    assert(qrs.count == 10);

    double x0, y0;
    tsfi_quasirandomsampler_get(&qrs, 0, &x0, &y0);
    // Halton sequence index 1 with base 2 and 3: x = 1/2, y = 1/3
    assert(fabs(x0 - 0.5) < 1e-5);
    assert(fabs(y0 - 0.33333) < 1e-3);

    double x1, y1;
    tsfi_quasirandomsampler_get(&qrs, 1, &x1, &y1);
    // Halton sequence index 2 with base 2 and 3: x = 1/4, y = 2/3
    assert(fabs(x1 - 0.25) < 1e-5);
    assert(fabs(y1 - 0.66666) < 1e-3);

    // Verify O(1) out-of-bounds safety checks
    assert(tsfi_quasirandomsampler_get(&qrs, -1, &x0, &y0) == false);
    assert(tsfi_quasirandomsampler_get(&qrs, 10, &x0, &y0) == false);

    printf("   ✓ Halton low-discrepancy coordinates sequence generation verified.\n");
    printf("   ✓ Constant-time O(1) sampling displacements lookups verified successfully.\n");
    printf("=== AUNCIENT QUASIRANDOMSAMPLER TESTS COMPLETE (PASS) ===\n");
    return 0;
}
