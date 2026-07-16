#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tsfi_winchester_bridge.h"

TSFiWinchesterBridge* tsfi_winchester_bridge_create(TSFiSynthPerfEngine *perf_engine) {
    TSFiWinchesterBridge *bridge = calloc(1, sizeof(TSFiWinchesterBridge));
    if (!bridge) return NULL;

    bridge->perf_engine = perf_engine;
    bridge->registers.status_reg = 0; // Idle
    bridge->registers.keycode_reg = 0;
    bridge->registers.data_reg = 0;
    bridge->loopback_socket_fd = 99; // Mock virtual loopback socket fd

    return bridge;
}

int tsfi_winchester_bridge_handshake(TSFiWinchesterBridge *bridge) {
    if (!bridge || !bridge->perf_engine) return -1;

    // 1. Read frequency of primary tone-wheel (wheel index 0)
    float freq = bridge->perf_engine->synth->wheels[0].frequency;
    
    // 2. Map frequency to keycode (e.g. 440Hz -> Key 'D' keycode 32, 880Hz -> Key 'A' keycode 30)
    uint32_t keycode = 0;
    if (freq >= 800.0f) {
        keycode = 30; // Key 'A'
    } else if (freq >= 400.0f) {
        keycode = 32; // Key 'D'
    } else {
        keycode = 10; // Default keycode
    }

    // 3. Perform WinchesterMQ SCSI register status handshake loop
    bridge->registers.status_reg = 1; // Request
    bridge->registers.keycode_reg = keycode;

    // Simulate SCSI loop handshake transitions (Req -> Ack -> Complete)
    if (bridge->registers.status_reg == 1) {
        bridge->registers.status_reg = 2; // Ack received
        bridge->registers.data_reg = keycode * 2; // Process data modification
    }

    if (bridge->registers.status_reg == 2) {
        bridge->registers.status_reg = 3; // Complete
    }

    return (bridge->registers.status_reg == 3) ? 0 : -2;
}

void tsfi_winchester_bridge_destroy(TSFiWinchesterBridge *bridge) {
    if (!bridge) return;
    free(bridge);
}
