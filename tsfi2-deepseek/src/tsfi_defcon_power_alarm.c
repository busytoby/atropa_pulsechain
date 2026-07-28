#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "tsfi_defcon_power_alarm.h"
#include "tsfi_dvfs_governor.h"

uint8_t g_defcon_latched = 0;
tsfi_defcon_level_t g_latched_level = DEFCON_LEVEL_5;

void tsfi_defcon_power_alarm_approve_reset(void) {
    g_defcon_latched = 0;
    g_latched_level = DEFCON_LEVEL_5;
    printf("[DEFCON CONTROL] Manual Reset Approved by Operator. Latch Cleared.\n");
}

int tsfi_defcon_power_alarm_eval(
    double current_power_watts,
    tsfi_defcon_alarm_status_t *out_status
) {
    if (!out_status) return -1;
    memset(out_status, 0, sizeof(tsfi_defcon_alarm_status_t));
    out_status->current_power_watts = current_power_watts;

    tsfi_defcon_level_t calculated_level = DEFCON_LEVEL_5;
    const char *calculated_msg = NULL;
    uint8_t calculated_throttle = 0;

    if (current_power_watts > 0.045) {
        calculated_level = DEFCON_LEVEL_1;
        calculated_msg = "[DEFCON 1 WARNING] CRITICAL OVERLOAD! Power > 0.045W! Emergency Under-Volt Active!";
        calculated_throttle = 1;
    } else if (current_power_watts > 0.030) {
        calculated_level = DEFCON_LEVEL_2;
        calculated_msg = "[DEFCON 2 WARNING] CRITICAL POWER! Power > 0.030W! Throttling Frequency!";
        calculated_throttle = 1;
    } else if (current_power_watts > 0.020) {
        calculated_level = DEFCON_LEVEL_3;
        calculated_msg = "[DEFCON 3 WARNING] HIGH POWER SPIKE! Power > 0.020W! Clamping DVFS Governor!";
        calculated_throttle = 1;
    } else if (current_power_watts > 0.015) {
        calculated_level = DEFCON_LEVEL_4;
        calculated_msg = "[DEFCON 4 NOTICE] Elevated Power Usage (P > 0.015W). Monitoring Voltage.";
        calculated_throttle = 0;
    } else {
        calculated_level = DEFCON_LEVEL_5;
        calculated_msg = "[DEFCON 5 NORMAL] Power Optimal (P <= 0.015W). System at 78.2% Low-Power Floor.";
        calculated_throttle = 0;
    }

    // Handle latching mechanics
    if (calculated_level <= DEFCON_LEVEL_3) { // Lower enum values are higher threat (1, 2, 3)
        g_defcon_latched = 1;
        if (calculated_level < g_latched_level) {
            g_latched_level = calculated_level; // Latch to the highest threat level reached
        }
    }

    if (g_defcon_latched) {
        out_status->defcon_level = g_latched_level;
        out_status->emergency_throttled = 1;
        if (g_latched_level == DEFCON_LEVEL_1) {
            out_status->warning_msg = "[LATCHED DEFCON 1] CRITICAL OVERLOAD ENFORCED! Awaiting Operator Approval to Reset.";
        } else if (g_latched_level == DEFCON_LEVEL_2) {
            out_status->warning_msg = "[LATCHED DEFCON 2] CRITICAL POWER ENFORCED! Awaiting Operator Approval to Reset.";
        } else {
            out_status->warning_msg = "[LATCHED DEFCON 3] HIGH POWER SPIKE ENFORCED! Awaiting Operator Approval to Reset.";
        }
    } else {
        out_status->defcon_level = calculated_level;
        out_status->warning_msg = calculated_msg;
        out_status->emergency_throttled = calculated_throttle;
    }

    if (out_status->emergency_throttled) {
        tsfi_dvfs_governor_t gov;
        tsfi_dvfs_governor_init(&gov);
        tsfi_dvfs_governor_adapt(&gov, 10.0f); // Emergency clamp to 0.5GHz / 1.8V
    }

    printf("%s (Power: %.4fW)\n", out_status->warning_msg, current_power_watts);
    return 0;
}
