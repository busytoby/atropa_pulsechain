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
    assert(dof.wavefront_coding_enabled == false);

    // Verify wavefront coding EDoF toggle and eval_blur invariance
    tsfi_depthoffield_enable_wavefront_coding(&dof, true, 2.0);
    assert(dof.wavefront_coding_enabled == true);
    double wfc_blur_near = tsfi_depthoffield_eval_blur(&dof, 5.0);
    double wfc_blur_far = tsfi_depthoffield_eval_blur(&dof, 50.0);
    // Wavefront coding blur must be constant (defocus-invariant)
    assert(fabs(wfc_blur_near - 1.0) < 1e-5);
    assert(fabs(wfc_blur_far - 1.0) < 1e-5);

    // Disable wavefront coding for standard CoC tests
    tsfi_depthoffield_enable_wavefront_coding(&dof, false, 1.0);

    // Verify unconfigured warning trigger (focal_distance = 0.0)
    TSFiDepthOfField unconfig_dof;
    tsfi_depthoffield_init(&unconfig_dof, 0.0, 0.5, 10.0);
    tsfi_depthoffield_eval_blur(&unconfig_dof, 10.0);

    // Verify Circle of Confusion evaluations
    double blur_at_focus = tsfi_depthoffield_eval_blur(&dof, 10.0);
    double blur_out = tsfi_depthoffield_eval_blur(&dof, 20.0);

    assert(fabs(blur_at_focus - 0.0) < 1e-5);
    assert(fabs(blur_out - 0.25) < 1e-5);

    // Register zmachine address in the thread-safe dynamic interop registry to avoid mocks
    tsfi_depthoffield_register_zmachine(0x5, 15.0, 0.5, 15.0);

    // Verify Z-machine resolution target retrieves registered parameters
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

    // Verify last fallback: Zero-Op sharp bypass copies input exactly
    double output_img_bypass[9] = {0.0};
    tsfi_depthoffield_bokeh_replace_gaussian(input_img, output_img_bypass, 3, 3, 0.0);
    assert(output_img_bypass[4] == 10.0);

    // Verify Wiener deconvolution restoration filter
    double deconvolved_img[9] = {0.0};
    tsfi_depthoffield_wiener_deconvolve(output_img, deconvolved_img, 3, 3, 0.01);
    // The center pixel should recover high-frequency sharpness after Wiener restoration
    assert(deconvolved_img[4] > output_img[4]);

    // Verify 1.85:1 Super 8 Vaesen camera configuration aspect ratios
    int s8_height = 0;
    tsfi_depthoffield_set_super8_vaesen(&dof, 185, &s8_height);
    assert(s8_height == 100);
    assert(fabs(dof.focal_distance - 10.0) < 1e-5);

    // Verify chromatic defocus evaluations
    double red_blur = tsfi_depthoffield_eval_chromatic_blur(&dof, 10.0, 0); // Red
    double green_blur = tsfi_depthoffield_eval_chromatic_blur(&dof, 10.0, 1); // Green
    double blue_blur = tsfi_depthoffield_eval_chromatic_blur(&dof, 10.0, 2); // Blue
    assert(fabs(green_blur - 0.0) < 1e-5); // Green is in focus at 10.0
    assert(red_blur > 0.0); // Red dispersion shift causes defocus
    assert(blue_blur > 0.0); // Blue dispersion shift causes defocus

    // Verify chromatic Wiener deconvolution
    double red_deconvolved[9] = {0.0};
    double blue_deconvolved[9] = {0.0};
    tsfi_depthoffield_wiener_deconvolve_chromatic(output_img, red_deconvolved, 3, 3, 0.01, 0); // Red channel
    tsfi_depthoffield_wiener_deconvolve_chromatic(output_img, blue_deconvolved, 3, 3, 0.01, 2); // Blue channel
    assert(red_deconvolved[4] > 0.0);
    assert(blue_deconvolved[4] > 0.0);

    // Verify joint optimization loop (Slide 22)
    double opt_alpha = tsfi_depthoffield_optimize_joint_loop(&dof, input_img, output_img, 3, 3, 10);
    assert(opt_alpha > 0.0);

    // Verify resource-constrained joint optimizer (Slides 10 & 73)
    double constrained_mse = 0.0;
    double constrained_alpha = tsfi_depthoffield_optimize_constrained(&dof, input_img, output_img, 3, 3, 15.0, &constrained_mse);
    assert(constrained_alpha > 0.0);

    printf("   ✓ Focal distance and lens parameters initialized.\n");
    printf("   ✓ Wavefront coding defocus-invariant blur evaluated successfully.\n");
    printf("   ✓ Joint optimizer inner deconvolution loop verified successfully.\n");
    printf("   ✓ Resource-constrained joint optimizer verified successfully.\n");


    printf("   ✓ Chromatic Wiener deconvolution filters verified successfully.\n");

    printf("   ✓ Chromatic dispersion defocus factors evaluated successfully.\n");

    printf("   ✓ Circle of Confusion blur factor calculated successfully.\n");
    printf("   ✓ Z-machine target resolution dynamic registry verified successfully.\n");
    printf("   ✓ Dynamic shot parameters swaps verified successfully.\n");
    printf("   ✓ Post-process Gaussian blur bokeh replacement verified successfully.\n");
    printf("   ✓ Last fallback Zero-Op sharp bypass verified successfully.\n");
    printf("   ✓ Wiener deconvolution restoration filter verified successfully.\n");
    printf("   ✓ 1.85:1 Super 8 Vaesen widescreen aspect ratio verified successfully.\n");
    printf("=== AUNCIENT DEPTHOFFIELD TESTS COMPLETE (PASS) ===\n");
    return 0;
}

