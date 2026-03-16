#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <sched.h>
#include <time.h>
#include <unistd.h>
#include <stdint.h>
#include <linux/sched.h>

#define ITERATIONS 50000

static double get_time_ms() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000.0 + ts.tv_nsec / 1000000.0;
}

static inline int tsfi_raw_sched_setaffinity(pid_t pid, size_t cpusetsize, const cpu_set_t *mask) {
    long ret;
    __asm__ volatile (
        "syscall\n\t"
        : "=a" (ret)
        : "a" (203), "D" (pid), "S" (cpusetsize), "d" (mask)
        : "rcx", "r11", "memory"
    );
    if (ret < 0) return -1;
    return 0;
}

void bench_glibc_affinity() {
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(0, &cpuset);

    double start = get_time_ms();
    for (int i = 0; i < ITERATIONS; i++) {
        sched_setaffinity(0, sizeof(cpu_set_t), &cpuset);
    }
    double end = get_time_ms();
    printf("[BENCH] glibc sched_setaffinity: %.2f ms (%.6f ms/iter)\n", end - start, (end - start) / ITERATIONS);
}

void bench_tsfi_raw_affinity() {
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(0, &cpuset);

    double start = get_time_ms();
    for (int i = 0; i < ITERATIONS; i++) {
        tsfi_raw_sched_setaffinity(0, sizeof(cpu_set_t), &cpuset);
    }
    double end = get_time_ms();
    printf("[BENCH] tsfi_raw_sched_setaffinity: %.2f ms (%.6f ms/iter)\n", end - start, (end - start) / ITERATIONS);
}

int main() {
    printf("=== Syscall 203 (sched_setaffinity) Benchmark ===\n");
    bench_glibc_affinity();
    bench_tsfi_raw_affinity();
        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}
