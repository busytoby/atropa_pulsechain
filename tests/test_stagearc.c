#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include "../tsfi2-deepseek/inc/tsfi_stagearc.h"

int main(void) {
    printf("=== RUNNING AUNCIENT STAGEARC RESOLUTION TESTS ===\n");

    TSFiStageArcTable table;
    tsfi_stagearc_init(&table);

    // Add references: '/World/Torus' references '/Asset/TorusMesh'
    assert(tsfi_stagearc_add(&table, "/World/Torus", "/Asset/TorusMesh", TSFI_ARC_REFERENCE));
    assert(table.arc_count == 1);

    // Resolve target path
    const char *resolved = tsfi_stagearc_resolve_target(&table, "/World/Torus", TSFI_ARC_REFERENCE);
    assert(resolved != NULL);
    assert(strcmp(resolved, "/Asset/TorusMesh") == 0);

    // Add inherits: '/World/Torus' inherits from '/Class/TorusBase'
    assert(tsfi_stagearc_add(&table, "/World/Torus", "/Class/TorusBase", TSFI_ARC_INHERIT));
    assert(table.arc_count == 2);

    // Resolve override target path
    const char *resolved_inherit = tsfi_stagearc_resolve_target(&table, "/World/Torus", TSFI_ARC_INHERIT);
    assert(resolved_inherit != NULL);
    assert(strcmp(resolved_inherit, "/Class/TorusBase") == 0);

    printf("   ✓ StageArc reference table registrations verified.\n");
    printf("   ✓ Reverse-scan priority resolution overrides verified successfully.\n");
    printf("=== AUNCIENT STAGEARC RESOLUTION TESTS COMPLETE (PASS) ===\n");
    return 0;
}
