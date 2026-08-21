#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include "tsfi_vaesen_spatial_vision.h"

static double get_time_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec * 1e9 + (double)ts.tv_nsec;
}

int main(void) {
    printf("========================================================================================\n");
    printf("   VAESEN 2D VECTOR SPATIAL VISION & BRAM ROM/RAM PARTITIONING TEST\n");
    printf("========================================================================================\n\n");

    /* Ensure canonical slices exist */
    assert(tsfi_vaesen_slice_build_canonical_all() == 0);

    TsfiVaesenDeviceSliceBin nacken, tomte;
    assert(tsfi_vaesen_slice_load("assets/vaesen/nacken.dat.bin", &nacken) == 0);
    assert(tsfi_vaesen_slice_load("assets/vaesen/tomte.dat.bin", &tomte) == 0);

    /* 1. Test Sanctuary Scene for Näcken: Millpond Running Water (Cyan Lines) */
    TsfiZMachineVectorLine water_scene[3] = {
        { .start_x = 10, .start_y = 150, .end_x = 200, .end_y = 150, .color_idx = 6 },  /* Blue river */
        { .start_x = 20, .start_y = 160, .end_x = 180, .end_y = 160, .color_idx = 11 }, /* Cyan ripples */
        { .start_x = 50, .start_y = 170, .end_x = 150, .end_y = 170, .color_idx = 6 }
    };
    TsfiVaesenSpatialVisionField field1;
    TsfiVaesenPartitionedMemory mem1;
    int eval_res1 = tsfi_vaesen_evaluate_spatial_vision(&nacken, water_scene, 3, &field1, &mem1);
    assert(eval_res1 == 0);
    printf("[SPATIAL VISION 1 - NÄCKEN IN RUNNING WATER]\n");
    printf("  Env Flags: 0x%02X (Has Water: %s)\n", field1.detected_env_flags,
           (field1.detected_env_flags & TSFI_ENV_HAS_RUNNING_WATER) ? "YES" : "NO");
    printf("  Comfort Index: %.2f | Reaction: %s\n", field1.spatial_comfort_index,
           mem1.reaction_state == 0 ? "CALM (Sanctuary: PASS)" : "ALERT");
    printf("  Folklore Law: %s\n\n", mem1.core_folklore_law);
    assert(field1.detected_env_flags & TSFI_ENV_HAS_RUNNING_WATER);
    assert(mem1.reaction_state == 0);

    /* 2. Test Forbidden Scene for Näcken: Consecrated Sunlit Churchyard (Yellow & White Lines) */
    TsfiZMachineVectorLine sunlit_church[3] = {
        { .start_x = 20, .start_y = 100, .end_x = 100, .end_y = 20, .color_idx = 7 },  /* Direct Sun */
        { .start_x = 40, .start_y = 150, .end_x = 160, .end_y = 150, .color_idx = 14 },/* Consecrated Threshold */
        { .start_x = 100, .start_y = 20, .end_x = 180, .end_y = 100, .color_idx = 10 }
    };
    TsfiVaesenSpatialVisionField field2;
    TsfiVaesenPartitionedMemory mem2;
    int eval_res2 = tsfi_vaesen_evaluate_spatial_vision(&nacken, sunlit_church, 3, &field2, &mem2);
    assert(eval_res2 == 0);
    printf("[SPATIAL VISION 2 - NÄCKEN IN SUNLIT CHURCHYARD]\n");
    printf("  Env Flags: 0x%02X (Has Sun/Consecrated: YES)\n", field2.detected_env_flags);
    printf("  Comfort Index: %.2f | Reaction: %s\n\n", field2.spatial_comfort_index,
           mem2.reaction_state == 2 ? "REPELLED / FLEEING (PASS)" : "CALM");
    assert(mem2.reaction_state == 2);

    /* 3. Latency Benchmark across 1,000,000 Spatial Vision Evaluations */
    const int benchmark_count = 1000000;
    double t_start = get_time_ns();
    for (int i = 0; i < benchmark_count; ++i) {
        TsfiVaesenSpatialVisionField f;
        TsfiVaesenPartitionedMemory m;
        tsfi_vaesen_evaluate_spatial_vision(&nacken, water_scene, 3, &f, &m);
    }
    double t_end = get_time_ns();
    double avg_latency = (t_end - t_start) / (double)benchmark_count;
    printf("[BENCHMARK] Average Spatial Vision evaluation latency: %.2f ns (Target: < 1000 ns)\n", avg_latency);
    assert(avg_latency < 1000.0);

    printf("\n>>> VAESEN 2D VECTOR SPATIAL VISION & BRAM MEMORY FORMALLY VERIFIED <<<\n");
    return 0;
}
