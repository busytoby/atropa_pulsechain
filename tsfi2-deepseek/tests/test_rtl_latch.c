#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "tsfi_rtl_latch.h"

int main() {
    printf("=== TSFi2 & Computer Design Issue #3 RTL Latch Test ===\n");

    TsfiRtlLatch latch;
    tsfi_rtl_latch_init(&latch);

    // Initial check
    printf("[RTL] Initial states: Q = %fV, Q_bar = %fV\n", latch.q, latch.q_bar);
    assert(latch.q == 0.2f);
    assert(latch.q_bar == 3.0f);

    // Set pulse
    printf("[RTL] Applying SET pulse (Set = 3.0V, Reset = 0.0V)...\n");
    tsfi_rtl_latch_update(&latch, 3.0f, 0.0f);
    assert(latch.q == 3.0f);
    assert(latch.q_bar == 0.2f);

    // Remove pulse (state held)
    printf("[RTL] Removing pulses (Set = 0.0V, Reset = 0.0V)...\n");
    tsfi_rtl_latch_update(&latch, 0.0f, 0.0f);
    assert(latch.q == 3.0f);
    assert(latch.q_bar == 0.2f);

    // Reset pulse
    printf("[RTL] Applying RESET pulse (Set = 0.0V, Reset = 3.0V)...\n");
    tsfi_rtl_latch_update(&latch, 0.0f, 3.0f);
    assert(latch.q == 0.2f);
    assert(latch.q_bar == 3.0f);

    // Both active (invalid logic state)
    printf("[RTL] Applying both active pulses (Set = 3.0V, Reset = 3.0V)...\n");
    tsfi_rtl_latch_update(&latch, 3.0f, 3.0f);
    assert(latch.q == 0.2f);
    assert(latch.q_bar == 0.2f);

    // Check for memory leaks
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    extern void lau_assert_zero_unsealed_leaks(const char *context, void *teardown_ptr);
    lau_assert_zero_unsealed_leaks("test_rtl_latch", NULL);

    printf("[PASS] RTL Bistable Latch emulation completed successfully.\n");
    return 0;
}
