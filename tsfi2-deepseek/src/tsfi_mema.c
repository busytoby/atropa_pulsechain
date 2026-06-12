#include "tsfi_mema.h"
#include <string.h>
#include <math.h>

void tsfi_mema_init(TsfiMemaModule *module) {
    memset(module, 0, sizeof(TsfiMemaModule));
    
    // Default design parameters
    module->trace_distance_um = 30.0f;       // 30 microns spacing
    module->coupling_length_mm = 15.0f;      // 15 mm length
    module->propagation_delay_ns = 1.0f;     // 1 ns delay
    module->noise_margin_v = 0.4f;           // 0.4V noise margin (typical logic)

    // Thermal parameters
    module->temp_ambient = 25.0f;
    module->temp_substrate = 25.0f;
    module->temp_ic[0] = 25.0f;
    module->temp_ic[1] = 25.0f;
    
    module->power_ic[0] = 1.5f;              // 1.5W dissipation
    module->power_ic[1] = 1.2f;              // 1.2W dissipation

    module->thermal_res_jc = 10.0f;          // 10 C/W junction-to-substrate
    module->thermal_res_sa = 25.0f;          // 25 C/W substrate-to-ambient

    module->heat_capacity_ic = 1e-6f;        // Low scale to see heating quickly
    module->heat_capacity_sub = 5e-6f;
}

int tsfi_mema_step(TsfiMemaModule *module, float v_agg, float dt_ns, float *v_victim_noise) {
    float dt_sec = dt_ns * 1e-9f;

    // 1. Update Aggressor History Ring Buffer
    module->v_agg_history[module->history_index] = v_agg;
    
    // Determine historical index for t - 2 * t_d
    int steps_back = (int)roundf(2.0f * module->propagation_delay_ns / dt_ns);
    if (steps_back < 1) steps_back = 1;
    if (steps_back >= 100) steps_back = 99;

    int hist_idx = module->history_index - steps_back;
    if (hist_idx < 0) hist_idx += 100;

    float v_agg_delayed = module->v_agg_history[hist_idx];

    // Increment and wrap ring buffer index
    module->history_index = (module->history_index + 1) % 100;

    // 2. Calculate Crosstalk Coupling Coefficient
    // K goes down with square of distance d
    float d = module->trace_distance_um;
    float k_next = 225.0f / (d * d); // Normalized so d=30um gives k_next = 0.25
    float k_fext = 15.0f / (d * d);  // Far-end coupling coefficient

    // NEXT: proportional to difference between current and delayed aggressor voltage
    float v_next = k_next * (v_agg - v_agg_delayed);

    // FEXT: proportional to derivative of aggressor voltage and coupling length
    float dv_dt = (v_agg - module->last_v_agg) / dt_ns;
    float v_fext = k_fext * module->coupling_length_mm * dv_dt;

    module->last_v_agg = v_agg;

    // Output worst-case peak absolute noise
    float noise = fabsf(v_next);
    if (fabsf(v_fext) > noise) {
        noise = fabsf(v_fext);
    }
    if (v_victim_noise) {
        *v_victim_noise = noise;
    }

    // 3. Thermal Network Simulation
    // Update IC temperatures
    for (int i = 0; i < 2; i++) {
        float q_jc = (module->temp_ic[i] - module->temp_substrate) / module->thermal_res_jc;
        float dT_ic = (module->power_ic[i] - q_jc) / module->heat_capacity_ic * dt_sec;
        module->temp_ic[i] += dT_ic;
    }

    // Update Substrate temperature
    float q_in = ((module->temp_ic[0] - module->temp_substrate) / module->thermal_res_jc) +
                 ((module->temp_ic[1] - module->temp_substrate) / module->thermal_res_jc);
    float q_out = (module->temp_substrate - module->temp_ambient) / module->thermal_res_sa;
    float dT_sub = (q_in - q_out) / module->heat_capacity_sub * dt_sec;
    module->temp_substrate += dT_sub;

    module->t += dt_sec;

    // 4. Glitch & Over-temperature detection
    int failure = 0;
    if (noise > module->noise_margin_v) {
        failure = 1; // Glitch due to crosstalk
    }
    if (module->temp_ic[0] > 125.0f || module->temp_ic[1] > 125.0f) {
        failure = 1; // Thermal overload
    }

    return failure;
}
