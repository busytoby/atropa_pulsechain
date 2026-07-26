#include "tsfi_curvestension.h"

void tsfi_curvestension_init(TSFiCurvesTension *ct, double tension) {
    if (!ct) return;
    
    // Clamp tension between 0.0 and 1.0
    if (tension < 0.0) tension = 0.0;
    if (tension > 1.0) tension = 1.0;
    
    ct->tension = tension;
    // Derived scale factor using standard Catmull-Rom tangent formula: (1.0 - tension) * 0.5
    ct->scale_factor = (1.0 - tension) * 0.5;
}

void tsfi_curvestension_calculate_tangent(const TSFiCurvesTension *ct, const double p0[3], const double p2[3], double out_tangent[3]) {
    if (!ct || !p0 || !p2 || !out_tangent) return;
    
    double factor = ct->scale_factor;
    
    for (int i = 0; i < 3; i++) {
        // Tangent vector = factor * (p2 - p0)
        out_tangent[i] = factor * (p2[i] - p0[i]);
    }
}
