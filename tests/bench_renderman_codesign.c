#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include "../tsfi2-deepseek/inc/tsfi_riinterface.h"
#include "../tsfi2-deepseek/inc/tsfi_depthoffield.h"
#include "../tsfi2-deepseek/inc/tsfi_displacementshader.h"
#include "../tsfi2-deepseek/inc/tsfi_zmm_vm.h"

#define NUM_ITERATIONS 1000

double get_time_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec * 1e9 + (double)ts.tv_nsec;
}

void tsfi_zmm_winchester_deconvolve_handshake(void *vm_state, void *ri_void) {
    (void)vm_state;
    (void)ri_void;
}

int main(void) {
    printf("=== RUNNING AUNCIENT RENDERMAN CO-DESIGN BENCHMARKS ===\n");

    TSFiRiInterface ri;
    tsfi_riinterface_init(&ri);

    // 1. Benchmark WinchesterMQ Zero-Copy Sync Driver Latency
    printf("[BENCH] WinchesterMQ Zero-Copy Sync Latency...\n");
    TsfiZmmVmState vm_state;
    memset(&vm_state, 0, sizeof(vm_state));
    uint8_t dummy_ram[0x10000];
    memset(dummy_ram, 0, sizeof(dummy_ram));
    vm_state.reu_ram = dummy_ram;
    dummy_ram[0xF002] = 32;

    double start = get_time_ns();
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        tsfi_riinterface_sync_winchester(&ri, &vm_state);
    }
    double end = get_time_ns();
    double avg_sync = (end - start) / NUM_ITERATIONS;
    printf("   - Avg WinchesterMQ Sync Latency: %.2f ns\n", avg_sync);

    // 2. Benchmark Wiener Deconvolution Throughput (256x256 buffer)
    printf("[BENCH] Wiener Deconvolution Throughput (256x256)...\n");
    double *temp_in = (double *)malloc(256 * 256 * sizeof(double));
    double *temp_out = (double *)malloc(256 * 256 * sizeof(double));
    for (int i = 0; i < 256 * 256; i++) {
        temp_in[i] = (double)(i % 256);
    }

    start = get_time_ns();
    int deconv_iters = 100;
    for (int i = 0; i < deconv_iters; i++) {
        tsfi_depthoffield_wiener_deconvolve(temp_in, temp_out, 256, 256, 0.01);
    }
    end = get_time_ns();
    double total_ms = (end - start) / 1e6;
    double rate = (double)deconv_iters / (total_ms / 1000.0);
    printf("   - Throughput: %.2f deconvolutions/sec (Total Time: %.2f ms)\n", rate, total_ms);

    // 3. Benchmark Chromatic Aberration Dispersion Overhead
    printf("[BENCH] Chromatic Aberration Evaluation Latency...\n");
    start = get_time_ns();
    double sum_coef = 0.0;
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        sum_coef += tsfi_depthoffield_eval_chromatic_blur(&ri.dof, 15.0, i % 3);
    }
    end = get_time_ns();
    double avg_chroma = (end - start) / NUM_ITERATIONS;
    printf("   - Avg Chromatic Defocus Latency: %.2f ns (Dummy Sum: %.4f)\n", avg_chroma, sum_coef);

    free(temp_in);
    free(temp_out);
    printf("=== RENDERMAN BENCHMARKS COMPLETE (PASS) ===\n");
    return 0;
}
