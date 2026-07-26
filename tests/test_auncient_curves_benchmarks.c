#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <time.h>
#include <math.h>
#include "../tsfi2-deepseek/inc/tsfi_curveswidths.h"
#include "../tsfi2-deepseek/inc/tsfi_curveswrap.h"

#define BENCH_ITERATIONS 500000

static double get_time_us(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (ts.tv_sec * 1000000.0) + (ts.tv_nsec / 1000.0);
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT USD CURVES EVALUATION LATENCY BENCHMARKS\n");
    printf("=============================================================\n");

    // 1. Benchmark CurvesWidths Interpolation
    TSFiCurvesWidths cw;
    tsfi_curveswidths_init(&cw);
    tsfi_curveswidths_add(&cw, 1.0);
    tsfi_curveswidths_add(&cw, 2.0);
    tsfi_curveswidths_add(&cw, 4.0);
    tsfi_curveswidths_add(&cw, 8.0);

    double start = get_time_us();
    volatile double dummy_width = 0.0;
    for (int i = 0; i < BENCH_ITERATIONS; i++) {
        double t = (double)(i % 1000) / 1000.0;
        dummy_width += tsfi_curveswidths_interpolate(&cw, t);
    }
    double elapsed_widths = get_time_us() - start;
    double lat_widths = (elapsed_widths * 1000.0) / BENCH_ITERATIONS;
    printf("[BENCHMARK] Running %d iterations of CurvesWidths Interpolation...\n", BENCH_ITERATIONS);
    printf("   ✓ CurvesWidths Average Latency: %.2f ns/run (Accumulator: %.2f)\n", lat_widths, dummy_width);

    // 2. Benchmark CurvesWrap Index Resolution
    TSFiCurvesWrap wrap;
    tsfi_curveswrap_init(&wrap, TSFI_WRAP_PERIODIC);

    start = get_time_us();
    volatile int dummy_idx = 0;
    for (int i = 0; i < BENCH_ITERATIONS; i++) {
        dummy_idx += tsfi_curveswrap_resolve_index(&wrap, i, 16);
    }
    double elapsed_wrap = get_time_us() - start;
    double lat_wrap = (elapsed_wrap * 1000.0) / BENCH_ITERATIONS;
    printf("[BENCHMARK] Running %d iterations of CurvesWrap Index Resolution...\n", BENCH_ITERATIONS);
    printf("   ✓ CurvesWrap Average Latency: %.2f ns/run (Accumulator: %d)\n", lat_wrap, dummy_idx);

    printf("=============================================================\n");
    printf("BENCHMARKS COMPLETE\n");
    printf("=============================================================\n");
    return 0;
}
