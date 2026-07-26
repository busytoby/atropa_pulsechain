#ifndef TSFI_GEOMCURVES_H
#define TSFI_GEOMCURVES_H

#include <stdint.h>
#include <stdbool.h>

#define TSFI_MAX_CURVE_VERTS 16

typedef struct {
    char curve_path[64];
    float control_points[TSFI_MAX_CURVE_VERTS][3];
    int control_point_count;
    float curve_width;
    char wrap_mode[16]; // "nonperiodic" or "periodic"
} TSFiGeomCurves;

// Initialize the GeomCurves context
void tsfi_geomcurves_init(TSFiGeomCurves *curves, const char *path);

// Appends control vertices to the spline buffer
bool tsfi_geomcurves_add_point(TSFiGeomCurves *curves, const float point[3]);

// Interpolates a point along the cubic B-spline curve based on coordinate t (0.0 to 1.0)
bool tsfi_geomcurves_interpolate_b_spline(const TSFiGeomCurves *curves, float t, float out_point[3]);

#endif // TSFI_GEOMCURVES_H
