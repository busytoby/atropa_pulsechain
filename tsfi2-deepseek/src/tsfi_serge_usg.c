#include "tsfi_serge_usg.h"
#include <string.h>

void tsfi_serge_usg_init(TsfiSergeUsgProcessor *proc, float sample_rate) {
    memset(proc, 0, sizeof(TsfiSergeUsgProcessor));
    proc->sample_rate = sample_rate;
    proc->value = 0.0f;
    proc->rise_time = 0.05f; // 50 ms
    proc->fall_time = 0.10f; // 100 ms
    proc->loop_enable = 0;
    proc->is_rising = 0;
    proc->eoc_out = 1.0f;
    proc->last_trigger = 0.0f;
}

void tsfi_serge_usg_process(TsfiSergeUsgProcessor *proc, const float *trigger_input, float *output, int num_samples) {
    for (int i = 0; i < num_samples; i++) {
        float trig = (trigger_input != NULL) ? trigger_input[i] : 0.0f;
        int trig_fired = (trig > 0.5f && proc->last_trigger <= 0.5f);
        proc->last_trigger = trig;

        // Loop triggering: if output hits bottom and loop mode is enabled, trigger rise
        if (proc->loop_enable && !proc->is_rising && proc->value <= 0.001f) {
            proc->is_rising = 1;
        }

        // Trigger input fires a rising edge
        if (trig_fired && !proc->is_rising) {
            proc->is_rising = 1;
        }

        if (proc->is_rising) {
            float step = 1.0f / (proc->rise_time * proc->sample_rate);
            proc->value += step;
            if (proc->value >= 1.0f) {
                proc->value = 1.0f;
                proc->is_rising = 0; // Toggles to falling stage
            }
        } else {
            if (proc->value > 0.0f) {
                float step = 1.0f / (proc->fall_time * proc->sample_rate);
                proc->value -= step;
                if (proc->value < 0.0f) {
                    proc->value = 0.0f;
                }
            }
        }

        // End of Cycle output is low during rise stage, high during fall/idle stages
        proc->eoc_out = proc->is_rising ? 0.0f : 1.0f;
        output[i] = proc->value;
    }
}
