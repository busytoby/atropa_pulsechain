#include "tsfi_nebula.h"
#include <math.h>
#include <string.h>

void tsfi_nebula_init(TsfiNebulaProcessor *proc, float sample_rate) {
    memset(proc, 0, sizeof(TsfiNebulaProcessor));
    proc->sample_rate = sample_rate;
    proc->envelope = 0.0f;
    proc->octave_state = 1.0f;
    proc->last_sign = 1;
    proc->cutoff_base = 500.0f;
    proc->resonance = 0.7f;
    proc->compressor_alpha = 5.0f;
    proc->fuzz_gain = 8.0f;
    proc->fuzz_mix = 0.6f;
    proc->octave_mix = 0.5f;

    // Control parameters default initialization
    proc->ext_vcf_cv = 0.0f;
    proc->lfo_rate = 6.0f;   // 6 Hz default auto-wah speed
    proc->lfo_depth = 0.25f; // LFO modulation depth
    proc->lfo_phase = 0.0f;
    proc->gate_out = 0.0f;
    proc->trigger_out = 0.0f;
}

void tsfi_nebula_process(TsfiNebulaProcessor *proc, const float *input, float *output, int num_samples) {
    for (int i = 0; i < num_samples; i++) {
        float in_val = input[i];
        float abs_in = fabsf(in_val);
        
        // Envelope follower with fast attack, slower release
        float coeff = (abs_in > proc->envelope) ? 0.05f : 0.005f;
        proc->envelope += coeff * (abs_in - proc->envelope);

        // Compandor/Compressor stage
        float compressed = in_val / (1.0f + proc->compressor_alpha * proc->envelope);

        // Zero-crossing octave divider (flip-flop state toggled every cycle)
        int current_sign = (compressed >= 0.0f) ? 1 : -1;
        if (current_sign != proc->last_sign && proc->last_sign == -1) {
            // Toggle sub-octave flip-flop on positive-going zero crossing
            proc->octave_state = -proc->octave_state;
        }
        proc->last_sign = current_sign;

        // Combine compressed signal and divided octave square-wave
        float sub_octave = proc->octave_state * fabsf(compressed);
        float divided = (1.0f - proc->octave_mix) * compressed + proc->octave_mix * sub_octave;

        // Fuzz/Clipping stage
        float fuzz_in = divided * proc->fuzz_gain;
        float fuzz_out = tanhf(fuzz_in);
        float pre_filter = (1.0f - proc->fuzz_mix) * divided + proc->fuzz_mix * fuzz_out;

        // Control process LFO phase accumulation
        proc->lfo_phase += 2.0f * 3.14159265f * proc->lfo_rate / proc->sample_rate;
        if (proc->lfo_phase > 2.0f * 3.14159265f) {
            proc->lfo_phase -= 2.0f * 3.14159265f;
        }
        float lfo_val = sinf(proc->lfo_phase);

        // Control process Gate and Trigger generation
        float gate_thresh = 0.03f;
        float new_gate = (proc->envelope > gate_thresh) ? 1.0f : 0.0f;
        proc->trigger_out = (new_gate > 0.5f && proc->gate_out < 0.5f) ? 1.0f : 0.0f;
        proc->gate_out = new_gate;

        // 4-pole SSM-style low-pass VCF modulated by envelope, internal LFO, and external CV
        float cutoff_modulation = 3000.0f * proc->envelope 
                                  + 1500.0f * (lfo_val * proc->lfo_depth) 
                                  + 2000.0f * proc->ext_vcf_cv;
        float cutoff = proc->cutoff_base + cutoff_modulation;
        float max_cutoff = proc->sample_rate * 0.45f;
        if (cutoff > max_cutoff) cutoff = max_cutoff;
        if (cutoff < 20.0f) cutoff = 20.0f;

        // Bilinear transform style coefficient approximation
        float w = 2.0f * 3.14159265f * cutoff / proc->sample_rate;
        // Clamp w to avoid instability
        if (w > 0.99f) w = 0.99f;

        // Feedback loop with negative feedback resonance
        float feedback = proc->resonance * 4.0f * proc->filter_state[3];
        float v_in = pre_filter - feedback;

        // 4 cascaded one-pole filters
        proc->filter_state[0] += w * (v_in - proc->filter_state[0]);
        proc->filter_state[1] += w * (proc->filter_state[0] - proc->filter_state[1]);
        proc->filter_state[2] += w * (proc->filter_state[1] - proc->filter_state[2]);
        proc->filter_state[3] += w * (proc->filter_state[2] - proc->filter_state[3]);

        // VCA scaled by envelope
        float vca_out = proc->filter_state[3] * (0.2f + 0.8f * proc->envelope);
        output[i] = vca_out;
    }
}
