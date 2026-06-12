#ifndef TSFI_SERGE_USG_H
#define TSFI_SERGE_USG_H

#include <stdint.h>

typedef struct {
    float sample_rate;
    float value;
    float rise_time; // in seconds
    float fall_time; // in seconds
    int loop_enable;
    int is_rising;
    float eoc_out;
    float last_trigger;
} TsfiSergeUsgProcessor;

void tsfi_serge_usg_init(TsfiSergeUsgProcessor *proc, float sample_rate);
void tsfi_serge_usg_process(TsfiSergeUsgProcessor *proc, const float *trigger_input, float *output, int num_samples);

#endif /* TSFI_SERGE_USG_H */
