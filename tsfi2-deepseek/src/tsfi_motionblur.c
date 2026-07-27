#include "tsfi_motionblur.h"
#include <string.h>

void tsfi_motionblur_init(TSFiMotionBlur *mb, double shutter_open, double shutter_close) {
    if (!mb) return;
    double duration = shutter_close - shutter_open;
    for (int i = 0; i < TSFI_BLUR_SAMPLES; i++) {
        mb->sample_times[i] = shutter_open + (duration * i) / (TSFI_BLUR_SAMPLES - 1);
        mb->camera_coords_x[i] = 0.0;
        mb->camera_coords_y[i] = 0.0;
    }
}

void tsfi_motionblur_add_sample(TSFiMotionBlur *mb, int slot_idx, double cam_x, double cam_y) {
    if (!mb || slot_idx < 0 || slot_idx >= TSFI_BLUR_SAMPLES) return;
    mb->camera_coords_x[slot_idx] = cam_x;
    mb->camera_coords_y[slot_idx] = cam_y;
}

void tsfi_motionblur_integrate(const TSFiMotionBlur *mb, double *out_x, double *out_y) {
    if (!mb || !out_x || !out_y) return;
    
    double total_x = 0.0;
    double total_y = 0.0;
    
    for (int i = 0; i < TSFI_BLUR_SAMPLES; i++) {
        total_x += mb->camera_coords_x[i];
        total_y += mb->camera_coords_y[i];
    }
    
    // Average coordinate values over the active time samples
    *out_x = total_x / TSFI_BLUR_SAMPLES;
    *out_y = total_y / TSFI_BLUR_SAMPLES;
}
