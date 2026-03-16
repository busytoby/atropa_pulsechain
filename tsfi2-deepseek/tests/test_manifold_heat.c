#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_helmholtz.h"

int main() {
    printf("=== TSFi Manifold Heat & Banach Rigidity Unit Test ===\n");
    tsfi_helmholtz_init();
    
    // 1 MiB Manifold (aligned for ZMM)
    void *manifold = aligned_alloc(64, 1024 * 1024);
    memset(manifold, 0, 1024 * 1024);
    
    // 1. Check Initial Heat
    float h0 = tsfi_helmholtz_get_heat();
    printf("[STILLPOINT] Initial Heat: %.4f\n", h0);
    assert(h0 == 0.0f);
    
    // 2. Trigger Banach Rigidity Violation
    // Fill leaf 0 with max-norm limbs to trigger the threshold in reduction
    uint64_t *l0 = (uint64_t*)manifold;
    for(int i=0; i<32; i++) l0[i] = 0xFFFFFFFFFFFFFFFFULL;
    
    printf("[ENTROPY] Phase 1: Reducing high-norm manifold...\n");
    uint8_t root[32], receipt[32];
    tsfi_helmholtz_reduce_11(root, receipt, NULL, NULL, manifold, 1, 2026, NULL);
    
    // 3. Verify Heat Accumulation
    float h1 = tsfi_helmholtz_get_heat();
    printf("[RESIST] Final Heat: %.4f\n", h1);
    assert(h1 > 0.0f);
    printf("[PASS] Banach Rigidity correctly identified high-norm entropy.\n");
    
    free(manifold);
    tsfi_helmholtz_cleanup();
    printf("[SUCCESS] Heat Monitoring is rigid.\n");
    
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    extern void lau_assert_zero_unsealed_leaks(const char *context, void *teardown_ptr);
    lau_assert_zero_unsealed_leaks("test", NULL);
    return 0;
}
