#include "tsfi_schmitt.h"
#include <string.h>

void tsfi_schmitt_init(TsfiSchmittTrigger *trigger) {
    memset(trigger, 0, sizeof(TsfiSchmittTrigger));
    
    // Component values
    trigger->r_c1 = 3000.0f; // 3k
    trigger->r_c2 = 1000.0f; // 1k
    trigger->r_e  = 1000.0f; // 1k
    trigger->r_1  = 5000.0f; // 5k
    trigger->r_2  = 5000.0f; // 5k

    trigger->v_cc = 5.0f;
    trigger->v_be_on = 0.7f;

    // Initial state: input low, Q1 off, Q2 on
    trigger->q1_state = 0;
    trigger->q2_state = 1;
    
    // Calculate initial Ve
    float v_b2_initial = trigger->v_cc * trigger->r_2 / (trigger->r_c1 + trigger->r_1 + trigger->r_2);
    trigger->v_e = v_b2_initial - trigger->v_be_on;
    trigger->v_out = trigger->v_e + 0.2f; // Q2 collector is low (saturated)
}

int tsfi_schmitt_step(TsfiSchmittTrigger *trigger, float v_in, float dt_ns) {
    // 1. Calculate thresholds based on current state
    float v_utp = 0.0f;
    float v_ltp = 0.0f;

    // UTP threshold occurs when Q1 is OFF, Q2 is ON
    // Ve = V_b2 - V_be_on
    float v_b2_q2_on = trigger->v_cc * trigger->r_2 / (trigger->r_c1 + trigger->r_1 + trigger->r_2);
    v_utp = v_b2_q2_on; // Input must exceed V_b2 to turn Q1 on

    // LTP threshold occurs when Q1 is ON, Q2 is ON (at the edge of switching)
    // At the transition: Q2 is just turning OFF, so V_b2 = V_e + V_be_on
    // And V_e = V_in - V_be_on.
    // Collector of Q1: V_c1 = V_cc - I_c1 * R_c1 = V_cc - (V_e / R_e) * R_c1 (approx)
    // V_b2 = V_c1 * R_2 / (R_1 + R_2)
    // Setting V_b2 = V_e + V_be_on and solving for V_in yields V_ltp:
    // Let's use a simplified BJT switching limit model
    float r12 = trigger->r_1 + trigger->r_2;
    float factor = trigger->r_2 / r12;
    float num = trigger->v_cc * factor - trigger->v_be_on;
    float den = 1.0f + (trigger->r_c1 / trigger->r_e) * factor;
    v_ltp = (num / den) + trigger->v_be_on;

    // 2. Regenerative BJT switching logic
    if (trigger->q1_state == 0) {
        // We are currently in Q1 OFF, Q2 ON state
        if (v_in >= v_utp) {
            trigger->q1_state = 1;
            trigger->q2_state = 0;
        }
    } else {
        // We are currently in Q1 ON, Q2 OFF state
        if (v_in <= v_ltp) {
            trigger->q1_state = 0;
            trigger->q2_state = 1;
        }
    }

    // 3. Resolve voltages based on settled state
    if (trigger->q1_state == 0 && trigger->q2_state == 1) {
        // Q1 off, Q2 on
        trigger->v_e = v_b2_q2_on - trigger->v_be_on;
        trigger->v_out = trigger->v_e + 0.2f; // Low output
    } else {
        // Q1 on, Q2 off
        trigger->v_e = v_in - trigger->v_be_on;
        if (trigger->v_e < 0.0f) trigger->v_e = 0.0f;
        trigger->v_out = trigger->v_cc;       // High output (pulled up to Vcc)
    }

    // Add dynamics low pass on output to model propagation delay
    float dt_sec = dt_ns * 1e-9f;
    trigger->t += dt_sec;

    // Return output state
    return (trigger->v_out > 2.5f) ? 1 : 0;
}
