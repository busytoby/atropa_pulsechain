#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <unistd.h>
#include "tsfi_raw.h"

void test_raw_clock() {
    printf("[TEST] Verifying tsfi_raw_clock_gettime parity...\n");
    struct timespec ts_raw, ts_glibc;
    
    int r1 = tsfi_raw_clock_gettime(1, &ts_raw); // CLOCK_MONOTONIC
    int r2 = clock_gettime(CLOCK_MONOTONIC, &ts_glibc);
    
    assert(r1 == 0);
    assert(r2 == 0);
    
    long diff = (ts_glibc.tv_sec - ts_raw.tv_sec) * 1000000000L + (ts_glibc.tv_nsec - ts_raw.tv_nsec);
    if (diff < 0) diff = -diff;
    
    printf("[INFO] Raw vs Glibc clock diff: %ld ns\n", diff);
    assert(diff < 10000000); // Relaxed to 10ms for CI jitter
    printf("[PASS] Raw clock verified.\n");
}

void test_raw_usleep() {
    printf("[TEST] Verifying tsfi_raw_usleep stall duration...\n");
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    
    tsfi_raw_usleep(100000); // 100ms
    
    clock_gettime(CLOCK_MONOTONIC, &end);
    long elapsed_us = (end.tv_sec - start.tv_sec) * 1000000L + (end.tv_nsec - start.tv_nsec) / 1000L;
    
    printf("[INFO] Raw usleep(100000) took %ld us\n", elapsed_us);
    assert(elapsed_us >= 100000);
    assert(elapsed_us < 120000); // Allow 20ms jitter
    printf("[PASS] Raw usleep verified.\n");
}

void test_cpu_macros() {
    printf("[TEST] Verifying tsfi_cpu_set_t macros...\n");
    tsfi_cpu_set_t s;
    TSFI_CPU_ZERO(&s);
    for(int i=0; i<16; i++) assert(s.__bits[i] == 0);
    
    TSFI_CPU_SET(0, &s);
    assert(s.__bits[0] == 1);
    
    TSFI_CPU_SET(64, &s);
    assert(s.__bits[1] == 1);
    
    TSFI_CPU_SET(1023, &s);
    assert(s.__bits[15] == (1UL << 63));
    
    printf("[PASS] CPU macros verified.\n");
}

int main() {
    printf("=== TSFi Raw Syscall Unit Tests ===\n");
    test_raw_clock();
    test_raw_usleep();
    test_cpu_macros();
    printf("All Raw Syscall Tests Passed!\n");
        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}
