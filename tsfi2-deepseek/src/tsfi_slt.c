#include "tsfi_slt.h"
#include <string.h>
#include <math.h>

#define V_CC 3.0f      // Supply voltage
#define V_EE -1.5f     // Bias supply voltage

void tsfi_slt_init(TsfiSltGate *gate, int trimmed) {
    memset(gate, 0, sizeof(TsfiSltGate));
    gate->trimmed = trimmed;

    if (trimmed) {
        // Trimmed to 1% tolerance
        gate->r_pullup = 4032.0f; // +0.8%
        gate->r_bias   = 9940.0f; // -0.6%
        gate->r_base   = 2010.0f; // +0.5%
        gate->r_load   = 993.0f;  // -0.7%
    } else {
        // Untrimmed (15% manufacturing tolerance)
        gate->r_pullup = 4560.0f; // +14.0%
        gate->r_bias   = 8800.0f; // -12.0%
        gate->r_base   = 2260.0f; // +13.0%
        gate->r_load   = 890.0f;  // -11.0%
    }

    gate->temp_ambient = 25.0f;
    gate->temp_substrate = 25.0f;
    gate->heat_capacity = 1e-6f;   // Scaled down to observe thermal changes in microseconds
    gate->thermal_cond = 0.05f;    // Thermal coupling to ambient
    
    gate->v_out = V_CC;
    gate->state = 1;
    gate->t = 0.0f;
}

int tsfi_slt_step(TsfiSltGate *gate, float v_in1, float v_in2, float dt_ns) {
    float dt_sec = dt_ns * 1e-9f;

    // 1. Calculate temperature-dependent diode and transistor parameters
    // V_be turn-on drops by 2mV/C
    float v_be_on = 0.7f - 0.002f * (gate->temp_substrate - 25.0f);
    if (v_be_on < 0.3f) v_be_on = 0.3f; // Lower limit

    float beta = 30.0f - 0.1f * (gate->temp_substrate - 25.0f); // Beta decreases with high temp in this regime
    if (beta < 5.0f) beta = 5.0f;

    // 2. Diode charge carrier and reverse recovery dynamics
    // Input diodes check: lowest input voltage dominates
    float v_in_min = (v_in1 < v_in2) ? v_in1 : v_in2;
    float v_diode_drop = 0.7f;
    
    float v_p = v_in_min + v_diode_drop;
    if (v_p > V_CC) v_p = V_CC;

    // Transistor base-emitter voltage depending on node P
    // V_b is set by the divider between P and V_EE (-1.5V)
    float v_b_unclamped = (v_p * gate->r_bias + V_EE * gate->r_base) / (gate->r_base + gate->r_bias);
    
    float i_pullup = 0.0f;
    float i_bias = 0.0f;
    float i_base = 0.0f;
    float i_load = 0.0f;

    int trans_on = 0;
    if (v_b_unclamped >= v_be_on) {
        trans_on = 1;
        float v_b = v_be_on;
        i_pullup = (V_CC - v_b) / (gate->r_pullup + gate->r_base);
        i_bias = (v_b - V_EE) / gate->r_bias;
        i_base = i_pullup - i_bias;
        if (i_base < 0.0f) i_base = 0.0f;
    } else {
        float v_b = v_b_unclamped;
        i_pullup = (V_CC - v_p) / gate->r_pullup;
        i_bias = (v_b - V_EE) / gate->r_bias;
        i_base = 0.0f;
    }

    // Dynamic transistor charge and output voltage transition
    // Load current
    float v_out_target = V_CC;
    if (trans_on) {
        float i_collector = i_base * beta;
        i_load = (V_CC - 0.2f) / gate->r_load;
        
        if (i_collector >= i_load) {
            v_out_target = 0.2f; // Saturated
            i_collector = i_load;
        } else {
            v_out_target = V_CC - i_collector * gate->r_load;
            if (v_out_target < 0.2f) v_out_target = 0.2f;
        }
    }

    // Propagation delay modeling using a simple low-pass step
    float r_out_effective = trans_on ? 50.0f : gate->r_load;
    float c_out_parasitic = 15e-12f; // 15 pF
    float tau_out = r_out_effective * c_out_parasitic;
    gate->v_out += (v_out_target - gate->v_out) * (dt_sec / (tau_out + dt_sec));

    // Resolve binary state
    gate->state = (gate->v_out > 1.5f) ? 1 : 0;

    // 3. Thermal Dissipation
    // Total power dissipated by the module
    float p_diss = (V_CC - gate->v_out) * (trans_on ? i_load : 0.0f) + i_pullup * V_CC + i_bias * fabsf(V_EE);
    float dT = (p_diss - gate->thermal_cond * (gate->temp_substrate - gate->temp_ambient)) / gate->heat_capacity * dt_sec;
    gate->temp_substrate += dT;
    
    gate->t += dt_sec;

    return gate->state;
}
