#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include "../tsfi2-deepseek/inc/tsfi_stagespecializes.h"

int main(void) {
    printf("=== RUNNING AUNCIENT STAGESPECIALIZES RESOLUTION TESTS ===\n");

    TSFiStageSpecializesTable table;
    tsfi_stagespecializes_init(&table);

    // Add specializes link: '/World/SpecialTorus' specializes '/Class/BaseTorus'
    assert(tsfi_stagespecializes_add(&table, "/World/SpecialTorus", "/Class/BaseTorus"));
    assert(table.link_count == 1);

    // Resolve template path
    const char *resolved = tsfi_stagespecializes_resolve_spec(&table, "/World/SpecialTorus");
    assert(resolved != NULL);
    assert(strcmp(resolved, "/Class/BaseTorus") == 0);

    // Add overriding specializes link on same path
    assert(tsfi_stagespecializes_add(&table, "/World/SpecialTorus", "/Class/OverrideTorus"));
    assert(table.link_count == 2);

    // Verify reverse-scan returns the stronger specializes override
    const char *resolved_override = tsfi_stagespecializes_resolve_spec(&table, "/World/SpecialTorus");
    assert(resolved_override != NULL);
    assert(strcmp(resolved_override, "/Class/OverrideTorus") == 0);

    printf("   ✓ StageSpecializes link entries verified.\n");
    printf("   ✓ Reverse-scan override priority resolutions verified successfully.\n");
    printf("=== AUNCIENT STAGESPECIALIZES RESOLUTION TESTS COMPLETE (PASS) ===\n");
    return 0;
}
