#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "tsfi_lowpower_mode.h"

int tsfi_lowpower_mode_init(tsfi_lowpower_controller_t *ctrl) {
    if (!ctrl) return -1;
    memset(ctrl, 0, sizeof(tsfi_lowpower_controller_t));
    return tsfi_lowpower_mode_set(ctrl, TSFI_POWER_MODE_SCALED_VOLTAGE);
}

int tsfi_lowpower_mode_set(tsfi_lowpower_controller_t *ctrl, tsfi_power_mode_t mode) {
    if (!ctrl) return -1;
    ctrl->active_mode = mode;

    switch (mode) {
        case TSFI_POWER_MODE_STANDARD:
            ctrl->energy_cost_multiplier = 1.00;
            ctrl->mode_name = "STANDARD_EVM_FULL_POWER";
            break;
        case TSFI_POWER_MODE_BASE2I:
            ctrl->energy_cost_multiplier = 0.50; // 50% Cost
            ctrl->mode_name = "KNUTH_BASE2I_PACKED";
            break;
        case TSFI_POWER_MODE_SCALED_VOLTAGE:
            ctrl->energy_cost_multiplier = 0.218; // 21.8% Cost (78.2% Reduction)
            ctrl->mode_name = "LOWPOWER_SCALED_VOLTAGE_ULTRALOW_COST";
            break;
        default:
            return -1;
    }

    printf("[LOW-POWER MODE] System Mode Set to: %s | Transaction Cost Multiplier: %.3f (%.1f%% Savings!)\n",
           ctrl->mode_name, ctrl->energy_cost_multiplier, (1.0 - ctrl->energy_cost_multiplier) * 100.0);

    return 0; // Mode set successfully
}
