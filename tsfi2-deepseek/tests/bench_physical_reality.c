#define _GNU_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include "tsfi_wave_any.h"
#include "lau_memory.h"
#include "tsfi_operator.h"
#include "tsfi_wiring.h"
#include <unistd.h>
#include <sched.h>
#include <pthread.h>
#include <sys/mman.h>
#include <x86intrin.h>
#include "tsfi_raw.h"
#include <linux/sched.h>
#include "lau_thunk.h"
#include "tsfi_types.h"
#include "tsfi_trace.h"
#include <stdatomic.h>
#include <stdbool.h>
#include <poll.h>
#include <sys/time.h>

// For CLONE_ flags
#include <linux/sched.h> 

#ifndef CLONE_THREAD
#define CLONE_VM	0x00000100
#define CLONE_FS	0x00000200
#define CLONE_FILES	0x00000400
#define CLONE_SIGHAND	0x00000800
#define CLONE_THREAD	0x00010000
#define CLONE_SYSVSEM	0x00040000
#define CLONE_SETTLS	0x00080000
#define CLONE_PARENT_SETTID	0x00100000
#define CLONE_CHILD_CLEARTID	0x00200000
#endif

#define ITERATIONS_RAW 10000
#define YIELD_ITERATIONS 5000
#define CLONE_ITERATIONS 1000

static double get_time_ms() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000.0 + ts.tv_nsec / 1000000.0;
}

void bench_glibc_clock() {
    struct timespec ts;
    double start = get_time_ms();
    for (int i = 0; i < ITERATIONS_RAW; i++) {
        clock_gettime(CLOCK_MONOTONIC, &ts);
    }
    double end = get_time_ms();
    printf("[BENCH] glibc clock_gettime: %.2f ms (%.2f ns/iter)\n", end - start, ((end - start) * 1000000.0) / ITERATIONS_RAW);
}

void bench_raw_clock() {
    struct timespec ts;
    double start = get_time_ms();
    for (int i = 0; i < ITERATIONS_RAW; i++) {
        tsfi_raw_clock_gettime(1, &ts); // 1 = CLOCK_MONOTONIC
    }
    double end = get_time_ms();
    printf("[BENCH] tsfi_raw_clock_gettime: %.2f ms (%.2f ns/iter)\n", end - start, ((end - start) * 1000000.0) / ITERATIONS_RAW);
}

void bench_raw_tsc() {
    double start = get_time_ms();
    for (int i = 0; i < ITERATIONS_RAW; i++) {
        volatile unsigned long long tsc = __rdtsc();
        (void)tsc;
    }
    double end = get_time_ms();
    printf("[BENCH] tsfi_raw_tsc (__rdtsc): %.2f ms (%.2f ns/iter)\n", end - start, ((end - start) * 1000000.0) / ITERATIONS_RAW);
}

void bench_glibc_yield() {
    double start = get_time_ms();
    for (int i = 0; i < YIELD_ITERATIONS; i++) {
        usleep(0); // Forces minimum POSIX yield bound
    }
    double end = get_time_ms();
    printf("[BENCH] glibc usleep(0): %.2f ms (%.2f ns/iter)\n", end - start, ((end - start) * 1000000.0) / YIELD_ITERATIONS);
}

void bench_raw_yield() {
    double start = get_time_ms();
    for (int i = 0; i < YIELD_ITERATIONS; i++) {
        tsfi_raw_usleep(0); // Natively mapped nanosleep(0)
    }
    double end = get_time_ms();
    printf("[BENCH] tsfi_raw_usleep(0): %.2f ms (%.2f ns/iter)\n", end - start, ((end - start) * 1000000.0) / YIELD_ITERATIONS);
}

void* noop_pthread(void *arg) {
    (void)arg;
    return NULL;
}

int noop_raw_thread(void *arg) {
    (void)arg;
    return 0;
}

void bench_glibc_thread() {
    double start = get_time_ms();
    for (int i = 0; i < CLONE_ITERATIONS; i++) {
        pthread_t t;
        pthread_create(&t, NULL, noop_pthread, NULL);
        pthread_join(t, NULL);
    }
    double end = get_time_ms();
    printf("[BENCH] glibc pthread_create: %.2f ms (%.2f ns/iter)\n", end - start, ((end - start) * 1000000.0) / CLONE_ITERATIONS);
}

