#define _GNU_SOURCE
#include "tsfi_stagelock.h"
#include <stdio.h>

void tsfi_stagelock_init(TSFiStageLock *lock) {
    if (!lock) return;
    pthread_rwlock_init(&lock->rwlock, NULL);
    lock->reader_count = 0;
    lock->is_write_locked = false;
}

void tsfi_stagelock_destroy(TSFiStageLock *lock) {
    if (!lock) return;
    pthread_rwlock_destroy(&lock->rwlock);
}

bool tsfi_stagelock_lock_read(TSFiStageLock *lock) {
    if (!lock) return false;
    if (pthread_rwlock_rdlock(&lock->rwlock) == 0) {
        lock->reader_count++;
        return true;
    }
    return false;
}

void tsfi_stagelock_unlock_read(TSFiStageLock *lock) {
    if (!lock) return;
    if (pthread_rwlock_unlock(&lock->rwlock) == 0) {
        lock->reader_count--;
    }
}

bool tsfi_stagelock_lock_write(TSFiStageLock *lock) {
    if (!lock) return false;
    if (pthread_rwlock_wrlock(&lock->rwlock) == 0) {
        lock->is_write_locked = true;
        return true;
    }
    return false;
}

void tsfi_stagelock_unlock_write(TSFiStageLock *lock) {
    if (!lock) return;
    if (pthread_rwlock_unlock(&lock->rwlock) == 0) {
        lock->is_write_locked = false;
    }
}
