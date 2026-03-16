#include "lau_memory.h"
#include "lau_thunk.h"
#include "tsfi_types.h"
#include "tsfi_trace.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <pthread.h>
#include <time.h>
#include <x86intrin.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>

#define NUM_THREADS 100
#define NUM_ITERATIONS 10000

// --- Pthreads Baseline ---

typedef struct {
    int id;
    int *shared_counter;
    pthread_mutex_t *mutex;
} PthreadArg;

void* pthread_worker(void* arg) {
    PthreadArg *a = (PthreadArg*)arg;
    for (int i = 0; i < NUM_ITERATIONS / NUM_THREADS; i++) {
        pthread_mutex_lock(a->mutex);
        (*a->shared_counter)++;
        pthread_mutex_unlock(a->mutex);
    }
    return NULL;
}

// --- TSFi Wire Thread (Resonance) ---

DEFINE_MAPPED_STRUCT(LauWireThread,
    int id;
    int *res_counter;
    double current_intensity;
    bool finished;
)

static void wire_thread_resume(void *obj) {
    LauWireThread *wt = (LauWireThread *)obj;
    if (wt->current_intensity > 0.0) {
        (*wt->res_counter)++;
        wt->current_intensity -= 0.01; // Decay to 0 after 100 iterations
    } else {
        wt->finished = true;
    }
}

static void wire_thread_yield_check(void *obj) {
    (void)obj;
}

static void wire_thread_provenance_check(void *obj) {
    (void)obj;
}

static inline uint64_t rdtsc_val() {
    unsigned int lo, hi;
    __asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
    return ((uint64_t)hi << 32) | lo;
}

static ThunkProxy* ThunkProxy_create_large(size_t size) {
    size_t pg = sysconf(_SC_PAGESIZE);
    if (size < pg) size = pg;
    size = (size + pg - 1) & ~(pg - 1);

    ThunkProxy *proxy = (ThunkProxy *)lau_malloc(sizeof(ThunkProxy));
    proxy->thunk_pool = (uint8_t *)lau_memalign(pg, size);
    mprotect(proxy->thunk_pool, size, PROT_READ | PROT_WRITE | PROT_EXEC);
    memset(proxy->thunk_pool, 0, size);
    proxy->thunk_cursor = proxy->thunk_pool;
    proxy->pool_size = size;
    return proxy;
}

int main() {
    printf("TSFi Wire Thread (Intensity Cleanup) vs Pthreads Benchmark\n");
    printf("Threads: %d, Target Iterations: %d\n\n", NUM_THREADS, NUM_ITERATIONS);

    // 1. Pthreads Benchmark
    int pthread_counter = 0;
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_t threads[NUM_THREADS];
    PthreadArg args[NUM_THREADS];

    uint64_t start = rdtsc_val();
    for (int i = 0; i < NUM_THREADS; i++) {
        args[i].id = i;
        args[i].shared_counter = &pthread_counter;
        args[i].mutex = &mutex;
        pthread_create(&threads[i], NULL, pthread_worker, &args[i]);
    }
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    uint64_t end = rdtsc_val();
    uint64_t pthread_cycles = end - start;
    printf("Pthreads (Mutex) Time: %lu cycles (Counter: %d)\n", pthread_cycles, pthread_counter);

    // 2. TSFi Resonance Benchmark
    int tsfi_counter = 0;
    LauWireThread *wts[NUM_THREADS];
    ThunkProxy *tp = ThunkProxy_create_large(128*1024);

    for (int i = 0; i < NUM_THREADS; i++) {
        wts[i] = (LauWireThread *)lau_malloc_wired(sizeof(LauWireThread));
        wts[i]->id = i;
        wts[i]->res_counter = &tsfi_counter;
        wts[i]->current_intensity = 1.0;
        wts[i]->finished = false;

        LauSystemHeader *h = (LauSystemHeader *)((char *)wts[i] - offsetof(LauSystemHeader, payload));
        h->logic_epoch = (void (*)(int*))wire_thread_resume;
        h->logic_state = (bool (*)(void*))wire_thread_yield_check;
        h->logic_provenance = (void (*)(void*))wire_thread_provenance_check;

        if (!h->proxy) h->proxy = tp; 
        
        wts[i]->step_safety_epoch = (void(*)(void))ThunkProxy_emit_baked(tp, (void*)h->logic_epoch, 1, wts[i]);
        wts[i]->step_safety_state = (void(*)(void))ThunkProxy_emit_baked(tp, (void*)h->logic_state, 1, wts[i]);
        wts[i]->provenance = (void(*)(void))ThunkProxy_emit_baked(tp, (void*)h->logic_provenance, 1, wts[i]);
    }

    start = rdtsc_val();
    bool all_finished = false;
    while (!all_finished) {
        all_finished = true;
        for (int j = 0; j < NUM_THREADS; j++) {
            if (!wts[j]->finished) {
                wts[j]->provenance();
                wts[j]->step_safety_state();
                wts[j]->step_safety_epoch();
                all_finished = false;
            }
        }
    }
    end = rdtsc_val();
    uint64_t tsfi_cycles = end - start;
    printf("TSFi (Resonance) Time: %lu cycles (Counter: %d)\n", tsfi_cycles, tsfi_counter);

    double speedup = (double)pthread_cycles / tsfi_cycles;
    printf("\nSpeedup: %.2f faster than pthreads\n", speedup);
    printf("Per-Switch Latency (Inc. Decay/Cleanup): %.2f cycles\n", (double)tsfi_cycles / NUM_ITERATIONS);

    for (int i = 0; i < NUM_THREADS; i++) {
        LauSystemHeader *h = (LauSystemHeader *)((char *)wts[i] - offsetof(LauSystemHeader, payload));
        h->proxy = NULL; 
        lau_free(wts[i]);
    }
    ThunkProxy_destroy(tp);

        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}
