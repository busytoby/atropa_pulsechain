#ifndef TSFI_STAGETIMECODES_H
#define TSFI_STAGETIMECODES_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    double start_timecode;
    double end_timecode;
    double timecodes_per_second;
    double frames_per_second;
} TSFiStageTimeCodes;

// Initialize the StageTimeCodes context
void tsfi_stagetimecodes_init(TSFiStageTimeCodes *tc, double start, double end, double rate, double fps);

// Converts a real-time duration in seconds to a stage timecode value
double tsfi_stagetimecodes_to_timecode(const TSFiStageTimeCodes *tc, double seconds);

// Checks if a given timecode value falls within the animation bounds range
bool tsfi_stagetimecodes_is_in_bounds(const TSFiStageTimeCodes *tc, double timecode);

#endif // TSFI_STAGETIMECODES_H
