#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>

#define NUM_DROID_STATIONS 4
#define SAMPLING_RATE 96000.0
#define SPEED_OF_LIGHT 3e8
#define CARRIER_FREQ 24000.0
#define MOTZKIN_PRIME 953467954114363ULL

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

typedef struct {
    double x, y, z;
    double vx, vy, vz;
} MobileStation;

typedef struct {
    float real;
    float imag;
    float energy;
    float phase;
} HelmholtzState;

typedef struct {
    uint32_t op_type;
    uint32_t kernel_id;
    float energy_addend;
    float frequency;
} HelmholtzOperator;

typedef struct {
    HelmholtzState state;
    double track_x;
    double track_y;
    double track_z;
} HelmholtzFilter;

// Minimal memory-footprint Helmholtz list evaluator matching main
static void evaluate_helmholtz_list(const HelmholtzOperator *ops, size_t op_count, HelmholtzState *state) {
    for (size_t i = 0; i < op_count && i < 32; i++) {
        const HelmholtzOperator *op = &ops[i];
        switch (op->op_type) {
            case 1: // HILBERT_ENCODE
                state->phase += op->frequency;
                break;
            case 2: // BANACH_NORM
                state->energy = (state->energy + op->energy_addend) / 2.0f;
                break;
            case 3: // DIFFUSION
                state->real += 0.01f;
                state->imag -= 0.01f;
                break;
            case 4: // RESONANCE
                state->energy = (float)((uint64_t)(state->energy * 1000.0f) % MOTZKIN_PRIME) / 1000.0f;
                break;
        }
    }
}

// 1. 3D TDOA + FDOA Gradient Descent Localizer
static double compute_3d_cost(double tx, double ty, double tz, const MobileStation *stations, const double *tdoas, const double *fdoas) {
    double cost = 0.0;
    
    // Node 0 is reference
    double dist0 = sqrt((stations[0].x - tx)*(stations[0].x - tx) + (stations[0].y - ty)*(stations[0].y - ty) + (stations[0].z - tz)*(stations[0].z - tz));
    double delay0 = dist0 / SPEED_OF_LIGHT;
    
    // Doppler 0
    double dx0 = tx - stations[0].x;
    double dy0 = ty - stations[0].y;
    double dz0 = tz - stations[0].z;
    double range_rate0 = 0.0;
    if (dist0 > 1e-9) {
        range_rate0 = (stations[0].vx * dx0 + stations[0].vy * dy0 + stations[0].vz * dz0) / dist0;
    }
    double freq0 = CARRIER_FREQ * (1.0 - range_rate0 / SPEED_OF_LIGHT);

    for (int i = 1; i < NUM_DROID_STATIONS; i++) {
        double disti = sqrt((stations[i].x - tx)*(stations[i].x - tx) + (stations[i].y - ty)*(stations[i].y - ty) + (stations[i].z - tz)*(stations[i].z - tz));
        double delayi = disti / SPEED_OF_LIGHT;
        
        // Expected TDOA
        double expected_tdoa = delayi - delay0;
        double tdoa_diff = expected_tdoa - tdoas[i];
        
        // Expected FDOA
        double dxi = tx - stations[i].x;
        double dyi = ty - stations[i].y;
        double dzi = tz - stations[i].z;
        double range_ratei = (stations[i].vx * dxi + stations[i].vy * dyi + stations[i].vz * dzi) / disti;
        double freqi = CARRIER_FREQ * (1.0 - range_ratei / SPEED_OF_LIGHT);
        double expected_fdoa = freqi - freq0;
        double fdoa_diff = expected_fdoa - fdoas[i];

        cost += tdoa_diff * tdoa_diff * 1e12 + fdoa_diff * fdoa_diff * 1e-2;
    }
    return cost;
}

static void localize_3d_tdoa_fdoa(const MobileStation *stations, const double *tdoas, const double *fdoas, double start_x, double start_y, double start_z, double *out_x, double *out_y, double *out_z) {
    double tx = start_x;
    double ty = start_y;
    double tz = start_z;
    double h = 0.01; // meters perturbation
    double step = 0.5;
    int max_iters = 250;

    for (int iter = 0; iter < max_iters; iter++) {
        double cost_xp = compute_3d_cost(tx + h, ty, tz, stations, tdoas, fdoas);
        double cost_xm = compute_3d_cost(tx - h, ty, tz, stations, tdoas, fdoas);
        double cost_yp = compute_3d_cost(tx, ty + h, tz, stations, tdoas, fdoas);
        double cost_ym = compute_3d_cost(tx, ty - h, tz, stations, tdoas, fdoas);
        double cost_zp = compute_3d_cost(tx, ty, tz + h, stations, tdoas, fdoas);
        double cost_zm = compute_3d_cost(tx, ty, tz - h, stations, tdoas, fdoas);

        double grad_x = (cost_xp - cost_xm) / (2.0 * h);
        double grad_y = (cost_yp - cost_ym) / (2.0 * h);
        double grad_z = (cost_zp - cost_zm) / (2.0 * h);

        double len = sqrt(grad_x * grad_x + grad_y * grad_y + grad_z * grad_z);
        if (len < 1e-15) break;

        tx -= step * (grad_x / len);
        ty -= step * (grad_y / len);
        tz -= step * (grad_z / len);
        
        step *= 0.98;
    }
    *out_x = tx;
    *out_y = ty;
    *out_z = tz;
}

