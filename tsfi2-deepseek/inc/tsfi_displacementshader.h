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

// Computes the cubic phase plate wavefront displacement in synchronization with WinchesterMQ boundary constraints
double tsfi_displacementshader_eval_cubic(const TSFiDisplacementShader *ds, double x, double y);

// Evaluates radially symmetric spherical aberration phase displacement (Slide 71)
double tsfi_displacementshader_eval_spherical(const TSFiDisplacementShader *ds, double x, double y);

// Registers the evaluator in the XPLSM dynamic symbol table
void tsfi_displacementshader_register_xplsm(void (*register_func)(const char *, void *));

// Computes the geometric page-curl displacement for a vertex in synchronization with WinchesterMQ boundary constraints
bool tsfi_displacementshader_eval_page_curl(const TSFiDisplacementShader *ds, double u, double v, double curl_angle, double *displaced_y, double *displaced_z);

// Aligns the Aho-Corasick pattern offset to WinchesterMQ SCSI register boundary constraints
uint32_t tsfi_displacementshader_align_ahocorasick_offset(const TSFiDisplacementShader *ds, uint32_t unaligned_offset);

#endif // TSFI_DISPLACEMENTSHADER_H
