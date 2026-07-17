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
    tsfi_ring_init(&bridge->event_ring);

    return bridge;
}

int tsfi_winchester_bridge_handshake(TSFiWinchesterBridge *bridge) {
    if (!bridge || !bridge->perf_engine) return -1;

    float freq = bridge->perf_engine->synth->wheels[0].frequency;
    uint32_t keycode = bridge->registers.keycode_reg & 0xFFFF;
    if (keycode == 0) {
        if (freq >= 800.0f) {
            keycode = 30;
        } else if (freq >= 400.0f) {
            keycode = 32;
        } else {
            keycode = 10;
        }
        bridge->registers.keycode_reg |= keycode;
    }

    tsfi_ring_push(&bridge->event_ring, 1, keycode);

    uint8_t mcs_indicator = (bridge->registers.keycode_reg >> 16) & 0xFF;
    uint8_t mcs_status = (bridge->registers.keycode_reg >> 24) & 0xFF;
    uint8_t mcs_dest = (bridge->registers.status_reg >> 8) & 0xFF;
    (void)mcs_status;
    (void)mcs_dest;
    
    bridge->registers.status_reg = (bridge->registers.status_reg & 0xFFFFFF00) | 1;

    if ((bridge->registers.status_reg & 0xFF) == 1) {
        bridge->registers.status_reg = (bridge->registers.status_reg & 0xFFFFFF00) | 2;
        if (mcs_indicator == 0x01) {
            bridge->registers.data_reg += 1;
        } else if (mcs_indicator == 0x02) {
            bridge->registers.data_reg += 2;
        }
    }

    if ((bridge->registers.status_reg & 0xFF) == 2) {
        bridge->registers.status_reg = (bridge->registers.status_reg & 0xFFFFFF00) | 3;
    }

    return ((bridge->registers.status_reg & 0xFF) == 3) ? 0 : -2;
}

void tsfi_winchester_bridge_destroy(TSFiWinchesterBridge *bridge) {
    if (!bridge) return;
    free(bridge);
}

int tsfi_winchester_bridge_send_packet(TSFiWinchesterBridge *bridge, const TSFiWinchesterMCSHeader *header, const char *payload) {
    if (!bridge || !header || !payload) return -1;
    uint32_t w1;
    memcpy(&w1, header->source_terminal, 4);
    bridge->registers.status_reg = (1 << 8) | 1;
    bridge->registers.keycode_reg = (0x01 << 16) | 32;
    bridge->registers.data_reg = w1;
    tsfi_winchester_bridge_handshake(bridge);
    
    bridge->registers.status_reg = (1 << 8) | 1;
    bridge->registers.keycode_reg = (0x01 << 16) | 32;
    bridge->registers.data_reg = header->message_len;
    tsfi_winchester_bridge_handshake(bridge);
    
    uint32_t pay_word = 0;
    memcpy(&pay_word, payload, (strlen(payload) < 4) ? strlen(payload) : 4);
    bridge->registers.status_reg = (1 << 8) | 1;
    bridge->registers.keycode_reg = (0x02 << 16) | 32;
    bridge->registers.data_reg = pay_word;
    tsfi_winchester_bridge_handshake(bridge);
    return 0;
}

void tsfi_winchester_bridge_map_dbtg_exception(TSFiWinchesterBridge *bridge, int db_status) {
    if (!bridge) return;
    bridge->registers.status_reg = (bridge->registers.status_reg & 0x0000FFFF) | ((uint32_t)db_status << 16);
}
