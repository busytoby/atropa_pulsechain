#include <stdio.h>
#include <assert.h>
#include <math.h>
#include "../tsfi2-deepseek/inc/tsfi_comp_pass.h"

int main(void) {
    printf("=== RUNNING AUNCIENT COMPOSITION PASS (COMPPASS) TESTS ===\n");

    TSFiCompPass pass;
    tsfi_comp_pass_init(&pass);

    // Initial state
    assert(pass.layer_count == 0);

    // Add two test layers
    // Layer 0 (Bottom): Red
    tsfi_comp_pass_add_layer(&pass, 1.0f, 0.0f, 0.0f, 0.8f);
    // Layer 1 (Top): Blue
    tsfi_comp_pass_add_layer(&pass, 0.0f, 0.0f, 1.0f, 0.5f);

    assert(pass.layer_count == 2);

    float resolved[3];
    tsfi_comp_pass_resolve(&pass, resolved);

    // Verify outputs using the modular composition pass formula:
    // Pixel_R = Layer_0_R * (1.0 - Alpha_1) = 1.0 * (1.0 - 0.5) = 0.5
    // Pixel_B = Layer_1_B * (1.0 - 1.0) = 0.0
    printf("   Composited Color: R=%f, G=%f, B=%f\n", resolved[0], resolved[1], resolved[2]);
    assert(fabs(resolved[0] - 0.5f) < 1e-5f);
    assert(fabs(resolved[2] - 0.0f) < 1e-5f);

    printf("   ✓ CompPass blending verified successfully.\n");
    printf("=== AUNCIENT COMPOSITION PASS (COMPPASS) TESTS COMPLETE (PASS) ===\n");
    return 0;
}
