#ifndef TSFI_LOWPOWER_MODE_H
#define TSFI_LOWPOWER_MODE_H

#include <stdint.h>
#include <stddef.h>

typedef enum {
    TSFI_POWER_MODE_STANDARD       = 0, // Standard EVM (40k Gas / 100% Power)
    TSFI_POWER_MODE_BASE2I         = 1, // Knuth Base 2i (20k Gas / 50% Power)
    TSFI_POWER_MODE_SCALED_VOLTAGE = 2  // Low-Power Scaled Vdd (10k Gas equiv / 21.8% Power)
} tsfi_power_mode_t;

typedef struct {
    tsfi_power_mode_t active_mode;
    double energy_cost_multiplier;
    const char *mode_name;
} tsfi_lowpower_controller_t;

/* Initialize System Low-Power Mode Controller */
int tsfi_lowpower_mode_init(tsfi_lowpower_controller_t *ctrl);

/* Set System Power Mode and compute transaction cost multiplier */
int tsfi_lowpower_mode_set(tsfi_lowpower_controller_t *ctrl, tsfi_power_mode_t mode);

#endif // TSFI_LOWPOWER_MODE_H
