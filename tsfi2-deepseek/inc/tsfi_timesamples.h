#ifndef TSFI_TIMESAMPLES_H
#define TSFI_TIMESAMPLES_H

#include <stdint.h>
#include <stdbool.h>

#define TSFI_MAX_SAMPLES 128

typedef struct {
    double times[TSFI_MAX_SAMPLES];
    uint32_t register_offsets[TSFI_MAX_SAMPLES];
    int count;
} TSFiTimeSamples;

// Initialize the TimeSamples context
void tsfi_timesamples_init(TSFiTimeSamples *ts);

// Adds a pre-computed sample time and its mapped register offset
bool tsfi_timesamples_add(TSFiTimeSamples *ts, double time_val, uint32_t reg_offset);

// Performs O(1) direct slot lookup of the register offset for a given sample slot index
uint32_t tsfi_timesamples_lookup_slot(const TSFiTimeSamples *ts, int slot_index);

#endif // TSFI_TIMESAMPLES_H
