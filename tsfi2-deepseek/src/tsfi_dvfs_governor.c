#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "tsfi_dvfs_governor.h"
#include "tsfi_lowpower_fet.h"

int tsfi_dvfs_governor_init(tsfi_dvfs_governor_t *gov) {
    if (!gov) return -1;
    memset(gov, 0, sizeof(tsfi_dvfs_governor_t));
    return tsfi_dvfs_governor_adapt(gov, 10.0f); // Default to idle eco-saver mode
}

int tsfi_dvfs_governor_adapt(tsfi_dvfs_governor_t *gov, float load_percentage) {
    if (!gov) return -1;

    if (load_percentage > 75.0f) {
        gov->state = DVFS_PERFORMANCE;
        gov->current_freq_hz = 3.0e9f;     // 3.0 GHz
        gov->current_voltage_volts = 5.0f; // 5.0V
    } else if (load_percentage > 25.0f) {
        gov->state = DVFS_BALANCED;
        gov->current_freq_hz = 1.5e9f;     // 1.5 GHz
        gov->current_voltage_volts = 3.3f; // 3.3V
    } else {
        gov->state = DVFS_ECO_SAVER;
        gov->current_freq_hz = 0.5e9f;     // 0.5 GHz
        gov->current_voltage_volts = 1.8f; // 1.8V
    }

    // P_dyn = f * C * Vdd^2 (Assuming gate capacitance C = 1e-12 F)
    gov->current_power_watts = (double)gov->current_freq_hz * 1e-12 * (double)(gov->current_voltage_volts * gov->current_voltage_volts);

    printf("[DVFS GOVERNOR] Load: %.1f%% -> State: %d | Freq: %.1f GHz | Voltage: %.1f V | Power: %.6f W\n",
           load_percentage, gov->state, gov->current_freq_hz / 1e9, gov->current_voltage_volts, gov->current_power_watts);

    return 0; // DVFS adaptation complete
}
