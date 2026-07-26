#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include "../tsfi2-deepseek/inc/tsfi_stageinherits.h"

int main(void) {
    printf("=== RUNNING AUNCIENT STAGEINHERITS VALIDATION TESTS ===\n");

    TSFiStageInheritsTable table;
    tsfi_stageinherits_init(&table);

    // Define class '_class_Torus'
    assert(tsfi_stageinherits_define_class(&table, "/_class_Torus"));
    assert(table.class_count == 1);

    // Set class attribute: 'radius' = 15.0
    TSFiStageClass *c = &table.classes[0];
    assert(tsfi_primroot_set_attribute(&c->class_prim, "radius", 15.0f));

    // Create instance primitive: '/World/Torus1'
    TSFiPrimNode instance;
    tsfi_primroot_init(&instance, "/World/Torus1", "Mesh");

    // Resolve radius (should inherit 15.0 from class)
    float rad1 = tsfi_stageinherits_resolve_inherited_attribute(&table, "/_class_Torus", &instance, "radius", 1.0f);
    assert(fabs(rad1 - 15.0f) < 1e-5f);

    // Override local opinion: set radius locally on instance = 25.0
    assert(tsfi_primroot_set_attribute(&instance, "radius", 25.0f));

    // Resolve radius again (should override with local opinion 25.0)
    float rad2 = tsfi_stageinherits_resolve_inherited_attribute(&table, "/_class_Torus", &instance, "radius", 1.0f);
    assert(fabs(rad2 - 25.0f) < 1e-5f);

    printf("   ✓ StageInherits class definitions verified.\n");
    printf("   ✓ Local instance opinion overrides verified successfully.\n");
    printf("=== AUNCIENT STAGEINHERITS VALIDATION TESTS COMPLETE (PASS) ===\n");
    return 0;
}
