#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include "../tsfi2-deepseek/inc/tsfi_stagereferences.h"

int main(void) {
    printf("=== RUNNING AUNCIENT STAGEREFERENCES RESOLUTION TESTS ===\n");

    TSFiStageReferencesTable table;
    tsfi_stagereferences_init(&table);

    // Add references: '/World/Torus' references '/Asset/Torus.dat.bin'
    assert(tsfi_stagereferences_add(&table, "/World/Torus", "/Asset/Torus.dat.bin", "/TorusMesh"));
    assert(table.ref_count == 1);

    // Resolve asset path
    const char *resolved = tsfi_stagereferences_resolve_asset(&table, "/World/Torus");
    assert(resolved != NULL);
    assert(strcmp(resolved, "/Asset/Torus.dat.bin") == 0);

    // Add override reference on same path
    assert(tsfi_stagereferences_add(&table, "/World/Torus", "/Asset/Torus_V2.dat.bin", "/TorusMesh"));
    assert(table.ref_count == 2);

    // Verify reverse-scan returns the stronger V2 override
    const char *resolved_override = tsfi_stagereferences_resolve_asset(&table, "/World/Torus");
    assert(resolved_override != NULL);
    assert(strcmp(resolved_override, "/Asset/Torus_V2.dat.bin") == 0);

    printf("   ✓ StageReferences table entries additions verified.\n");
    printf("   ✓ Reverse-scan priority overrides verified successfully.\n");
    printf("=== AUNCIENT STAGEREFERENCES RESOLUTION TESTS COMPLETE (PASS) ===\n");
    return 0;
}
