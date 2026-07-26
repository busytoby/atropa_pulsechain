#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include "../tsfi2-deepseek/inc/tsfi_stagerelationship.h"

int main(void) {
    printf("=== RUNNING AUNCIENT STAGERELATIONSHIP CONNECTION TESTS ===\n");

    TSFiPrimRelationships pr;
    tsfi_stagerelationship_init(&pr, "/World/Torus");

    assert(strcmp(pr.prim_path, "/World/Torus") == 0);
    assert(pr.relationship_count == 0);

    // Add material:binding target
    assert(tsfi_stagerelationship_add_target(&pr, "material:binding", "/World/Material/Gold"));
    assert(pr.relationship_count == 1);
    assert(pr.relationships[0].target_count == 1);

    // Retrieve target path
    const char *target = tsfi_stagerelationship_get_first_target(&pr, "material:binding");
    assert(target != NULL);
    assert(strcmp(target, "/World/Material/Gold") == 0);

    // Verify non-existent query
    const char *missing = tsfi_stagerelationship_get_first_target(&pr, "light:binding");
    assert(missing == NULL);

    printf("   ✓ StageRelationship target connections verified.\n");
    printf("   ✓ Target path query lookups verified successfully.\n");
    printf("=== AUNCIENT STAGERELATIONSHIP CONNECTION TESTS COMPLETE (PASS) ===\n");
    return 0;
}
