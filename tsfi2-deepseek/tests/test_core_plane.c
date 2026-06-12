#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "tsfi_core_plane.h"

int main() {
    printf("=== TSFi2 & Computer Design Issue #5 2D Core Plane Test ===\n");

    TsfiCorePlane plane;
    tsfi_core_plane_init(&plane);

    // Write bits to coordinates
    printf("[PLANE] Writing '1' to core (2, 4) and (5, 7)...\n");
    tsfi_core_plane_write(&plane, 2, 4, 1);
    tsfi_core_plane_write(&plane, 5, 7, 1);

    // Destructive read of written coordinates
    printf("[PLANE] Reading core (2, 4) destructively...\n");
    int read_val1 = tsfi_core_plane_read_destructive(&plane, 2, 4);
    printf("  Read 1 value: %d (expected 1)\n", read_val1);
    assert(read_val1 == 1);

    // Read again (should be 0 now due to destructive read)
    printf("[PLANE] Reading core (2, 4) again destructively...\n");
    int read_val2 = tsfi_core_plane_read_destructive(&plane, 2, 4);
    printf("  Read 2 value: %d (expected 0)\n", read_val2);
    assert(read_val2 == 0);

    // Read unwritten coordinate
    printf("[PLANE] Reading unwritten core (1, 1) destructively...\n");
    int read_val3 = tsfi_core_plane_read_destructive(&plane, 1, 1);
    printf("  Read 3 value: %d (expected 0)\n", read_val3);
    assert(read_val3 == 0);

    // Check for memory leaks
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    extern void lau_assert_zero_unsealed_leaks(const char *context, void *teardown_ptr);
    lau_assert_zero_unsealed_leaks("test_core_plane", NULL);

    printf("[PASS] 2D Coincident-Current Core Memory Plane verified.\n");
    return 0;
}
