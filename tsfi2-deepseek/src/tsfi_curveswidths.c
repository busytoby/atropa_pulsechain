#include "tsfi_curveswidths.h"

void tsfi_curveswidths_init(TSFiCurvesWidths *cw) {
    if (!cw) return;
    cw->count = 0;
    for (int i = 0; i < TSFI_MAX_WIDTHS; i++) {
        cw->widths[i] = 0.0;
    }
}

bool tsfi_curveswidths_add(TSFiCurvesWidths *cw, double width) {
    if (!cw || cw->count >= TSFI_MAX_WIDTHS) return false;
    if (width < 0.0) width = 0.0;
    cw->widths[cw->count++] = width;
    return true;
}

double tsfi_curveswidths_interpolate(const TSFiCurvesWidths *cw, double t) {
    if (!cw || cw->count == 0) return 0.0;
    if (cw->count == 1) return cw->widths[0];
    
    if (t <= 0.0) return cw->widths[0];
    if (t >= 1.0) return cw->widths[cw->count - 1];
    
    double scaled_t = t * (cw->count - 1);
    int idx = (int)scaled_t;
    double local_t = scaled_t - idx;
    
    if (idx >= cw->count - 1) {
        return cw->widths[cw->count - 1];
    }
    
    // Perform linear interpolation of widths representing Catmull-Rom spline spans
    return (1.0 - local_t) * cw->widths[idx] + local_t * cw->widths[idx + 1];
}
