#include <stdio.h>
#include <assert.h>
#include <math.h>
#include "../tsfi2-deepseek/inc/tsfi_fbm_landscape.h"
#include "../tsfi2-deepseek/inc/tsfi_volumetric.h"
#include "../tsfi2-deepseek/inc/tsfi_motionblur.h"

int main(void) {
    printf("=== RUNNING AUNCIENT ADVANCED CINEFEX TESTS ===\n");

    // 1. Test fBm Landscapes
    TSFiFbmLandscape land;
    tsfi_fbm_landscape_init(&land, 4, 2.0, 0.5, 12345U);
    double h0 = tsfi_fbm_landscape_eval(&land, 10.0, 20.0);
    double h1 = tsfi_fbm_landscape_eval(&land, 10.0, 20.0);
    assert(fabs(h0 - h1) < 1e-5);
    printf("   ✓ Fractional Brownian Motion (fBm) landscape evaluated successfully.\n");

    // 2. Test Volumetric light raymarching
    TSFiShadowMap sm;
    tsfi_shadowmap_init(&sm);
    tsfi_shadowmap_set_depth(&sm, 5, 5, 8.0);
    
    TSFiVolumetric vol;
    tsfi_volumetric_init(&vol, 0.1, 0.05);
    double light_intensity = tsfi_volumetric_raymarch(&vol, &sm, 0.0, 15.0, 30);
    assert(light_intensity > 0.0);
    printf("   ✓ Atmospheric volumetric light raymarching computed successfully.\n");

    // 3. Test Motion Blur time samples
    TSFiMotionBlur mb;
    tsfi_motionblur_init(&mb, 0.0, 1.0);
    for (int i = 0; i < TSFI_BLUR_SAMPLES; i++) {
        tsfi_motionblur_add_sample(&mb, i, (double)i * 10.0, (double)i * 20.0);
    }
    double blurred_x = 0.0;
    double blurred_y = 0.0;
    tsfi_motionblur_integrate(&mb, &blurred_x, &blurred_y);
    assert(blurred_x > 0.0);
    assert(blurred_y > 0.0);
    printf("   ✓ Shutter motion blur temporal integration computed successfully.\n");

    printf("=== AUNCIENT ADVANCED CINEFEX TESTS COMPLETE (PASS) ===\n");
    return 0;
}
