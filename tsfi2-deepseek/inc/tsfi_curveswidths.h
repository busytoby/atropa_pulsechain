#ifndef TSFI_CURVESWIDTHS_H
#define TSFI_CURVESWIDTHS_H

#include <stdint.h>
#include <stdbool.h>

#define TSFI_MAX_WIDTHS 64

typedef struct {
    double widths[TSFI_MAX_WIDTHS];
    int count;
} TSFiCurvesWidths;

// Initialize the CurvesWidths context
void tsfi_curveswidths_init(TSFiCurvesWidths *cw);

// Adds a width value to the array
bool tsfi_curveswidths_add(TSFiCurvesWidths *cw, double width);

// Interpolates the width at a normalized parameter t along the curve using Catmull-Rom constraints
double tsfi_curveswidths_interpolate(const TSFiCurvesWidths *cw, double t);

#endif // TSFI_CURVESWIDTHS_H
