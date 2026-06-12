#ifndef TSFI_NEBULA_H
#define TSFI_NEBULA_H

#include <stdint.h>

typedef struct {
    float sample_rate;
    float envelope;
    float octave_state;
    int last_sign;
    float filter_state[4];
    float cutoff_base;
    float resonance;
    float compressor_alpha;
    float fuzz_gain;
    float fuzz_mix;
    float octave_mix;

    // Control Process Extensions
    float ext_vcf_cv;     // External Cutoff CV input (-1.0 to 1.0)
    float lfo_rate;       // LFO speed in Hz
    float lfo_depth;      // LFO modulation depth
    float lfo_phase;      // LFO internal accumulator phase
    float gate_out;       // Synthesized Gate output (0.0 or 1.0)
    float trigger_out;    // Synthesized Trigger output pulse
} TsfiNebulaProcessor;

void tsfi_nebula_init(TsfiNebulaProcessor *proc, float sample_rate);
void tsfi_nebula_process(TsfiNebulaProcessor *proc, const float *input, float *output, int num_samples);

#endif /* TSFI_NEBULA_H */
