#ifndef TSFI_SUBDIVSCHEME_H
#define TSFI_SUBDIVSCHEME_H

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    TSFI_SCHEME_CATMULLROM,
    TSFI_SCHEME_LOOP,
    TSFI_SCHEME_BILINEAR,
    TSFI_SCHEME_NONE
} TSFiSubdivSchemeType;

typedef enum {
    TSFI_BOUNDARY_INTERPOLATE_NONE,
    TSFI_BOUNDARY_INTERPOLATE_EDGES,
    TSFI_BOUNDARY_INTERPOLATE_EDGES_AND_CORNERS
} TSFiBoundaryInterpolateType;

typedef struct {
    TSFiSubdivSchemeType scheme;
    TSFiBoundaryInterpolateType boundary_rule;
    double smoothness_factor;
} TSFiSubdivScheme;

// Initialize the SubdivScheme context
void tsfi_subdivscheme_init(TSFiSubdivScheme *ss, TSFiSubdivSchemeType scheme, TSFiBoundaryInterpolateType boundary_rule);

// Refines a vertex coordinate based on scheme type and neighbor average coordinates
void tsfi_subdivscheme_refine_vertex(const TSFiSubdivScheme *ss, const double current_pos[3], const double neighbors_avg[3], double out_pos[3]);

#endif // TSFI_SUBDIVSCHEME_H
