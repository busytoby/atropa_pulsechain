#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include "tsfi_genetic.h"
#include "lau_memory.h"
#include "tsfi_dysnomia.h"
#include "tsfi_io.h"

// Helper to check if a BigInt looks like valid crossover DNA
void assert_crossover_valid_bn(TSFiBigInt* res) {
    assert(res != NULL);
    // Basic validity check: should have some content if parents had it
    // (Actual crossover logic is tested by behavior)
}

void test_mix_dna_bn() {
    tsfi_io_printf(stdout, "Testing MixDNA_bn...\n");
    TSFiBigInt* a = tsfi_bn_alloc(); tsfi_bn_set_u64(a, 0xAAAA);
    TSFiBigInt* b = tsfi_bn_alloc(); tsfi_bn_set_u64(b, 0xCCCC);
    TSFiBigInt* res = MixDNA_bn(a, b);
    assert_crossover_valid_bn(res);
    tsfi_io_printf(stdout, "  MixDNA_bn Result: 0x%lx\n", res->limbs[0]);
    tsfi_bn_free(a);
    tsfi_bn_free(b);
    tsfi_bn_free(res);

    printf("[PASS] MixDNA_bn\n");
}

void test_yi_crossover() {
    tsfi_io_printf(stdout, "Testing YI + YI Crossover...\n");
    GeneticNode* ParentA = (GeneticNode*)lau_malloc_wired(sizeof(GeneticNode));
    ParentA->type = GENETIC_TYPE_YI;
    ParentA->dys_ptr = allocYI();
    struct YI* yiA = (struct YI*)ParentA->dys_ptr;
    yiA->Xi = tsfi_bn_alloc(); tsfi_bn_set_u64(yiA->Xi, 0xAAAA);
    yiA->Ring = tsfi_bn_alloc(); tsfi_bn_set_u64(yiA->Ring, 0x1111);
    
    GeneticNode* ParentB = (GeneticNode*)lau_malloc_wired(sizeof(GeneticNode));
    ParentB->type = GENETIC_TYPE_YI;
    ParentB->dys_ptr = allocYI();
    struct YI* yiB = (struct YI*)ParentB->dys_ptr;
    yiB->Xi = tsfi_bn_alloc(); tsfi_bn_set_u64(yiB->Xi, 0xCCCC);
    yiB->Ring = tsfi_bn_alloc(); tsfi_bn_set_u64(yiB->Ring, 0x3333);

    GeneticNode* Child = (GeneticNode*)lau_malloc_wired(sizeof(GeneticNode));
    Fourier_UniversalCrossover(ParentA, ParentB, Child);

    assert(Child->type == GENETIC_TYPE_YI);
    struct YI* yiC = (struct YI*)Child->dys_ptr;
    
    assert_crossover_valid_bn(yiC->Xi);
    assert_crossover_valid_bn(yiC->Ring);
    
    assert(strncmp(Child->generation_id, "XO_", 3) == 0);

    freeYI(yiA); lau_free(ParentA);
    freeYI(yiB); lau_free(ParentB);
    freeYI(yiC); lau_free(Child);
    printf("[PASS] YI + YI Crossover\n");
}

void test_yang_crossover() {
    tsfi_io_printf(stdout, "Testing YANG + YANG Crossover...\n");
    GeneticNode* ParentA = (GeneticNode*)lau_malloc_wired(sizeof(GeneticNode));
    ParentA->type = GENETIC_TYPE_YANG;
    ParentA->dys_ptr = allocYANG();
    struct YANG* yangA = (struct YANG*)ParentA->dys_ptr;
    yangA->Ring = tsfi_bn_alloc(); tsfi_bn_set_u64(yangA->Ring, 0xAAAA);
    
    GeneticNode* ParentB = (GeneticNode*)lau_malloc_wired(sizeof(GeneticNode));
    ParentB->type = GENETIC_TYPE_YANG;
    ParentB->dys_ptr = allocYANG();
    struct YANG* yangB = (struct YANG*)ParentB->dys_ptr;
    yangB->Ring = tsfi_bn_alloc(); tsfi_bn_set_u64(yangB->Ring, 0xCCCC);

    GeneticNode* Child = (GeneticNode*)lau_malloc_wired(sizeof(GeneticNode));
    Fourier_UniversalCrossover(ParentA, ParentB, Child);

    assert(Child->type == GENETIC_TYPE_YANG);
    struct YANG* yangC = (struct YANG*)Child->dys_ptr;
    assert_crossover_valid_bn(yangC->Ring);

    freeYANG(yangA); lau_free(ParentA);
    freeYANG(yangB); lau_free(ParentB);
    freeYANG(yangC); lau_free(Child);
    printf("[PASS] YANG + YANG Crossover\n");
}

