#ifndef TSFI_RESONANCE_H
#define TSFI_RESONANCE_H

#include "tsfi_types.h"
#include "lau_memory.h"
#include "lau_wire_log.h"
#include <stdatomic.h>

/**
 * @brief TSFi Wire Thread (Resonance Manifold)
 * 
 * A hardware-mapped concurrent primitive replacing pthreads.
 * Operates as a resident wavefront within the Helmholtz Resonance List.
 */
DEFINE_MAPPED_STRUCT(LauWireThread,
    int thread_id;
    _Atomic int lock_state;     // Atomic coordination
    float current_intensity;    // Energy (0.0 = At Rest/Cleanup)
    bool finished;              // Completion status
    
    // Coordination
    void *join_target;          // Pointer to LauWireThread to join
    _Atomic int *cond_var;      // Condition variable (Shared memory)
    int cond_expected;          // Value for wake-up
    
    // Resident State
    LauRegisterBank registers;  // ZMM0-31 Backup

    // Zhong Hardware Sync
    uint64_t *gpu_sem_addr;     // Mapped Vulkan Timeline Semaphore address
    uint64_t gpu_sem_target;    // Value to wait for

    // Parity Fields
    void* (*start_routine)(void*); 
    void *arg;
    bool persistent;            // If true, start_routine doesn't auto-reset intensity

    // Subshell/PTY Support
    int pty_master_fd;
    pid_t child_pid;
    void *pty_geometry; // Stores the LauWirePty* for explicit teardown
    
    // Direct Wavefront logs (Zero-Copy)
    LauWireLog log_stdin;
    LauWireLog log_stdout;
)

/**
 * @brief Create a new resonant wire thread.
 */
LauWireThread* tsfi_wire_thread_create(int id, double intensity);

/**
 * @brief Geometrically seal the wire thread memory matrix against tampering.
 */
void tsfi_wire_thread_seal(LauWireThread *wt);

/**
 * @brief Unseal the structural thread geometry.
 */
void tsfi_wire_thread_unseal(LauWireThread *wt);

/**
 * @brief Structurally dismantle and definitively unmap the Wire Thread geometry.
 */
void tsfi_wire_thread_destroy(LauWireThread *wt);

/**
 * @brief Create a new resonant wire thread with an attached PTY/Subshell.
 */
LauWireThread* tsfi_wire_thread_create_pty(int id, const char *cmd, void *arg);

/**
 * @brief Execute the resonance list for a specific thread.
 */
void tsfi_wire_thread_dispatch(LauWireThread *wt);

/**
 * @brief Check if the thread has reached terminal intensity (Rest).
 */
static inline bool tsfi_wire_thread_is_at_rest(LauWireThread *wt) {
    return wt->current_intensity <= 0.0 || wt->finished;
}

#endif // TSFI_RESONANCE_H
