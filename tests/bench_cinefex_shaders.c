#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include "../tsfi2-deepseek/inc/tsfi_sss.h"
#include "../tsfi2-deepseek/inc/tsfi_envmap.h"
#include "../tsfi2-deepseek/inc/tsfi_hairshading.h"

// Benchmark measuring performance of advanced Cinefex shaders (SSS, EnvMap, HairShading)
int main(void) {
    printf("=== AUNCIENT CINEFEX SHADERS PERFORMANCE BENCHMARK ===\n");

    struct timespec start, end;
    double elapsed_sec;

    // 1. Benchmark Sub-surface Scattering
    TSFiSubsurfaceScattering sss;
    tsfi_sss_init(&sss, 2.5, 0.35);

    clock_gettime(CLOCK_MONOTONIC, &start);
    volatile double sss_accum = 0.0;
    for (int i = 0; i < 2000000; i++) {
        double thickness = (double)(i % 100) * 0.1;
        sss_accum += tsfi_sss_eval(&sss, thickness, 1.0);
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    elapsed_sec = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) * 1e-9;
    double sss_rate = 2000000.0 / elapsed_sec;
    printf("   ✓ Sub-surface scattering: %f evaluations/sec (accum=%f)\n", sss_rate, sss_accum);

    // 2. Benchmark Environment Mapping
    uint8_t mock_texture[64 * 64];
    memset(mock_texture, 180, sizeof(mock_texture));
    TSFiEnvMap env;
    tsfi_envmap_init(&env, 64, 64, mock_texture);

    clock_gettime(CLOCK_MONOTONIC, &start);
    volatile uint32_t env_accum = 0;
    for (int i = 0; i < 2000000; i++) {
        double nx = cos((double)i * 0.01);
        double ny = sin((double)i * 0.01);
        double nz = sin((double)i * 0.005);
        env_accum += tsfi_envmap_sample(&env, nx, ny, nz);
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    elapsed_sec = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) * 1e-9;
    double env_rate = 2000000.0 / elapsed_sec;
    printf("   ✓ Environment map sampling: %f samplings/sec (accum=%u)\n", env_rate, env_accum);

    // 3. Benchmark Anisotropic Hair Shading
    TSFiHairShading hs;
    tsfi_hairshading_init(&hs, 12.0, 0.8);
    tsfi_hairshading_set_animatronic(&hs, 0.15, 0.4);

    double tangent[3] = {1.0, 0.0, 0.0};
    double light_dir[3] = {0.0, 1.0, 0.0};
    double view_dir[3] = {0.0, 1.0, 0.0};

    clock_gettime(CLOCK_MONOTONIC, &start);
    volatile double hs_accum = 0.0;
    for (int i = 0; i < 2000000; i++) {
        tangent[0] = sin((double)i * 0.001);
        hs_accum += tsfi_hairshading_eval(&hs, tangent, light_dir, view_dir);
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    elapsed_sec = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) * 1e-9;
    double hs_rate = 2000000.0 / elapsed_sec;
    printf("   ✓ Anisotropic hair shading: %f evaluations/sec (accum=%f)\n", hs_rate, hs_accum);

    // Output JSON metrics
    printf("JSON_METRICS: {\"sss_eval_rate\": %f, \"envmap_sample_rate\": %f, \"hairshading_eval_rate\": %f}\n",
           sss_rate, env_rate, hs_rate);

    printf("=== AUNCIENT CINEFEX SHADERS PERFORMANCE BENCHMARK COMPLETE ===\n");
    return 0;
}
