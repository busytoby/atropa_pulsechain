#ifndef TSFI_RING_BUFFER_H
#define TSFI_RING_BUFFER_H

#include <stdint.h>
#include <stdbool.h>

#define RING_BUFFER_SIZE 128

typedef struct {
    uint32_t nonce;
    uint32_t block_number;
    char target_key[64];
} RingTask;

typedef struct {
    RingTask tasks[RING_BUFFER_SIZE];
    uint64_t head; // Producer index
    uint64_t tail; // Consumer index
} tsfi_ring_buffer;

static inline void tsfi_ring_init(tsfi_ring_buffer *rb) {
    rb->head = 0;
    rb->tail = 0;
}

static inline bool tsfi_ring_produce(tsfi_ring_buffer *rb, RingTask task) {
    uint64_t current_head = __atomic_load_n(&rb->head, __ATOMIC_RELAXED);
    uint64_t current_tail = __atomic_load_n(&rb->tail, __ATOMIC_ACQUIRE);
    
    if (current_head - current_tail >= RING_BUFFER_SIZE) {
        return false; // Queue full
    }
    
    rb->tasks[current_head % RING_BUFFER_SIZE] = task;
    __atomic_store_n(&rb->head, current_head + 1, __ATOMIC_RELEASE);
    return true;
}

static inline bool tsfi_ring_consume(tsfi_ring_buffer *rb, RingTask *task_out) {
    uint64_t current_tail = __atomic_load_n(&rb->tail, __ATOMIC_RELAXED);
    uint64_t current_head = __atomic_load_n(&rb->head, __ATOMIC_ACQUIRE);
    
    if (current_tail == current_head) {
        return false; // Queue empty
    }
    
    *task_out = rb->tasks[current_tail % RING_BUFFER_SIZE];
    // Atomic CAS loop to advance tail index safely among multiple consumer threads
    while (!__atomic_compare_exchange_n(&rb->tail, &current_tail, current_tail + 1, false, __ATOMIC_RELEASE, __ATOMIC_ACQUIRE)) {
        current_head = __atomic_load_n(&rb->head, __ATOMIC_ACQUIRE);
        if (current_tail == current_head) {
            return false;
        }
    }
    return true;
}

#endif // TSFI_RING_BUFFER_H
