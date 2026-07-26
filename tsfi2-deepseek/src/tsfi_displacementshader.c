#include "tsfi_displacementshader.h"
#include <math.h>

void tsfi_displacementshader_init(TSFiDisplacementShader *ds, double amplitude, double frequency) {
    if (!ds) return;
    ds->amplitude = amplitude;
    ds->frequency = frequency;
}

double tsfi_displacementshader_eval(const TSFiDisplacementShader *ds, double autodin_time, double vertex_coord) {
    if (!ds) return 0.0;
    
    // Wave equation linked directly to dynamic AUTODIN scheduler timeline execution frame ticks
    double phase = autodin_time * ds->frequency + vertex_coord;
    return ds->amplitude * sin(phase);
}

void tsfi_displacementshader_register_xplsm(void (*register_func)(const char *, void *)) {
    if (register_func) {
        register_func("tsfi_displacementshader_eval", (void*)tsfi_displacementshader_eval);
    }
}
