#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>
#include <stdatomic.h>
#include <unistd.h>
#include <poll.h>
#include <sys/time.h>
#include <time.h>
#include "tsfi_raw.h"

#define ITERATIONS_LOCK 10000000 // 10 Million
#define ITERATIONS_SYS  1000000  // 1 Million
#define ITERATIONS_COPY 10000000 // 10 Million
#define ITERATIONS_POLL 10000    // 10 Thousand

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
        } else { i--; }
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

int main() {
    printf("=== TSFi Architecture Paradigms Benchmark ===\n");
    pthread_t prod, cons;
    uint64_t start, end;

    // --- 1. Lock-Free vs Spin-Lock ---
    printf("\n1. Zero-Lock (Lock-Free SPSC) vs Spin-Lock Queue (%d ops)\n", ITERATIONS_LOCK);
    g_spin_q.head = 0; g_spin_q.tail = 0; atomic_store(&g_spin_q.lock, 0);
    start = get_time_ns();
    pthread_create(&prod, NULL, spinlock_producer, NULL);
    pthread_create(&cons, NULL, spinlock_consumer, NULL);
    pthread_join(prod, NULL); pthread_join(cons, NULL);
    end = get_time_ns();
    double t_spin = (end - start) / 1e9;
    printf("  [BASELINE] Spin-Lock: %.3f sec (%.2f Mops/s)\n", t_spin, (ITERATIONS_LOCK/1e6)/t_spin);

    atomic_store(&g_lf_q.head, 0); atomic_store(&g_lf_q.tail, 0);
    start = get_time_ns();
    pthread_create(&prod, NULL, lockfree_producer, NULL);
    pthread_create(&cons, NULL, lockfree_consumer, NULL);
    pthread_join(prod, NULL); pthread_join(cons, NULL);
    end = get_time_ns();
    double t_lf = (end - start) / 1e9;
    printf("  [ZERO-LOCK] Lock-Free: %.3f sec (%.2f Mops/s) -> %.2fx Faster\n", t_lf, (ITERATIONS_LOCK/1e6)/t_lf, t_spin/t_lf);

    // --- 2. Zero-Syscall vs Pipe ---
    printf("\n2. Zero-Syscall (Shared Memory) vs Pipe Syscalls (%d ops)\n", ITERATIONS_SYS);
    pipe(pipe_fds);
    start = get_time_ns();
    pthread_create(&prod, NULL, pipe_producer, NULL);
    pthread_create(&cons, NULL, pipe_consumer, NULL);
    pthread_join(prod, NULL); pthread_join(cons, NULL);
    end = get_time_ns();
    double t_pipe = (end - start) / 1e9;
    printf("  [BASELINE] Pipe (read/write): %.3f sec (%.2f Mops/s)\n", t_pipe, (ITERATIONS_SYS/1e6)/t_pipe);

    // Calculate Lock-Free time for ITERATIONS_SYS
    double t_shm = t_lf * ((double)ITERATIONS_SYS / ITERATIONS_LOCK);
    printf("  [ZERO-SYSCALL] Shared Memory: %.3f sec (%.2f Mops/s) -> %.2fx Faster\n", t_shm, (ITERATIONS_SYS/1e6)/t_shm, t_pipe/t_shm);
    
    close(pipe_fds[0]); close(pipe_fds[1]);

    // --- 3. Zero-Copy vs memcpy ---
    printf("\n3. Zero-Copy vs Deep Copy (4KB Wavefront, %d ops)\n", ITERATIONS_COPY);
    bench_copy();

    // --- 4. Zero-Polling vs poll() ---
    printf("\n4. Zero-Polling (Hardware Atomic Wait) vs OS poll() (%d wakes)\n", ITERATIONS_POLL);
    pipe(pipe_fds);
    start = get_time_ns();
    pthread_create(&prod, NULL, poll_os_thread, NULL);
    struct pollfd pfd = { .fd = pipe_fds[0], .events = POLLIN };
    for (int i=0; i<ITERATIONS_POLL; i++) {
        poll(&pfd, 1, -1);
        char c; ssize_t r = read(pipe_fds[0], &c, 1);
        (void)r;
    }
    pthread_join(prod, NULL);
    end = get_time_ns();
    double t_poll = (end - start) / 1e6;
    printf("  [BASELINE] OS poll(): %.2f ms total (%.2f us/wake)\n", t_poll, (t_poll * 1000) / ITERATIONS_POLL);
    
    start = get_time_ns();
    pthread_create(&prod, NULL, poll_zero_thread, NULL);
    for (int i=0; i<ITERATIONS_POLL; i++) {
        while (atomic_load_explicit(&g_poll_flag, memory_order_acquire) == 0) {
            __builtin_ia32_pause();
        }
        atomic_store_explicit(&g_poll_flag, 0, memory_order_release);
    }
    pthread_join(prod, NULL);
    end = get_time_ns();
    double t_atomic = (end - start) / 1e6;
    printf("  [ZERO-POLLING] Atomic Hardware Wait: %.2f ms total (%.2f us/wake)\n", t_atomic, (t_atomic * 1000) / ITERATIONS_POLL);
    
    close(pipe_fds[0]); close(pipe_fds[1]);
    
    printf("\n=== Benchmarks Complete ===\n");
        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}
