#include "tsfi_ttl_gate.h"
#include <string.h>

#define VCC 5.0f             // Vcc for TTL
#define R1 4000.0f           // Q1 base resistor (Ohms)
#define R_UP 130.0f          // Totem-pole pull-up resistor (Ohms)
#define C_LOAD 15e-12f       // Output load capacitance (Farads)
#define Q_ON_THRESHOLD 0.5f  // Splitter turn-on charge (pico-Coulombs)

void tsfi_ttl_nand_init(TsfiTtlNandGate *gate) {
    memset(gate, 0, sizeof(TsfiTtlNandGate));
    gate->v_out = 3.5f;        // Nominal logic high for TTL (~3.5V under load)
    gate->state = 1;
    gate->base_charge_pf = 0.0f;
}

int tsfi_ttl_nand_step(TsfiTtlNandGate *gate, int input_a, int input_b, float dt_ns, float *out_v) {
    float dt_sec = dt_ns * 1e-9f;

    // 1. Multi-emitter input stage current steering logic
    float i_splitter_base = 0.0f;
    if (input_a == 1 && input_b == 1) {
        // Both inputs high: base-emitter junctions reverse-biased.
        // Current flows through base-collector junction into the phase-splitter Q2 base.
        i_splitter_base = (VCC - 1.4f) / R1; // ~0.9 mA
    } else {
        // At least one input low: steers base current to input.
        // Phase-splitter base charge discharges through pull-down path.
        float v_splitter_base = (gate->base_charge_pf > 0.0f) ? 0.7f : 0.0f;
        i_splitter_base = -v_splitter_base / 1300.0f; // Discharges through ~1.3k internal path
    }

    // 2. Integrate phase splitter base charge
    gate->base_charge_pf += (i_splitter_base * 1e12f) * dt_sec;
    if (gate->base_charge_pf < 0.0f) {
        gate->base_charge_pf = 0.0f;
    } else if (gate->base_charge_pf > 2.0f) {
        gate->base_charge_pf = 2.0f; // Saturation limit
    }

    // 3. Determine output drive current based on phase splitter status
    float i_charge = 0.0f;
    if (gate->base_charge_pf > Q_ON_THRESHOLD) {
        // Phase splitter and pull-down transistor Q3 are ON (Strong active pull-down)
        float i_pull_down = 15.0e-3f; // Strong 15 mA sink capacity
        i_charge = -i_pull_down;
    } else {
        // Splitter OFF, pull-down Q3 OFF, Active totem-pole pull-up Q4 ON
        float v_drop = 0.7f; // Vbe drop of pull-up transistor
        float i_pull_up = (VCC - v_drop - gate->v_out) / R_UP;
        i_charge = (i_pull_up > 0.0f) ? i_pull_up : 0.0f;
    }

    // 4. Update output node voltage
    float dv = (i_charge / C_LOAD) * dt_sec;
    gate->v_out += dv;

    // Clamp output voltage between saturation limit (0.2V) and Vcc minus diode drops (3.5V)
    if (gate->v_out > 3.5f) {
        gate->v_out = 3.5f;
    } else if (gate->v_out < 0.2f) {
        gate->v_out = 0.2f;
    }

    // TTL logic threshold is typically 1.4V
    gate->state = (gate->v_out > 1.4f) ? 1 : 0;

    if (out_v) {
        *out_v = gate->v_out;
    }

    return gate->state;
}
