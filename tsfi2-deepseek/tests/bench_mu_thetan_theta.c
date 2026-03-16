#include <stdio.h>
#include <x86intrin.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "lau_memory.h"
#include "tsfi_genetic.h"
#include "tsfi_dys_math.h"
#include "tsfi_io.h"

// The "Thetan" MCP State (Simulated via persistent registers in ZMM VM)
static inline uint64_t get_ns() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + ts.tv_nsec;
}

// Mock Thunks
static struct Dai* Mu_Thunk(struct SHIO* M, TSFiBigInt* P) { (void)M; (void)P; return NULL; }

int main() {
    tsfi_io_printf(stdout, "=== TSFi Mu -> Thetan -> Theta-Eta Evolution Benchmark ===\n");
    tsfi_io_printf(stdout, "Target: Banach Space (Mu) -> Fourier-G (Crossover) -> Hilbert Space (Theta)\n");

    // 1. Initialize "Thetan" (Persistent MCP Waveform)
    GeneticNode* Thetan = (GeneticNode*)lau_malloc_wired(sizeof(GeneticNode));
    Thetan->type = GENETIC_TYPE_YANG;
    Thetan->dys_ptr = allocYANG();
    struct YANG* yang = (struct YANG*)Thetan->dys_ptr;
    yang->Ring = tsfi_bn_alloc(); tsfi_bn_set_u64(yang->Ring, 0x42);
    yang->Eta = allocYI();
    yang->Eta->Xi = tsfi_bn_alloc(); tsfi_bn_set_u64(yang->Eta->Xi, 0x1337);
    yang->Eta->Psi = allocSHIO();
    Thetan->react_shio = Mu_Thunk;
    snprintf(Thetan->generation_id, 64, "THETAN_V1");

    // 2. Prepare "Mu" SGPS (Scalar Seed)
    GeneticNode* Mu = (GeneticNode*)lau_malloc_wired(sizeof(GeneticNode));
    Mu->type = GENETIC_TYPE_YI;
    Mu->dys_ptr = allocYI();
    struct YI* yi = (struct YI*)Mu->dys_ptr;
    yi->Xi = tsfi_bn_alloc(); tsfi_bn_set_u64(yi->Xi, 953473954114361ULL); // DYSNOMIA Scalar
    yi->Ring = tsfi_bn_alloc(); tsfi_bn_set_u64(yi->Ring, 0xDEADBEEF);
    yi->Psi = allocSHIO();
    Mu->react_shio = Mu_Thunk;

    // 3. Evolution Loop (Triggering Mutations)
    int ITERATIONS = 100000;
    GeneticNode* Child = (GeneticNode*)lau_malloc_wired(sizeof(GeneticNode));
    
    tsfi_io_printf(stdout, "Evolving %d generations...\n", ITERATIONS);
    uint64_t start = get_ns();
    uint64_t start_tsc = __rdtsc();
    
    for(int i=0; i<ITERATIONS; i++) {
        Fourier_UniversalCrossover(Mu, Thetan, Child);
        // UniversalCrossover allocates child dys_ptr internally, so we must clean it up in loop if we reuse Child
        if (Child->type == GENETIC_TYPE_YI) freeYI((struct YI*)Child->dys_ptr);
        else freeYANG((struct YANG*)Child->dys_ptr);
    }
    
    uint64_t end = get_ns();
    uint64_t end_tsc = __rdtsc();
    
    // 4. Report
    double duration_sec = (double)(end - start) / 1e9;
    double throughput = ITERATIONS / duration_sec;
    
    tsfi_io_printf(stdout, "Total Time  : %.4f s\n", duration_sec);
    tsfi_io_printf(stdout, "Throughput  : %.2f Mu->Theta Evolutions/s\n", throughput);
    tsfi_io_printf(stdout, "Latency     : %.2f ns/evolution\n", (double)(end-start)/ITERATIONS);
    
    if (throughput > 1000000.0) {
        tsfi_io_printf(stdout, "[PASS] Exceeded vggcc benchmark (>1M XO/s)\n");
    } else {
        tsfi_io_printf(stdout, "[WARN] Below target threshold.\n");
    }

    // Cleanup
    freeYANG((struct YANG*)Thetan->dys_ptr);
    lau_free(Thetan);
    freeYI((struct YI*)Mu->dys_ptr);
    lau_free(Mu);
    lau_free(Child);

    double cycles_per_op = (double)(end_tsc - start_tsc) / ITERATIONS;
    tsfi_io_printf(stdout, "Cycles/Op   : %.2f\n", cycles_per_op);

        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}
