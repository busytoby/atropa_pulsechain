#ifndef TSFI_DIVIDER_H
#define TSFI_DIVIDER_H

#include <stdint.h>

typedef struct {
    float sample_rate;
    float v_high;
    float v_low;
    int last_gate;
    int states[7];
    float weights[7];
} TsfiDividerProcessor;

void tsfi_divider_init(TsfiDividerProcessor *proc, float sample_rate);
void tsfi_divider_process(TsfiDividerProcessor *proc, const float *input, float *output, int num_samples);

#endif /* TSFI_DIVIDER_H */
