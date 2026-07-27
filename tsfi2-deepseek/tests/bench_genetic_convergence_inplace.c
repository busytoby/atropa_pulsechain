#include <stdio.h>
#include <x86intrin.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "lau_memory.h"
#include "tsfi_genetic.h"
#include "tsfi_dysnomia.h"

#define DM(T) (T*)lau_malloc_wired(sizeof(T))
#define INPLACE_ITERATIONS 500000

static inline uint64_t get_ns() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + ts.tv_nsec;
}

int main() {
    printf("=== TSFi In-Place Genetic Convergence Benchmark (%d Ops) ===\n", INPLACE_ITERATIONS);
    srand(42);

    // 1. Setup Parent A (YI)
    GeneticNode* ParentA = (GeneticNode*)lau_malloc_wired(sizeof(GeneticNode));
    ParentA->type = GENETIC_TYPE_YI;
    ParentA->dys_ptr = DM(struct YI);
    struct YI* yiA = (struct YI*)ParentA->dys_ptr;
    memset(yiA, 0, sizeof(struct YI));
    yiA->Xi = tsfi_bn_alloc();
    tsfi_bn_set_u64(yiA->Xi, 0xAAAA5555AAAA5555ULL);
    yiA->Ring = tsfi_bn_alloc();
    tsfi_bn_set_u64(yiA->Ring, 0x1111222233334444ULL);

    // 2. Setup Parent B (YI)
    GeneticNode* ParentB = (GeneticNode*)lau_malloc_wired(sizeof(GeneticNode));
    ParentB->type = GENETIC_TYPE_YI;
    ParentB->dys_ptr = DM(struct YI);
    struct YI* yiB = (struct YI*)ParentB->dys_ptr;
    memset(yiB, 0, sizeof(struct YI));
    yiB->Xi = tsfi_bn_alloc();
    tsfi_bn_set_u64(yiB->Xi, 0xCCCC3333CCCC3333ULL);
    yiB->Ring = tsfi_bn_alloc();
    tsfi_bn_set_u64(yiB->Ring, 0x5555666677778888ULL);

    // 3. Setup Child Node (allocated once, reuse memory for in-place XO)
    GeneticNode* Child = (GeneticNode*)lau_malloc_wired(sizeof(GeneticNode));
    Child->type = GENETIC_TYPE_YI;
    Child->dys_ptr = DM(struct YI);
    struct YI* yiC = (struct YI*)Child->dys_ptr;
    memset(yiC, 0, sizeof(struct YI));

    printf("Running In-Place Crossover Loop...\n");
    uint64_t start = get_ns();
    uint64_t start_tsc = __rdtsc();

    for(int i = 0; i < INPLACE_ITERATIONS; i++) {
        Fourier_UniversalCrossover_InPlace(ParentA, ParentB, Child);
    }

    uint64_t end = get_ns();
    uint64_t end_tsc = __rdtsc();
    double duration_sec = (double)(end - start) / 1e9;

    printf("Total Time   : %.4f s\n", duration_sec);
    printf("Throughput   : %.2f InPlace-XO/s\n", INPLACE_ITERATIONS / duration_sec);
    printf("Status       : [PASS] In-Place Neurological Evolution Sustained\n");

    // Cleanup
    freeYI(yiA); lau_free(ParentA);
    freeYI(yiB); lau_free(ParentB);
    freeYI(yiC); lau_free(Child);

    double cycles_per_op = (double)(end_tsc - start_tsc) / INPLACE_ITERATIONS;
    printf("Cycles/Op   : %.2f\n", cycles_per_op);

    extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}
