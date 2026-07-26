#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include "../tsfi2-deepseek/inc/tsfi_bboxcache.h"

int main(void) {
    printf("=== RUNNING AUNCIENT BBOXCACHE BOUNDARY TESTS ===\n");

    TSFiBBoxCache bbc;
    tsfi_bboxcache_init(&bbc);

    assert(bbc.is_valid == false);

    double min_extents[3] = {-100.0, -100.0, -50.0};
    double max_extents[3] = {100.0, 100.0, 50.0};
    tsfi_bboxcache_set(&bbc, min_extents, max_extents);

    assert(bbc.is_valid == true);
    assert(fabs(bbc.min_extents[0] - (-100.0)) < 1e-5);
    assert(fabs(bbc.max_extents[2] - 50.0) < 1e-5);

    double check_inside[3] = {0.0, 50.0, 10.0};
    double check_outside[3] = {150.0, 0.0, 0.0};

    // Verify boundary culling checks
    assert(tsfi_bboxcache_contains(&bbc, check_inside) == true);
    assert(tsfi_bboxcache_contains(&bbc, check_outside) == false);

    // Invalidate and verify status
    tsfi_bboxcache_invalidate(&bbc);
    assert(bbc.is_valid == false);
    assert(tsfi_bboxcache_contains(&bbc, check_inside) == false);

    printf("   ✓ Bounding box coordinates boundary containment verified.\n");
    printf("   ✓ Cache invalidation safety flags verified successfully.\n");
    printf("=== AUNCIENT BBOXCACHE BOUNDARY TESTS COMPLETE (PASS) ===\n");
    return 0;
}
