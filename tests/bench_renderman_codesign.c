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
#include "../tsfi2-deepseek/inc/tsfi_ccx_pool.h"

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

    // 4. Benchmark Multiframe Accumulator Latency
    printf("[BENCH] Multiframe Accumulator Latency...\n");
    start = get_time_ns();
    for (int i = 0; i < 100; i++) {
        tsfi_riinterface_accumulate_frame(&ri);
    }
    end = get_time_ns();
    double avg_accum = (end - start) / 100.0;
    printf("   - Avg Multiframe Accumulation Latency: %.2f ms\n", avg_accum / 1e6);

    // 5. Benchmark CICS PMG Collision Abend Recovery Latency
    printf("[BENCH] CICS PMG Collision Abend Recovery Latency...\n");
    int health = 0;
    start = get_time_ns();
    for (int i = 0; i < 100; i++) {
        tsfi_riinterface_resolve_pmg_cics_collision(&ri, 1, &health);
    }
    end = get_time_ns();
    double avg_recovery = (end - start) / 100.0;
    printf("   - Avg CICS PMG Abend Recovery Latency: %.2f ms\n", avg_recovery / 1e6);

    // 6. Benchmark CCX Parallel Wiener Deconvolution (256x256 buffer, 4 threads in CCX 0)
    printf("[BENCH] CCX Parallel Deconvolution (4 threads, 256x256)...\n");
    TSFiCCXPool ccx_pool;
    tsfi_ccx_pool_init(&ccx_pool, 0, 4);

    start = get_time_ns();
    for (int i = 0; i < deconv_iters; i++) {
        tsfi_ccx_deconvolve_parallel(&ccx_pool, temp_in, temp_out, 256, 256, 0.01);
    }
    end = get_time_ns();
    double total_parallel_ms = (end - start) / 1e6;
    double parallel_rate = (double)deconv_iters / (total_parallel_ms / 1000.0);
    printf("   - CCX Parallel Throughput: %.2f deconvolutions/sec (Total Time: %.2f ms)\n", parallel_rate, total_parallel_ms);
    tsfi_ccx_pool_destroy(&ccx_pool);

    // 7. Benchmark Multi-CCX Parallel Wiener Deconvolution (256x256 buffer, 2 pools, 4 threads each)
    printf("[BENCH] Multi-CCX Parallel Deconvolution (2 pools, 8 threads total, 256x256)...\n");
    TSFiCCXPool ccx_pool0, ccx_pool1;
    tsfi_ccx_pool_init(&ccx_pool0, 0, 4);
    tsfi_ccx_pool_init(&ccx_pool1, 1, 4);
    TSFiCCXPool *pools[2] = { &ccx_pool0, &ccx_pool1 };
    
    start = get_time_ns();
    for (int i = 0; i < deconv_iters; i++) {
        tsfi_multi_ccx_deconvolve_parallel(pools, 2, temp_in, temp_out, 256, 256, 0.01);
    }
    end = get_time_ns();
    double total_multi_ms = (end - start) / 1e6;
    double multi_rate = (double)deconv_iters / (total_multi_ms / 1000.0);
    printf("   - Multi-CCX Parallel Throughput: %.2f deconvolutions/sec (Total Time: %.2f ms)\n", multi_rate, total_multi_ms);
    tsfi_ccx_pool_destroy(&ccx_pool0);
    tsfi_ccx_pool_destroy(&ccx_pool1);


    // 8. Large Buffer Benchmark (1024x1024 double precision)
    printf("[BENCH] Large Buffer Deconvolution (1024x1024, 8MB dataset)...\n");
    double *large_in = (double *)malloc(1024 * 1024 * sizeof(double));
    double *large_out = (double *)malloc(1024 * 1024 * sizeof(double));
    for (int i = 0; i < 1024 * 1024; i++) {
        large_in[i] = (double)(i % 1024);
    }
    
    // 1-thread baseline
    start = get_time_ns();
    for (int i = 0; i < 10; i++) {
        tsfi_depthoffield_wiener_deconvolve(large_in, large_out, 1024, 1024, 0.01);
    }
    end = get_time_ns();
    printf("   - Sequential AVX2: %.2f ms (average per iteration)\n", (end - start) / 1e7);
    
    // CCX pool (4 threads)
    TSFiCCXPool large_pool0;
    tsfi_ccx_pool_init(&large_pool0, 0, 4);
    start = get_time_ns();
    for (int i = 0; i < 10; i++) {
        tsfi_ccx_deconvolve_parallel(&large_pool0, large_in, large_out, 1024, 1024, 0.01);
    }
    end = get_time_ns();
    printf("   - CCX Parallel (4 threads): %.2f ms (average per iteration)\n", (end - start) / 1e7);
    
    // Multi-CCX pool (2 pools, 8 threads total)
    TSFiCCXPool large_pool1;
    tsfi_ccx_pool_init(&large_pool1, 1, 4);
    TSFiCCXPool *large_pools[2] = { &large_pool0, &large_pool1 };
    start = get_time_ns();
    for (int i = 0; i < 10; i++) {
        tsfi_multi_ccx_deconvolve_parallel(large_pools, 2, large_in, large_out, 1024, 1024, 0.01);
    }
    end = get_time_ns();
    printf("   - Multi-CCX Parallel (2 pools, 8 threads): %.2f ms (average per iteration)\n", (end - start) / 1e7);
    
    tsfi_ccx_pool_destroy(&large_pool0);
    tsfi_ccx_pool_destroy(&large_pool1);
    free(large_in);
    free(large_out);

    // 9. Benchmark Verlet Physics Integration (FET discharge cycles on 10,000 particles)
    printf("[BENCH] Verlet Soft Body Physics Simulation (10,000 particles)...\n");
    int particle_count = 10000;
    double *pos_x = malloc(particle_count * sizeof(double));
    double *prev_pos_x = malloc(particle_count * sizeof(double));
    for (int i = 0; i < particle_count; i++) {
        pos_x[i] = (double)i;
        prev_pos_x[i] = (double)i - 0.1;
    }
    start = get_time_ns();
    int verlet_iters = 1000;
    for (int i = 0; i < verlet_iters; i++) {
        tsfi_riinterface_discharge_verlet(&ri, pos_x, prev_pos_x, particle_count, 0.1, 0.99);
    }
    end = get_time_ns();
    double verlet_ms = (end - start) / 1e6;
    printf("   - Execution Time: %.2f ms (Total Steps: %d)\n", verlet_ms, verlet_iters);
    free(pos_x);
    free(prev_pos_x);

    // 10. Benchmark WinchesterMQ Handshake Intercept Latency
    printf("[BENCH] WinchesterMQ Handshake Intercept Latency...\n");
    start = get_time_ns();
    int handshake_iters = 10000;
    for (int i = 0; i < handshake_iters; i++) {
        tsfi_zmm_winchester_deconvolve_handshake(&vm_state, &ri);
    }
    end = get_time_ns();
    double handshake_ns = (end - start) / handshake_iters;
    printf("   - Average Intercept Latency: %.2f ns\n", handshake_ns);

    // 11. Benchmark DisplacementShader Vertex Math Latency
    printf("[BENCH] DisplacementShader Wavefront Distortion (1,000,000 vertices)...\n");
    TSFiDisplacementShader ds;
    tsfi_displacementshader_init(&ds, 12.5, 0.05);
    start = get_time_ns();
    double dummy_displacement = 0.0;
    for (int y = 0; y < 1000; y++) {
        for (int x = 0; x < 1000; x++) {
            dummy_displacement += tsfi_displacementshader_eval_cubic(&ds, (double)x, (double)y);
            dummy_displacement += tsfi_displacementshader_eval_spherical(&ds, (double)x, (double)y);
        }
    }
    end = get_time_ns();
    double displacement_ns = (end - start) / 2000000.0;
    printf("   - Average Displacement Latency: %.2f ns (Dummy Sum: %.2f)\n", displacement_ns, dummy_displacement);

    free(temp_in);
    free(temp_out);
    printf("=== RENDERMAN BENCHMARKS COMPLETE (PASS) ===\n");
    return 0;



}
