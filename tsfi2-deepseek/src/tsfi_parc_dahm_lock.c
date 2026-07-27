#include <stdio.h>
#include <stdatomic.h>
#include "tsfi_parc_dahm_lock.h"

void tsfi_dahm_lock_init(tsfi_dahm_lock_t *lock) {
    if (!lock) return;
    lock->lock_word = 0;
    lock->owner_pid = 0;
    lock->spin_count = 0;
}

int tsfi_dahm_lock_buzz_acquire(tsfi_dahm_lock_t *lock, uint32_t pid) {
    if (!lock) return -1;
    uint32_t expected = 0;
    while (!atomic_compare_exchange_weak((_Atomic uint32_t *)&lock->lock_word, &expected, 1)) {
        lock->spin_count++;
        expected = 0;
    }
    lock->owner_pid = pid;
    return 0;
}

int tsfi_dahm_lock_acquire_two_phase(tsfi_dahm_lock_t *lock, uint32_t pid, uint32_t max_spins) {
    if (!lock) return -1;
    uint32_t spins = 0;
    uint32_t expected = 0;
    
    // Phase 1: Fast Buzz Spin Lock
    while (spins < max_spins) {
        expected = 0;
        if (atomic_compare_exchange_weak((_Atomic uint32_t *)&lock->lock_word, &expected, 1)) {
            lock->owner_pid = pid;
            return 0;
        }
        spins++;
        lock->spin_count++;
    }

    // Phase 2: Backoff Yield / Lock Gate
    expected = 0;
    if (atomic_compare_exchange_strong((_Atomic uint32_t *)&lock->lock_word, &expected, 1)) {
        lock->owner_pid = pid;
        return 0;
    }

    return -2; // Lock contention timeout
}

void tsfi_dahm_lock_release(tsfi_dahm_lock_t *lock, uint32_t pid) {
    if (!lock) return;
    if (lock->owner_pid == pid || lock->owner_pid == 0) {
        lock->owner_pid = 0;
        atomic_store((_Atomic uint32_t *)&lock->lock_word, 0);
    }
}
