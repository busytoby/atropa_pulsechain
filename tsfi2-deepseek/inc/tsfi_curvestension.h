#ifndef TSFI_CURVESTENSION_H
#define TSFI_CURVESTENSION_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    double tension; // Tension parameter ranging from 0.0 to 1.0
    double scale_factor; // Derived scale factor for Catmull-Rom tangents
} TSFiCurvesTension;

// Initialize the CurvesTension context
void tsfi_curvestension_init(TSFiCurvesTension *ct, double tension);

// Calculates the tangent vector for a segment using Catmull-Rom constraints and tension
void tsfi_curvestension_calculate_tangent(const TSFiCurvesTension *ct, const double p0[3], const double p2[3], double out_tangent[3]);

#endif // TSFI_CURVESTENSION_H
