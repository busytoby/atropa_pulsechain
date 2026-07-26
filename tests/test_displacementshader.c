#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include "../tsfi2-deepseek/inc/tsfi_displacementshader.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

int main(void) {
    printf("=== RUNNING AUNCIENT DISPLACEMENTSHADER TESTS ===\n");

    TSFiDisplacementShader ds;
    tsfi_displacementshader_init(&ds, 2.5, 1.5);

    assert(fabs(ds.amplitude - 2.5) < 1e-5);
    assert(fabs(ds.frequency - 1.5) < 1e-5);

    // Verify displacement values driven by AUTODIN timeline times
    double disp0 = tsfi_displacementshader_eval(&ds, 0.0, 0.0);
    double disp_peak = tsfi_displacementshader_eval(&ds, M_PI / 3.0, 0.0); // sin(pi/2) = 1.0

    assert(fabs(disp0 - 0.0) < 1e-5);
    assert(fabs(disp_peak - 2.5) < 1e-5);

    // Verify WinchesterMQ 8-bit boundary constraint scaling wrapping
    double disp_wrap = tsfi_displacementshader_eval(&ds, 256.0 + M_PI / 3.0, 0.0);
    assert(fabs(disp_wrap - 2.5) < 1e-5);

    // Verify registration callback gets execution pointer
    void *registered_ptr = NULL;
    void mock_register(const char *name, void *ptr) {
        if (strcmp(name, "tsfi_displacementshader_eval") == 0) {
            registered_ptr = ptr;
        }
    }
    tsfi_displacementshader_register_xplsm(mock_register);
    assert(registered_ptr == (void*)tsfi_displacementshader_eval);

    printf("   ✓ Displacement amplitude and frequency initialized.\n");
    printf("   ✓ AUTODIN lock-paced coordinate displacement evaluated successfully.\n");
    printf("   ✓ WinchesterMQ register boundary constraint scaling verified successfully.\n");
    printf("   ✓ XPLSM dynamic symbol registration verified successfully.\n");
    printf("=== AUNCIENT DISPLACEMENTSHADER TESTS COMPLETE (PASS) ===\n");
    return 0;
}
