#ifndef TSFI_CURVESWRAP_H
#define TSFI_CURVESWRAP_H

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    TSFI_WRAP_NONPERIODIC,
    TSFI_WRAP_PERIODIC
} TSFiCurvesWrapType;

typedef struct {
    TSFiCurvesWrapType wrap_type;
} TSFiCurvesWrap;

// Initialize the CurvesWrap context
void tsfi_curveswrap_init(TSFiCurvesWrap *cw, TSFiCurvesWrapType wrap_type);

// Resolves a control point index based on the wrapping rules and control points count
int tsfi_curveswrap_resolve_index(const TSFiCurvesWrap *cw, int index, int count);

#endif // TSFI_CURVESWRAP_H
