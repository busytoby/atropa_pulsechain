#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include "tsfi_valve.h"
#include "lau_memory.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define NUM_SAMPLES 256
#define SAMPLING_RATE 44100.0

int main() {
    printf("=== TSFi Issue #2: Oscillating Audion (Regenerative Feedback) Test ===\n");

    TsfiValveTriode valve;
    tsfi_valve_init(&valve, 100.0, 0.00002, 250.0, -1.5); // mu=100, K=0.00002, Vp=250V, Vg_bias=-1.5V

    float *vg_in = (float*)lau_memalign(64, NUM_SAMPLES * sizeof(float));
    float *vp_out_no_fb = (float*)lau_memalign(64, NUM_SAMPLES * sizeof(float));
    float *vp_out_fb = (float*)lau_memalign(64, NUM_SAMPLES * sizeof(float));
    float *vp_out_self_osc = (float*)lau_memalign(64, NUM_SAMPLES * sizeof(float));
    float *vg_zero = (float*)lau_memalign(64, NUM_SAMPLES * sizeof(float));

    // Generate fundamental input 1 kHz sine wave with very small amplitude (0.1 V)
    for (int i = 0; i < NUM_SAMPLES; i++) {
        vg_in[i] = 0.1f * sinf(2.0f * (float)M_PI * 1000.0f * (float)i / (float)SAMPLING_RATE);
        vg_zero[i] = 0.0f;
    }

    // 1. Process without feedback (beta = 0)
    tsfi_valve_process_regenerative(&valve, vg_in, vp_out_no_fb, NUM_SAMPLES, 0.0, 1.0, 0.0);

    // 2. Process with regenerative feedback (beta = 0.3)
    tsfi_valve_process_regenerative(&valve, vg_in, vp_out_fb, NUM_SAMPLES, 0.0, 1.0, 0.3);

    // 3. Process self-oscillation with zero input and high feedback (beta = 0.8)
    // Send a pulse at the beginning to kick-start it
    vg_zero[0] = 0.5f;
    tsfi_valve_process_regenerative(&valve, vg_zero, vp_out_self_osc, NUM_SAMPLES, 0.0, 1.0, 0.85);

    // Calculate RMS/peak-to-peak amplitude of outputs to verify gain enhancement and oscillation
    float min_no_fb = vp_out_no_fb[0], max_no_fb = vp_out_no_fb[0];
    float min_fb = vp_out_fb[0], max_fb = vp_out_fb[0];
    float min_osc = vp_out_self_osc[0], max_osc = vp_out_self_osc[0];

    for (int i = 0; i < NUM_SAMPLES; i++) {
        if (vp_out_no_fb[i] < min_no_fb) min_no_fb = vp_out_no_fb[i];
        if (vp_out_no_fb[i] > max_no_fb) max_no_fb = vp_out_no_fb[i];

        if (vp_out_fb[i] < min_fb) min_fb = vp_out_fb[i];
        if (vp_out_fb[i] > max_fb) max_fb = vp_out_fb[i];

        if (vp_out_self_osc[i] < min_osc) min_osc = vp_out_self_osc[i];
        if (vp_out_self_osc[i] > max_osc) max_osc = vp_out_self_osc[i];
    }

    float p2p_no_fb = max_no_fb - min_no_fb;
    float p2p_fb = max_fb - min_fb;
    float p2p_osc = max_osc - min_osc;

    printf("[RESULTS] Peak-to-Peak Output Voltage:\n");
    printf("  No Feedback (beta=0.0):       %.4f V\n", p2p_no_fb);
    printf("  Regenerative (beta=0.3):      %.4f V\n", p2p_fb);
    printf("  Self-Oscillation (beta=0.85):  %.4f V\n", p2p_osc);

    // Assert that regenerative feedback increases gain
    assert(p2p_fb > p2p_no_fb);

    // Assert that high feedback maintains self-sustained oscillation even with zero input
    assert(p2p_osc > 1.0f);

    // 4. Verify Chebyshev Waveshaping Alternative
    float *vp_out_cheb = (float*)lau_memalign(64, NUM_SAMPLES * sizeof(float));
    tsfi_valve_process_chebyshev(vg_in, vp_out_cheb, NUM_SAMPLES, 0.50, 0.35, 0.15, 0.00);

    float min_cheb = vp_out_cheb[0], max_cheb = vp_out_cheb[0];
    for (int i = 0; i < NUM_SAMPLES; i++) {
        if (vp_out_cheb[i] < min_cheb) min_cheb = vp_out_cheb[i];
        if (vp_out_cheb[i] > max_cheb) max_cheb = vp_out_cheb[i];
    }
    float p2p_cheb = max_cheb - min_cheb;
    printf("  Chebyshev Waveshaper (c1=0.5, c2=0.35): %.4f V\n", p2p_cheb);

    // Assert Chebyshev bounds
    assert(max_cheb <= 1.0f && min_cheb >= -1.0f);

    lau_free(vg_in);
    lau_free(vp_out_no_fb);
    lau_free(vp_out_fb);
    lau_free(vp_out_self_osc);
    lau_free(vg_zero);
    lau_free(vp_out_cheb);

    printf("[SUCCESS] Regenerative feedback and Chebyshev waveshaping verified successfully!\n");
    return 0;
}
