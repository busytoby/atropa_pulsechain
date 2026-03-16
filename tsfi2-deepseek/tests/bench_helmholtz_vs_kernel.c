#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdatomic.h>
#include <pthread.h>
#include <sys/eventfd.h>
#include <unistd.h>
#include <time.h>
#include "tsfi_raw.h" 

#define ITERATIONS 100000 // 100k Operations

static inline uint64_t get_time_ns() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + ts.tv_nsec;
}

// ==========================================
// 1. KERNEL ASYNC (eventfd / epoll analog)
// ==========================================
int efd;

void* eventfd_producer(void* arg) {
    (void)arg;
    uint64_t val = 1;
    for (int i=0; i<ITERATIONS; i++) {
        if (write(efd, &val, sizeof(uint64_t)) != sizeof(uint64_t)) i--;
    }
    return NULL;
}

void* eventfd_consumer(void* arg) {
    (void)arg;
    uint64_t val;
    for (int i=0; i<ITERATIONS; i++) {
        if (read(efd, &val, sizeof(uint64_t)) != sizeof(uint64_t)) i--;
    }
    return NULL;
}

// ==========================================
// 2. TSFi HELMHOLTZ (Lock-Free Log + Spin)
// ==========================================
#define BUFFER_SIZE 4096
typedef struct {
    char buffer[BUFFER_SIZE];
    _Atomic uint32_t head;
    _Atomic uint32_t tail;
} HelmholtzLog;

static HelmholtzLog hl;

void* helmholtz_producer(void* arg) {
    (void)arg;
    for (int i=0; i<ITERATIONS; i++) {
        uint32_t current_head = atomic_load_explicit(&hl.head, memory_order_relaxed);
        uint32_t next = (current_head + 1) % BUFFER_SIZE;
        
        while (next == atomic_load_explicit(&hl.tail, memory_order_acquire)) {
            __builtin_ia32_pause(); // Queue full
        }
        
        hl.buffer[current_head] = 1;
        atomic_store_explicit(&hl.head, next, memory_order_release);
    }
    return NULL;
}

void* helmholtz_consumer(void* arg) {
    (void)arg;
    int consumed = 0;
    while (consumed < ITERATIONS) {
        uint32_t current_tail = atomic_load_explicit(&hl.tail, memory_order_relaxed);
        
        // This while loop represents the HELMHOLTZ_RESONANCE_LIST continuously pulsing
        while (current_tail == atomic_load_explicit(&hl.head, memory_order_acquire)) {
            __builtin_ia32_pause();
        }
        
        volatile char c = hl.buffer[current_tail];
        (void)c;
        atomic_store_explicit(&hl.tail, (current_tail + 1) % BUFFER_SIZE, memory_order_release);
        consumed++;
    }
    return NULL;
}

int main() {
    printf("=== TSFi Helmholtz vs Kernel Async (io_uring/eventfd analog) ===\n");
    printf("Iterations: %d\n\n", ITERATIONS);
    fflush(stdout);

    uint64_t start, end;

    // --- 1. Kernel EventFD ---
    pthread_t p, c;
    efd = eventfd(0, EFD_SEMAPHORE);
    start = get_time_ns();
    pthread_create(&p, NULL, eventfd_producer, NULL);
    pthread_create(&c, NULL, eventfd_consumer, NULL);
    pthread_join(p, NULL);
    pthread_join(c, NULL);
    end = get_time_ns();
    double t_efd = (end - start) / 1000000000.0;
    printf("[KERNEL] eventfd (Syscall Blocking): %.3f sec (%.2f Mops/s)\n", t_efd, (ITERATIONS/1000000.0)/t_efd);
    fflush(stdout);
    close(efd);

    // --- 2. TSFi Helmholtz Emulation ---
    atomic_store(&hl.head, 0);
    atomic_store(&hl.tail, 0);
    start = get_time_ns();
    pthread_create(&p, NULL, helmholtz_producer, NULL);
    pthread_create(&c, NULL, helmholtz_consumer, NULL);
    pthread_join(p, NULL);
    pthread_join(c, NULL);
    end = get_time_ns();
    double t_helm = (end - start) / 1000000000.0;
    printf("[TSFi] Helmholtz Resonance (Zero-Syscall): %.3f sec (%.2f Mops/s)\n", t_helm, (ITERATIONS/1000000.0)/t_helm);

    printf("\n[RESULT] TSFi Architecture is %.2fx faster than Kernel Async signaling.\n", t_efd / t_helm);
    fflush(stdout);
        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}
