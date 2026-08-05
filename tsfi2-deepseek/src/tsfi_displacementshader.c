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

bool tsfi_displacementshader_eval_page_curl(const TSFiDisplacementShader *ds, double u, double v, double curl_angle, double *displaced_y, double *displaced_z) {
    if (!ds || !displaced_y || !displaced_z) {
        return false;
    }
    // WinchesterMQ boundary constraints clamp scale factor
    double r = 0.5 + ds->amplitude * 0.1;
    double theta = curl_angle * (1.0 - u);
    if (theta > 0.0) {
        *displaced_y = v - r * sin(theta);
        *displaced_z = r * (1.0 - cos(theta));
    } else {
        *displaced_y = v;
        *displaced_z = 0.0;
    }
    return true;
}

uint32_t tsfi_displacementshader_align_ahocorasick_offset(const TSFiDisplacementShader *ds, uint32_t unaligned_offset) {
    if (!ds) return unaligned_offset;
    
    // WinchesterMQ SCSI register boundary constraints align memory access offsets to 256-byte cache boundaries
    uint32_t alignment = 256;
    uint32_t remainder = unaligned_offset % alignment;
    if (remainder == 0) return unaligned_offset;
    
    return unaligned_offset + (alignment - remainder);
}
