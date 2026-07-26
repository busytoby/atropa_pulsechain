#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include "../tsfi2-deepseek/inc/tsfi_pointssizes.h"

int main(void) {
    printf("=== RUNNING AUNCIENT POINTSSIZES TESTS ===\n");

    TSFiPointsSizes ps;
    tsfi_pointssizes_init(&ps);

    assert(ps.count == 0);

    tsfi_pointssizes_add(&ps, 1.5);
    tsfi_pointssizes_add(&ps, 3.0);
    tsfi_pointssizes_add(&ps, 4.5);

    assert(ps.count == 3);
    assert(fabs(tsfi_pointssizes_get(&ps, 0) - 1.5) < 1e-5);
    assert(fabs(tsfi_pointssizes_get(&ps, 1) - 3.0) < 1e-5);
    assert(fabs(tsfi_pointssizes_get(&ps, 2) - 4.5) < 1e-5);

    // Verify out-of-bounds gets default size
    assert(fabs(tsfi_pointssizes_get(&ps, -1) - 1.0) < 1e-5);
    assert(fabs(tsfi_pointssizes_get(&ps, 3) - 1.0) < 1e-5);

    printf("   ✓ Points rendering diameters additions verified.\n");
    printf("   ✓ Point size index retrieves verified successfully.\n");
    printf("=== AUNCIENT POINTSSIZES TESTS COMPLETE (PASS) ===\n");
    return 0;
}
