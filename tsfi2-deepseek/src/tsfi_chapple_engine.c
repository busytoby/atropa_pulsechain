#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "tsfi_chapple_engine.h"
#include "tsfi_lowpower_fet.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

int tsfi_chapple_engine_init(
    uint32_t chapple_id,
    tsfi_chapple_engine_t *engine
) {
    if (!engine) return -1;
    memset(engine, 0, sizeof(tsfi_chapple_engine_t));

    engine->chapple_id = chapple_id;
    engine->evm_gas_units = 280; // 280 Gas / Auncient Ether Units per evaluation

    // Rule 10: Verlet Soft-Body FET Discharge Physics Solver (3.3V Low-Power Floor)
    tsfi_lowpower_fet_metrics_t fet_metrics;
    tsfi_lowpower_fet_calculate(1e9f, 1e-12f, 5.0f, 3.3f, &fet_metrics);
    engine->fet_power_watts = (double)fet_metrics.optimized_power_watts; // 0.0109 W (78.2% Cut!)

    // Format Rule 13 dataset filename (.DAT.BIN)
    snprintf(engine->tape_dat_bin, sizeof(engine->tape_dat_bin), "CHAPPLE_MOOG_%08X.DAT.BIN", chapple_id);

    // Default Moog VCF setup (1000 Hz Cutoff, Q=2.0, Drive=1.0)
    tsfi_chapple_moog_vcf_init(engine, 1000.0, 2.0, 1.0);
    return 0;
}

int tsfi_chapple_revert_power_series(
    tsfi_chapple_engine_t *engine,
    const double *a_coeffs,
    size_t degree,
    double *b_coeffs_out
) {
    if (!engine || !a_coeffs || degree == 0 || degree > MAX_SERIES_DEGREE || !b_coeffs_out) return -1;
    if (a_coeffs[0] == 0.0) return -2; // Non-invertible if a1 == 0

    memset(b_coeffs_out, 0, degree * sizeof(double));

    double a1 = a_coeffs[0];
    double a2 = (degree >= 2) ? a_coeffs[1] : 0.0;
    double a3 = (degree >= 3) ? a_coeffs[2] : 0.0;
    double a4 = (degree >= 4) ? a_coeffs[3] : 0.0;
    double a5 = (degree >= 5) ? a_coeffs[4] : 0.0;

    // M. A. Chapple CACM Nov 1961 Power Series Reversion Formulations:
    b_coeffs_out[0] = 1.0 / a1;

    if (degree >= 2) {
        b_coeffs_out[1] = -a2 / (a1 * a1 * a1);
    }

    if (degree >= 3) {
        b_coeffs_out[2] = (2.0 * a2 * a2 - a1 * a3) / pow(a1, 5.0);
    }

    if (degree >= 4) {
        b_coeffs_out[3] = (5.0 * a1 * a2 * a3 - 5.0 * a2 * a2 * a2 - a1 * a1 * a4) / pow(a1, 7.0);
    }

    if (degree >= 5) {
        b_coeffs_out[4] = (6.0 * a1 * a1 * a2 * a4 + 3.0 * a1 * a1 * a3 * a3 - 21.0 * a1 * a2 * a2 * a3 + 14.0 * pow(a2, 4.0) - pow(a1, 3.0) * a5) / pow(a1, 9.0);
    }

    for (size_t k = 5; k < degree; k++) {
        double sign = (k % 2 == 1) ? -1.0 : 1.0;
        b_coeffs_out[k] = sign * a_coeffs[k] / pow(a1, (double)(2 * k + 1));
    }

    engine->series_inversions_count++;
    return 0;
}

int tsfi_chapple_moog_vcf_init(
    tsfi_chapple_engine_t *engine,
    double cutoff_hz,
    double resonance,
    double drive_saturation
) {
    if (!engine) return -1;
    tsfi_chapple_moog_vcf_t *vcf = &engine->moog_vcf;

    vcf->cutoff_hz = cutoff_hz;
    vcf->resonance = resonance;
    vcf->drive_saturation = drive_saturation;
    memset(vcf->pole_state, 0, sizeof(vcf->pole_state));

    // Derive tanh(x) forward expansion coefficients: a1=1.0, a2=0.0, a3=-1/3, a4=0.0, a5=2/15
    double tanh_a_coeffs[5] = {1.0, 0.0, -1.0 / 3.0, 0.0, 2.0 / 15.0};
    tsfi_chapple_revert_power_series(engine, tanh_a_coeffs, 5, vcf->inverse_b_coeffs);

    printf("[CHAPPLE MOOG VCF INIT] Cutoff: %.1f Hz | Resonance: %.2f | Drive: %.2f | Chapple b1=%.2f, b3=%.2f, b5=%.2f\n",
           cutoff_hz, resonance, drive_saturation, vcf->inverse_b_coeffs[0], vcf->inverse_b_coeffs[2], vcf->inverse_b_coeffs[4]);

    return 0;
}

double tsfi_chapple_moog_vcf_process_sample(
    tsfi_chapple_engine_t *engine,
    double sample_in,
    double sample_rate
) {
    if (!engine || sample_rate <= 0.0) return 0.0;
    tsfi_chapple_moog_vcf_t *vcf = &engine->moog_vcf;

    // Apply Chapple Pre-Distortion Inverse Waveshaper: x = b1*y + b3*y^3 + b5*y^5
    double y = sample_in * vcf->drive_saturation;
    double y3 = y * y * y;
    double y5 = y3 * y * y;
    double linear_x = vcf->inverse_b_coeffs[0] * y + vcf->inverse_b_coeffs[2] * y3 + vcf->inverse_b_coeffs[4] * y5;

    // Calculate 4-Pole Moog RC ladder cutoff coefficient g
    double f = vcf->cutoff_hz / sample_rate;
    if (f > 0.45) f = 0.45;
    double g = 1.0 - exp(-2.0 * M_PI * f);

    // Moog Feedback loop with resonance Q
    double feedback = vcf->resonance * vcf->pole_state[3];
    double in_stage = linear_x - feedback;

    // 4 RC Pole Cascade (One-Pole Low-Pass Filter Chain)
    vcf->pole_state[0] += g * (in_stage - vcf->pole_state[0]);
    vcf->pole_state[1] += g * (vcf->pole_state[0] - vcf->pole_state[1]);
    vcf->pole_state[2] += g * (vcf->pole_state[1] - vcf->pole_state[2]);
    vcf->pole_state[3] += g * (vcf->pole_state[2] - vcf->pole_state[3]);

    return vcf->pole_state[3];
}
