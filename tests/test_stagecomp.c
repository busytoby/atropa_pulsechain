#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "../tsfi2-deepseek/inc/tsfi_stagecomp.h"

int main(void) {
    printf("=== RUNNING AUNCIENT USD STAGE COMPOSITION (STAGECOMP) TESTS ===\n");

    TSFiStageComp comp;
    tsfi_stagecomp_init(&comp);

    char resolved[64];

    // 1. Weakest opinion only
    tsfi_stagecomp_set_opinion(&comp, LIVRPS_SPECIALIZES, "weakest_specializes");
    bool ok = tsfi_stagecomp_resolve(&comp, resolved);
    assert(ok == true);
    assert(strcmp(resolved, "weakest_specializes") == 0);

    // 2. Middle opinion override
    tsfi_stagecomp_set_opinion(&comp, LIVRPS_REFERENCES, "middle_references");
    ok = tsfi_stagecomp_resolve(&comp, resolved);
    assert(ok == true);
    assert(strcmp(resolved, "middle_references") == 0);

    // 3. Local override (strongest)
    tsfi_stagecomp_set_opinion(&comp, LIVRPS_LOCAL, "strongest_local");
    ok = tsfi_stagecomp_resolve(&comp, resolved);
    assert(ok == true);
    assert(strcmp(resolved, "strongest_local") == 0);

    printf("   ✓ LIVRPS composition priority overrides verified successfully.\n");
    printf("=== AUNCIENT USD STAGE COMPOSITION (STAGECOMP) TESTS COMPLETE (PASS) ===\n");
    return 0;
}
