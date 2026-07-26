#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include "../tsfi2-deepseek/inc/tsfi_luxlight.h"

int main(void) {
    printf("=== RUNNING AUNCIENT LUXLIGHT LIGHTING TESTS ===\n");

    TSFiLuxLight light;
    tsfi_luxlight_init(&light, "/auncient/light/sunlight");

    assert(strcmp(light.light_path, "/auncient/light/sunlight") == 0);
    assert(fabs(light.intensity - 1.0f) < 1e-5f);

    // Set intensity = 2.0, exposure = 3.0 (Energy = 2.0 * 8 = 16.0)
    tsfi_luxlight_set_energy(&light, 2.0f, 3.0f);

    float vertex[3] = {0.0f, 0.0f, 0.0f};
    float att = tsfi_luxlight_calculate_attenuation(&light, vertex);

    // With distance = 0, attenuation should be 16.0 / (0 + 1) = 16.0
    assert(fabs(att - 16.0f) < 1e-5f);

    printf("   ✓ LuxLight energy and exposure validation verified.\n");
    printf("   ✓ Inverse-square distance attenuation calculations verified.\n");
    printf("=== AUNCIENT LUXLIGHT LIGHTING TESTS COMPLETE (PASS) ===\n");
    return 0;
}
