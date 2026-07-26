#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include "../tsfi2-deepseek/inc/tsfi_quadtreeslicer.h"

int main(void) {
    printf("=== RUNNING AUNCIENT QUADTREESLICER TESTS ===\n");

    TSFiQuadtreeSlicer qs;
    tsfi_quadtreeslicer_init(&qs);

    assert(qs.count == 0);

    TSFiQuadRect parent = {0.0, 0.0, 100.0, 100.0};
    tsfi_quadtreeslicer_slice(&qs, &parent, 1000);

    assert(qs.count == 4);

    // Verify compliance: target path matches expected compliance string ending with .dat.bin
    const char *path0 = tsfi_quadtreeslicer_query(&qs, 25.0, 25.0);
    assert(path0 != NULL);
    assert(strcmp(path0, "assets/slice_1000.dat.bin") == 0);

    const char *path3 = tsfi_quadtreeslicer_query(&qs, 75.0, 75.0);
    assert(path3 != NULL);
    assert(strcmp(path3, "assets/slice_1003.dat.bin") == 0);

    // Verify out-of-bounds query return value
    assert(tsfi_quadtreeslicer_query(&qs, 200.0, 200.0) == NULL);

    printf("   ✓ Quad boundary slicing splits verified.\n");
    printf("   ✓ Compliant .dat.bin asset path queries verified successfully.\n");
    printf("=== AUNCIENT QUADTREESLICER TESTS COMPLETE (PASS) ===\n");
    return 0;
}
