#define _GNU_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <sched.h>
#include <pthread.h>
#include <sys/mman.h>
#include <x86intrin.h>
#include "tsfi_raw.h"

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

#define ITERATIONS 1000000
#define YIELD_ITERATIONS 50000
#define CLONE_ITERATIONS 10000

static double get_time_ms() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000.0 + ts.tv_nsec / 1000000.0;
}

void bench_glibc_clock() {
    struct timespec ts;
    double start = get_time_ms();
    for (int i = 0; i < ITERATIONS; i++) {
        clock_gettime(CLOCK_MONOTONIC, &ts);
    }
    double end = get_time_ms();
    printf("[BENCH] glibc clock_gettime: %.2f ms (%.2f ns/iter)\n", end - start, ((end - start) * 1000000.0) / ITERATIONS);
}

void bench_raw_clock() {
    struct timespec ts;
    double start = get_time_ms();
    for (int i = 0; i < ITERATIONS; i++) {
        tsfi_raw_clock_gettime(1, &ts); // 1 = CLOCK_MONOTONIC
    }
    double end = get_time_ms();
    printf("[BENCH] tsfi_raw_clock_gettime: %.2f ms (%.2f ns/iter)\n", end - start, ((end - start) * 1000000.0) / ITERATIONS);
}

void bench_raw_tsc() {
    double start = get_time_ms();
    for (int i = 0; i < ITERATIONS; i++) {
        volatile unsigned long long tsc = __rdtsc();
        (void)tsc;
    }
    double end = get_time_ms();
    printf("[BENCH] tsfi_raw_tsc (__rdtsc): %.2f ms (%.2f ns/iter)\n", end - start, ((end - start) * 1000000.0) / ITERATIONS);
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

int main() {
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
    
    // Explicitly unmap global tracking arrays
    extern void lau_registry_teardown(void);
    lau_registry_teardown();
    
    // Mathematically certify strictly zero geometric memory leaks
    extern void lau_report_memory_metrics(void);
    printf("\n[TEST] Expecting PERFECTLY ZERO LEAKS...\n");
    lau_report_memory_metrics();
    
    
    extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}