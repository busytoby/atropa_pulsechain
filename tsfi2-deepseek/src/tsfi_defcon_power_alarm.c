#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "tsfi_defcon_power_alarm.h"
#include "tsfi_dvfs_governor.h"

int tsfi_defcon_power_alarm_eval(
    double current_power_watts,
    tsfi_defcon_alarm_status_t *out_status
) {
    if (!out_status) return -1;
    memset(out_status, 0, sizeof(tsfi_defcon_alarm_status_t));
    out_status->current_power_watts = current_power_watts;

    if (current_power_watts > 0.045) {
        out_status->defcon_level = DEFCON_LEVEL_1;
        out_status->warning_msg = "[DEFCON 1 WARNING] CRITICAL OVERLOAD! Power > 0.045W! Emergency Under-Volt Active!";
        out_status->emergency_throttled = 1;
    } else if (current_power_watts > 0.030) {
        out_status->defcon_level = DEFCON_LEVEL_2;
        out_status->warning_msg = "[DEFCON 2 WARNING] CRITICAL POWER! Power > 0.030W! Throttling Frequency!";
        out_status->emergency_throttled = 1;
    } else if (current_power_watts > 0.020) {
        out_status->defcon_level = DEFCON_LEVEL_3;
        out_status->warning_msg = "[DEFCON 3 WARNING] HIGH POWER SPIKE! Power > 0.020W! Clamping DVFS Governor!";
        out_status->emergency_throttled = 1;
    } else if (current_power_watts > 0.015) {
        out_status->defcon_level = DEFCON_LEVEL_4;
        out_status->warning_msg = "[DEFCON 4 NOTICE] Elevated Power Usage (P > 0.015W). Monitoring Voltage.";
        out_status->emergency_throttled = 0;
    } else {
        out_status->defcon_level = DEFCON_LEVEL_5;
        out_status->warning_msg = "[DEFCON 5 NORMAL] Power Optimal (P <= 0.015W). System at 78.2% Low-Power Floor.";
        out_status->emergency_throttled = 0;
    }

    if (out_status->emergency_throttled) {
        tsfi_dvfs_governor_t gov;
        tsfi_dvfs_governor_init(&gov);
        tsfi_dvfs_governor_adapt(&gov, 10.0f); // Emergency clamp to 0.5GHz / 1.8V
    }

    printf("%s (Power: %.4fW)\n", out_status->warning_msg, current_power_watts);
    return 0;
}
