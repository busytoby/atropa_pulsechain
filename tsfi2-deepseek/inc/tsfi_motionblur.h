#ifndef TSFI_MOTIONBLUR_H
#define TSFI_MOTIONBLUR_H

#include <stdint.h>

#define TSFI_BLUR_SAMPLES 8

typedef struct {
    double sample_times[TSFI_BLUR_SAMPLES];
    double camera_coords_x[TSFI_BLUR_SAMPLES];
    double camera_coords_y[TSFI_BLUR_SAMPLES];
} TSFiMotionBlur;

// Initializes the motion blur context with temporal time sample values
void tsfi_motionblur_init(TSFiMotionBlur *mb, double shutter_open, double shutter_close);

// Adds camera spatial coordinates to a specific temporal time sample slot index
void tsfi_motionblur_add_sample(TSFiMotionBlur *mb, int slot_idx, double cam_x, double cam_y);

// Integrates coordinates over the active time samples to resolve blurred coordinate positions
void tsfi_motionblur_integrate(const TSFiMotionBlur *mb, double *out_x, double *out_y);

#endif // TSFI_MOTIONBLUR_H
