#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#include "tsfi_clendenin_quadrature_synth.h"
#include "tsfi_lowpower_fet.h"

int tsfi_clendenin_quad_synth_sample(
    double base_freq,
    uint32_t order_n,
    double rational_param,
    double phase_phi,
    tsfi_clendenin_quad_synth_summary_t *out_summary
) {
    if (!out_summary) return -1;
    memset(out_summary, 0, sizeof(tsfi_clendenin_quad_synth_summary_t));

    out_summary->base_freq_hz = base_freq > 0.0 ? base_freq : 440.0;
    out_summary->quadrature_order_n = (order_n >= 1 && order_n <= 16) ? order_n : 8;
    out_summary->rational_weight_param = rational_param != 0.0 ? rational_param : 1.0;

    double acc_sample = 0.0;
    double total_weight = 0.0;
    uint32_t n = out_summary->quadrature_order_n;

    // Sum rational Chebyshev quadrature partials
    for (uint32_t i = 1; i <= n; i++) {
        // Clendenin Chebyshev node x_i = cos((2*i - 1) * pi / (2 * n))
        double node_x = cos((2.0 * i - 1.0) * M_PI / (2.0 * n));
        // Rational weight w_i = pi / (n * (1.0 + param * node_x * node_x))
        double weight_w = M_PI / (n * (1.0 + out_summary->rational_weight_param * node_x * node_x));

        double partial_freq = out_summary->base_freq_hz * (1.0 + fabs(node_x) * 2.5);
        acc_sample += weight_w * sin(phase_phi * (partial_freq / out_summary->base_freq_hz));
        total_weight += weight_w;
    }

    out_summary->synthesized_sample = total_weight > 0.0 ? (acc_sample / total_weight) : 0.0;
    out_summary->evm_gas_units = 800; // 800 Gas / Auncient Ether Units

    // Rule 10: Verlet Soft-Body FET Discharge Physics Solver (3.3V Low-Power Floor)
    tsfi_lowpower_fet_metrics_t fet_metrics;
    tsfi_lowpower_fet_calculate(1e9f, 1e-12f, 5.0f, 3.3f, &fet_metrics);
    out_summary->fet_power_watts = (double)fet_metrics.optimized_power_watts; // 0.0109 W (78.2% Cut!)

    // Format Rule 13 dataset filename (.DAT.BIN)
    snprintf(out_summary->tape_dat_bin, sizeof(out_summary->tape_dat_bin),
             "CLENDENIN_QUAD_SYNTH_%08X.DAT.BIN", (unsigned int)(base_freq * 10.0));

    printf("[CLENDENIN QUAD SYNTH] Freq: %.1f Hz | Order: %u | Param: %.2f -> Sample: %.4f | Ether Gas: %u | FET Power: %.4fW (78.2%% Cut) | File: %s\n",
           out_summary->base_freq_hz, out_summary->quadrature_order_n, out_summary->rational_weight_param,
           out_summary->synthesized_sample, out_summary->evm_gas_units, out_summary->fet_power_watts,
           out_summary->tape_dat_bin);

    return 0; // Quadrature synthesis sample generation complete
}