void bench_raw_clone_thread() {
    void *stack = mmap(NULL, 2 * 1024 * 1024, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK, -1, 0);
    void *stack_top = (char*)stack + 2 * 1024 * 1024;
    
    void *tls = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    
    double start = get_time_ms();
    for (int i = 0; i < CLONE_ITERATIONS; i++) {
        int ptid = 0;
        int ctid = 0;
        
        int flags = CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND | CLONE_THREAD | CLONE_SYSVSEM | CLONE_SETTLS | CLONE_PARENT_SETTID | CLONE_CHILD_CLEARTID;
        
        pid_t pid = tsfi_raw_clone_thread(noop_raw_thread, stack_top, flags, NULL, &ptid, tls, &ctid);
        if (pid > 0) {
            // Geometrically poll the child clearing its TID pointer upon exit natively
            while (ctid != 0) {
                tsfi_raw_futex_wait(&ctid, ctid);
            }
        }
    }
    double end = get_time_ms();
    
    munmap(stack, 2 * 1024 * 1024);
    munmap(tls, 4096);
    printf("[BENCH] tsfi_raw_clone_thread: %.2f ms (%.2f ns/iter)\n", end - start, ((end - start) * 1000000.0) / CLONE_ITERATIONS);
}

void bench_glibc_affinity() {
    cpu_set_t set;
    CPU_ZERO(&set);
    CPU_SET(0, &set);
    pid_t pid = getpid();
    
    double start = get_time_ms();
    for (int i = 0; i < YIELD_ITERATIONS; i++) {
        sched_setaffinity(pid, sizeof(cpu_set_t), &set);
    }
    double end = get_time_ms();
    printf("[BENCH] glibc sched_setaffinity: %.2f ms (%.2f ns/iter)\n", end - start, ((end - start) * 1000000.0) / YIELD_ITERATIONS);
}

void bench_raw_affinity() {
    tsfi_cpu_set_t set;
    TSFI_CPU_ZERO(&set);
    TSFI_CPU_SET(0, &set);
    pid_t pid = getpid();
    
    double start = get_time_ms();
    for (int i = 0; i < YIELD_ITERATIONS; i++) {
        tsfi_raw_sched_setaffinity(pid, sizeof(tsfi_cpu_set_t), &set);
    }
    double end = get_time_ms();
    printf("[BENCH] tsfi_raw_sched_setaffinity: %.2f ms (%.2f ns/iter)\n", end - start, ((end - start) * 1000000.0) / YIELD_ITERATIONS);
}

#define ITERATIONS_LOCK 100000 // 100 Thousand
#define ITERATIONS_SYS  10000  // 10 Thousand
#define ITERATIONS_COPY 100000 // 100 Thousand
#define ITERATIONS_POLL 1000   // 1 Thousand

#define BUFFER_SIZE 4096

static inline uint64_t get_time_ns() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + ts.tv_nsec;
}

// ==========================================
// 1. Zero-Lock (SPSC) vs Spin-Lock
// ==========================================
typedef struct {
    char buffer[BUFFER_SIZE];
    uint32_t head;
    uint32_t tail;
    _Atomic uint32_t lock;
} SpinLockQueue;

static SpinLockQueue g_spin_q;

void* spinlock_producer(void* arg) {
    (void)arg;
    for (int i = 0; i < ITERATIONS_LOCK; i++) {
        while (atomic_exchange_explicit(&g_spin_q.lock, 1, memory_order_acquire)) {
            __builtin_ia32_pause();
        }
        uint32_t next = (g_spin_q.head + 1) % BUFFER_SIZE;
        if (next != g_spin_q.tail) {
            g_spin_q.buffer[g_spin_q.head] = (char)(i & 0xFF);
            g_spin_q.head = next;
        } else { 
            atomic_store_explicit(&g_spin_q.lock, 0, memory_order_release);
            sched_yield();
            i--;
            continue;
        }
        atomic_store_explicit(&g_spin_q.lock, 0, memory_order_release);
    }
    return NULL;
}

