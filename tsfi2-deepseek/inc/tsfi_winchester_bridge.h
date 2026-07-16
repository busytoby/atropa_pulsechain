#ifndef TSFI_WINCHESTER_BRIDGE_H
#define TSFI_WINCHESTER_BRIDGE_H

#include "tsfi_synth_perf.h"

// WinchesterMQ SCSI Handshake Registers
typedef struct {
    volatile uint32_t status_reg; // SCSI status (0: Idle, 1: Request, 2: Ack, 3: Complete)
    volatile uint32_t keycode_reg; // Keycode transmission word
    volatile uint32_t data_reg;    // Data buffer register
} TSFiWinchesterRegisters;

// WinchesterMQ Bridge Context
typedef struct {
    TSFiSynthPerfEngine *perf_engine;
    TSFiWinchesterRegisters registers;
    int loopback_socket_fd; // Virtual socket fd for SCSI tracking
} TSFiWinchesterBridge;

// Create WinchesterMQ Bridge
TSFiWinchesterBridge* tsfi_winchester_bridge_create(TSFiSynthPerfEngine *perf_engine);

// Translate active tone-wheel frequencies to keycodes and execute WinchesterMQ SCSI handshake
int tsfi_winchester_bridge_handshake(TSFiWinchesterBridge *bridge);

// Destroy WinchesterMQ Bridge
void tsfi_winchester_bridge_destroy(TSFiWinchesterBridge *bridge);

#endif // TSFI_WINCHESTER_BRIDGE_H
