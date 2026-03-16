#include "tsfi_reaction.h"
#include "tsfi_io.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#define NUM_SESSIONS 8

typedef struct {
    TSFiHelmholtzAdductState rod;
    TSFiHelmholtzAdductState cone;
    TSFiBigInt* Prime;
    int session_id;
} HelmholtzSession;

void* run_session(void* arg) {
    HelmholtzSession* s = (HelmholtzSession*)arg;
    
    tsfi_helmholtz_step(&s->rod, NULL);
    tsfi_helmholtz_step(&s->cone, NULL);
    
    tsfi_helmholtz_step(&s->rod, NULL);
    tsfi_helmholtz_step(&s->cone, NULL);
    
    tsfi_helmholtz_step(&s->rod, s->cone.Mu->Mu->Contour);
    tsfi_helmholtz_step(&s->cone, s->rod.Mu->Mu->Contour);
    
    tsfi_helmholtz_step(&s->rod, NULL);
    tsfi_helmholtz_step(&s->cone, NULL);
    
    tsfi_helmholtz_step(&s->rod, s->cone.Mu->Mu->Pole);
    tsfi_helmholtz_step(&s->cone, s->rod.Mu->Mu->Pole);
    
    tsfi_helmholtz_step(&s->rod, NULL);
    tsfi_helmholtz_step(&s->cone, NULL);
    
    tsfi_helmholtz_step(&s->rod, s->cone.Mu->Mu->Foundation);
    tsfi_helmholtz_step(&s->cone, s->rod.Mu->Mu->Foundation);
    
    tsfi_helmholtz_step(&s->rod, NULL);
    tsfi_helmholtz_step(&s->cone, NULL);
    
    tsfi_helmholtz_step(&s->rod, NULL);
    tsfi_helmholtz_step(&s->cone, NULL);

    if (s->rod.current_epoch != HELMHOLTZ_EPOCH_DONE) return (void*)1;
    return NULL;
}

int main() {
    TSFiBigInt* Prime = tsfi_bn_alloc();
    tsfi_bn_set_u64(Prime, 953467954114363ULL);

    HelmholtzSession sessions[NUM_SESSIONS];
    pthread_t threads[NUM_SESSIONS];

    tsfi_io_printf(stdout, "[PARALLEL] Launching %d concurrent Helmholtz sessions...\n", NUM_SESSIONS);

    for (int i = 0; i < NUM_SESSIONS; i++) {
        memset(&sessions[i], 0, sizeof(HelmholtzSession));
        sessions[i].Prime = Prime;
        sessions[i].session_id = i;
        sessions[i].rod.Prime = Prime;
        sessions[i].cone.Prime = Prime;
        sessions[i].rod.Xi = tsfi_bn_alloc(); tsfi_bn_randomize(sessions[i].rod.Xi);
        sessions[i].cone.Xi = tsfi_bn_alloc(); tsfi_bn_copy(sessions[i].cone.Xi, sessions[i].rod.Xi);
        sessions[i].rod.Beta = tsfi_bn_alloc(); tsfi_bn_randomize(sessions[i].rod.Beta);
        sessions[i].cone.Beta = tsfi_bn_alloc(); tsfi_bn_randomize(sessions[i].cone.Beta);
        
        pthread_create(&threads[i], NULL, run_session, &sessions[i]);
    }

    for (int i = 0; i < NUM_SESSIONS; i++) {
        void* res;
        pthread_join(threads[i], &res);
        if (res != NULL) {
            tsfi_io_printf(stderr, "[FAIL] Session %d failed.\n", i);
            return 1;
        }
    }

    tsfi_io_printf(stdout, "[SUCCESS] %d parallel Helmholtz sessions verified.\n", NUM_SESSIONS);
        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}
