#include "tsfi_depthoffield.h"
#include <math.h>

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
