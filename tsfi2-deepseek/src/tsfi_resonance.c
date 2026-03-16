#include "tsfi_resonance.h"
#include "lau_thunk.h"
#include "lau_memory.h"
#include "tsfi_io.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <errno.h>
#include <unistd.h>
#include "tsfi_wire_pty.h"
#include <fcntl.h>
#include <sys/wait.h>

// Default Resonance Logic

/**
 * logic_provenance: Verification and Preemption Check.
 */
static void default_logic_provenance(void *obj) {
    LauWireThread *wt = (LauWireThread *)obj;
    if (wt->finished) return; 
}

/**
 * logic_state: Coordination Boundary.
 * Implements Yield, Join, Condition Variable, and Zhong Timeline logic.
 * Returns true if ready to execute logic_epoch, false to yield.
 */
static bool default_logic_state(void *obj) {
    LauWireThread *wt = (LauWireThread *)obj;
    if (wt->finished) return true;

    // 1. Intensity Check (Threshold = 0.1f)
    if (wt->current_intensity <= 0.1f) return true; // Low energy bypass

    // 2. Zhong Hardware Sync (Timeline Semaphore)
    if (wt->gpu_sem_addr != NULL) {
        if (*wt->gpu_sem_addr < wt->gpu_sem_target) return false; // Not ready
    }

    // 3. Join check
    if (wt->join_target != NULL) {
        LauWireThread *target = (LauWireThread *)wt->join_target;
        if (!target->finished) return false;
        wt->join_target = NULL;
    }

    // 4. Condition Variable check
    if (wt->cond_var != NULL) {
        if (atomic_load(wt->cond_var) != wt->cond_expected) return false;
        wt->cond_var = NULL;
    }

    return true;
}

/**
 * logic_epoch: Execution Unit.
 */
static void default_logic_epoch(void *obj) {
    LauWireThread *wt = (LauWireThread *)obj;

    // Handle PTY if present (Autonomous Subshell)
    if (wt->pty_master_fd > 0) {
        // 1. Extract from PTY to stdout wavefront
        if (wt->log_stdout.buffer) {
            char *buf = wt->log_stdout.buffer;
            _Atomic uint32_t *head_ptr = (_Atomic uint32_t *)wt->log_stdout.head;
            _Atomic uint32_t *tail_ptr = (_Atomic uint32_t *)wt->log_stdout.tail;

            uint32_t head = atomic_load_explicit(head_ptr, memory_order_relaxed);
            uint32_t tail = atomic_load_explicit(tail_ptr, memory_order_acquire);
            
            char read_buf[4096];
            ssize_t n = read(wt->pty_master_fd, read_buf, sizeof(read_buf));
            if (n > 0) {
                // tsfi_io_printf(stdout, "[RES] Read %zd bytes from PTY to stdout wavefront\n", n);
                for (ssize_t i = 0; i < n; i++) {
                    uint32_t next = (head + 1) % 4096;
                    if (next != tail) {
                        buf[head] = read_buf[i];
                        head = next;
                    } else break;
                }
                atomic_store_explicit(head_ptr, head, memory_order_release);
            } else if (n == 0) {
                wt->finished = true;
                wt->current_intensity = 0.0;
            }
        }

        // 2. Inject from stdin wavefront to PTY
        if (wt->log_stdin.buffer) {
            char *buf = wt->log_stdin.buffer;
            _Atomic uint32_t *head_ptr = (_Atomic uint32_t *)wt->log_stdin.head;
            _Atomic uint32_t *tail_ptr = (_Atomic uint32_t *)wt->log_stdin.tail;

            uint32_t head = atomic_load_explicit(head_ptr, memory_order_acquire);
            uint32_t tail = atomic_load_explicit(tail_ptr, memory_order_relaxed);
            
            if (tail != head) {
                if (head > tail) {
                    ssize_t nw = write(wt->pty_master_fd, &buf[tail], head - tail);
                    if (nw > 0) tail = (tail + nw) % 4096;
                } else {
                    ssize_t n1 = write(wt->pty_master_fd, &buf[tail], 4096 - tail);
                    if (n1 > 0) {
                        tail = (tail + n1) % 4096;
                        if (tail == 0 && head > 0) {
                            ssize_t n2 = write(wt->pty_master_fd, &buf[0], head);
                            if (n2 > 0) tail = n2;
                        }
                    }
                }
                atomic_store_explicit(tail_ptr, tail, memory_order_release);
            }
        }

        // 3. Check if child died
        if (wt->child_pid > 0) {
            int status;
            pid_t wp = waitpid(wt->child_pid, &status, WNOHANG);
            if (wp > 0 || (wp < 0 && errno == ECHILD)) {
                wt->finished = true;
                wt->current_intensity = 0.0;
                wt->child_pid = -1;
            }
        }
    }

    if (wt->current_intensity > 0.0) {
        if (wt->start_routine) {
            wt->start_routine(wt->arg);
            if (!wt->persistent) wt->current_intensity = 0.0;
        }
    } else {
        wt->finished = true;
    }
}

