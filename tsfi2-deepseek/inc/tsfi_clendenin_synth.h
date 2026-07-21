#ifndef TSFI_CLENDENIN_SYNTH_H
#define TSFI_CLENDENIN_SYNTH_H

#include <stdint.h>
#include <stddef.h>

typedef struct {
    double freq_hz;             // Base synthesis frequency in Hz (e.g. 440.0 Hz)
    double elliptic_modulus_k;  // Elliptic modulus k [0.0 = Sine, 1.0 = Pulse]
    double phase_angle_phi;     // Current phase angle phi in radians
    double sample_out;          // Synthesized acoustic sample output [-1.0, 1.0]
    uint32_t evm_gas_units;     // 650 Gas / Auncient Ether Units
    double fet_power_watts;     // 0.0109 W under 78.2% Power Cut
    char tape_dat_bin[128];     // Rule 13 Filename (.DAT.BIN)
} tsfi_clendenin_synth_summary_t;

/* Synthesize audio sample using Clendenin rational approximation of incomplete elliptic integrals */
int tsfi_clendenin_synth_sample(
    double freq_hz,
    double modulus_k,
    double phase_phi,
    tsfi_clendenin_synth_summary_t *out_summary
);

#endif // TSFI_CLENDENIN_SYNTH_H
