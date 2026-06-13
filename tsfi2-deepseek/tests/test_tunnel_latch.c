#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "tsfi_tunnel_latch.h"

int main() {
    printf("=== TSFi2 & Computer Design Issue #7 Tunnel Diode Latch Test ===\n");

    TsfiTunnelLatch latch;
    tsfi_tunnel_latch_init(&latch);

    // Initial check
    printf("[TUNNEL] Initial states: V_out = %fV, State = %d\n", latch.v_out, latch.state);
    assert(latch.state == 0);
    assert(latch.v_out == 0.1f);

    // Apply positive write-1 pulse
    printf("[TUNNEL] Applying write-1 pulse (I_trigger = +0.5mA)...\n");
    float v1 = tsfi_tunnel_latch_step(&latch, 0.5f);
    assert(latch.state == 1);
    assert(v1 == 0.5f);

    // Remove pulse (state held)
    printf("[TUNNEL] Removing trigger pulse (I_trigger = 0.0mA)...\n");
    float v2 = tsfi_tunnel_latch_step(&latch, 0.0f);
    assert(latch.state == 1);
    assert(v2 == 0.5f);

    // Apply negative write-0 pulse
    printf("[TUNNEL] Applying write-0 pulse (I_trigger = -0.5mA)...\n");
    float v3 = tsfi_tunnel_latch_step(&latch, -0.5f);
    assert(latch.state == 0);
    assert(v3 == 0.1f);

    // Remove pulse (state held)
    printf("[TUNNEL] Removing trigger pulse (I_trigger = 0.0mA)...\n");
    float v4 = tsfi_tunnel_latch_step(&latch, 0.0f);
    assert(latch.state == 0);
    assert(v4 == 0.1f);
    
    // --- Test 2: W3UZN Tunnel Dipper RF Oscillator Simulation ---
    printf("[W3UZN] Simulating Heathkit HM-10 Tunnel Dipper RF oscillator loop...\n");
    float v_cap = 0.15f; // Initial voltage in negative resistance region
    float i_ind = 0.0f;
    float dt = 1e-6f;   // 1 us timestep
    float l = 1e-3f;    // 1 mH
    float c = 10e-9f;   // 10 nF
    float v_bias = 0.15f; // 0.15V bias voltage in NDR region
    
    float min_osc = 999.0f;
    float max_osc = -999.0f;
    
    for (int step = 0; step < 5000; step++) {
        float v_rf = tsfi_w3uzn_dipper_step(&v_cap, &i_ind, v_bias, l, c, dt);
        if (v_rf > max_osc) max_osc = v_rf;
        if (v_rf < min_osc) min_osc = v_rf;
    }
    printf("  [W3UZN] RF voltage peak-to-peak swing: min = %.4f V, max = %.4f V\n", min_osc, max_osc);
    assert(max_osc > 0.25f);
    assert(min_osc < 0.10f);
    printf("  -> [PASS] Self-sustained Tunnel Diode RF oscillation confirmed!\n");

    // Check for memory leaks
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    extern void lau_assert_zero_unsealed_leaks(const char *context, void *teardown_ptr);
    lau_assert_zero_unsealed_leaks("test_tunnel_latch", NULL);

    printf("[PASS] Tunnel Diode Bistable Latch and W3UZN RF Oscillator verified.\n");
    return 0;
}
