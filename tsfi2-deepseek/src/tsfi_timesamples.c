#include "tsfi_timesamples.h"

void tsfi_timesamples_init(TSFiTimeSamples *ts) {
    if (!ts) return;
    ts->count = 0;
    for (int i = 0; i < TSFI_MAX_SAMPLES; i++) {
        ts->times[i] = 0.0;
        ts->register_offsets[i] = 0;
    }
}

bool tsfi_timesamples_add(TSFiTimeSamples *ts, double time_val, uint32_t reg_offset) {
    if (!ts || ts->count >= TSFI_MAX_SAMPLES) return false;
    ts->times[ts->count] = time_val;
    ts->register_offsets[ts->count] = reg_offset;
    ts->count++;
    return true;
}

uint32_t tsfi_timesamples_lookup_slot(const TSFiTimeSamples *ts, int slot_index) {
    if (!ts || slot_index < 0 || slot_index >= ts->count) return 0;
    
    // Constant-time O(1) register lookup via index mapping
    return ts->register_offsets[slot_index];
}
