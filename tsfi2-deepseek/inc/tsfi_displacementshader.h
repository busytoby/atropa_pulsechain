#ifndef TSFI_DISPLACEMENTSHADER_H
#define TSFI_DISPLACEMENTSHADER_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    double amplitude;
    double frequency;
    double lfo_phase_offset;
} TSFiDisplacementShader;

// Initialize the DisplacementShader context
void tsfi_displacementshader_init(TSFiDisplacementShader *ds, double amplitude, double frequency);

// Set RenderMan LFO phase synchronization offset
void tsfi_displacementshader_set_lfo_phase(TSFiDisplacementShader *ds, double lfo_phase);

// Computes the geometric displacement value for a vertex based on active AUTODIN timeline time
double tsfi_displacementshader_eval(const TSFiDisplacementShader *ds, double autodin_time, double vertex_coord);

// Registers the evaluator in the XPLSM dynamic symbol table
void tsfi_displacementshader_register_xplsm(void (*register_func)(const char *, void *));

#endif // TSFI_DISPLACEMENTSHADER_H
