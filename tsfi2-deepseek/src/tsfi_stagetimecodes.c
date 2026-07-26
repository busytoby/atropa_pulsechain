#include "tsfi_stagetimecodes.h"

void tsfi_stagetimecodes_init(TSFiStageTimeCodes *tc, double start, double end, double rate, double fps) {
    if (!tc) return;
    
    tc->start_timecode = start;
    tc->end_timecode = end;
    tc->timecodes_per_second = (rate > 0.0) ? rate : 24.0;
    tc->frames_per_second = (fps > 0.0) ? fps : 24.0;
}

double tsfi_stagetimecodes_to_timecode(const TSFiStageTimeCodes *tc, double seconds) {
    if (!tc) return 0.0;
    
    // Scale duration in seconds using timecodes_per_second conversion factor
    double val = tc->start_timecode + seconds * tc->timecodes_per_second;
    
    // Clamp to timeline boundaries
    if (val < tc->start_timecode) return tc->start_timecode;
    if (val > tc->end_timecode) return tc->end_timecode;
    
    return val;
}

bool tsfi_stagetimecodes_is_in_bounds(const TSFiStageTimeCodes *tc, double timecode) {
    if (!tc) return false;
    
    return (timecode >= tc->start_timecode && timecode <= tc->end_timecode);
}
