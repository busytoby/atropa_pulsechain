#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#include "../src/tsfi_computel_carrier_monitor.c"

int main(void) {
    printf("[TEST] Initializing Auncient Computel Carrier Monitor test suite...\n");

    ComputelMonitorState state;
    init_computel_monitor(&state);

    // Verify initial values
    assert(state.channels[0].channel_id == 1);
    assert(state.channels[0].carrier_detect == true);
    assert(state.channels[2].channel_id == 3);
    assert(state.channels[2].carrier_detect == true);

    printf("[TEST] Mocking FSK input buffer signals...\n");
    fflush(stdout);

    // Mock buffer with 1200 Hz sine wave for Channel 0 (B1)
    float buffer1200[BUFFER_SIZE];
    for (int i = 0; i < BUFFER_SIZE; i++) {
        double t = (double)i / SAMPLE_RATE;
        buffer1200[i] = (float)sin(2.0 * M_PI * 1200.0 * t);
    }

    // Demodulate Channel 0 buffer
    demodulate_fsk_buffer(&state, 0, buffer1200, BUFFER_SIZE);
    assert(state.channels[0].carrier_detect == true);
    assert(state.channels[0].current_frequency == 1200);

    // Mock buffer with 2200 Hz sine wave for Channel 2 (D)
    float buffer2200[BUFFER_SIZE];
    for (int i = 0; i < BUFFER_SIZE; i++) {
        double t = (double)i / SAMPLE_RATE;
        buffer2200[i] = (float)sin(2.0 * M_PI * 2200.0 * t);
    }

    // Demodulate Channel 2 buffer
    demodulate_fsk_buffer(&state, 2, buffer2200, BUFFER_SIZE);
    assert(state.channels[2].carrier_detect == true);
    assert(state.channels[2].current_frequency == 2200);

    printf("[TEST] Executing hypervisor MCP switching loops...\n");
    fflush(stdout);
    execute_mcp_switching_loop(&state);

    printf("[SUCCESS] All Computel Carrier Line & MCP switching tests passed successfully.\n");
    fflush(stdout);
    return 0;
}
