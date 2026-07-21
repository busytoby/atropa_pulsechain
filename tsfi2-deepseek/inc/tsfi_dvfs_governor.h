#ifndef TSFI_DVFS_GOVERNOR_H
#define TSFI_DVFS_GOVERNOR_H

#include <stdint.h>
#include <stddef.h>

typedef enum {
    DVFS_PERFORMANCE = 0, // 3.0 GHz @ 5.0V (High Load)
    DVFS_BALANCED    = 1, // 1.5 GHz @ 3.3V (Medium Load)
    DVFS_ECO_SAVER   = 2  // 0.5 GHz @ 1.8V (Idle / Runcible TTY Waiting)
} tsfi_dvfs_state_t;

typedef struct {
    tsfi_dvfs_state_t state;
    float current_freq_hz;
    float current_voltage_volts;
    double current_power_watts;
} tsfi_dvfs_governor_t;

/* Initialize Dynamic Voltage & Frequency Scaling Governor */
int tsfi_dvfs_governor_init(tsfi_dvfs_governor_t *gov);

/* Adapt frequency and voltage based on active workload load percentage */
int tsfi_dvfs_governor_adapt(tsfi_dvfs_governor_t *gov, float load_percentage);

#endif // TSFI_DVFS_GOVERNOR_H
