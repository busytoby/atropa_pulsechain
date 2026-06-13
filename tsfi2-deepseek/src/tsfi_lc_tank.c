#include "tsfi_lc_tank.h"
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Semi-empirical Medhurst factor approximation for H/D between 0.1 and 5.0
static double get_medhurst_factor(double height_over_diameter) {
    if (height_over_diameter <= 0.2) return 0.35;
    if (height_over_diameter <= 0.5) return 0.38;
    if (height_over_diameter <= 1.0) return 0.46;
    if (height_over_diameter <= 2.0) return 0.50;
    if (height_over_diameter <= 3.0) return 0.61;
    if (height_over_diameter <= 4.0) return 0.72;
    return 0.81; // Extrapolated / capped
}

void tsfi_distributed_coil_init(
    TsfiDistributedCoil *coil,
    double diameter,
    double height,
    double turns,
    double wire_gauge_mm,
    double C_tune
) {
    if (!coil) return;

    coil->coil_diameter = diameter;
    coil->coil_height = height;
    coil->wire_radius = (wire_gauge_mm / 1000.0) / 2.0;
    coil->C_tune = C_tune;

    // 1. Calculate Inductance using Wheeler's formula: L = (d^2 * n^2) / (18d + 40l)
    // where d is diameter in inches, l is length in inches. Converts to meters:
    double d_inches = diameter * 39.3701;
    double h_inches = height * 39.3701;
    double L_uH = (d_inches * d_inches * turns * turns) / (18.0 * d_inches + 40.0 * h_inches);
    coil->L_total = L_uH * 1e-6;

    // 2. Calculate Medhurst self-capacitance: C0 = K * D (in pF, where D is diameter in cm)
    double D_cm = diameter * 100.0;
    double K_factor = get_medhurst_factor(height / diameter);
    coil->C_total_parasitic = K_factor * D_cm * 1e-12; // Farads

    // 3. DC Resistance of copper wire
    // Wire length = turns * pi * diameter
    double wire_len = turns * M_PI * diameter;
    double resistivity_copper = 1.68e-8; // Ohm-meters
    double area = M_PI * coil->wire_radius * coil->wire_radius;
    coil->R_dc = resistivity_copper * wire_len / area;

    // 4. Distribute values across 16 segments
    for (int j = 0; j < COIL_SEGMENTS; j++) {
        coil->v[j] = 0.0;
        coil->i_l[j] = 0.0;
        coil->L_seg[j] = coil->L_total / COIL_SEGMENTS;
        coil->C_seg[j] = (coil->C_total_parasitic) / COIL_SEGMENTS;
        coil->R_seg[j] = coil->R_dc / COIL_SEGMENTS;
    }

    // Node 15 (last segment output) incorporates the parallel tuning capacitor C_tune
    coil->C_seg[COIL_SEGMENTS - 1] += C_tune;
}

void tsfi_distributed_coil_process(
    TsfiDistributedCoil *coil,
    const float *input_rf,
    float *output_grid,
    size_t count,
    double sample_rate,
    double current_freq_hz
) {
    if (!coil || !input_rf || !output_grid || count == 0) return;

    double dt = 1.0 / sample_rate;

    // Update dynamic resistance due to RF Skin Effect: R_ac = R_dc * (wire_radius / (2 * skin_depth))
    // Skin depth delta = sqrt(rho / (pi * f * mu0))
    double resistivity_copper = 1.68e-8;
    double mu0 = 4.0 * M_PI * 1e-7;
    double skin_depth = sqrt(resistivity_copper / (M_PI * current_freq_hz * mu0));
    
    double r_multiplier = 1.0;
    if (skin_depth < coil->wire_radius) {
        r_multiplier = coil->wire_radius / (2.0 * skin_depth);
        if (r_multiplier < 1.0) r_multiplier = 1.0;
    }

    for (int j = 0; j < COIL_SEGMENTS; j++) {
        coil->R_seg[j] = (coil->R_dc / COIL_SEGMENTS) * r_multiplier;
    }

    // Process sample by sample
    for (size_t step = 0; step < count; step++) {
        double v_in = (double)input_rf[step];

        // Numerical finite difference solver:
        // Segment 0 is fed directly by the RF source
        // Segment j current update: d(i_l[j])/dt = (v[j-1] - v[j] - i_l[j] * R_seg[j]) / L_seg[j]
        // Segment j voltage update: d(v[j])/dt = (i_l[j] - i_l[j+1]) / C_seg[j]

        // Current solver step
        for (int j = 0; j < COIL_SEGMENTS; j++) {
            double v_prev = (j == 0) ? v_in : coil->v[j - 1];
            double v_curr = coil->v[j];
            double dil = (v_prev - v_curr - coil->i_l[j] * coil->R_seg[j]) / coil->L_seg[j];
            coil->i_l[j] += dil * dt;
        }

        // Voltage solver step
        for (int j = 0; j < COIL_SEGMENTS; j++) {
            double i_next = (j == COIL_SEGMENTS - 1) ? 0.0 : coil->i_l[j + 1];
            double dvc = (coil->i_l[j] - i_next) / coil->C_seg[j];
            coil->v[j] += dvc * dt;
        }

        // The grid of our Audion is tapped across the last segment (node 15)
        output_grid[step] = (float)coil->v[COIL_SEGMENTS - 1];
    }
}