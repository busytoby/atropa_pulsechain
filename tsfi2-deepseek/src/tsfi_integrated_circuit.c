#include "tsfi_integrated_circuit.h"
#include <string.h>
#include <math.h>

#define VCC 3.0f          // Vcc for early RTL
#define RC 640.0f         // Collector load resistor (Ohms)
#define C_LOAD 15e-12f    // External load capacitance (Farads)
#define I0_LEAK 10e-6f    // Base leakage at 25C (Amperes)

void tsfi_monolithic_nor_init(TsfiMonolithicNorGate *gate) {
    memset(gate, 0, sizeof(TsfiMonolithicNorGate));
    gate->temp_c = 25.0f;
    gate->c_sub = 5e-12f;      // 5 pF parasitic substrate capacitance
    gate->v_substrate = 0.0f;  // Substrate tied to GND
    gate->v_out = 3.0f;        // Start high (inputs default to low)
    gate->state = 1;
    gate->active_charge_pf = 0.0f;
}

int tsfi_monolithic_nor_step(TsfiMonolithicNorGate *gate, int input_a, int input_b, float dt_ns, float *out_v) {
    // 1. Calculate temperature-dependent substrate junction leakage current
    // Leakage doubles every 10 degrees C above 25C
    float temp_factor = powf(2.0f, (gate->temp_c - 25.0f) / 10.0f);
    float i_leak = I0_LEAK * temp_factor;

    // 2. Determine target state and drive current
    float i_charge = 0.0f;
    float c_total = C_LOAD + gate->c_sub;

    if (input_a || input_b) {
        // Active pull-down: output transitions towards logic low (V ~ 0.15V)
        // Strong saturated transistor current
        float i_pull_down = 4.0e-3f; // 4 mA
        i_charge = -i_pull_down - i_leak;
    } else {
        // Pull-up: charging through collector resistor Rc
        float i_drive = (VCC - gate->v_out) / RC;
        i_charge = i_drive - i_leak;
    }

    // 3. Update output voltage based on integration
    float dt_sec = dt_ns * 1e-9f;
    float dv = (i_charge / c_total) * dt_sec;
    gate->v_out += dv;

    // Clamp output voltage between saturation voltage (0.15V) and Vcc (3.0V)
    if (gate->v_out > VCC) {
        gate->v_out = VCC;
    } else if (gate->v_out < 0.15f) {
        gate->v_out = 0.15f;
    }

    // Update digital logic threshold (typically 1.5V for RTL)
    gate->state = (gate->v_out > 1.5f) ? 1 : 0;

    // Track charge metric (for telemetry and leakage analysis)
    gate->active_charge_pf = gate->v_out * c_total * 1e12f;

    if (out_v) {
        *out_v = gate->v_out;
    }

    return gate->state;
}
