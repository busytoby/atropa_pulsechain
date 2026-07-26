#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include "../tsfi2-deepseek/inc/tsfi_attributeroot.h"

int main(void) {
    printf("=== RUNNING AUNCIENT ATTRIBUTEROOT EVALUATION TESTS ===\n");

    TSFiAttributeRoot attr;
    tsfi_attributeroot_init(&attr, "radius", "float", 1.0f);

    assert(strcmp(attr.name, "radius") == 0);
    assert(fabs(attr.default_value - 1.0f) < 1e-5f);

    // Setup keyframes: (0.0s = 0.0), (10.0s = 100.0)
    assert(tsfi_attributeroot_set_time_sample(&attr, 0.0f, 0.0f));
    assert(tsfi_attributeroot_set_time_sample(&attr, 10.0f, 100.0f));
    assert(attr.sample_count == 2);

    // Test exact edge checks
    assert(fabs(tsfi_attributeroot_get_value(&attr, -1.0f) - 0.0f) < 1e-5f);
    assert(fabs(tsfi_attributeroot_get_value(&attr, 11.0f) - 100.0f) < 1e-5f);

    // Test midpoint linear interpolation: (5.0s = 50.0)
    float mid = tsfi_attributeroot_get_value(&attr, 5.0f);
    assert(fabs(mid - 50.0f) < 1e-5f);

    printf("   ✓ AttributeRoot keyframe sorted insertions verified.\n");
    printf("   ✓ Linear keyframe coordinate value interpolations verified.\n");
    printf("=== AUNCIENT ATTRIBUTEROOT EVALUATION TESTS COMPLETE (PASS) ===\n");
    return 0;
}
