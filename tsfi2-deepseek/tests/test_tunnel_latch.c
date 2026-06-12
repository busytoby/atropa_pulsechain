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

    // Check for memory leaks
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    extern void lau_assert_zero_unsealed_leaks(const char *context, void *teardown_ptr);
    lau_assert_zero_unsealed_leaks("test_tunnel_latch", NULL);

    printf("[PASS] Tunnel Diode Bistable Latch emulation verified.\n");
    return 0;
}
