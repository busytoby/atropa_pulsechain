#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "tsfi_lynch_sync.h"

int tsfi_lynch_sem_init(tsfi_lynch_sem_t *sem, int32_t initial_count) {
    if (!sem) return -1;
    sem->count = initial_count;
    sem->lock_owner_pid = 0;
    printf("[LYNCH SYNC] Initialized Counting Semaphore (Count: %d)\n", initial_count);
    return 0;
}

int tsfi_lynch_sem_wait(tsfi_lynch_sem_t *sem, uint32_t pid) {
    if (!sem) return -1;
    sem->count--;
    sem->lock_owner_pid = pid;
    printf("[LYNCH SYNC] Semaphore Wait (PID %u) -> Remaining Count: %d\n", pid, sem->count);
    return 0;
}

int tsfi_lynch_sem_signal(tsfi_lynch_sem_t *sem) {
    if (!sem) return -1;
    sem->count++;
    printf("[LYNCH SYNC] Semaphore Signal -> Updated Count: %d\n", sem->count);
    return 0;
}

int tsfi_lynch_latch_acquire(tsfi_lynch_index_latch_t *latch, uint32_t node_id, tsfi_lynch_latch_mode_t mode) {
    if (!latch) return -1;
    latch->node_id = node_id;
    latch->mode = mode;
    if (mode == LYNCH_LATCH_SHARED) {
        latch->active_readers++;
    }
    printf("[LYNCH LATCH] Acquired %s Latch on Index Node %u (Active Readers: %u)\n",
           (mode == LYNCH_LATCH_EXCLUSIVE) ? "EXCLUSIVE" : "SHARED", node_id, latch->active_readers);
    return 0;
}

int tsfi_lynch_latch_release(tsfi_lynch_index_latch_t *latch) {
    if (!latch) return -1;
    if (latch->mode == LYNCH_LATCH_SHARED && latch->active_readers > 0) {
        latch->active_readers--;
    }
    if (latch->active_readers == 0) {
        latch->mode = LYNCH_LATCH_NONE;
    }
    printf("[LYNCH LATCH] Released Latch on Index Node %u\n", latch->node_id);
    return 0;
}
