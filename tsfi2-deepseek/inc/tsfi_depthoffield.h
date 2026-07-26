#ifndef TSFI_DEPTHOFFIELD_H
#define TSFI_DEPTHOFFIELD_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    double focal_distance;
    double lens_radius;
    double target_z; // TARG focal coordinate boundary
} TSFiDepthOfField;

// Initialize the DepthOfField context
void tsfi_depthoffield_init(TSFiDepthOfField *dof, double focal_distance, double lens_radius, double target_z);

// Calculates the bokeh blur radius coefficient for a given geometry vertex Z depth
double tsfi_depthoffield_eval_blur(const TSFiDepthOfField *dof, double z_depth);

// Resolves target configuration from Z-machine virtual execution bounds
bool tsfi_depthoffield_resolve_zmachine(TSFiDepthOfField *dof, uint32_t zmachine_address);

// Updates focus configuration based on active shot index for cinematic sequences
void tsfi_depthoffield_set_shot(TSFiDepthOfField *dof, int shot_index);

// Fast 1D separable post-process Gaussian convolution replacing multi-sample lens bokeh
void tsfi_depthoffield_bokeh_replace_gaussian(const double *input_image, double *output_image, int width, int height, double coc_radius);

#endif // TSFI_DEPTHOFFIELD_H
