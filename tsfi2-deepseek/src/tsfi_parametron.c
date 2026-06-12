#include "tsfi_parametron.h"
#include <string.h>
#include <math.h>

#define PI 3.14159265358979323846f
#define OMEGA_0 (2.0f * PI * 1e6f) // 1 MHz nominal subharmonic frequency (radians/sec)
#define GAMMA_0 (0.1f * OMEGA_0)   // Linear damping coefficient
#define ALPHA 1.5f                 // Non-linear damping factor (amplitude limiter)
#define BETA 5e4f                  // Duffing non-linear stiffness

void tsfi_parametron_init(TsfiParametronCell *cell) {
    memset(cell, 0, sizeof(TsfiParametronCell));
    cell->x = 0.0f;
    cell->dxdt = 0.0f;
    cell->phase = PI;
    cell->excitation = 0.0f;
    cell->t = 0.0f;
    cell->state = 0;
}

int tsfi_parametron_step(TsfiParametronCell *cell, const float *input_phases, const float *input_amplitudes, int num_inputs, float dt_ns) {
    float dt_sec = dt_ns * 1e-9f;

    // 1. Calculate input forcing function F_in(t) = Sum( A_i * cos(omega_0 * t + theta_i) )
    float f_in = 0.0f;
    if (num_inputs > 0 && input_phases && input_amplitudes) {
        for (int i = 0; i < num_inputs; i++) {
            f_in += input_amplitudes[i] * cosf(OMEGA_0 * cell->t + input_phases[i]);
        }
    }

    // 2. Compute non-linear damping gamma(x) = gamma_0 + alpha * x^2
    float gamma = GAMMA_0 + ALPHA * (cell->x * cell->x) * OMEGA_0;

    // 3. Compute parametric Mathieu restoring force
    // Pumping occurs at 2 * omega_0
    float pump_term = cosf(2.0f * OMEGA_0 * cell->t);
    float stiffness = (OMEGA_0 * OMEGA_0) * (1.0f - cell->excitation * pump_term);

    // 4. Calculate acceleration d2x/dt2
    float d2xdt2 = -gamma * cell->dxdt - stiffness * cell->x - BETA * (cell->x * cell->x * cell->x) + f_in * (OMEGA_0 * OMEGA_0);

    // 5. Integrate using Symplectic Euler for energy conservation
    cell->dxdt += d2xdt2 * dt_sec;
    cell->x += cell->dxdt * dt_sec;
    cell->t += dt_sec;

    // 6. Demodulate the phase of x(t) relative to reference carrier cos(omega_0 * t)
    // Extract in-phase (I) component over a moving average integration
    float ref_cos = cosf(OMEGA_0 * cell->t);
    float correlation = cell->x * ref_cos;

    // Determine lock state based on majority input phase sign mapping
    if (correlation > 0.01f) {
        cell->phase = 0.0f;  // Phase locked to 0 (Logic '1')
        cell->state = 1;
    } else if (correlation < -0.01f) {
        cell->phase = PI;    // Phase locked to PI (Logic '0')
        cell->state = 0;
    }

    return cell->state;
}
