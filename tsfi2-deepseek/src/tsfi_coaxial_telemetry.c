#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tsfi_coaxial_telemetry.h"

void tsfi_coaxial_telemetry_init(TSFiCoaxialTelemetryScheduler *sched, TsfiZmmVmState *zmm) {
    if (!sched) return;
    sched->zmm = zmm;
    sched->transmit_count = 0;
    memset(sched->last_tx_packet, 0, sizeof(sched->last_tx_packet));
    tsfi_monotonic_buckets_init(&sched->buckets);
}

int tsfi_coaxial_telemetry_schedule(TSFiCoaxialTelemetryScheduler *sched, int priority_time, int keycode) {
    if (!sched) return -1;
    return tsfi_monotonic_buckets_push(&sched->buckets, priority_time, keycode);
}

int tsfi_coaxial_telemetry_transmit(TSFiCoaxialTelemetryScheduler *sched) {
    if (!sched) return -1;

    int keycode = 0;
    int status = tsfi_monotonic_buckets_pop(&sched->buckets, &keycode);
    if (status == 0) {
        // Mock transmission packet format
        snprintf(sched->last_tx_packet, sizeof(sched->last_tx_packet), "TX_TELEMETRY_PACKET KEYCODE=%d ID=%d", keycode, sched->transmit_count);
        sched->transmit_count++;
        
        // Execute transaction trace in ZMM
        if (sched->zmm) {
            char log_cmd[256];
            snprintf(log_cmd, sizeof(log_cmd), "CALL telemetry_logger log_packet %d", keycode);
            tsfi_zmm_vm_exec(sched->zmm, log_cmd);
        }
        return 1;
    }
    return 0; // No packets to transmit
}