/**
 * logic_directive: Lifecycle and Control.
 */
static void default_logic_directive(void *obj, char *cmd) {
    LauWireThread *wt = (LauWireThread *)obj;
    if (strcmp(cmd, "STOP") == 0) {
        wt->current_intensity = 0.0;
        wt->finished = true;
    } else if (strcmp(cmd, "RESONATE") == 0) {
        wt->current_intensity = 1.0;
        wt->finished = false;
    }
}

LauWireThread* tsfi_wire_thread_create(int id, double intensity) {
    LauWireThread *wt = (LauWireThread *)lau_malloc_wired(sizeof(LauWireThread));
    if (!wt) return NULL;

    // Zero the entire payload safely
    memset(wt, 0, sizeof(LauWireThread));
    wt->thread_id = id;
    wt->current_intensity = intensity;
    wt->gpu_sem_addr = NULL; // Explicitly NULL for timeline guard bypass
    wt->gpu_sem_target = 0;
    wt->pty_master_fd = -1;
    wt->child_pid = 0;
    atomic_store(&wt->lock_state, 0);

    LauSystemHeader *h = (LauSystemHeader *)((char *)wt - offsetof(LauSystemHeader, payload));
    // Zero registers in the header
    memset(&h->regs, 0, sizeof(LauRegisterBank));
    
    h->logic_epoch = (void (*)(int*))default_logic_epoch;
    h->logic_state = (bool (*)(void*))default_logic_state;
    h->logic_provenance = (void (*)(void*))default_logic_provenance;
    h->logic_directive = (void (*)(int*, char*))default_logic_directive;

    if (!h->proxy) h->proxy = ThunkProxy_create();
    ThunkProxy *p = h->proxy;
    if (p) {
        // RESET CURSOR: Ensure we start at pool beginning for the safety chain
        p->thunk_cursor = p->thunk_pool;
        
        // --- The Unified Safety Chain (Simplified Hybrid) ---
        // 1. CALL logic_state (Hardware Sync + Software Coordination)
        // 2. Conditional CALL logic_epoch (Execution)
        void* state_entry = ThunkProxy_emit_safety_chain(p, h->logic_state, (void(*)(void*))h->logic_epoch, wt);
        
        if (!state_entry) {
            tsfi_io_printf(stderr, "[RES_ERR] Failed to emit safety chain\n");
            return wt;
        }

        wt->step_safety_epoch = (void(*)(void))state_entry;
        wt->provenance = (void(*)(void))ThunkProxy_emit_baked(p, (void*)h->logic_provenance, 1, wt);
        wt->step_executor_directive = (void(*)(char*))ThunkProxy_emit_mixed(p, (void*)h->logic_directive, wt);
        
        mprotect(p->thunk_pool, p->pool_size, PROT_READ | PROT_EXEC);
        __builtin___clear_cache((char*)p->thunk_pool, (char*)p->thunk_cursor);
    }

    return wt;
}