void* spinlock_consumer(void* arg) {
    (void)arg;
    int consumed = 0;
    while (consumed < ITERATIONS_LOCK) {
        while (atomic_exchange_explicit(&g_spin_q.lock, 1, memory_order_acquire)) {
            __builtin_ia32_pause();
        }
        if (g_spin_q.head != g_spin_q.tail) {
            g_spin_q.tail = (g_spin_q.tail + 1) % BUFFER_SIZE;
            consumed++;
        } else {
            atomic_store_explicit(&g_spin_q.lock, 0, memory_order_release);
            sched_yield();
            continue;
        }
        atomic_store_explicit(&g_spin_q.lock, 0, memory_order_release);
    }
    return NULL;
}

typedef struct {
    char buffer[BUFFER_SIZE];
    _Atomic uint32_t head;
    _Atomic uint32_t tail;
} LockFreeQueue;

static LockFreeQueue g_lf_q;

void* lockfree_producer(void* arg) {
    (void)arg;
    for (int i = 0; i < ITERATIONS_LOCK; i++) {
        uint32_t current_head = atomic_load_explicit(&g_lf_q.head, memory_order_relaxed);
        uint32_t next = (current_head + 1) % BUFFER_SIZE;
        while (next == atomic_load_explicit(&g_lf_q.tail, memory_order_acquire)) {
            __builtin_ia32_pause();
            sched_yield();
        }
        g_lf_q.buffer[current_head] = (char)(i & 0xFF);
        atomic_store_explicit(&g_lf_q.head, next, memory_order_release);
    }
    return NULL;
}

void* lockfree_consumer(void* arg) {
    (void)arg;
    int consumed = 0;
    while (consumed < ITERATIONS_LOCK) {
        uint32_t current_tail = atomic_load_explicit(&g_lf_q.tail, memory_order_relaxed);
        while (current_tail == atomic_load_explicit(&g_lf_q.head, memory_order_acquire)) {
            __builtin_ia32_pause();
            sched_yield();
        }
        atomic_store_explicit(&g_lf_q.tail, (current_tail + 1) % BUFFER_SIZE, memory_order_release);
        consumed++;
    }
    return NULL;
}

// ==========================================
// 2. Zero-Syscall vs Pipe
// ==========================================
int pipe_fds[2];

void* pipe_producer(void* arg) {
    (void)arg;
    char c = 'A';
    for (int i = 0; i < ITERATIONS_SYS; i++) {
        if (write(pipe_fds[1], &c, 1) <= 0) i--;
    }
    return NULL;
}

void* pipe_consumer(void* arg) {
    (void)arg;
    char c;
    for (int i = 0; i < ITERATIONS_SYS; i++) {
        if (read(pipe_fds[0], &c, 1) <= 0) i--;
    }
    return NULL;
}

// ==========================================
// 3. Zero-Copy vs memcpy
// ==========================================
void bench_copy() {
    volatile char *src = malloc(4096);
    volatile char *dst = malloc(4096);
    src[0] = 'X';
    
    uint64_t start = get_time_ns();
    for(int i=0; i<ITERATIONS_COPY; i++) {
        memcpy((void*)dst, (void*)src, 4096);
        // Force evaluation
        __asm__ __volatile__("" : : "m" (dst[0]) : "memory");
    }
    uint64_t end = get_time_ns();
    printf("  [BASELINE] Deep Copy 4KB (10M iter): %.2f ms\n", (end - start)/1000000.0);
    free((void*)src); free((void*)dst);
    
    char *ptr1 = malloc(4096);
    char *ptr2 = malloc(4096);
    start = get_time_ns();
    for(int i=0; i<ITERATIONS_COPY; i++) {
        // Simulating pointer swap
        char *tmp = ptr1;
        ptr1 = ptr2;
        ptr2 = tmp;
        __asm__ __volatile__("" : "+r" (ptr1), "+r" (ptr2) : : "memory");
    }
    end = get_time_ns();
    printf("  [ZERO-COPY] Pointer Swap (10M iter): %.2f ms\n", (end - start)/1000000.0);
    free(ptr1); free(ptr2);
}

