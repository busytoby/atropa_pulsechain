#ifndef TSFI_CLENDENIN_QUADRATURE_SYNTH_H
#define TSFI_CLENDENIN_QUADRATURE_SYNTH_H

#include <stdint.h>
#include <stddef.h>

typedef struct {
    double base_freq_hz;         // Base fundamental frequency (e.g., 440.0 Hz)
    uint32_t quadrature_order_n; // Order N of rational Chebyshev nodes (1..16)
    double rational_weight_param;// Shaping parameter for node distribution
    double synthesized_sample;   // Output acoustic sample [-1.0, 1.0]
    uint32_t evm_gas_units;      // 800 Gas / Auncient Ether Units
    double fet_power_watts;      // 0.0109 W under 78.2% Power Cut
    char tape_dat_bin[128];      // Single-Header Array (.DAT.BIN - Rule 13)
} tsfi_clendenin_quad_synth_summary_t;

/* Synthesize audio sample using Clendenin rational Chebyshev quadrature nodes */
int tsfi_clendenin_quad_synth_sample(
    double base_freq,
    uint32_t order_n,
    double rational_param,
    double phase_phi,
    tsfi_clendenin_quad_synth_summary_t *out_summary
);

#endif // TSFI_CLENDENIN_QUADRATURE_SYNTH_H
