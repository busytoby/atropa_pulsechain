#include "tsfi_geomcurves.h"
#include <string.h>
#include <math.h>

void tsfi_geomcurves_init(TSFiGeomCurves *curves, const char *path) {
    if (!curves) return;
    
    if (path) {
        strncpy(curves->curve_path, path, sizeof(curves->curve_path) - 1);
    } else {
        strcpy(curves->curve_path, "/auncient/curves/main");
    }
    
    curves->control_point_count = 0;
    curves->curve_width = 1.0f;
    strcpy(curves->wrap_mode, "nonperiodic");
    memset(curves->control_points, 0, sizeof(curves->control_points));
}

bool tsfi_geomcurves_add_point(TSFiGeomCurves *curves, const float point[3]) {
    if (!curves || !point || curves->control_point_count >= TSFI_MAX_CURVE_VERTS) return false;
    
    curves->control_points[curves->control_point_count][0] = point[0];
    curves->control_points[curves->control_point_count][1] = point[1];
    curves->control_points[curves->control_point_count][2] = point[2];
    curves->control_point_count++;
    return true;
}

bool tsfi_geomcurves_interpolate_b_spline(const TSFiGeomCurves *curves, float t, float out_point[3]) {
    if (!curves || !out_point || curves->control_point_count < 4) return false;
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;
    
    // Determine target segment index
    int num_segments = curves->control_point_count - 3;
    float scaled_t = t * (float)num_segments;
    int seg_idx = (int)floorf(scaled_t);
    if (seg_idx >= num_segments) seg_idx = num_segments - 1;
    
    float local_t = scaled_t - (float)seg_idx;
    float t2 = local_t * local_t;
    float t3 = t2 * local_t;
    
    // Four control points for the active segment
    const float *p0 = curves->control_points[seg_idx];
    const float *p1 = curves->control_points[seg_idx + 1];
    const float *p2 = curves->control_points[seg_idx + 2];
    const float *p3 = curves->control_points[seg_idx + 3];
    
    // Cubic B-spline matrix blends
    float b0 = (-t3 + 3.0f*t2 - 3.0f*local_t + 1.0f) / 6.0f;
    float b1 = (3.0f*t3 - 6.0f*t2 + 4.0f) / 6.0f;
    float b2 = (-3.0f*t3 + 3.0f*t2 + 3.0f*local_t + 1.0f) / 6.0f;
    float b3 = t3 / 6.0f;
    
    out_point[0] = b0*p0[0] + b1*p1[0] + b2*p2[0] + b3*p3[0];
    out_point[1] = b0*p0[1] + b1*p1[1] + b2*p2[1] + b3*p3[1];
    out_point[2] = b0*p0[2] + b1*p1[2] + b2*p2[2] + b3*p3[2];
    
    return true;
}