// Update the Helmholtz filter using the list operators
static void helmholtz_filter_update(HelmholtzFilter *filter, double mz_x, double mz_y, double mz_z) {
    double target_phase = atan2(mz_y, mz_x);
    double target_energy = sqrt(mz_x * mz_x + mz_y * mz_y + mz_z * mz_z);

    // Formulate a Helmholtz operation list to transition state smoothly
    HelmholtzOperator ops[2];
    ops[0].op_type = 1; // HILBERT_ENCODE (phase adjust)
    ops[0].frequency = (float)((target_phase - filter->state.phase) * 0.7);

    ops[1].op_type = 2; // BANACH_NORM (energy blending)
    ops[1].energy_addend = (float)target_energy;

    evaluate_helmholtz_list(ops, 2, &filter->state);

    // Map the complex phase and energy variables back to 3D Cartesian space
    filter->track_x = filter->state.energy * cos(filter->state.phase);
    filter->track_y = filter->state.energy * sin(filter->state.phase);
    filter->track_z = mz_z; // Direct height tracking
}

int main() {
    printf("=== TSFI2 Advanced Direction Finding Simulation ===\n");

    // Initialize 4 mobile receiver stations (drones) at different positions and velocities
    MobileStation stations[NUM_DROID_STATIONS] = {
        {0.0, 0.0, 50.0, 50.0, 0.0, 0.0},
        {1000.0, 0.0, 80.0, 0.0, 50.0, 0.0},
        {0.0, 1000.0, 60.0, -50.0, 0.0, 5.0},
        {1000.0, 1000.0, 100.0, 0.0, -50.0, -5.0}
    };

    // True target initial state: pos=(500, 500, 20) m, vel=(15, -10, 2) m/s
    double true_x = 500.0, true_y = 500.0, true_z = 20.0;
    double true_vx = 15.0, true_vy = -10.0, true_vz = 2.0;

    // Initialize Helmholtz filter
    HelmholtzFilter filter;
    memset(&filter, 0, sizeof(filter));
    filter.track_x = 500.0;
    filter.track_y = 500.0;
    filter.track_z = 20.0;
    filter.state.phase = (float)atan2(500.0, 500.0);
    filter.state.energy = (float)sqrt(500.0 * 500.0 + 500.0 * 500.0 + 20.0 * 20.0);

    double dt = 1.0;

    printf("\n[SIMULATION] Starting Emitter Track Run (15 time steps) using Helmholtz Filter:\n");
    for (int step = 0; step < 15; step++) {
        // Move true target
        true_x += true_vx * dt;
        true_y += true_vy * dt;
        true_z += true_vz * dt;

        // Generate measured TDOA and FDOA signals at receiver stations
        double tdoas[NUM_DROID_STATIONS];
        double fdoas[NUM_DROID_STATIONS];
        
        double dist0 = sqrt((stations[0].x - true_x)*(stations[0].x - true_x) + (stations[0].y - true_y)*(stations[0].y - true_y) + (stations[0].z - true_z)*(stations[0].z - true_z));
        double delay0 = dist0 / SPEED_OF_LIGHT;
        
        double dx0 = true_x - stations[0].x;
        double dy0 = true_y - stations[0].y;
        double dz0 = true_z - stations[0].z;
        double range_rate0 = (stations[0].vx * dx0 + stations[0].vy * dy0 + stations[0].vz * dz0) / dist0;
        double freq0 = CARRIER_FREQ * (1.0 - range_rate0 / SPEED_OF_LIGHT);

        tdoas[0] = 0.0;
        fdoas[0] = 0.0;

        for (int i = 1; i < NUM_DROID_STATIONS; i++) {
            double disti = sqrt((stations[i].x - true_x)*(stations[i].x - true_x) + (stations[i].y - true_y)*(stations[i].y - true_y) + (stations[i].z - true_z)*(stations[i].z - true_z));
            double delayi = disti / SPEED_OF_LIGHT;
            tdoas[i] = delayi - delay0;

            double dxi = true_x - stations[i].x;
            double dyi = true_y - stations[i].y;
            double dzi = true_z - stations[i].z;
            double range_ratei = (stations[i].vx * dxi + stations[i].vy * dyi + stations[i].vz * dzi) / disti;
            double freqi = CARRIER_FREQ * (1.0 - range_ratei / SPEED_OF_LIGHT);
            fdoas[i] = freqi - freq0;
        }

        // Resolve 3D target coordinates via TDOA/FDOA gradient descent localization
        double localized_x, localized_y, localized_z;
        localize_3d_tdoa_fdoa(stations, tdoas, fdoas, filter.track_x, filter.track_y, filter.track_z, &localized_x, &localized_y, &localized_z);

        // Update Helmholtz tracking filter with measurement
        helmholtz_filter_update(&filter, localized_x, localized_y, localized_z);

        double horizontal_error = sqrt((filter.track_x - true_x)*(filter.track_x - true_x) + (filter.track_y - true_y)*(filter.track_y - true_y));
        double tracking_error = sqrt((filter.track_x - true_x)*(filter.track_x - true_x) + (filter.track_y - true_y)*(filter.track_y - true_y) + (filter.track_z - true_z)*(filter.track_z - true_z));

        printf("  Step %d -> True: (%.1f, %.1f, %.1f) | Tracked: (%.1f, %.1f, %.1f) | H-Error: %.2f m | Total-Error: %.2f m\n",
               step, true_x, true_y, true_z, filter.track_x, filter.track_y, filter.track_z, horizontal_error, tracking_error);
        
        // Horizontal tracking error must stay within first-order low-pass filter tracking lag boundary (< 40.0 meters)
        if (step == 14) {
            assert(horizontal_error < 40.0);
        }
    }

    printf("\n[SUCCESS] Helmholtz Filter and 3D TDOA/FDOA localization features validated successfully!\n");
    return 0;
}
