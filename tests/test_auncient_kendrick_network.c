#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include "tsfi_mainframe_computerworld.h"

// -------------------------------------------------------------
// Unit Tests calling the compiled Kendrick library functions
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT KENDRICK MAINFRAME ROUTING ADAPTATION SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    // 1. Telephony Switch for DECnet session routing capacity checks
    printf("[TEST] Running Kendrick routing switch capacity checks...\n");
    fflush(stdout);
    tsfi_cw_kendrick_switch sw = {
        .active_lines = 0,
        .capacity = 3,
        .trunk_id = 101
    };

    int connected = 0;
    // Route 1st call (valid digit: 4)
    int res = tsfi_cw_kendrick_route_call(4, &sw, &connected);
    assert(res == 0);
    assert(connected == 1);
    assert(sw.active_lines == 1);

    // Route 2nd and 3rd calls
    tsfi_cw_kendrick_route_call(4, &sw, &connected);
    tsfi_cw_kendrick_route_call(4, &sw, &connected);
    assert(sw.active_lines == 3);

    // Route 4th call (should be rejected due to capacity limit)
    connected = 0;
    res = tsfi_cw_kendrick_route_call(4, &sw, &connected);
    assert(res == 0);
    assert(connected == 0);
    assert(sw.active_lines == 3);
    printf("   ✓ Kendrick session capacity limits verified successfully.\n");
    fflush(stdout);

    // 2. Multiplexer for raw packet stream aggregation
    printf("[TEST] Running Kendrick signal multiplexer...\n");
    fflush(stdout);
    int input_signals[3] = { 100, 200, 300 }; // Packet sizes from 3 queues
    int combined_signal = 0;
    
    res = tsfi_cw_kendrick_multiplex(3, input_signals, &combined_signal);
    assert(res == 0);
    assert(combined_signal == 600); // 100 + 200 + 300
    printf("   ✓ Kendrick packet sizing multiplexer pass.\n");
    fflush(stdout);

    // 3. Integrator for flow-control RTT updates
    printf("[TEST] Running Kendrick numerical integrator...\n");
    fflush(stdout);
    double rtt_result = 0.0;
    
    // Integrate dynamic step increments over 5 iterations starting at 100.0 ms
    res = tsfi_cw_kendrick_integrate(0.05, 100.0, 5, &rtt_result);
    assert(res == 0);
    assert(rtt_result > 120.0); // 1.05^5 * 100.0
    printf("   ✓ Kendrick RTT integration step pass (Result: %.2f ms).\n", rtt_result);
    fflush(stdout);

    printf("=============================================================\n");
    printf("KENDRICK MAINFRAME OPERATIONS INTEGRATION VERIFIED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