// ==========================================
// 4. Zero-Polling vs OS Poll()
// ==========================================
_Atomic uint32_t g_poll_flag = 0;
void* poll_os_thread(void* arg) {
    (void)arg;
    for (int i=0; i<ITERATIONS_POLL; i++) {
        tsfi_raw_usleep(10); // 10us delay to let consumer sleep in poll()
        char c = 'A';
        ssize_t ret = write(pipe_fds[1], &c, 1);
        (void)ret;
    }
    return NULL;
}

void* poll_zero_thread(void* arg) {
    (void)arg;
    for (int i=0; i<ITERATIONS_POLL; i++) {
        tsfi_raw_usleep(10); // 10us delay to let consumer sleep in pause()
        atomic_store_explicit(&g_poll_flag, 1, memory_order_release);
    }
    return NULL;
}

#define NUM_THREADS 10
#define NUM_ITERATIONS 1000

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

static bool wire_thread_yield_check(void *obj) {
    (void)obj;
    return false;
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

static double get_time_s() {
    struct timespec ts; clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

int main() {
printf("=== TSFi Physical Reality Benchmark: Bijective Wave Field ===\\n");
    size_t count = 1024 * 1024 * 16;
    size_t bytes = count * 16;
    void* field_data = lau_malloc_wired(bytes);
    WaveStream field = {0};
    field.data = field_data;
    field.count = (uint32_t)count;
    field.atom_size = 16;
    field.stride = 16;
    double w0 = get_time_s(); memset(field_data, 0, bytes); double w1 = get_time_s();
    printf("  [Wiring] Field Zero-Init: %.2f GB/s\\n", (bytes / (w1 - w0)) / 1e9);
    TSFiOperator* op = tsfi_op_create("Identity", NULL, NULL, 0);
    int iterations = 100000;
    double d0 = get_time_s();
    for(int i=0; i<iterations; i++) tsfi_op_dispatch(op, NULL, &field, NULL, 0);
    double d1 = get_time_s();
    printf("  [Thunk]  Dispatch Frequency: %.2f MHz | Latency: %.2f ns\\n", (iterations / (d1 - d0)) / 1e6, ((d1 - d0) / iterations) * 1e9);
    lau_free(field_data); tsfi_op_destroy(op, NULL);

// Explicitly boot the global vDSO bounds for fair clock geometry
    extern void tsfi_init_vdso(void);
    tsfi_init_vdso();

    printf("=== TSFi Raw Hardware Interfacing Benchmark ===\n");
    bench_glibc_clock();
    bench_raw_clock();
    bench_raw_tsc();
    
    printf("\n=== Sovereign Thread Spawning Topology (%d iterations) ===\n", CLONE_ITERATIONS);
    bench_glibc_thread();
    bench_raw_clone_thread();
    
    printf("\n=== High-Frequency Thread Yielding (%d iterations) ===\n", YIELD_ITERATIONS);
    bench_glibc_yield();
    bench_raw_yield();
    
    printf("\n=== Thread CPU Pinning Topology (%d iterations) ===\n", YIELD_ITERATIONS);
    bench_glibc_affinity();
    bench_raw_affinity();

    printf("\n=== TSFi Wire Thread (Intensity Cleanup) vs Pthreads Benchmark ===\n");
    
    printf("Threads: %d, Target Iterations: %d\n\n", NUM_THREADS, NUM_ITERATIONS);

    // 1. Pthreads Benchmark
    uint64_t start, end;
    int pthread_counter = 0;
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_t threads[NUM_THREADS];
    PthreadArg args[NUM_THREADS];

    start = rdtsc_val();
    for (int i = 0; i < NUM_THREADS; i++) {
        args[i].id = i;
        args[i].shared_counter = &pthread_counter;
        args[i].mutex = &mutex;
        pthread_create(&threads[i], NULL, pthread_worker, &args[i]);
    }
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    end = rdtsc_val();
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

        if (h->proxy) { ThunkProxy_destroy(h->proxy); }
        h->proxy = tp; 
        
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
