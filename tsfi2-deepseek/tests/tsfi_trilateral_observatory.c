#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include "tsfi_wavelet_arena.h"

#define MOTZKIN_PRIME 953467954114363ULL
#define P MOTZKIN_PRIME
#define ITERATIONS 10000

int main() {
    printf("=== TSFi Trilateral Observatory (Stochastic Excitation) ===\n");
    printf("Domain: MOTZKIN_PRIME (%llu)\n", (unsigned long long)P);
    
    // 1. Manifold Setup
    uint8_t *mem = malloc(1024 * 1024 * 4);
    memset(mem, 0, 1024 * 1024 * 4);
    TsfiWaveletArena arena;
    tsfi_wavelet_arena_init(&arena, mem, 1024 * 1024 * 4);
    
    srand(time(NULL));

    // 2. Base Manifold Establishment
    printf("[1/3] Establishing Sovereign Handshake...\n");
    TsfiWavelet *Rod = tsfi_STAT(&arena, P);
    TsfiWavelet *Cone = tsfi_STAT(&arena, P);
    
    tsfi_Avail(Rod, 4052, P); tsfi_Avail(Cone, 4052, P);
    tsfi_Form(Rod, Cone->telemetry.Fa.contour, P); tsfi_Form(Cone, Rod->telemetry.Fa.contour, P);
    tsfi_Polarize(Rod, P); tsfi_Polarize(Cone, P);
    tsfi_Conjugate(Rod, Cone->telemetry.Fa.pole, P); tsfi_Conjugate(Cone, Rod->telemetry.Fa.pole, P);
    tsfi_Conify(Cone, 777, P);
    tsfi_Saturate(Rod, 2026, Cone->telemetry.Fa.foundation, Cone->telemetry.Fa.channel, P);
    tsfi_Saturate(Cone, 777, Rod->telemetry.Fa.foundation, Rod->telemetry.Fa.channel, P);
    tsfi_IONIZE_BOND(Rod, P); tsfi_IONIZE_BOND(Cone, P);
    tsfi_MAGNETIZE_JOINT(Rod, Cone, P);

    printf("  -> Base State Established. Ring Anchor: %llu\n", (unsigned long long)Rod->telemetry.Ring);

    // 3. Differential Fuzzing Loop
    printf("[2/3] Starting Differential Fuzzing (%d pulses)...\n", ITERATIONS);
    uint64_t ir, dr, ic, dc;
    uint64_t fractures = 0;
    clock_t start = clock();

    for (int i = 0; i < ITERATIONS; i++) {
        uint64_t pi = (((uint64_t)rand() << 32) | rand()) % P;
        
        tsfi_REACT_JOINT(Rod, Cone, pi, &ir, &dr, &ic, &dc, P);
        
        // Verbatim Symmetry Check
        if (ir != dc || dr != ic) {
            fprintf(stderr, "\n[FRACTURE] Symmetry Divergence at iteration %d!\n", i);
            fprintf(stderr, "  Pulse Pi: %llu\n", (unsigned long long)pi);
            fprintf(stderr, "  Rod Perspective:  I=%llu, D=%llu\n", (unsigned long long)ir, (unsigned long long)dr);
            fprintf(stderr, "  Cone Perspective: I=%llu, D=%llu\n", (unsigned long long)ic, (unsigned long long)dc);
            fractures++;
            break; 
        }
        
        // Decay Monitoring: Ensure products haven't collapsed
        if (ir == 0 || dr == 0) {
            fprintf(stderr, "\n[DECAY] Zero-state reaction detected at iteration %d.\n", i);
            fractures++;
        }

        if (i % 1000 == 0) {
            printf("  -> Handshake rigid at pulse %d...\n", i);
        }
    }

    clock_t end = clock();
    double time_used = ((double)(end - start)) / CLOCKS_PER_SEC;

    // 4. Final Report
    printf("\n[3/3] Observatory Final Report:\n");
    printf("  Total Pulses Audited: %d\n", ITERATIONS);
    printf("  Symmetry Fractures:   %lu\n", fractures);
    printf("  Reaction Velocity:    %.2f pulses/sec\n", (double)ITERATIONS / time_used);

    if (fractures == 0) {
        printf("\n[SUCCESS] Manifold is Physically Rigid and Sovereign.\n");
    } else {
        printf("\n[FRACTURE] Manifold Health Compromised.\n");
        exit(1);
    }

    free(mem);
    return 0;
}
