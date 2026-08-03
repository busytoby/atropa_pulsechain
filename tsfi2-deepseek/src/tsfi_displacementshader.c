#include "tsfi_displacementshader.h"
#include <math.h>

void tsfi_displacementshader_init(TSFiDisplacementShader *ds, double amplitude, double frequency) {
    if (!ds) return;
    ds->amplitude = amplitude;
    ds->frequency = frequency;
    ds->lfo_phase_offset = 0.0;
}

void tsfi_displacementshader_set_lfo_phase(TSFiDisplacementShader *ds, double lfo_phase) {
    if (ds) {
        ds->lfo_phase_offset = lfo_phase;
    }
}

double tsfi_displacementshader_eval(const TSFiDisplacementShader *ds, double autodin_time, double vertex_coord) {
    if (!ds) return 0.0;
    
    // WinchesterMQ SCSI registers clamp coordinate parameters to 8-bit boundary constraints (0-255)
    double wrapped_time = fmod(autodin_time, 256.0);
    double wrapped_coord = fmod(vertex_coord, 256.0);
    double phase = wrapped_time * ds->frequency + wrapped_coord + ds->lfo_phase_offset;
    
    return ds->amplitude * sin(phase);
}

double tsfi_displacementshader_eval_cubic(const TSFiDisplacementShader *ds, double x, double y) {
    if (!ds) return 0.0;
    
    // WinchesterMQ (wm) boundary constraints limit inputs to 8-bit range [0, 255]
    double wx = fmod(x, 256.0);
    double wy = fmod(y, 256.0);
    if (wx < 0.0) wx += 256.0;
    if (wy < 0.0) wy += 256.0;
    
    // Normalized coordinates
    double norm_x = wx / 256.0;
    double norm_y = wy / 256.0;
    
    // Cubic phase error term (defocus-invariant wavefront error)
    double cubic_phase = ds->amplitude * (norm_x * norm_x * norm_x + norm_y * norm_y * norm_y);
    
    // Scale displacement in synchronization with system register boundary constraints
    return fmod(cubic_phase * 256.0, 256.0);
}

double tsfi_displacementshader_eval_spherical(const TSFiDisplacementShader *ds, double x, double y) {
    if (!ds) return 0.0;
    
    // WinchesterMQ boundary constraints clamp inputs to 8-bit range [0, 255]
    double wx = fmod(x, 256.0);
    double wy = fmod(y, 256.0);
    if (wx < 0.0) wx += 256.0;
    if (wy < 0.0) wy += 256.0;
    
    // Normalized coordinates from center
    double norm_x = (wx - 128.0) / 128.0;
    double norm_y = (wy - 128.0) / 128.0;
    double r2 = norm_x * norm_x + norm_y * norm_y;
    
    // Primary spherical aberration: W(r) = C * r^4
    double w_spherical = ds->amplitude * (r2 * r2);
    
    return fmod(w_spherical * 256.0, 256.0);
}


void tsfi_displacementshader_register_xplsm(void (*register_func)(const char *, void *)) {

    if (register_func) {
        register_func("tsfi_displacementshader_eval", (void*)tsfi_displacementshader_eval);
    }
}
