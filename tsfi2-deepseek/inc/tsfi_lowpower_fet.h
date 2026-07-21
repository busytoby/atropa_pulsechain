#ifndef TSFI_LOWPOWER_FET_H
#define TSFI_LOWPOWER_FET_H

#include <stdint.h>
#include <stddef.h>

typedef struct {
    double standard_power_watts; // Standard 2-word Dual-Axis Power Draw
    double optimized_power_watts;// Knuth Base 2i Single-Word Voltage-Scaled Power Draw
    double power_reduction_pct;  // Net Percentage Reduction in FET Power
    double joules_saved_per_sec; // Energy Saved per Second (Watts)
} tsfi_lowpower_fet_metrics_t;

/* Compute FET Power Reduction under Knuth Base 2i packing and Verlet voltage scaling */
int tsfi_lowpower_fet_calculate(
    float clock_freq_hz,
    float gate_cap_farads,
    float v_dd_standard,
    float v_dd_scaled,
    tsfi_lowpower_fet_metrics_t *out_metrics
);

#endif // TSFI_LOWPOWER_FET_H