void tsfi_wire_thread_seal(LauWireThread *wt) {
    if (!wt) return;
    lau_seal_object(wt);
    if (wt->pty_geometry) lau_seal_object(wt->pty_geometry);
}

void tsfi_wire_thread_unseal(LauWireThread *wt) {
    if (!wt) return;
    lau_unseal_object(wt);
}

void tsfi_wire_thread_destroy(LauWireThread *wt) {
    if (!wt) return;
    
    printf("[RESONANCE] Commencing teardown of LauWireThread %d...\n", wt->thread_id);
    
    // Mathematically evaluate active topological leaks BEFORE unsealing the structural thread envelope

    
    tsfi_wire_thread_unseal(wt);
    
    // Explicit runtime diagnostic verification mandated
    extern void lau_assert_zero_unsealed_leaks(const char*, void*);
    printf("[RESONANCE] Thread unsealed. Mathematically sweeping tracking boundary for unmapped transient allocations...\n");
    lau_assert_zero_unsealed_leaks("tsfi_wire_thread_destroy", wt);
    
    // Mathematically dismantle PTY if bound
    if (wt->pty_master_fd >= 0) {
        close(wt->pty_master_fd);
        wt->pty_master_fd = -1;
    }
    
    // Harvest zombie processes geometrically if attached
    if (wt->child_pid > 0) {
        int status;
        waitpid(wt->child_pid, &status, WNOHANG);
    }
    
    // Mathematically dissolve the local PTY matrix
    if (wt->pty_geometry) {
        lau_unseal_object(wt->pty_geometry);
        lau_free(wt->pty_geometry);
        wt->pty_geometry = NULL;
    }
    
    LauSystemHeader *h = (LauSystemHeader *)((char *)wt - offsetof(LauSystemHeader, payload));
    if (h->proxy) {
        ThunkProxy_unseal(h->proxy);
        ThunkProxy_destroy(h->proxy);
        h->proxy = NULL;
    }
    
    lau_free(wt);
}

LauWireThread* tsfi_wire_thread_create_pty(int id, const char *cmd, void *arg) {
    LauWireThread *wt = tsfi_wire_thread_create(id, 1.0);
    if (!wt) return NULL;

    wt->arg = arg;
    wt->persistent = true;

    LauWirePty *pty = tsfi_wire_pty_fork(cmd);
    if (!pty) {
        tsfi_io_printf(stderr, "[RES] tsfi_wire_pty_fork failed\n");
        return wt; // Returns with pty_master_fd = -1
    }

    wt->child_pid = pty->child_pid;
    wt->pty_master_fd = pty->master_fd;
    wt->pty_geometry = (void*)pty;

    tsfi_io_printf(stdout, "[RES] Forked child PID %d, PTY master FD %d for thread ID %d\n", wt->child_pid, wt->pty_master_fd, wt->thread_id);

    return wt;
}

void tsfi_wire_thread_dispatch(LauWireThread *wt) {
    if (!wt || wt->finished) return;

    void (*dispatch_fn)(void) = (void(*)(void))wt->step_safety_epoch;
    if (!dispatch_fn) return;

    static int dispatch_count = 0;
    if (dispatch_count++ % 100 == 0) {
        // tsfi_io_printf(stdout, "[RES] Dispatching thread %d, entry %p\n", wt->thread_id, (void*)dispatch_fn);
    }

    // Use a simpler assembly block to set R15 and call
    // We must ensure the stack is aligned to 16 bytes for the call
    // We also set RDI to wt as the thunk expects it (System V AMD64 ABI)
    __asm__ __volatile__(
        "push %%r15\n\t"
        "mov %%rdi, %%r15\n\t"   // Set physical anchor
        "sub $8, %%rsp\n\t"      // Maintain 16-byte alignment after push
        "call *%%rax\n\t"
        "add $8, %%rsp\n\t"
        "pop %%r15\n\t"
        :
        : "D"(wt), "a"(dispatch_fn)
        : "rcx", "rdx", "rsi", "r8", "r9", "r10", "r11", "memory"
    );
}
