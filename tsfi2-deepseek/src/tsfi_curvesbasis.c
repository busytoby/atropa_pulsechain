#include "tsfi_curvesbasis.h"

void tsfi_curvesbasis_init(TSFiCurvesBasis *cb, TSFiCurveBasisType basis, TSFiCurveWrapType wrap) {
    if (!cb) return;
    
    cb->basis = basis;
    cb->wrap = wrap;
}

void tsfi_curvesbasis_interpolate(const TSFiCurvesBasis *cb, double t, const double p0[3], const double p1[3], const double p2[3], const double p3[3], double out[3]) {
    if (!cb || !p0 || !p1 || !p2 || !p3 || !out) return;
    
    double t2 = t * t;
    double t3 = t2 * t;
    double b0 = 0.0, b1 = 0.0, b2 = 0.0, b3 = 0.0;
    
    switch (cb->basis) {
        case TSFI_BASIS_BEZIER: {
            double mt = 1.0 - t;
            double mt2 = mt * mt;
            double mt3 = mt2 * mt;
            b0 = mt3;
            b1 = 3.0 * mt2 * t;
            b2 = 3.0 * mt * t2;
            b3 = t3;
            break;
        }
        case TSFI_BASIS_BSPLINE:
            b0 = (-t3 + 3.0 * t2 - 3.0 * t + 1.0) / 6.0;
            b1 = (3.0 * t3 - 6.0 * t2 + 4.0) / 6.0;
            b2 = (-3.0 * t3 + 3.0 * t2 + 3.0 * t + 1.0) / 6.0;
            b3 = t3 / 6.0;
            break;
        case TSFI_BASIS_CATMULLROM:
            b0 = (-t3 + 2.0 * t2 - t) * 0.5;
            b1 = (3.0 * t3 - 5.0 * t2 + 2.0) * 0.5;
            b2 = (-3.0 * t3 + 4.0 * t2 + t) * 0.5;
            b3 = (t3 - t2) * 0.5;
            break;
    }
    
    for (int i = 0; i < 3; i++) {
        out[i] = b0 * p0[i] + b1 * p1[i] + b2 * p2[i] + b3 * p3[i];
    }
}
