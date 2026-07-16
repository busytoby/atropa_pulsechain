#ifndef TSFI_COAXIAL_TELEMETRY_H
#define TSFI_COAXIAL_TELEMETRY_H

#include "tsfi_spec_queues.h"
#include "tsfi_zmm_vm.h"

// Coaxial Telemetry Scheduler Context
typedef struct {
    TSFiMonotonicBuckets buckets;
    TsfiZmmVmState *zmm;
    int transmit_count;
    char last_tx_packet[256];
} TSFiCoaxialTelemetryScheduler;

// Initialize the Telemetry Scheduler
void tsfi_coaxial_telemetry_init(TSFiCoaxialTelemetryScheduler *sched, TsfiZmmVmState *zmm);

// Schedule a telemetry packet with a monotonic priority timestamp
int tsfi_coaxial_telemetry_schedule(TSFiCoaxialTelemetryScheduler *sched, int priority_time, int keycode);

// Transmit the next scheduled telemetry packet (O(1) extraction)
int tsfi_coaxial_telemetry_transmit(TSFiCoaxialTelemetryScheduler *sched);

#endif // TSFI_COAXIAL_TELEMETRY_H
