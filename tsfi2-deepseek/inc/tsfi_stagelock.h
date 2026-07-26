#ifndef TSFI_STAGELOCK_H
#define TSFI_STAGELOCK_H

#define _GNU_SOURCE
#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>

// USD Stage Lock and Mutex Registry
typedef struct {
    pthread_rwlock_t rwlock;
    int reader_count;
    bool is_write_locked;
} TSFiStageLock;

// Initialize the stage lock context
void tsfi_stagelock_init(TSFiStageLock *lock);

// Destroy the stage lock context
void tsfi_stagelock_destroy(TSFiStageLock *lock);

// Acquires shared read lock
bool tsfi_stagelock_lock_read(TSFiStageLock *lock);

// Releases shared read lock
void tsfi_stagelock_unlock_read(TSFiStageLock *lock);

// Acquires exclusive write lock
bool tsfi_stagelock_lock_write(TSFiStageLock *lock);

// Releases exclusive write lock
void tsfi_stagelock_unlock_write(TSFiStageLock *lock);

#endif // TSFI_STAGELOCK_H
