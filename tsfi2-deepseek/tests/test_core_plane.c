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

    // 3. Inhibit Driver Verification
    printf("[PLANE] Resetting plane and testing Inhibit Driver current cancellation...\n");
    tsfi_core_plane_init(&plane);

    // Write a '0' utilizing Inhibit pulse (this applies Ix = Iy = 0.75, Inhibit = -0.75)
    // The target core (3, 3) must remain 0
    tsfi_core_plane_write_inhibited(&plane, 3, 3, 0);
    int read_inhibit_0 = tsfi_core_plane_read_destructive(&plane, 3, 3);
    printf("  Read inhibited '0': %d (expected 0)\n", read_inhibit_0);
    assert(read_inhibit_0 == 0);

    // Write a '1' utilizing Inhibit pulse (this applies Ix = Iy = 0.75, Inhibit = 0)
    // The target core (3, 3) must switch to 1
    tsfi_core_plane_write_inhibited(&plane, 3, 3, 1);
    int read_inhibit_1 = tsfi_core_plane_read_destructive(&plane, 3, 3);
    printf("  Read inhibited '1': %d (expected 1)\n", read_inhibit_1);
    assert(read_inhibit_1 == 1);

    // Check for memory leaks
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    extern void lau_assert_zero_unsealed_leaks(const char *context, void *teardown_ptr);
    lau_assert_zero_unsealed_leaks("test_core_plane", NULL);

    printf("[PASS] 2D Coincident-Current Core Memory Plane verified.\n");
    return 0;
}
