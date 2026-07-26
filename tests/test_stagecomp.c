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

    printf("4. Testing SAGE PLL integration bridge...\n");
    tsfi_sage_pll_fabric pll = {0};
    tsfi_stagecomp_set_opinion(&comp, LIVRPS_LOCAL, "high_frequency");
    tsfi_stagecomp_tune_pll(&comp, &pll);
    assert(pll.reference_phase == 1000);

    tsfi_stagecomp_set_opinion(&comp, LIVRPS_LOCAL, "low_frequency");
    tsfi_stagecomp_tune_pll(&comp, &pll);
    assert(pll.reference_phase == 100);

    tsfi_stagecomp_set_opinion(&comp, LIVRPS_LOCAL, "unknown_opinion");
    tsfi_stagecomp_tune_pll(&comp, &pll);
    assert(pll.reference_phase == 500);
    printf("   ✓ SAGE PLL integration bridge verified successfully.\n");

    printf("   ✓ LIVRPS composition priority overrides verified successfully.\n");
    printf("=== AUNCIENT USD STAGE COMPOSITION (STAGECOMP) TESTS COMPLETE (PASS) ===\n");
    return 0;
}
