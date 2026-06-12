#include "tsfi_divider.h"
#include <string.h>

void tsfi_divider_init(TsfiDividerProcessor *proc, float sample_rate) {
    memset(proc, 0, sizeof(TsfiDividerProcessor));
    proc->sample_rate = sample_rate;
    proc->v_high = 0.05f;
    proc->v_low = -0.05f;
    proc->last_gate = 0;
    
    // Default sub-harmonic mix weights (/2, /4, /8, /16, /32, /64, /128)
    proc->weights[0] = 0.40f;
    proc->weights[1] = 0.25f;
    proc->weights[2] = 0.15f;
    proc->weights[3] = 0.10f;
    proc->weights[4] = 0.05f;
    proc->weights[5] = 0.03f;
    proc->weights[6] = 0.02f;
}

void tsfi_divider_process(TsfiDividerProcessor *proc, const float *input, float *output, int num_samples) {
    for (int i = 0; i < num_samples; i++) {
        float in_val = input[i];
        
        // 1. Schmitt Trigger Comparator
        int gate = proc->last_gate;
        if (in_val > proc->v_high) {
            gate = 1;
        } else if (in_val < proc->v_low) {
            gate = 0;
        }

        // 2. Binary Ripple Counter (toggles on falling edge of previous stage)
        if (gate == 0 && proc->last_gate == 1) {
            // First stage (/2) toggles on Schmitt falling edge
            proc->states[0] = !proc->states[0];
            
            // Ripple through subsequent stages
            for (int k = 1; k < 7; k++) {
                if (proc->states[k-1] == 0) {
                    // Previous stage just fell from 1 to 0 (since it was !1)
                    proc->states[k] = !proc->states[k];
                } else {
                    // If previous stage rose to 1, ripple stops
                    break;
                }
            }
        }
        proc->last_gate = gate;

        // 3. Stage Mixer (convert binary 0/1 to bipolar -1.0/1.0 signals)
        float out_mix = 0.0f;
        for (int k = 0; k < 7; k++) {
            float stage_val = proc->states[k] ? 1.0f : -1.0f;
            out_mix += stage_val * proc->weights[k];
        }
        output[i] = out_mix;
    }
}
