#include "tsfi_subdivinterpolateboundary.h"

void tsfi_subdivinterpolateboundary_init(TSFiSubdivInterpolateBoundary *sib, TSFiInterpolateBoundaryRule rule) {
    if (!sib) return;
    
    sib->rule = rule;
    
    // Set coefficients using standard Catmull-Rom constraints
    switch (rule) {
        case TSFI_INTERPOLATE_NONE:
            sib->rom_coefficient = 0.0;
            break;
        case TSFI_INTERPOLATE_EDGE_ONLY:
            sib->rom_coefficient = 0.5; // Standard Catmull-Rom edge midpoint step
            break;
        case TSFI_INTERPOLATE_EDGE_AND_CORNER:
            sib->rom_coefficient = 0.75;
            break;
    }
}

void tsfi_subdivinterpolateboundary_adjust(const TSFiSubdivInterpolateBoundary *sib, const double input_coord[3], const double tangent[3], double out_coord[3]) {
    if (!sib || !input_coord || !tangent || !out_coord) return;
    
    double factor = sib->rom_coefficient;
    
    for (int i = 0; i < 3; i++) {
        // Adjust coordinates using deterministic Catmull-Rom tangent vectors
        out_coord[i] = input_coord[i] + factor * tangent[i];
    }
}
