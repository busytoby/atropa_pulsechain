#include "tsfi_depthoffield.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

void tsfi_depthoffield_init(TSFiDepthOfField *dof, double focal_distance, double lens_radius, double target_z) {
    if (!dof) return;
    dof->focal_distance = focal_distance;
    dof->lens_radius = lens_radius;
    dof->target_z = target_z;
}

double tsfi_depthoffield_eval_blur(const TSFiDepthOfField *dof, double z_depth) {
    if (!dof || fabs(z_depth) < 1e-5) return 0.0;
    
    // Circle of confusion math based on target focus distance
    double coc = dof->lens_radius * fabs(z_depth - dof->focal_distance) / z_depth;
    return coc;
}

bool tsfi_depthoffield_resolve_zmachine(TSFiDepthOfField *dof, uint32_t zmachine_address) {
    if (!dof || zmachine_address == 0) return false;
    
    // Mock simulation resolving targets from Z-machine virtual registers in constant-time
    dof->focal_distance = 15.0;
    dof->lens_radius = 0.5;
    dof->target_z = 15.0; // Set TARG focus to Z-machine target state
    return true;
}

void tsfi_depthoffield_set_shot(TSFiDepthOfField *dof, int shot_index) {
    if (!dof) return;
    
    // O(1) constant-time shot configuration tables for dynamic cinematics swaps
    if (shot_index == 1) {
        dof->focal_distance = 8.0;
        dof->lens_radius = 0.8;
        dof->target_z = 8.0;
    } else if (shot_index == 2) {
        dof->focal_distance = 25.0;
        dof->lens_radius = 0.2;
        dof->target_z = 25.0;
    } else {
        dof->focal_distance = 12.0;
        dof->lens_radius = 0.4;
        dof->target_z = 12.0;
    }
}

void tsfi_depthoffield_bokeh_replace_gaussian(const double *input_image, double *output_image, int width, int height, double coc_radius) {
    if (!input_image || !output_image || width <= 0 || height <= 0) return;
    
    static bool warned_bypass = false;
    static bool warned_gaussian = false;
    
    // Last fallback: Zero-Op sharp bypass if CoC radius is below threshold
    if (coc_radius < 0.5) {
        if (!warned_bypass) {
            printf("[WARNING] Bokeh disabled: falling back to Zero-Op sharp bypass.\n");
            warned_bypass = true;
        }
        for (int i = 0; i < width * height; i++) {
            output_image[i] = input_image[i];
        }
        return;
    }
    
    if (!warned_gaussian) {
        printf("[WARNING] Bokeh disabled: falling back to Gaussian post-process.\n");
        warned_gaussian = true;
    }
    
    double *temp_image = (double *)malloc(width * height * sizeof(double));
    if (!temp_image) return;
    
    int radius = (int)ceil(coc_radius);
    if (radius < 1) radius = 1;
    if (radius > 10) radius = 10; // Cap blur filter size
    
    // Horizontal Gaussian pass
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            double sum = 0.0;
            double weight_sum = 0.0;
            for (int k = -radius; k <= radius; k++) {
                int px = x + k;
                if (px >= 0 && px < width) {
                    double weight = exp(-(double)(k * k) / (2.0 * radius * radius));
                    sum += input_image[y * width + px] * weight;
                    weight_sum += weight;
                }
            }
            temp_image[y * width + x] = sum / weight_sum;
        }
    }
    
    // Vertical Gaussian pass
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            double sum = 0.0;
            double weight_sum = 0.0;
            for (int k = -radius; k <= radius; k++) {
                int py = y + k;
                if (py >= 0 && py < height) {
                    double weight = exp(-(double)(k * k) / (2.0 * radius * radius));
                    sum += temp_image[py * width + x] * weight;
                    weight_sum += weight;
                }
            }
            output_image[y * width + x] = sum / weight_sum;
        }
    }
    
    free(temp_image);
}
