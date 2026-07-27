#include "tsfi_usdshade.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT USDSHADE PARSER UNIT TESTS\n");
    printf("=============================================================\n");

    // Test Case 1: Initializing USD Shade Context
    printf("[TEST] Initializing USD Shade Context...\n");
    TSFiUsdShade shade;
    tsfi_usdshade_init(&shade, "RedPlastic");
    assert(strcmp(shade.material_id, "RedPlastic") == 0);
    // Initial parameters check
    assert(shade.roughness == 0.5f);
    assert(shade.metallic == 0.0f);
    printf("   ✓ USD Shade Context initialized (Material: %s).\n", shade.material_id);

    // Test Case 2: Bind dynamic registers to shader parameters
    printf("[TEST] Binding dynamic register charge to shade parameters...\n");
    // Under Verlet discharge physics: albedo and roughness are modulated by FET discharge nodes
    tsfi_usdshade_bind_registers(&shade, 1.5f, 0.1f);
    printf("   ✓ Register binding complete.\n");

    // Test Case 3: Execute Verlet soft-body physics discharge cycle step
    printf("[TEST] Executing Verlet soft-body physics discharge step...\n");
    tsfi_usdshade_step_discharge(&shade, 0.016f); // 1 frame step
    printf("   ✓ Verlet FET discharge physics step completed successfully.\n");

    printf("=============================================================\n");
    printf("ALL USDSHADE PARSER TESTS PASSED SUCCESSFULLY\n");
    printf("=============================================================\n");
    return 0;
}
