#include "tsfi_neurology.h"
#include "lau_memory.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <immintrin.h>

// Link against the Thetan Kernel
// Defined in src/tsfi_yang.c
extern void tsfi_yang_process(const TSFiSGP *sgp, TSFiVGP *vgp);

int main() {
    printf("=== TSFi Mu -> YANG -> Eta Benchmark ===\n");

    // Allocate aligned memory for SGP (Mu) and VGP (Eta)
    TSFiSGP *mu_sgp;
    TSFiVGP *eta_vgp;
    
    mu_sgp = lau_memalign(64, sizeof(TSFiSGP));
    eta_vgp = lau_memalign(64, sizeof(TSFiVGP));
    
    // Initialize Mu (SGP)
    for (int i=0; i<8; i++) {
        mu_sgp->weights[i] = (double)i * 0.1;
    }
    mu_sgp->banach_norm = 1.0;
    mu_sgp->generation = 1;
    mu_sgp->thunk_id = 42;
    
    // Warmup
    tsfi_yang_process(mu_sgp, eta_vgp);
    
    // Benchmark Loop
    long iterations = 10000000;
    struct timespec start, end;
    
    clock_gettime(CLOCK_MONOTONIC, &start);
    
    for (long i = 0; i < iterations; i++) {
        // Update generation to force mutation seed change
        mu_sgp->generation = i;
        mu_sgp->thunk_id = i;
        
        tsfi_yang_process(mu_sgp, eta_vgp);
        
        // Use result to prevent optimization (optional)
        if (i % 1000000 == 0) {
            // volatile read?
            __asm__ volatile("" : : "r"(eta_vgp->op_ctrl) : "memory");
        }
    }
    
    clock_gettime(CLOCK_MONOTONIC, &end);
    
    double seconds = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    double ops_per_sec = iterations / seconds;
    
    printf("Iterations: %ld\n", iterations);
    printf("Time      : %.4f s\n", seconds);
    printf("Throughput: %.2f M Waveforms/s\n", ops_per_sec / 1e6);
    
    // Verify Output (Eta)
    printf("\n--- Sample Output (Eta VGP) ---\n");
    printf("Op Ctrl   : 0x%X\n", eta_vgp->op_ctrl);
    printf("Mutation  : %u\n", eta_vgp->mutation_id);
    printf("Voxels    : [");
    for(int i=0; i<8; i++) printf("%.2f ", eta_vgp->voxels[i]);
    printf("...]\n");
    
    if (eta_vgp->op_ctrl != 0xCAFEBABE) {
        printf("FAIL: Bad Op Ctrl\n");
        return 1;
    }
    
    lau_free(mu_sgp);
    lau_free(eta_vgp);
    
        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}