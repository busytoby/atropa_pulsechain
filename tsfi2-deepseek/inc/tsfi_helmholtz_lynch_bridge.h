#ifndef TSFI_HELMHOLTZ_LYNCH_BRIDGE_H
#define TSFI_HELMHOLTZ_LYNCH_BRIDGE_H

#include <stdint.h>
#include <stddef.h>
#include "tsfi_lynch_exec.h"

typedef struct {
    uint32_t active_pid;
    uint32_t wave_time_slice_ms;
    uint64_t fet_evicted_pages;
    int is_autodin_aligned;
} tsfi_hl_bridge_sync_t;

/* Synchronize Lynch's Executive Kernel PCB schedule with Helmholtz wave phase and FET energy */
int tsfi_hl_bridge_sync_wave(
    tsfi_lynch_exec_t *exec,
    float wave_phase,
    float fet_voltage,
    uint32_t autodin_priority,
    tsfi_hl_bridge_sync_t *out_sync
);

#endif // TSFI_HELMHOLTZ_LYNCH_BRIDGE_H
