#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include "../tsfi2-deepseek/inc/tsfi_mvarsel.h"

int main(void) {
    printf("=== RUNNING AUNCIENT MATERIAL VARIANT SELECTOR (MVARSEL) TESTS ===\n");

    TSFiMVarSel sel;
    tsfi_mvarsel_init(&sel);

    // Initial variant matches GOLD
    assert(sel.variant_index == MVARSEL_GOLD);
    assert(strcmp(tsfi_mvarsel_get_descriptor(&sel), "GOLD") == 0);
    assert(fabs(tsfi_mvarsel_resolve_phase(&sel) - 0.0f) < 1e-5f);

    // Switch to CLAY
    tsfi_mvarsel_set_index(&sel, MVARSEL_CLAY);
    assert(sel.variant_index == MVARSEL_CLAY);
    assert(strcmp(tsfi_mvarsel_get_descriptor(&sel), "CLAY") == 0);
    float expected_clay = 3.14159265f / 3.0f;
    assert(fabs(tsfi_mvarsel_resolve_phase(&sel) - expected_clay) < 1e-5f);

    // Switch to CLOTH
    tsfi_mvarsel_set_index(&sel, MVARSEL_CLOTH);
    assert(sel.variant_index == MVARSEL_CLOTH);
    assert(strcmp(tsfi_mvarsel_get_descriptor(&sel), "CLOTH") == 0);
    float expected_cloth = (2.0f * 3.14159265f) / 3.0f;
    assert(fabs(tsfi_mvarsel_resolve_phase(&sel) - expected_cloth) < 1e-5f);

    printf("   ✓ Variant descriptor and phase mappings verified successfully.\n");

    printf("2. Testing Hogan market compliance integration...\n");
    // High market price yields GOLD variant
    tsfi_mvarsel_update_from_hogan_market(&sel, 1500.0, 0.05);
    assert(sel.variant_index == MVARSEL_GOLD);

    // High tax rate yields CLAY variant
    tsfi_mvarsel_update_from_hogan_market(&sel, 500.0, 0.25);
    assert(sel.variant_index == MVARSEL_CLAY);

    // Low price and normal tax yields CLOTH variant
    tsfi_mvarsel_update_from_hogan_market(&sel, 500.0, 0.10);
    assert(sel.variant_index == MVARSEL_CLOTH);
    printf("   ✓ Hogan market compliance integration verified successfully.\n");

    printf("=== AUNCIENT MATERIAL VARIANT SELECTOR (MVARSEL) TESTS COMPLETE (PASS) ===\n");
    return 0;
}
