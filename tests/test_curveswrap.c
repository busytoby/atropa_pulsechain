#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include "../tsfi2-deepseek/inc/tsfi_curveswrap.h"

int main(void) {
    printf("=== RUNNING AUNCIENT CURVESWRAP TESTS ===\n");

    TSFiCurvesWrap cw;
    tsfi_curveswrap_init(&cw, TSFI_WRAP_PERIODIC);

    assert(cw.wrap_type == TSFI_WRAP_PERIODIC);

    // Verify periodic wrap modulo conversions
    assert(tsfi_curveswrap_resolve_index(&cw, 5, 5) == 0);
    assert(tsfi_curveswrap_resolve_index(&cw, -1, 5) == 4);

    // Verify non-periodic clamp conversions
    tsfi_curveswrap_init(&cw, TSFI_WRAP_NONPERIODIC);
    assert(tsfi_curveswrap_resolve_index(&cw, 5, 5) == 4);
    assert(tsfi_curveswrap_resolve_index(&cw, -1, 5) == 0);

    printf("   ✓ Periodic index wraps modulo verification verified.\n");
    printf("   ✓ Non-periodic index clamp verification verified successfully.\n");
    printf("=== AUNCIENT CURVESWRAP TESTS COMPLETE (PASS) ===\n");
    return 0;
}
