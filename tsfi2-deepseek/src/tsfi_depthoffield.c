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
