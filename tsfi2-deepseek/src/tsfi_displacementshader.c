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

void tsfi_displacementshader_register_xplsm(void (*register_func)(const char *, void *)) {
    if (register_func) {
        register_func("tsfi_displacementshader_eval", (void*)tsfi_displacementshader_eval);
    }
}
