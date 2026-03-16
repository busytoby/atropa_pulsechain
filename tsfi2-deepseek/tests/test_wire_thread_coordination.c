#include "lau_memory.h"
#include "lau_thunk.h"
#include "tsfi_types.h"
#include "tsfi_trace.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <string.h>

// 1. Define the Mapped Struct for our Wire Thread
DEFINE_MAPPED_STRUCT(LauWireThread,
    int thread_id;
    int work_counter;
    double current_intensity;
    bool finished;
    
    // Coordination primitives
    void *join_target;          // Thread to wait for (cast to LauWireThread*)
    _Atomic int *cond_var;      // Condition variable to wait on
    int cond_expected;          // Value to wake up on
)

// 2. Implementation of the Resonating Logic

static void wire_thread_resume(void *obj) {
    LauWireThread *wt = (LauWireThread *)obj;
    
    // Simulated work
    wt->work_counter++;
    printf("[THREAD %d] Working... (Count: %d)\n", wt->thread_id, wt->work_counter);
    
    wt->current_intensity -= 0.5; 
    if (wt->current_intensity <= 0.0) {
        printf("[THREAD %d] Finished work.\n", wt->thread_id);
        wt->finished = true;
    }
}

static void wire_thread_yield_check(void *obj) {
    LauWireThread *wt = (LauWireThread *)obj;
    
    if (wt->finished) {
        return; // Already finished
    }

    // --- pthread_join equivalent ---
    if (wt->join_target != NULL) {
        LauWireThread *target = (LauWireThread *)wt->join_target;
        if (!target->finished) {
            printf("[THREAD %d] JOIN WAIT: Yielding until Thread %d finishes.\n", 
                   wt->thread_id, target->thread_id);
            // Skip the next logic_epoch by returning here
            return; 
        } else {
            printf("[THREAD %d] JOIN WAKE: Thread %d finished.\n", 
                   wt->thread_id, target->thread_id);
            wt->join_target = NULL; // Join complete
        }
    }

    // --- pthread_cond_wait equivalent ---
    if (wt->cond_var != NULL) {
        if (atomic_load(wt->cond_var) != wt->cond_expected) {
            printf("[THREAD %d] COND WAIT: Yielding until condition meets %d.\n", 
                   wt->thread_id, wt->cond_expected);
            return;
        } else {
            printf("[THREAD %d] COND WAKE: Condition met!\n", wt->thread_id);
            wt->cond_var = NULL; // Condition met
        }
    }
}

static void wire_thread_provenance_check(void *obj) {
    (void)obj; // No preemption in this test
}

// 3. Execution Engine
#undef HELMHOLTZ_RESONANCE_LIST
#define HELMHOLTZ_RESONANCE_LIST(X, wt) \
    X(wt, provenance);    \
    X(wt, safety_state);  \
    X(wt, safety_epoch);

#define THREAD_RESONANCE_X(wt, func) \
    do { \
        if (strcmp(#func, "provenance") == 0) wt->provenance(); \
        else if (strcmp(#func, "safety_state") == 0) wt->step_safety_state(); \
        else if (strcmp(#func, "safety_epoch") == 0) { \
            if(!wt->finished && wt->join_target == NULL && wt->cond_var == NULL) { \
                wt->step_safety_epoch(); \
            } \
        } \
    } while(0)

LauWireThread* create_thread(int id, double intensity, ThunkProxy *tp) {
    LauWireThread *wt = (LauWireThread *)lau_malloc_wired(sizeof(LauWireThread));
    wt->thread_id = id;
    wt->work_counter = 0;
    wt->current_intensity = intensity;
    wt->finished = false;
    wt->join_target = NULL;
    wt->cond_var = NULL;

    LauSystemHeader *h = (LauSystemHeader *)((char *)wt - offsetof(LauSystemHeader, payload));
    h->logic_epoch = (void (*)(int*))wire_thread_resume;
    h->logic_state = (void (*)(bool*))wire_thread_yield_check;
    h->logic_provenance = (void (*)(void*))wire_thread_provenance_check;

    if (!h->proxy) h->proxy = tp; 
    
    wt->step_safety_epoch = (void(*)(void))ThunkProxy_emit_baked(tp, (void*)h->logic_epoch, 1, wt);
    wt->step_safety_state = (void(*)(void))ThunkProxy_emit_baked(tp, (void*)h->logic_state, 1, wt);
    wt->provenance = (void(*)(void))ThunkProxy_emit_baked(tp, (void*)h->logic_provenance, 1, wt);
    return wt;
}

int main() {
    printf("Starting TSFi Wire Thread Coordination Test (Join/Cond)...\n");

    ThunkProxy *tp = ThunkProxy_create();
    
    LauWireThread *t1 = create_thread(1, 1.0, tp); // Takes 2 compute cycles to finish
    
    LauWireThread *t2 = create_thread(2, 0.5, tp);
    t2->join_target = t1; 

    _Atomic int shared_cond = 0;
    LauWireThread *t3 = create_thread(3, 0.5, tp);
    t3->cond_var = &shared_cond;
    t3->cond_expected = 1;

    LauWireThread *t4 = create_thread(4, 0.5, tp);

    LauWireThread *threads[] = {t1, t2, t3, t4};

    for (int i = 0; i < 5; i++) {
        printf("\n--- RESONANCE CYCLE %d ---\n", i);
        
        if (i == 1) {
            printf("[MAIN] Setting shared condition to 1...\n");
            atomic_store(&shared_cond, 1);
        }

        for (int t = 0; t < 4; t++) {
            HELMHOLTZ_RESONANCE_LIST(THREAD_RESONANCE_X, threads[t]);
        }
    }

    for (int t = 0; t < 4; t++) {
        LauSystemHeader *h = (LauSystemHeader *)((char *)threads[t] - offsetof(LauSystemHeader, payload));
        h->proxy = NULL;
        lau_free(threads[t]);
    }
    ThunkProxy_destroy(tp);

    printf("\nTSFi Wire Thread Coordination Test Complete.\n");
        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}
