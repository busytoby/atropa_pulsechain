#ifndef TSFI_TIMESAMPLEINTERPOLATION_H
#define TSFI_TIMESAMPLEINTERPOLATION_H

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    TSFI_INTERPOLATE_LINEAR,
    TSFI_INTERPOLATE_HELD
} TSFiTimeInterpolateMode;

typedef struct {
    TSFiTimeInterpolateMode mode;
} TSFiTimeSampleInterpolation;

// Initialize the TimeSampleInterpolation context
void tsfi_timesampleinterpolation_init(TSFiTimeSampleInterpolation *tsi, TSFiTimeInterpolateMode mode);

// Interpolates a value between two time samples based on current time
double tsfi_timesampleinterpolation_eval(const TSFiTimeSampleInterpolation *tsi, double t, double t0, double val0, double t1, double val1);

#endif // TSFI_TIMESAMPLEINTERPOLATION_H
