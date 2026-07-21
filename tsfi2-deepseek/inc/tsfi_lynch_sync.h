#ifndef TSFI_LYNCH_SYNC_H
#define TSFI_LYNCH_SYNC_H

#include <stdint.h>
#include <stddef.h>

typedef struct {
    int32_t count;
    uint32_t lock_owner_pid;
} tsfi_lynch_sem_t;

typedef enum {
    LYNCH_LATCH_NONE  = 0,
    LYNCH_LATCH_SHARED = 1,
    LYNCH_LATCH_EXCLUSIVE = 2
} tsfi_lynch_latch_mode_t;

typedef struct {
    uint32_t node_id;
    tsfi_lynch_latch_mode_t mode;
    uint32_t active_readers;
} tsfi_lynch_index_latch_t;

/* Initialize William C. Lynch's Counting Semaphore */
int tsfi_lynch_sem_init(tsfi_lynch_sem_t *sem, int32_t initial_count);

/* Decrement semaphore (wait) with priority inversion prevention */
int tsfi_lynch_sem_wait(tsfi_lynch_sem_t *sem, uint32_t pid);

/* Increment semaphore (signal) releasing blocked processes */
int tsfi_lynch_sem_signal(tsfi_lynch_sem_t *sem);

/* Acquire fine-grained index node latch for concurrent quadtree .dat.bin reads/writes */
int tsfi_lynch_latch_acquire(tsfi_lynch_index_latch_t *latch, uint32_t node_id, tsfi_lynch_latch_mode_t mode);

/* Release fine-grained index node latch */
int tsfi_lynch_latch_release(tsfi_lynch_index_latch_t *latch);

#endif // TSFI_LYNCH_SYNC_H
