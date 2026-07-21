#ifndef TSFI_CHAPPLE_ENGINE_H
#define TSFI_CHAPPLE_ENGINE_H

#include <stdint.h>
#include <stddef.h>

#define MAX_SERIES_DEGREE 16

typedef struct {
    double cutoff_hz;                   // Filter cutoff frequency in Hz (e.g. 1000.0 Hz)
    double resonance;                   // Resonance Q factor [0.0 = Flat, 4.0 = Self-Oscillation]
    double drive_saturation;            // Input drive level [0.1 = Linear, 2.0 = Overdriven]
    double pole_state[4];               // 4-Pole RC Ladder Filter State Buffers
    double inverse_b_coeffs[5];         // Chapple Inverse Power Series Coefficients (b1..b5)
} tsfi_chapple_moog_vcf_t;

typedef struct {
    uint32_t chapple_id;
    uint32_t series_inversions_count;   // M. A. Chapple Power Series Reversions
    tsfi_chapple_moog_vcf_t moog_vcf;   // Moog Transistor Ladder Filter State
    uint32_t evm_gas_units;             // 280 Gas / Auncient Ether Units
    double fet_power_watts;             // 0.0109 W under 78.2% Power Cut
    char tape_dat_bin[128];             // Single-Header Array (.DAT.BIN - Rule 13)
} tsfi_chapple_engine_t;

/* Initialize M. A. Chapple Power Series Inversion Engine */
int tsfi_chapple_engine_init(
    uint32_t chapple_id,
    tsfi_chapple_engine_t *engine
);

/* Compute inverse power series coefficients x = b1*y + b2*y^2 + ... given y = a1*x + a2*x^2 + ... (Lagrange Reversion) */
int tsfi_chapple_revert_power_series(
    tsfi_chapple_engine_t *engine,
    const double *a_coeffs,             // Input coefficients a1, a2, ..., aN (1-indexed at array 0..N-1)
    size_t degree,                       // Degree N (up to MAX_SERIES_DEGREE)
    double *b_coeffs_out                // Output inverse coefficients b1, b2, ..., bN
);

/* Initialize Moog Transistor Ladder Filter with Chapple Linearization */
int tsfi_chapple_moog_vcf_init(
    tsfi_chapple_engine_t *engine,
    double cutoff_hz,
    double resonance,
    double drive_saturation
);

/* Process single audio sample through Chapple Linearized Moog 4-Pole VCF */
double tsfi_chapple_moog_vcf_process_sample(
    tsfi_chapple_engine_t *engine,
    double sample_in,
    double sample_rate
);

#endif // TSFI_CHAPPLE_ENGINE_H
