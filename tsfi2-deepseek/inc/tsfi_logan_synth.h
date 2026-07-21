#ifndef TSFI_LOGAN_SYNTH_H
#define TSFI_LOGAN_SYNTH_H

#include <stdint.h>
#include <stddef.h>

#define LOGAN_TABLE_SIZE 512

typedef struct {
    float log_sine_table[LOGAN_TABLE_SIZE];
    uint32_t phase_accumulator;
    uint32_t phase_increment;
    float resonance_q;
} tsfi_logan_synth_t;

/* Initialize William A. Logan's 512-sample logarithmic sine lookup table and resonator */
int tsfi_logan_synth_init(tsfi_logan_synth_t *synth, float frequency_hz, float sample_rate);

/* Step Logan wavetable synthesis generator returning non-linear sample value */
float tsfi_logan_synth_step(tsfi_logan_synth_t *synth);

/* Map Logan synthesis output directly to Vulkan Lissajous render phase parameters */
int tsfi_logan_synth_map_lissajous(const tsfi_logan_synth_t *synth, float *out_phi, float *out_fx);

#endif // TSFI_LOGAN_SYNTH_H
