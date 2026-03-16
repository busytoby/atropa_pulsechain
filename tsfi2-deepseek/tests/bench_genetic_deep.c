#include <stdio.h>
#include <x86intrin.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "lau_memory.h"
#include "tsfi_genetic.h"
#include "tsfi_dysnomia.h"

static inline uint64_t get_ns() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + ts.tv_nsec;
}

#define DM(T) (T*)lau_malloc_wired(sizeof(T))

static struct Dai* Thunk_A(struct SHIO* M, TSFiBigInt* P) { (void)M; (void)P; return NULL; }
static struct Dai* Thunk_B(struct SHIO* M, TSFiBigInt* P) { (void)M; (void)P; return NULL; }

// Build a deep, fully populated YI tree
static void populate_deep_yi(struct YI* yi, const char* seed) {
    yi->Xi = tsfi_bn_alloc();
    yi->Ring = tsfi_bn_alloc();
    yi->Psi = DM(struct SHIO);
    memset(yi->Psi, 0, sizeof(struct SHIO));
    
    yi->Psi->Rho = DM(struct SHAO);
    memset(yi->Psi->Rho, 0, sizeof(struct SHAO));
    
    yi->Psi->Rho->Rod = DM(struct SHA);
    memset(yi->Psi->Rho->Rod, 0, sizeof(struct SHA));
    yi->Psi->Rho->Rod->Mu = DM(struct Fa);
    memset(yi->Psi->Rho->Rod->Mu, 0, sizeof(struct Fa));
    yi->Psi->Rho->Rod->Mu->Base = tsfi_bn_alloc();
    yi->Psi->Rho->Rod->Mu->Identity = tsfi_bn_alloc();
    
    yi->Psi->Rho->Cone = DM(struct SHA);
    memset(yi->Psi->Rho->Cone, 0, sizeof(struct SHA));
    yi->Psi->Rho->Cone->Mu = DM(struct Fa);
    memset(yi->Psi->Rho->Cone->Mu, 0, sizeof(struct Fa));
    yi->Psi->Rho->Cone->Mu->Base = tsfi_bn_alloc();
}

int main() {
    printf("=== TSFi Deep Recursive Genetic Crossover Benchmark ===\n");
    srand(time(NULL));

    // 1. Setup Deep Parents
    GeneticNode* ParentA = (GeneticNode*)lau_malloc_wired(sizeof(GeneticNode));
    ParentA->type = GENETIC_TYPE_YI;
    ParentA->dys_ptr = DM(struct YI);
    populate_deep_yi((struct YI*)ParentA->dys_ptr, "AAAAAAAAAAAA");
    ParentA->react_shio = Thunk_A;

    GeneticNode* ParentB = (GeneticNode*)lau_malloc_wired(sizeof(GeneticNode));
    ParentB->type = GENETIC_TYPE_YI;
    ParentB->dys_ptr = DM(struct YI);
    populate_deep_yi((struct YI*)ParentB->dys_ptr, "CCCCCCCCCCCC");
    ParentB->react_shio = Thunk_B;

    // 2. Loop
    int ITERATIONS = 10000;
    GeneticNode* Child = (GeneticNode*)lau_malloc_wired(sizeof(GeneticNode));
    
    printf("Running %d deep recursive crossovers...\n", ITERATIONS);
    uint64_t start = get_ns();
    uint64_t start_tsc = __rdtsc();
    
    for(int i=0; i<ITERATIONS; i++) {
        Fourier_UniversalCrossover(ParentA, ParentB, Child);
        freeYI((struct YI*)Child->dys_ptr);
    }
    
    uint64_t end = get_ns();
    uint64_t end_tsc = __rdtsc();
    
    // 3. Report
    double duration_sec = (double)(end - start) / 1e9;
    double throughput = ITERATIONS / duration_sec;
    
    printf("Total Time  : %.4f s\n", duration_sec);
    printf("Throughput  : %.2f Deep-XO/s\n", throughput);
    printf("Latency     : %.2f us/op\n", (duration_sec * 1e6) / ITERATIONS);

    // Cleanup Parents
    freeYI((struct YI*)ParentA->dys_ptr);
    lau_free(ParentA);
    freeYI((struct YI*)ParentB->dys_ptr);
    lau_free(ParentB);
    lau_free(Child);

    double cycles_per_op = (double)(end_tsc - start_tsc) / ITERATIONS;
    printf("Cycles/Op   : %.2f\n", cycles_per_op);

        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}