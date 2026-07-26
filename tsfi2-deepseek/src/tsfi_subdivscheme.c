#include "tsfi_subdivscheme.h"

void tsfi_subdivscheme_init(TSFiSubdivScheme *ss, TSFiSubdivSchemeType scheme, TSFiBoundaryInterpolateType boundary_rule) {
    if (!ss) return;
    
    ss->scheme = scheme;
    ss->boundary_rule = boundary_rule;
    
    switch (scheme) {
        case TSFI_SCHEME_CATMULLROM:
            ss->smoothness_factor = 0.625; // Catmull-Rom refinement centroid weight
            break;
        case TSFI_SCHEME_LOOP:
            ss->smoothness_factor = 0.375; // Loop standard weight
            break;
        case TSFI_SCHEME_BILINEAR:
            ss->smoothness_factor = 0.5;
            break;
        case TSFI_SCHEME_NONE:
        default:
            ss->smoothness_factor = 0.0;
            break;
    }
}

void tsfi_subdivscheme_refine_vertex(const TSFiSubdivScheme *ss, const double current_pos[3], const double neighbors_avg[3], double out_pos[3]) {
    if (!ss || !current_pos || !neighbors_avg || !out_pos) return;
    
    double alpha = ss->smoothness_factor;
    
    // Boundary rule overrides
    if (ss->boundary_rule == TSFI_BOUNDARY_INTERPOLATE_NONE) {
        alpha = 0.0; // Do not smooth boundary vertices
    }
    
    for (int i = 0; i < 3; i++) {
        out_pos[i] = (1.0 - alpha) * current_pos[i] + alpha * neighbors_avg[i];
    }
}
