#ifndef TSFI_DEFCON_POWER_ALARM_H
#define TSFI_DEFCON_POWER_ALARM_H

#include <stdint.h>
#include <stddef.h>

typedef enum {
    DEFCON_LEVEL_5 = 5, // Normal Low-Power Floor (P <= 0.012 W)
    DEFCON_LEVEL_4 = 4, // Elevated Power (P > 0.015 W)
    DEFCON_LEVEL_3 = 3, // WARNING: High Power Spike (P > 0.020 W)
    DEFCON_LEVEL_2 = 2, // CRITICAL: Exceeded Threshold (P > 0.030 W)
    DEFCON_LEVEL_1 = 1  // EMERGENCY: Maximum Power Overload (P > 0.045 W)
} tsfi_defcon_level_t;

typedef struct {
    double current_power_watts;     // Current System Power Consumption
    tsfi_defcon_level_t defcon_level;// Active DEFCON Warning Level
    const char *warning_msg;        // Human-Readable DEFCON Warning String
    uint8_t emergency_throttled;    // 1 if emergency DVFS throttle triggered
} tsfi_defcon_alarm_status_t;

/* Evaluate current power consumption and trigger DEFCON warning if excessive */
int tsfi_defcon_power_alarm_eval(
    double current_power_watts,
    tsfi_defcon_alarm_status_t *out_status
);

#endif // TSFI_DEFCON_POWER_ALARM_H
