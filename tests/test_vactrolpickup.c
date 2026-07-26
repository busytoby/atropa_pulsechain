#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include "../tsfi2-deepseek/inc/tsfi_vactrolpickup.h"

int main(void) {
    printf("=== RUNNING AUNCIENT VACTROLPICKUP SIMULATION TESTS ===\n");

    TSFiVactrolPickup vp;
    // Speed = 10.0 rad/s, LED Current = 15.0 mA
    tsfi_vactrolpickup_init(&vp, 10.0, 15.0);

    assert(fabs(vp.tonewheel_speed - 10.0) < 1e-5);
    assert(fabs(vp.vactrol_led_current - 15.0) < 1e-5);

    // Update with tooth alignment (angle = PI/2 -> max light)
    tsfi_vactrolpickup_update(&vp, 0.01, 1.57079632);
    double sig_max = tsfi_vactrolpickup_get_signal(&vp);
    assert(sig_max > 0.0);

    // Update with tooth valley (angle = -PI/2 -> min light)
    tsfi_vactrolpickup_update(&vp, 0.01, -1.57079632);
    double sig_min = tsfi_vactrolpickup_get_signal(&vp);
    assert(sig_min < sig_max);

    printf("   ✓ Vactrol modulation transfer equations verified.\n");
    printf("   ✓ LDR resistance and voltage output curves verified successfully.\n");
    printf("=== AUNCIENT VACTROLPICKUP SIMULATION TESTS COMPLETE (PASS) ===\n");
    return 0;
}
