#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "tsfi_defcon_power_alarm.h"

int main() {
    printf("=== RUNNING TSFI DEFCON LATCHING & RESET UNIT TESTS ===\n");

    tsfi_defcon_alarm_status_t status;
    int res;

    // Reset everything initially
    tsfi_defcon_power_alarm_approve_reset();

    // Test 1: Normal state (Optimal Power)
    res = tsfi_defcon_power_alarm_eval(0.012, &status);
    assert(res == 0);
    assert(status.defcon_level == DEFCON_LEVEL_5);
    assert(g_defcon_latched == 0);
    printf("[PASS] Normal Power -> DEFCON 5 (Unlatched)\n");

    // Test 2: Trigger overload spike (DEFCON 1)
    res = tsfi_defcon_power_alarm_eval(0.052, &status);
    assert(res == 0);
    assert(status.defcon_level == DEFCON_LEVEL_1);
    assert(g_defcon_latched == 1);
    assert(g_latched_level == DEFCON_LEVEL_1);
    printf("[PASS] Overload Spike -> DEFCON 1 Triggered & Latched\n");

    // Test 3: Power returns to optimal, but system must remain latched to DEFCON 1
    res = tsfi_defcon_power_alarm_eval(0.012, &status);
    assert(res == 0);
    assert(status.defcon_level == DEFCON_LEVEL_1); // Still latched to 1!
    assert(g_defcon_latched == 1);
    printf("[PASS] Power Decreased -> Enforced Latch at DEFCON 1 (Safety Lock)\n");

    // Test 4: Operator manual reset clears latch
    tsfi_defcon_power_alarm_approve_reset();
    assert(g_defcon_latched == 0);
    assert(g_latched_level == DEFCON_LEVEL_5);
    printf("[PASS] Operator Manual Reset clears Latch\n");

    // Test 5: Re-evaluate post-reset normal power
    res = tsfi_defcon_power_alarm_eval(0.012, &status);
    assert(res == 0);
    assert(status.defcon_level == DEFCON_LEVEL_5); // Back to normal!
    assert(g_defcon_latched == 0);
    printf("[PASS] Post-Reset Normal Power -> Back to DEFCON 5\n");

    printf("=== ALL DEFCON LATCHING & RESET UNIT TESTS PASSED ===\n");
    return 0;
}
