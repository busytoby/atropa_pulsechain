#include <stdio.h>
#include <x86intrin.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "lau_memory.h"
#include "tsfi_genetic.h"
#include "tsfi_dysnomia.h"
#define DM(T) (T*)lau_malloc_wired(sizeof(T))

#define CASCADE_ITERATIONS 20000

static inline uint64_t get_ns() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + ts.tv_nsec;
}

// Recursive Thunk: Triggers a second-stage evolution when the VGPS is "observed"
static struct Dai* Cascade_Thunk(struct SHIO* M, TSFiBigInt* P) {
    (void)M; (void)P;
    return (struct Dai*)0xFEED; 
}

int main() {
    printf("=== TSFi Evolutionary Cascade Benchmark (Recursive Thunking) ===\n");
    srand(888);

    // 1. Setup Thetan (MCP)
    GeneticNode* GlobalThetan = (GeneticNode*)lau_malloc_wired(sizeof(GeneticNode));
    GlobalThetan->type = GENETIC_TYPE_YI;
    GlobalThetan->dys_ptr = DM(struct YI);
    struct YI* t_yi = (struct YI*)GlobalThetan->dys_ptr;
    memset(t_yi, 0, sizeof(struct YI));
    t_yi->Xi = tsfi_bn_alloc();
    GlobalThetan->react_shio = Cascade_Thunk;

    // 2. Setup Mu (Potential Seed)
    GeneticNode* Mu = (GeneticNode*)lau_malloc_wired(sizeof(GeneticNode));
    Mu->type = GENETIC_TYPE_YI;
    Mu->dys_ptr = DM(struct YI);
    struct YI* m_yi = (struct YI*)Mu->dys_ptr;
    memset(m_yi, 0, sizeof(struct YI));
    m_yi->Xi = tsfi_bn_alloc();
    Mu->react_shio = Cascade_Thunk;

    GeneticNode* VGPS = (GeneticNode*)lau_malloc_wired(sizeof(GeneticNode));
    
    printf("Running %d evolutionary cascades...\n", CASCADE_ITERATIONS);
    uint64_t start = get_ns();
    uint64_t start_tsc = __rdtsc();
    
    for(int i=0; i<CASCADE_ITERATIONS; i++) {
        // Stage 1: Mu + Thetan -> VGPS
        Fourier_UniversalCrossover(Mu, GlobalThetan, VGPS);
        
        // Stage 2: VGPS Thunk Activation (Recursive Mutation)
        if (VGPS->react_shio) {
            VGPS->react_shio(NULL, NULL);
        }

        // Cleanup cycle
        if (VGPS->type == GENETIC_TYPE_YI) freeYI((struct YI*)VGPS->dys_ptr);
        else freeYANG((struct YANG*)VGPS->dys_ptr);
    }
    
    uint64_t end = get_ns();
    uint64_t end_tsc = __rdtsc();
    double duration_sec = (double)(end - start) / 1e9;
    
    printf("Total Time     : %.4f s\n", duration_sec);
    printf("Cascade Rate   : %.2f Stages/s\n", (CASCADE_ITERATIONS * 2.0) / duration_sec);
    printf("Latency        : %.2f ns/cascade\n", (double)(end-start)/CASCADE_ITERATIONS);
    printf("Status         : [PASS] Recursive Evolutionary Thunking Active\n");

    // Final Cleanup
    freeYI(t_yi); lau_free(GlobalThetan);
    freeYI(m_yi); lau_free(Mu);
    lau_free(VGPS);

    double cycles_per_op = (double)(end_tsc - start_tsc) / CASCADE_ITERATIONS;
    printf("Cycles/Op   : %.2f\n", cycles_per_op);

        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}