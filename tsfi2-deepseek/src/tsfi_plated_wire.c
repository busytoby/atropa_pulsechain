#include "tsfi_plated_wire.h"
#include <string.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

void tsfi_plated_wire_init(TsfiPlatedWireCell *cell) {
    memset(cell, 0, sizeof(TsfiPlatedWireCell));
    cell->anisotropy_k = 2.0f;     // 2 Oe
    cell->magnetization_ms = 1.0f;  // Normalized
    cell->state = 1;                // Default state 1 (magnetization at 0)
    cell->theta = 0.0f;
}

int tsfi_plated_wire_step(TsfiPlatedWireCell *cell, float word_current, float bit_current, float dt_ns, float *v_sense) {
    float dt_sec = dt_ns * 1e-9f;

    // 1. Convert currents to fields
    // Word current creates hard axis field (longitudinal)
    float h_hard = word_current * 6.0f;
    // Bit current creates easy axis steering field (circumferential)
    float h_easy = bit_current * 1.5f;

    // 2. Coherent rotation dynamics (Landau-Lifshitz-Gilbert style relaxation)
    // dtheta/dt = - (1/tau) * dE/dtheta
    // E(theta) = K * sin^2(theta) - H_easy * cos(theta) - H_hard * sin(theta)
    // dE/dtheta = K * sin(2*theta) + H_easy * sin(theta) - H_hard * cos(theta)
    float k = cell->anisotropy_k;
    float dE_dtheta = k * sinf(2.0f * cell->theta) + h_easy * sinf(cell->theta) - h_hard * cosf(cell->theta);

    float tau = 15e-9f; // 15 ns relaxation time constant
    float dtheta = -(dE_dtheta / tau) * dt_sec;

    // Apply simple angular limit to keep theta within [0, PI]
    float old_cos = cosf(cell->theta);
    cell->theta += dtheta;

    // Keep theta bounded between 0 and PI
    if (cell->theta < 0.0f) cell->theta = 0.0f;
    if (cell->theta > (float)M_PI) cell->theta = (float)M_PI;

    float new_cos = cosf(cell->theta);

    // 3. Induced sense voltage V_sense = -alpha * Ms * d(cos theta)/dt
    float dcos_dt = (new_cos - old_cos) / dt_sec;
    float alpha = 10.0f; // Scale factor for sense voltage in mV
    if (v_sense) {
        *v_sense = -alpha * cell->magnetization_ms * dcos_dt * 1e-9f; // Convert back to mV range
    }

    // 4. Resolve binary state based on easy axis alignment
    // If word field is removed (h_hard is zero), the vector settles to 0 (State 1) or PI (State 0)
    if (word_current == 0.0f) {
        if (cell->theta < (float)M_PI / 2.0f) {
            cell->state = 1;
            cell->theta = 0.0f; // Settle fully to easy axis
        } else {
            cell->state = 0;
            cell->theta = (float)M_PI; // Settle fully to easy axis
        }
    }

    return cell->state;
}
