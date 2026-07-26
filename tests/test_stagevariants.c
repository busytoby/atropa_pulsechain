#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include "../tsfi2-deepseek/inc/tsfi_stagevariants.h"

int main(void) {
    printf("=== RUNNING AUNCIENT STAGEVARIANTS RESOLUTION TESTS ===\n");

    TSFiStageVariants sv;
    tsfi_stagevariants_init(&sv, "/World/Mesh/Torus");

    assert(strcmp(sv.prim_path, "/World/Mesh/Torus") == 0);
    assert(sv.set_count == 0);

    // Define variant set: 'lod' (level of detail)
    assert(tsfi_stagevariants_define_set(&sv, "lod"));
    assert(sv.set_count == 1);

    // Add options: 'high' (scale = 1.0), 'low' (scale = 0.25)
    assert(tsfi_stagevariants_add_option(&sv, "lod", "high", 1.0f));
    assert(tsfi_stagevariants_add_option(&sv, "lod", "low", 0.25f));

    // Verify default selection is the first added option ('high')
    assert(strcmp(sv.sets[0].selected_option, "high") == 0);
    assert(fabs(tsfi_stagevariants_get_selected_scale(&sv, "lod", 0.0f) - 1.0f) < 1e-5f);

    // Change variant selection to 'low'
    assert(tsfi_stagevariants_select_option(&sv, "lod", "low"));
    assert(strcmp(sv.sets[0].selected_option, "low") == 0);
    assert(fabs(tsfi_stagevariants_get_selected_scale(&sv, "lod", 0.0f) - 0.25f) < 1e-5f);

    printf("   ✓ StageVariants set definitions verified.\n");
    printf("   ✓ Active variant selections and scale factors lookups verified.\n");
    printf("=== AUNCIENT STAGEVARIANTS RESOLUTION TESTS COMPLETE (PASS) ===\n");
    return 0;
}