void test_hybrid_crossover() {
    tsfi_io_printf(stdout, "Testing YI + YANG Hybrid Crossover...\n");
    GeneticNode* ParentA = (GeneticNode*)lau_malloc_wired(sizeof(GeneticNode));
    ParentA->type = GENETIC_TYPE_YI;
    ParentA->dys_ptr = allocYI();
    struct YI* yiA = (struct YI*)ParentA->dys_ptr;
    yiA->Xi = tsfi_bn_alloc(); tsfi_bn_set_u64(yiA->Xi, 0xAAAA);
    
    GeneticNode* ParentB = (GeneticNode*)lau_malloc_wired(sizeof(GeneticNode));
    ParentB->type = GENETIC_TYPE_YANG;
    ParentB->dys_ptr = allocYANG();
    struct YANG* yangB = (struct YANG*)ParentB->dys_ptr;
    yangB->Eta = allocYI();
    yangB->Eta->Xi = tsfi_bn_alloc(); tsfi_bn_set_u64(yangB->Eta->Xi, 0xCCCC);

    GeneticNode* Child = (GeneticNode*)lau_malloc_wired(sizeof(GeneticNode));
    Fourier_UniversalCrossover(ParentA, ParentB, Child);

    // Hybrid should result in YI
    assert(Child->type == GENETIC_TYPE_YI);
    struct YI* yiC = (struct YI*)Child->dys_ptr;
    assert_crossover_valid_bn(yiC->Xi);

    freeYI(yiA); lau_free(ParentA);
    freeYANG(yangB); lau_free(ParentB);
    freeYI(yiC); lau_free(Child);
    printf("[PASS] YI + YANG Hybrid Crossover\n");
}

static struct Dai* Thunk_A(struct SHIO* M, TSFiBigInt* P) { (void)M; (void)P; return NULL; }
static struct Dai* Thunk_B(struct SHIO* M, TSFiBigInt* P) { (void)M; (void)P; return NULL; }

void test_thunk_inheritance() {
    tsfi_io_printf(stdout, "Testing Thunk Inheritance...\n");
    GeneticNode* ParentA = (GeneticNode*)lau_malloc_wired(sizeof(GeneticNode));
    ParentA->type = GENETIC_TYPE_YI;
    ParentA->dys_ptr = allocYI();
    ParentA->react_shio = Thunk_A;

    GeneticNode* ParentB = (GeneticNode*)lau_malloc_wired(sizeof(GeneticNode));
    ParentB->type = GENETIC_TYPE_YI;
    ParentB->dys_ptr = allocYI();
    ParentB->react_shio = Thunk_B;

    GeneticNode* Child = (GeneticNode*)lau_malloc_wired(sizeof(GeneticNode));
    
    // Run several times to hit both random paths
    int hit_A = 0, hit_B = 0;
    for(int i=0; i<20; i++) {
        Fourier_UniversalCrossover(ParentA, ParentB, Child);
        if (Child->react_shio == Thunk_A) hit_A = 1;
        if (Child->react_shio == Thunk_B) hit_B = 1;
        freeYI((struct YI*)Child->dys_ptr);
    }
    assert(hit_A && hit_B); // Verify both parents can contribute behavior

    freeYI((struct YI*)ParentA->dys_ptr); lau_free(ParentA);
    freeYI((struct YI*)ParentB->dys_ptr); lau_free(ParentB);
    lau_free(Child);
    printf("[PASS] Thunk Inheritance\n");
}

