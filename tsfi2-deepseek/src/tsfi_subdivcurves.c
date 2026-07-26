#include "tsfi_subdivcurves.h"

void tsfi_subdivcurves_init(TSFiSubdivCurves *sc, int level, double tension) {
    if (!sc) return;
    
    sc->subdivision_level = (level > 0) ? level : 1;
    sc->tension_parameter = (tension >= 0.0 && tension <= 1.0) ? tension : 0.5;
    sc->output_point_count = 1 << sc->subdivision_level;
}

void tsfi_subdivcurves_subdivide(const TSFiSubdivCurves *sc, const double p0[3], const double p1[3], const double p2[3], const double p3[3], double *out_pts, int max_pts) {
    if (!sc || !p0 || !p1 || !p2 || !p3 || !out_pts) return;
    
    int points_needed = sc->output_point_count;
    if (points_needed > max_pts) points_needed = max_pts;
    
    // Perform subdivision steps
    for (int idx = 0; idx < points_needed; idx++) {
        double t = (double)idx / (double)(points_needed - 1);
        
        // Midpoint coordinates weighted by tension parameter
        double w0 = (1.0 - t) * (1.0 - t) * (1.0 - t);
        double w1 = 3.0 * (1.0 - t) * (1.0 - t) * t;
        double w2 = 3.0 * (1.0 - t) * t * t;
        double w3 = t * t * t;
        
        // Output coordinate vector calculations
        out_pts[idx * 3 + 0] = w0 * p0[0] + w1 * p1[0] + w2 * p2[0] + w3 * p3[0];
        out_pts[idx * 3 + 1] = w0 * p0[1] + w1 * p1[1] + w2 * p2[1] + w3 * p3[1];
        out_pts[idx * 3 + 2] = w0 * p0[2] + w1 * p1[2] + w2 * p2[2] + w3 * p3[2];
        
        // Apply subdivision tension displacement
        for (int c = 0; c < 3; c++) {
            double midpoint = (p1[c] + p2[c]) * 0.5;
            out_pts[idx * 3 + c] = (1.0 - sc->tension_parameter) * out_pts[idx * 3 + c] + sc->tension_parameter * midpoint;
        }
    }
}
