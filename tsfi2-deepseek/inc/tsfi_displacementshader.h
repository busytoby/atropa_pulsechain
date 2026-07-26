#ifndef TSFI_DISPLACEMENTSHADER_H
#define TSFI_DISPLACEMENTSHADER_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    double amplitude;
    double frequency;
} TSFiDisplacementShader;

// Initialize the DisplacementShader context
void tsfi_displacementshader_init(TSFiDisplacementShader *ds, double amplitude, double frequency);

// Computes the geometric displacement value for a vertex based on active AUTODIN timeline time
double tsfi_displacementshader_eval(const TSFiDisplacementShader *ds, double autodin_time, double vertex_coord);

#endif // TSFI_DISPLACEMENTSHADER_H
