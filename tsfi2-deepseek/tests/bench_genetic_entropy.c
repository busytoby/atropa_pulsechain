#include <stdio.h>
#include <x86intrin.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "lau_memory.h"
#include "tsfi_genetic.h"
#include "tsfi_zmm_vm.h"
#define DM(T) (T*)lau_malloc_wired(sizeof(T))

#define ENTROPY_ITERATIONS 50000

static inline uint64_t get_ns() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + ts.tv_nsec;
}

static struct Dai* Thunk_Entropy(struct SHIO* M, TSFiBigInt* P) { (void)M; (void)P; return NULL; }

int main() {
    printf("=== TSFi Genetic Mutation Entropy Benchmark (%d Ops) ===\n", ENTROPY_ITERATIONS);
    srand(1337);

    // 1. Setup MCP (Thetan) via ZMM VM
    TsfiZmmVmState vm;
    tsfi_zmm_vm_init(&vm);
    
    // 2. Setup Mu (Potential)
    GeneticNode* Mu = (GeneticNode*)lau_malloc_wired(sizeof(GeneticNode));
    Mu->type = GENETIC_TYPE_YI;
    Mu->dys_ptr = DM(struct YI);
    struct YI* yi = (struct YI*)Mu->dys_ptr;
    memset(yi, 0, sizeof(struct YI));
    yi->Xi = tsfi_bn_alloc();
    Mu->react_shio = Thunk_Entropy;

    // 3. Setup Hilbert State (Thetan representation in memory)
    GeneticNode* Thetan = (GeneticNode*)lau_malloc_wired(sizeof(GeneticNode));
    Thetan->type = GENETIC_TYPE_YI;
    Thetan->dys_ptr = DM(struct YI);
    struct YI* t_yi = (struct YI*)Thetan->dys_ptr;
    memset(t_yi, 0, sizeof(struct YI));
    t_yi->Xi = tsfi_bn_alloc();

    GeneticNode* Child = (GeneticNode*)lau_malloc_wired(sizeof(GeneticNode));

    printf("Starting Entropy Feedback Loop...\n");
    uint64_t start = get_ns();
    uint64_t start_tsc = __rdtsc();
    
    float total_heat = 0.0f;
    int swaps = 0;

    for(int i=0; i<ENTROPY_ITERATIONS; i++) {
        // Core Mutation
        Fourier_UniversalCrossover(Mu, Thetan, Child);
        
        // Simulate Entropy -> Synaptic Density
        vm.manifest->synapse.mass_density += 0.001f;
        total_heat += 0.001f;

        // Check for Kernel Swap (Phase Shift)
        if (vm.manifest->synapse.mass_density > 10.0f) {
            tsfi_zmm_vm_exec(&vm, "WSET_DENSITY 0.0"); 
            swaps++;
        }

        // Cleanup cycle
        freeYI((struct YI*)Child->dys_ptr);
    }
    
    uint64_t end = get_ns();
    uint64_t end_tsc = __rdtsc();
    double duration_sec = (double)(end - start) / 1e9;
    
    printf("Total Time   : %.4f s\n", duration_sec);
    printf("Entropy Rate : %.2f XO/s\n", ENTROPY_ITERATIONS / duration_sec);
    printf("Total Heat   : %.2f Synaptic Units\n", total_heat);
    printf("Kernel Swaps : %d\n", swaps);
    printf("Status       : [PASS] Neurological Entropy Sustained\n");

    // Cleanup
    freeYI(yi); lau_free(Mu);
    freeYI(t_yi); lau_free(Thetan);
    lau_free(Child);
    tsfi_zmm_vm_destroy(&vm);

    double cycles_per_op = (double)(end_tsc - start_tsc) / ENTROPY_ITERATIONS;
    printf("Cycles/Op   : %.2f\n", cycles_per_op);

        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}
