#ifndef TSFI_COAXIAL_TELEMETRY_H
#define TSFI_COAXIAL_TELEMETRY_H

#include "tsfi_spec_queues.h"
#include "tsfi_zmm_vm.h"

#define TSFI_TELEMETRY_RING_SIZE 1024

// Coaxial Telemetry Scheduler Context
typedef struct {
    TSFiMonotonicBuckets buckets;
    TsfiZmmVmState *zmm;
    int transmit_count;
    char last_tx_packet[256];
    
    // Asynchronous double-buffered ring buffer
    int ring_buffer[TSFI_TELEMETRY_RING_SIZE];
    volatile int ring_head;
    volatile int ring_tail;
} TSFiCoaxialTelemetryScheduler;

// Initialize the Telemetry Scheduler
void tsfi_coaxial_telemetry_init(TSFiCoaxialTelemetryScheduler *sched, TsfiZmmVmState *zmm);

// Schedule a telemetry packet with a monotonic priority timestamp
int tsfi_coaxial_telemetry_schedule(TSFiCoaxialTelemetryScheduler *sched, int priority_time, int keycode);

// Transmit the next scheduled telemetry packet (O(1) extraction)
int tsfi_coaxial_telemetry_transmit(TSFiCoaxialTelemetryScheduler *sched);

// Transmit a batch of scheduled telemetry packets (up to max_batch) to the asynchronous ring buffer
int tsfi_coaxial_telemetry_transmit_batch(TSFiCoaxialTelemetryScheduler *sched, int max_batch);

// Flush buffered packets from the ring buffer and commit them to ZMM
int tsfi_coaxial_telemetry_flush_buffer(TSFiCoaxialTelemetryScheduler *sched);

#endif // TSFI_COAXIAL_TELEMETRY_H
