#include "lau_memory.h"
#include "lau_thunk.h"
#include "tsfi_types.h"
#include "tsfi_trace.h"
#include <stdio.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <string.h>

// 1. Define the Mapped Struct for our Wire Thread
DEFINE_MAPPED_STRUCT(LauWireThread,
    int thread_id;
    _Atomic int run_count;
    double current_intensity; // The energy of the wavefront
    bool preempt_requested;
    bool finished;
)

// 2. Implementation of the Resonating Logic

// Resume logic (logic_epoch) - Handles both compute and terminal cleanup
static void wire_thread_resume(void *obj) {
    LauWireThread *wt = (LauWireThread *)obj;
    
    if (wt->current_intensity > 0.0) {
        // ENERGY ACTIVE: Perform Compute
        atomic_fetch_add(&wt->run_count, 1);
        printf("[THREAD %d] Intensity: %.2f | Resumed (Count: %d)\n", 
               wt->thread_id, wt->current_intensity, atomic_load(&wt->run_count));
        
        // Simulation of entropy: intensity decay
        wt->current_intensity -= 0.25; 
    } else {
        // ENERGY AT REST (Intensity 0): Neutralize
        printf("[THREAD %d] Intensity: 0.00 | AT REST. Neutralizing via logic_epoch...\n", wt->thread_id);
        wt->finished = true;
    }
}

// Yield check (logic_state)
static void wire_thread_yield_check(void *obj) {
    LauWireThread *wt = (LauWireThread *)obj;
    if (wt->finished) {
        printf("[THREAD %d] State: NEUTRALIZED\n", wt->thread_id);
    } else {
        printf("[THREAD %d] State: ACTIVE\n", wt->thread_id);
    }
}

// Preemption check (logic_provenance)
static void wire_thread_provenance_check(void *obj) {
    LauWireThread *wt = (LauWireThread *)obj;
    if (wt->preempt_requested) {
        printf("[THREAD %d] PROVENANCE: Preemption Required. Saving context...\n", wt->thread_id);
        wt->preempt_requested = false;
    } else {
        // Only verify if we still have intensity
        if (wt->current_intensity > 0.0) {
            // In a real system, this check is a JIT invariant
            // printf("[THREAD %d] PROVENANCE: Verified (SIG_AFFIRMED)\n", wt->thread_id);
        }
    }
}

// Creation/Directive (logic_directive)
static void wire_thread_directive(void *obj, char *cmd) {
    LauWireThread *wt = (LauWireThread *)obj;
    printf("[THREAD %d] DIRECTIVE: %s\n", wt->thread_id, cmd ? cmd : "NULL");
}

// 3. Execution Engine (The "Resonance" Scheduler)
#undef HELMHOLTZ_RESONANCE_LIST
#define HELMHOLTZ_RESONANCE_LIST(X, wt) \
    X(wt, provenance)     \
    X(wt, safety_state)   \
    X(wt, safety_epoch)

// Execution handler for the resonance list
#define THREAD_RESONANCE_X(wt, func) \
    do { \
        if (strcmp(#func, "provenance") == 0) wt->provenance(); \
        else if (strcmp(#func, "safety_state") == 0) wt->step_safety_state(); \
        else if (strcmp(#func, "safety_epoch") == 0) { if(!wt->finished) wt->step_safety_epoch(); } \
    } while(0);

int main() {
    printf("Starting TSFi Wire Thread Unit Test (Intensity Decay & Cleanup)...\n");

    LauWireThread *wt = (LauWireThread *)lau_malloc_wired(sizeof(LauWireThread));
    if (!wt) return 1;

    wt->thread_id = 202;
    atomic_store(&wt->run_count, 0);
    wt->current_intensity = 1.0; // Starting energy
    wt->preempt_requested = false;
    wt->finished = false;

    LauSystemHeader *h = (LauSystemHeader *)((char *)wt - offsetof(LauSystemHeader, payload));
    h->logic_epoch = (void (*)(int*))wire_thread_resume;
    h->logic_state = (void (*)(bool*))wire_thread_yield_check;
    h->logic_provenance = (void (*)(void*))wire_thread_provenance_check;
    h->logic_directive = (void (*)(int*, char*))wire_thread_directive;

    if (!h->proxy) h->proxy = ThunkProxy_create();
    ThunkProxy *p = h->proxy;
    
    wt->step_safety_epoch = (void(*)(void))ThunkProxy_emit_baked(p, (void*)h->logic_epoch, 1, wt);
    wt->step_safety_state = (void(*)(void))ThunkProxy_emit_baked(p, (void*)h->logic_state, 1, wt);
    wt->step_executor_directive = (void(*)(char*))ThunkProxy_emit_mixed(p, (void*)h->logic_directive, wt);
    wt->provenance = (void(*)(void))ThunkProxy_emit_baked(p, (void*)h->logic_provenance, 1, wt);

    wt->step_executor_directive("START_INTENSITY_DECAY_LOOP");

    // 4. Resonance Loop: Runs until logic_epoch neutralizes the wavefront
    int cycle = 0;
    while (!wt->finished && cycle < 20) {
        printf("\n--- RESONANCE CYCLE %d ---\n", cycle++);
        HELMHOLTZ_RESONANCE_LIST(THREAD_RESONANCE_X, wt)
    }

    printf("\nFinalizing and Freeing wt (Post-Neutralization)...\n");
    lau_free(wt);
    printf("TSFi Wire Thread Intensity Test Complete.\n");
        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}
