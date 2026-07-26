#ifndef TSFI_CURVESBASIS_H
#define TSFI_CURVESBASIS_H

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    TSFI_BASIS_BEZIER,
    TSFI_BASIS_BSPLINE,
    TSFI_BASIS_CATMULLROM
} TSFiCurveBasisType;

typedef enum {
    TSFI_WRAP_NONPERIODIC,
    TSFI_WRAP_PERIODIC
} TSFiCurveWrapType;

typedef struct {
    TSFiCurveBasisType basis;
    TSFiCurveWrapType wrap;
} TSFiCurvesBasis;

// Initialize the CurvesBasis context
void tsfi_curvesbasis_init(TSFiCurvesBasis *cb, TSFiCurveBasisType basis, TSFiCurveWrapType wrap);

// Interpolates a point at parameter t (0.0 to 1.0) using the selected basis and 4 control points
void tsfi_curvesbasis_interpolate(const TSFiCurvesBasis *cb, double t, const double p0[3], const double p1[3], const double p2[3], const double p3[3], double out[3]);

#endif // TSFI_CURVESBASIS_H
