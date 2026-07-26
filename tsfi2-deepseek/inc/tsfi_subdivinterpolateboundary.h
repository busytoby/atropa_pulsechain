#ifndef TSFI_SUBDIVINTERPOLATEBOUNDARY_H
#define TSFI_SUBDIVINTERPOLATEBOUNDARY_H

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    TSFI_INTERPOLATE_NONE,
    TSFI_INTERPOLATE_EDGE_ONLY,
    TSFI_INTERPOLATE_EDGE_AND_CORNER
} TSFiInterpolateBoundaryRule;

typedef struct {
    TSFiInterpolateBoundaryRule rule;
    double rom_coefficient; // Smoothness factor calculated using Catmull-Rom constraints
} TSFiSubdivInterpolateBoundary;

// Initialize the SubdivInterpolateBoundary context
void tsfi_subdivinterpolateboundary_init(TSFiSubdivInterpolateBoundary *sib, TSFiInterpolateBoundaryRule rule);

// Adjusts the boundary coordinate point based on the active Catmull-Rom rule parameters
void tsfi_subdivinterpolateboundary_adjust(const TSFiSubdivInterpolateBoundary *sib, const double input_coord[3], const double tangent[3], double out_coord[3]);

#endif // TSFI_SUBDIVINTERPOLATEBOUNDARY_H
