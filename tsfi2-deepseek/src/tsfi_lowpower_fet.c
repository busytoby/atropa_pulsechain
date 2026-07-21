#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "tsfi_lowpower_fet.h"

int tsfi_lowpower_fet_calculate(
    float clock_freq_hz,
    float gate_cap_farads,
    float v_dd_standard,
    float v_dd_scaled,
    tsfi_lowpower_fet_metrics_t *out_metrics
) {
    if (!out_metrics || clock_freq_hz <= 0.0f || gate_cap_farads <= 0.0f) return -1;
    memset(out_metrics, 0, sizeof(tsfi_lowpower_fet_metrics_t));

    // 1. Compute Standard Dual-Word Switching Power (2 Bus Line Toggles @ Standard Vdd)
    // P_dyn = 2 * f * C * Vdd_std^2
    out_metrics->standard_power_watts = 2.0 * (double)clock_freq_hz * (double)gate_cap_farads * (double)(v_dd_standard * v_dd_standard);

    // 2. Compute Knuth Base 2i Packed Single-Word Switching Power (1 Bus Line Toggle @ Scaled Vdd)
    // P_dyn_opt = 1 * f * C * Vdd_scaled^2
    out_metrics->optimized_power_watts = 1.0 * (double)clock_freq_hz * (double)gate_cap_farads * (double)(v_dd_scaled * v_dd_scaled);

    // 3. Calculate Power Reduction Percentage and Energy Saved
    out_metrics->joules_saved_per_sec = out_metrics->standard_power_watts - out_metrics->optimized_power_watts;
    out_metrics->power_reduction_pct = (out_metrics->joules_saved_per_sec / out_metrics->standard_power_watts) * 100.0;

    printf("[LOW-POWER FET] Standard Power: %.6f W | Knuth Base 2i Power: %.6f W | Saved: %.2f%% (%.6f Joules/sec)\n",
           out_metrics->standard_power_watts, out_metrics->optimized_power_watts, out_metrics->power_reduction_pct, out_metrics->joules_saved_per_sec);

    return 0; // Calculation complete
}
