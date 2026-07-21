#ifndef TSFI_MINIMIZED_DRUM_BUFFER_H
#define TSFI_MINIMIZED_DRUM_BUFFER_H

#include <stdint.h>
#include <stddef.h>

#define MIN_DRUM_BUF_SIZE 256

typedef struct {
    float audio_ring_buffer[MIN_DRUM_BUF_SIZE];
    uint32_t write_cursor;
    uint32_t read_cursor;
    double measured_latency_us;
} tsfi_minimized_drum_buffer_t;

/* Initialize Lockless SPSC Audio Buffer for Ultra-Low Latency Percussion */
int tsfi_minimized_drum_buffer_init(tsfi_minimized_drum_buffer_t *buf);

/* Push percussive transient sample into lockless ring buffer (sub-microsecond execution) */
int tsfi_minimized_drum_buffer_push(tsfi_minimized_drum_buffer_t *buf, float sample);

#endif // TSFI_MINIMIZED_DRUM_BUFFER_H
