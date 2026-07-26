#include "tsfi_timesampleinterpolation.h"

void tsfi_timesampleinterpolation_init(TSFiTimeSampleInterpolation *tsi, TSFiTimeInterpolateMode mode) {
    if (!tsi) return;
    tsi->mode = mode;
}

double tsfi_timesampleinterpolation_eval(const TSFiTimeSampleInterpolation *tsi, double t, double t0, double val0, double t1, double val1) {
    if (!tsi) return 0.0;
    
    if (t <= t0) return val0;
    if (t >= t1) return val1;
    if (t0 >= t1) return val0;
    
    if (tsi->mode == TSFI_INTERPOLATE_HELD) {
        return val0; // Held mode returns first sample value until t1 is reached
    }
    
    // Linear mode interpolation calculation
    double fraction = (t - t0) / (t1 - t0);
    return val0 + fraction * (val1 - val0);
}
