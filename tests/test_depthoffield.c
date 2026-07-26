#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include "../tsfi2-deepseek/inc/tsfi_depthoffield.h"

int main(void) {
    printf("=== RUNNING AUNCIENT DEPTHOFFIELD TESTS ===\n");

    TSFiDepthOfField dof;
    tsfi_depthoffield_init(&dof, 10.0, 0.5, 10.0);

    assert(fabs(dof.focal_distance - 10.0) < 1e-5);
    assert(fabs(dof.lens_radius - 0.5) < 1e-5);
    assert(fabs(dof.target_z - 10.0) < 1e-5);

    // Verify Circle of Confusion evaluations
    double blur_at_focus = tsfi_depthoffield_eval_blur(&dof, 10.0);
    double blur_out = tsfi_depthoffield_eval_blur(&dof, 20.0);

    assert(fabs(blur_at_focus - 0.0) < 1e-5);
    assert(fabs(blur_out - 0.25) < 1e-5);

    // Verify Z-machine resolution target
    assert(tsfi_depthoffield_resolve_zmachine(&dof, 0x5) == true);
    assert(fabs(dof.focal_distance - 15.0) < 1e-5);

    // Verify dynamic shot parameters swap
    tsfi_depthoffield_set_shot(&dof, 1);
    assert(fabs(dof.focal_distance - 8.0) < 1e-5);
    assert(fabs(dof.lens_radius - 0.8) < 1e-5);

    tsfi_depthoffield_set_shot(&dof, 2);
    assert(fabs(dof.focal_distance - 25.0) < 1e-5);
    assert(fabs(dof.lens_radius - 0.2) < 1e-5);

    // Verify fast 1D Gaussian convolution replaces bokeh multi-sample loop
    double input_img[9] = {1.0, 1.0, 1.0, 1.0, 10.0, 1.0, 1.0, 1.0, 1.0};
    double output_img[9] = {0.0};
    tsfi_depthoffield_bokeh_replace_gaussian(input_img, output_img, 3, 3, 2.0);
    // Center pixel should be smoothed and distributed
    assert(output_img[4] < 10.0);

    printf("   ✓ Focal distance and lens parameters initialized.\n");
    printf("   ✓ Circle of Confusion blur factor calculated successfully.\n");
    printf("   ✓ Z-machine target resolution verified successfully.\n");
    printf("   ✓ Dynamic shot parameters swaps verified successfully.\n");
    printf("   ✓ Post-process Gaussian blur bokeh replacement verified successfully.\n");
    printf("=== AUNCIENT DEPTHOFFIELD TESTS COMPLETE (PASS) ===\n");
    return 0;
}
