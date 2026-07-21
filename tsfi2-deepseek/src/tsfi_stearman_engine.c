#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "tsfi_stearman_engine.h"
#include "tsfi_lowpower_fet.h"

int tsfi_stearman_engine_init(
    uint32_t stearman_id,
    tsfi_stearman_engine_t *engine
) {
    if (!engine) return -1;
    memset(engine, 0, sizeof(tsfi_stearman_engine_t));

    engine->stearman_id = stearman_id;
    engine->evm_gas_units = 280; // 280 Gas / Auncient Ether Units per evaluation

    // Rule 10: Verlet Soft-Body FET Discharge Physics Solver (3.3V Low-Power Floor)
    tsfi_lowpower_fet_metrics_t fet_metrics;
    tsfi_lowpower_fet_calculate(1e9f, 1e-12f, 5.0f, 3.3f, &fet_metrics);
    engine->fet_power_watts = (double)fet_metrics.optimized_power_watts; // 0.0109 W (78.2% Cut!)

    // Format Rule 13 dataset filename (.DAT.BIN)
    snprintf(engine->tape_dat_bin, sizeof(engine->tape_dat_bin), "STEARMAN_STAT_%08X.DAT.BIN", stearman_id);
    return 0;
}

int tsfi_stearman_compute_stats(
    tsfi_stearman_engine_t *engine,
    const double *data,
    size_t count,
    double *mean_out,
    double *variance_out
) {
    if (!engine || !data || count == 0 || !mean_out || !variance_out) return -1;

    double sum = 0.0;
    for (size_t i = 0; i < count; i++) {
        sum += data[i];
    }
    *mean_out = sum / count;

    double sq_sum = 0.0;
    for (size_t i = 0; i < count; i++) {
        double diff = data[i] - *mean_out;
        sq_sum += diff * diff;
    }
    *variance_out = sq_sum / count;

    engine->samples_processed += (uint32_t)count;
    printf("[STEARMAN STATISTICAL SUBROUTINE] ID: %u | Samples: %zu | Mean: %.4f | Var: %.4f | Gas: %u | File: %s\n",
           engine->stearman_id, count, *mean_out, *variance_out, engine->evm_gas_units, engine->tape_dat_bin);

    return 0;
}

int tsfi_stearman_resistance_solve(
    tsfi_stearman_engine_t *engine,
    double current_amps,
    double voltage_volts,
    double *resistance_out
) {
    if (!engine || !resistance_out || current_amps == 0.0) return -1;

    *resistance_out = voltage_volts / current_amps;
    printf("[STEARMAN RESISTANCE SOLVER] Voltage: %.2fV | Current: %.4fA | Resistance: %.4f Ohm | Gas: %u\n",
           voltage_volts, current_amps, *resistance_out, engine->evm_gas_units);

    return 0;
}
