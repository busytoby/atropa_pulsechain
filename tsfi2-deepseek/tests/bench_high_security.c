#include "tsfi_dys_math.h"
#include "lau_memory.h"
#include "tsfi_io.h"
#include <stdio.h>
#include <x86intrin.h>
#include <stdlib.h>
#include <time.h>

static inline uint64_t get_ns() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + ts.tv_nsec;
}

int main() {
    tsfi_io_printf(stdout, "=== TSFi High Security Benchmark (Dysnomia Math) ===\n");
    
    // 1. DYSNOMIA Generation
    uint64_t start = get_ns();
    TSFiBigInt* d = DYSNOMIA();
    uint64_t end = get_ns();
    tsfi_io_printf(stdout, "DYSNOMIA: %.2f us\n", (end-start)/1000.0);
    tsfi_bn_free(d);
    
    // 2. SLOPE (s = mx+b)
    start = get_ns();
    TSFiBigInt* s = SLOPE();
    end = get_ns();
    tsfi_io_printf(stdout, "SLOPE:    %.2f us\n", (end-start)/1000.0);
    tsfi_bn_free(s);
    
    // 3. LOVE (l = ms - 99)
    start = get_ns();
    TSFiBigInt* l = LOVE();
    end = get_ns();
    tsfi_io_printf(stdout, "LOVE:     %.2f us\n", (end-start)/1000.0);
    tsfi_bn_free(l);
    
    // 4. GAIN (g = m(my+b) - 49)
    start = get_ns();
    TSFiBigInt* g = GAIN();
    end = get_ns();
    tsfi_io_printf(stdout, "GAIN:     %.2f us\n", (end-start)/1000.0);
    tsfi_bn_free(g);
    
    // 5. Throughput Test (Repeated SLOPE)
    int N = 10000;
    start = get_ns();
    uint64_t start_tsc = __rdtsc();
    for(int i=0; i<N; i++) {
        TSFiBigInt* tmp = SLOPE();
        tsfi_bn_free(tmp);
    }
    uint64_t end_tsc = __rdtsc();
    end = get_ns();
    tsfi_io_printf(stdout, "Throughput (SLOPE): %.2f ops/ms\n", N / ((end-start)/1000000.0));

    double cycles_per_op = (double)(end_tsc - start_tsc) / N;
    tsfi_io_printf(stdout, "Cycles/Op (SLOPE): %.2f\n", cycles_per_op);

        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}
