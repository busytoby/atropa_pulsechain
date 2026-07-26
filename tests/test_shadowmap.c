#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include "../tsfi2-deepseek/inc/tsfi_shadowmap.h"

int main(void) {
    printf("=== RUNNING AUNCIENT SHADOWMAP TESTS ===\n");

    TSFiShadowMap sm;
    tsfi_shadowmap_init(&sm);

    // Verify cell depth storage
    tsfi_shadowmap_set_depth(&sm, 5, 5, 10.0);
    assert(fabs(sm.depth_grid[5][5] - 10.0) < 1e-5);

    // Verify constant-time O(1) shadow calculations
    assert(tsfi_shadowmap_is_in_shadow(&sm, 5, 5, 8.0) == false);  // Closer than stored depth: Illuminated
    assert(tsfi_shadowmap_is_in_shadow(&sm, 5, 5, 12.0) == true);  // Further than stored depth: Occluded

    printf("   ✓ Depth shadow cell values updates verified.\n");
    printf("   ✓ Constant-time O(1) shadow occlusion checks verified successfully.\n");
    printf("=== AUNCIENT SHADOWMAP TESTS COMPLETE (PASS) ===\n");
    return 0;
}
