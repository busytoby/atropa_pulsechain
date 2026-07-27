#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include "../tsfi2-deepseek/inc/tsfi_usdshade.h"

int main(void) {
    printf("=== RUNNING AUNCIENT USDSHADE MATERIAL & PHYSICS TESTS ===\n");

    TSFiUsdShade shade;
    tsfi_usdshade_init(&shade, "/auncient/material/gold_foil");

    assert(strcmp(shade.material_id, "/auncient/material/gold_foil") == 0);

    // Initial check on default parameters
    assert(fabs(shade.albedo[0] - 0.8f) < 1e-5f);
    assert(fabs(shade.roughness - 0.5f) < 1e-5f);
    assert(fabs(shade.metallic - 0.0f) < 1e-5f);

    // Bind registers and check material property modulation
    tsfi_usdshade_bind_registers(&shade, 1.0f, 0.1f);
    
    // Albedo, roughness, metallic should be modulated by the positive charge values
    assert(shade.metallic > 0.0f);
    assert(shade.roughness < 1.0f);

    // Capture initial positions before simulation step
    float initial_z = shade.nodes[0][0].position[2];

    // Step physics simulation
    tsfi_usdshade_step_discharge(&shade, 0.01f);

    // Position coordinate on Z axis should have changed due to discharge acceleration force
    float simulated_z = shade.nodes[0][0].position[2];
    assert(simulated_z > initial_z);

    printf("   ✓ UsdShade dynamic material parameter bindings verified.\n");
    printf("   ✓ Verlet soft-body physics FET node grid integration verified.\n");
    printf("=== AUNCIENT USDSHADE MATERIAL & PHYSICS TESTS COMPLETE (PASS) ===\n");
    return 0;
}