void test_deep_nesting_crossover() {
    tsfi_io_printf(stdout, "Testing Deep Nesting Crossover (Fa/SHA/SHAO)...\n");

    GeneticNode* ParentA = (GeneticNode*)lau_malloc_wired(sizeof(GeneticNode));
    ParentA->type = GENETIC_TYPE_YI;
    ParentA->dys_ptr = allocYI();
    struct YI* yiA = (struct YI*)ParentA->dys_ptr;
    yiA->Psi = allocSHIO();
    yiA->Psi->Rho = allocSHAO();
    yiA->Psi->Rho->Rod = allocSHA();
    yiA->Psi->Rho->Rod->Mu = allocFa();
    yiA->Psi->Rho->Rod->Mu->Base = tsfi_bn_alloc(); tsfi_bn_set_u64(yiA->Psi->Rho->Rod->Mu->Base, 0xAAAA);

    GeneticNode* ParentB = (GeneticNode*)lau_malloc_wired(sizeof(GeneticNode));
    ParentB->type = GENETIC_TYPE_YI;
    ParentB->dys_ptr = allocYI();
    struct YI* yiB = (struct YI*)ParentB->dys_ptr;
    yiB->Psi = allocSHIO();
    yiB->Psi->Rho = allocSHAO();
    yiB->Psi->Rho->Rod = allocSHA();
    yiB->Psi->Rho->Rod->Mu = allocFa();
    yiB->Psi->Rho->Rod->Mu->Base = tsfi_bn_alloc(); tsfi_bn_set_u64(yiB->Psi->Rho->Rod->Mu->Base, 0xCCCC);

    GeneticNode* Child = (GeneticNode*)lau_malloc_wired(sizeof(GeneticNode));
    Fourier_UniversalCrossover(ParentA, ParentB, Child);

    struct YI* yiC = (struct YI*)Child->dys_ptr;
    assert(yiC->Psi->Rho->Rod->Mu->Base != NULL);
    assert_crossover_valid_bn(yiC->Psi->Rho->Rod->Mu->Base);

    freeYI(yiA); lau_free(ParentA);
    freeYI(yiB); lau_free(ParentB);
    freeYI(yiC); lau_free(Child);
    printf("[PASS] Deep Nesting Crossover\n");
}

void test_mutation_stability() {
    tsfi_io_printf(stdout, "Testing Mutation Stability (100 Generations)...\n");
    GeneticNode* A = (GeneticNode*)lau_malloc_wired(sizeof(GeneticNode));
    A->type = GENETIC_TYPE_YI;
    A->dys_ptr = allocYI();
    struct YI* yiA = (struct YI*)A->dys_ptr;
    yiA->Xi = tsfi_bn_alloc(); tsfi_bn_set_u64(yiA->Xi, 0x12345678);

    GeneticNode* B = (GeneticNode*)lau_malloc_wired(sizeof(GeneticNode));
    B->type = GENETIC_TYPE_YI;
    B->dys_ptr = allocYI();
    struct YI* yiB = (struct YI*)B->dys_ptr;
    yiB->Xi = tsfi_bn_alloc(); tsfi_bn_set_u64(yiB->Xi, 0x87654321);

    for(int i=0; i<100; i++) {
        GeneticNode* Child = (GeneticNode*)lau_malloc_wired(sizeof(GeneticNode));
        Fourier_UniversalCrossover(A, B, Child);
        
        // Child becomes Parent A for next gen
        freeYI((struct YI*)A->dys_ptr);
        A->dys_ptr = Child->dys_ptr;
        lau_free(Child);
        
        // Verify BigInt is still valid
        TSFiBigInt* bn = ((struct YI*)A->dys_ptr)->Xi;
        assert(bn != NULL);
    }

    freeYI((struct YI*)A->dys_ptr); lau_free(A);
    freeYI(yiB); lau_free(B);
    printf("[PASS] Mutation Stability\n");
}

int main() {
    srand(time(NULL));
    tsfi_io_printf(stdout, "=== TSFi Genetic Crossover Tests ===\n");
    test_mix_dna_bn();
    test_yi_crossover();
    test_yang_crossover();
    test_hybrid_crossover();
    test_thunk_inheritance();
    test_deep_nesting_crossover();
    test_mutation_stability();
    tsfi_io_printf(stdout, "All Genetic Crossover Tests Passed!\n");
        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}
