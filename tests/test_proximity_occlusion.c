#include <stdio.h>
#include <assert.h>
#include <math.h>
#include "../tsfi2-deepseek/inc/tsfi_proximity_occlusion.h"

int main(void) {
    printf("=== RUNNING AUNCIENT PROXIMITY OCCLUSION TESTS ===\n");

    TSFiProximityOcclusion po;
    tsfi_proximity_occlusion_init(&po, 5.0, 0.8);

    assert(fabs(po.max_distance - 5.0) < 1e-5);
    assert(fabs(po.intensity - 0.8) < 1e-5);

    // Test zero distance occlusion has max contact shadow
    double val0 = tsfi_proximity_occlusion_eval(&po, 0.0);
    assert(fabs(val0 - 0.2) < 1e-5);

    // Test distance >= max_distance has no occlusion
    double val_max = tsfi_proximity_occlusion_eval(&po, 6.0);
    assert(fabs(val_max - 1.0) < 1e-5);

    // Test mid-distance linear decay
    double val_mid = tsfi_proximity_occlusion_eval(&po, 2.5);
    assert(fabs(val_mid - 0.6) < 1e-5);

    printf("   ✓ Proximity occlusion parameters initialized successfully.\n");
    printf("   ✓ Contact shadow calculations verified successfully.\n");
    printf("=== AUNCIENT PROXIMITY OCCLUSION TESTS COMPLETE (PASS) ===\n");
    return 0;
}
