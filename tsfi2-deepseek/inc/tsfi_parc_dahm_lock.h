#ifndef TSFI_PARC_DAHM_LOCK_H
#define TSFI_PARC_DAHM_LOCK_H

#include <stdint.h>
#include <stddef.h>

/* David M. Dahm Atomic Lock Structure (Burroughs B5000/B6500 RDLK Primitive) */
typedef struct {
    volatile uint32_t lock_word;  // 0 = Unlocked, 1 = Locked
    uint32_t owner_pid;           // Tracking process PID
    uint64_t spin_count;          // Spin count accumulator for latency profiling
} tsfi_dahm_lock_t;

/* Initialize Dahm Lock */
void tsfi_dahm_lock_init(tsfi_dahm_lock_t *lock);

/* Dahm Buzz Lock: Atomic Spinlock Acquire (RDLK operator simulation) */
int tsfi_dahm_lock_buzz_acquire(tsfi_dahm_lock_t *lock, uint32_t pid);

/* Dahm Two-Phase Lock: Acquire with Backoff Guard Gate */
int tsfi_dahm_lock_acquire_two_phase(tsfi_dahm_lock_t *lock, uint32_t pid, uint32_t max_spins);

/* Release Dahm Lock */
void tsfi_dahm_lock_release(tsfi_dahm_lock_t *lock, uint32_t pid);

#endif // TSFI_PARC_DAHM_LOCK_H
