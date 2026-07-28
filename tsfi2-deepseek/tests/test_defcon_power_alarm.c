#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "tsfi_defcon_power_alarm.h"

int main() {
    printf("=== RUNNING TSFI DEFCON POWER ALARM UNIT TESTS ===\n");

    tsfi_defcon_alarm_status_t status;
    int res;

    // Test Case 1: Optimal Power (DEFCON 5)
    res = tsfi_defcon_power_alarm_eval(0.012, &status);
    assert(res == 0);
    assert(status.defcon_level == DEFCON_LEVEL_5);
    assert(status.emergency_throttled == 0);
    printf("[PASS] Test Case 1: 0.012W -> DEFCON 5 (Normal)\n");

    // Test Case 2: Elevated Power (DEFCON 4)
    res = tsfi_defcon_power_alarm_eval(0.017, &status);
    assert(res == 0);
    assert(status.defcon_level == DEFCON_LEVEL_4);
    assert(status.emergency_throttled == 0);
    printf("[PASS] Test Case 2: 0.017W -> DEFCON 4 (Notice)\n");

    // Test Case 3: High Power Spike (DEFCON 3)
    res = tsfi_defcon_power_alarm_eval(0.024, &status);
    assert(res == 0);
    assert(status.defcon_level == DEFCON_LEVEL_3);
    assert(status.emergency_throttled == 1);
    printf("[PASS] Test Case 3: 0.024W -> DEFCON 3 (Warning)\n");

    // Test Case 4: Critical Power (DEFCON 2)
    res = tsfi_defcon_power_alarm_eval(0.035, &status);
    assert(res == 0);
    assert(status.defcon_level == DEFCON_LEVEL_2);
    assert(status.emergency_throttled == 1);
    printf("[PASS] Test Case 4: 0.035W -> DEFCON 2 (Critical Alert)\n");

    // Test Case 5: Overload Power (DEFCON 1)
    res = tsfi_defcon_power_alarm_eval(0.052, &status);
    assert(res == 0);
    assert(status.defcon_level == DEFCON_LEVEL_1);
    assert(status.emergency_throttled == 1);
    printf("[PASS] Test Case 5: 0.052W -> DEFCON 1 (Overload Emergency)\n");

    printf("=== ALL DEFCON POWER ALARM UNIT TESTS PASSED ===\n");
    return 0;
}
