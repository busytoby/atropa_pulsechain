#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "tsfi_coaxial_telemetry.h"

int main(void) {
    printf("=============================================================\n");
    printf("TESTING ASYNCHRONOUS COAXIAL TELEMETRY RING BUFFER\n");
    printf("=============================================================\n");

    TSFiCoaxialTelemetryScheduler sched;
    
    // Initialize with NULL ZMM VM state to mock operations
    tsfi_coaxial_telemetry_init(&sched, NULL);
    assert(sched.ring_head == 0);
    assert(sched.ring_tail == 0);
    assert(sched.transmit_count == 0);

    // Schedule 5 packets
    printf("Scheduling 5 priority packets...\n");
    for (int i = 0; i < 5; i++) {
        int status = tsfi_coaxial_telemetry_schedule(&sched, 10 + i, 100 + i);
        assert(status == 0);
    }

    // Transmit a batch of 3 to the ring buffer
    printf("Transmitting batch of 3 to the double-buffer queue...\n");
    int enqueued = tsfi_coaxial_telemetry_transmit_batch(&sched, 3);
    assert(enqueued == 3);
    assert(sched.ring_head == 3);
    assert(sched.ring_tail == 0);

    // Flush the ring buffer
    printf("Flushing buffered packets...\n");
    int flushed = tsfi_coaxial_telemetry_flush_buffer(&sched);
    assert(flushed == 3);
    assert(sched.ring_tail == 3);
    assert(sched.transmit_count == 3);

    // Transmit remaining packets (2)
    printf("Transmitting remaining batch...\n");
    enqueued = tsfi_coaxial_telemetry_transmit_batch(&sched, 5);
    assert(enqueued == 2);

    flushed = tsfi_coaxial_telemetry_flush_buffer(&sched);
    assert(flushed == 2);
    assert(sched.transmit_count == 5);

    printf("✓ Asynchronous ring buffer and batch APIs verified successfully.\n");
    printf("=============================================================\n");
    return 0;
}
