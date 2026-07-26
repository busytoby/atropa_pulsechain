#ifndef TSFI_SUBDIVCURVES_H
#define TSFI_SUBDIVCURVES_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    int subdivision_level;      // Refinement iterations (e.g. 1 to 4)
    double tension_parameter;   // Subdivision tension factor (0.0 to 1.0)
    int output_point_count;     // Generated subdivided points count
} TSFiSubdivCurves;

// Initialize the SubdivCurves context
void tsfi_subdivcurves_init(TSFiSubdivCurves *sc, int level, double tension);

// Subdivides 4 control points into subdivided output coordinates based on level
void tsfi_subdivcurves_subdivide(const TSFiSubdivCurves *sc, const double p0[3], const double p1[3], const double p2[3], const double p3[3], double *out_pts, int max_pts);

#endif // TSFI_SUBDIVCURVES_H
