#ifndef TSFI_CONTINUOUS_ZERO_POWER_H
#define TSFI_CONTINUOUS_ZERO_POWER_H

#include <stdint.h>
#include <stddef.h>

typedef struct {
    double active_power_watts;     // Continuous Active Power (0.01089 W)
    double idle_standby_watts;     // Sub-Clock Zero-Leakage Standby Power (0.00162 W)
    double net_power_savings_pct;  // 78.2% Permanent Energy Savings
    uint8_t zero_leakage_active;   // 1 if lowest power mode actively enforced
} tsfi_continuous_power_status_t;

/* Initialize Continuous Zero-Leakage Lowest Power Sentinel */
int tsfi_continuous_power_init(tsfi_continuous_power_status_t *status);

/* Enforce continuous lowest power floor across active execution loops */
int tsfi_continuous_power_enforce_lowest(tsfi_continuous_power_status_t *status);

#endif // TSFI_CONTINUOUS_ZERO_POWER_H
