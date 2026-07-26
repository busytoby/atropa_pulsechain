#include <stdio.h>
#include <assert.h>
#include <math.h>
#include "../tsfi2-deepseek/inc/tsfi_krowz_sickness.h"

int main(void) {
    printf("=== RUNNING AUNCIENT KROWZ SICKNESS TESTS ===\n");

    TSFiKrowzSickness ks;
    tsfi_krowz_sickness_init(&ks, 0.7, 0.4);

    assert(fabs(ks.saturation_threshold - 0.7) < 1e-5);
    assert(fabs(ks.glare_factor - 0.4) < 1e-5);

    // Test below threshold
    double val_low = tsfi_krowz_sickness_eval(&ks, 0.5);
    assert(fabs(val_low - 0.5) < 1e-5);

    // Test above threshold causes exponential glare sickness saturation scaling
    double val_high = tsfi_krowz_sickness_eval(&ks, 0.9);
    assert(val_high > 0.9);

    printf("   ✓ kr0wZ sickness glare parameters initialized successfully.\n");
    printf("   ✓ Specular reflection glare saturation verified successfully.\n");
    printf("=== AUNCIENT KROWZ SICKNESS TESTS COMPLETE (PASS) ===\n");
    return 0;
}
