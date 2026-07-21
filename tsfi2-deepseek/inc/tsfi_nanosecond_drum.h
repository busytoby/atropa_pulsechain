#ifndef TSFI_NANOSECOND_DRUM_H
#define TSFI_NANOSECOND_DRUM_H

#include <stdint.h>
#include <stddef.h>

typedef struct {
    double register_access_latency_ns; // AVX-512 L1 Cache Register Latency (ns)
    double scsi_swap_latency_ns;      // WinchesterMQ SCSI Keycode Swap Latency (ns)
    double total_nanosecond_latency;  // Total Round-Trip Nanosecond Latency (ns)
    uint8_t rule_11_latency_guard;    // 1 if total latency < 1000 ns
} tsfi_nanosecond_drum_t;

/* Initialize and profile AVX-512 L1 Cache Register Nanosecond Drum Engine */
int tsfi_nanosecond_drum_init(tsfi_nanosecond_drum_t *nano_drum);

/* Execute single-cycle drum trigger stroke returning transient amplitude in nanoseconds */
float tsfi_nanosecond_drum_trigger_stroke(tsfi_nanosecond_drum_t *nano_drum, uint8_t scsi_keycode);

#endif // TSFI_NANOSECOND_DRUM_H
