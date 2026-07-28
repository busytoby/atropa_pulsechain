#include <stdio.h>
#include <assert.h>
#include <math.h>
#include "tsfi_dvfs_governor.h"

int main() {
    printf("=== RUNNING TSFI DVFS GOVERNOR UNIT TESTS ===\n");

    tsfi_dvfs_governor_t gov;
    int res;

    // Test Case 1: Initial state
    res = tsfi_dvfs_governor_init(&gov);
    assert(res == 0);
    assert(gov.state == DVFS_ECO_SAVER);
    assert(gov.current_freq_hz == 0.5e9f);
    assert(gov.current_voltage_volts == 1.8f);
    printf("[PASS] Test Case 1: Governor Initialization\n");

    // Test Case 2: High load adaptation (Performance Mode)
    res = tsfi_dvfs_governor_adapt(&gov, 85.0f);
    assert(res == 0);
    assert(gov.state == DVFS_PERFORMANCE);
    assert(gov.current_freq_hz == 3.0e9f);
    assert(gov.current_voltage_volts == 5.0f);
    // P_dyn = 3e9 * 1e-12 * (5.0 * 5.0) = 3e-3 * 25 = 0.075 Watts
    assert(fabs(gov.current_power_watts - 0.075) < 1e-5);
    printf("[PASS] Test Case 2: Performance Mode (Load: 85.0%%)\n");

    // Test Case 3: Medium load adaptation (Balanced Mode)
    res = tsfi_dvfs_governor_adapt(&gov, 50.0f);
    assert(res == 0);
    assert(gov.state == DVFS_BALANCED);
    assert(gov.current_freq_hz == 1.5e9f);
    assert(gov.current_voltage_volts == 3.3f);
    // P_dyn = 1.5e9 * 1e-12 * (3.3 * 3.3) = 1.5e-3 * 10.89 = 0.016335 Watts
    assert(fabs(gov.current_power_watts - 0.016335) < 1e-5);
    printf("[PASS] Test Case 3: Balanced Mode (Load: 50.0%%)\n");

    // Test Case 4: Low load adaptation (Eco Saver Mode)
    res = tsfi_dvfs_governor_adapt(&gov, 15.0f);
    assert(res == 0);
    assert(gov.state == DVFS_ECO_SAVER);
    assert(gov.current_freq_hz == 0.5e9f);
    assert(gov.current_voltage_volts == 1.8f);
    // P_dyn = 0.5e9 * 1e-12 * (1.8 * 1.8) = 0.5e-3 * 3.24 = 0.00162 Watts
    assert(fabs(gov.current_power_watts - 0.00162) < 1e-5);
    printf("[PASS] Test Case 4: Eco Saver Mode (Load: 15.0%%)\n");

    printf("=== ALL DVFS GOVERNOR UNIT TESTS PASSED ===\n");
    return 0;
}
