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
    
    // Initialize ring buffer
    sched->ring_head = 0;
    sched->ring_tail = 0;
    memset(sched->ring_buffer, 0, sizeof(sched->ring_buffer));
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
        snprintf(sched->last_tx_packet, sizeof(sched->last_tx_packet), "TX_TELEMETRY_PACKET KEYCODE=%d ID=%d", keycode, sched->transmit_count);
        sched->transmit_count++;
        
        if (sched->zmm) {
            char log_cmd[256];
            snprintf(log_cmd, sizeof(log_cmd), "CALL telemetry_logger log_packet %d", keycode);
            tsfi_zmm_vm_exec(sched->zmm, log_cmd);
        }
        return 1;
    }
    return 0;
}

int tsfi_coaxial_telemetry_transmit_batch(TSFiCoaxialTelemetryScheduler *sched, int max_batch) {
    if (!sched) return -1;

    int enqueued = 0;
    for (int i = 0; i < max_batch; i++) {
        int keycode = 0;
        int status = tsfi_monotonic_buckets_pop(&sched->buckets, &keycode);
        if (status != 0) {
            break; // No more elements in buckets
        }

        // Push keycode to ring buffer
        int next_head = (sched->ring_head + 1) % TSFI_TELEMETRY_RING_SIZE;
        if (next_head == sched->ring_tail) {
            // Buffer full: push back to buckets or drop packet to prevent blocking
            tsfi_monotonic_buckets_push(&sched->buckets, 0, keycode);
            break;
        }

        sched->ring_buffer[sched->ring_head] = keycode;
        sched->ring_head = next_head;
        enqueued++;
    }

    return enqueued;
}

int tsfi_coaxial_telemetry_flush_buffer(TSFiCoaxialTelemetryScheduler *sched) {
    if (!sched) return -1;

    int flushed = 0;
    while (sched->ring_tail != sched->ring_head) {
        int keycode = sched->ring_buffer[sched->ring_tail];
        sched->ring_tail = (sched->ring_tail + 1) % TSFI_TELEMETRY_RING_SIZE;

        snprintf(sched->last_tx_packet, sizeof(sched->last_tx_packet), "TX_TELEMETRY_PACKET KEYCODE=%d ID=%d", keycode, sched->transmit_count);
        sched->transmit_count++;

        if (sched->zmm) {
            char log_cmd[256];
            snprintf(log_cmd, sizeof(log_cmd), "CALL telemetry_logger log_packet %d", keycode);
            tsfi_zmm_vm_exec(sched->zmm, log_cmd);
        }
        flushed++;
    }

    return flushed;
}
