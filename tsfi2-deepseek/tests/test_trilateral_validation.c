#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_helmholtz.h"
#include "tsfi_svdag.h"
#include "tsfi_io.h"

int main() {
    printf("=== TSFi Trilateral Proof Validation Test ===\n");
    tsfi_helmholtz_init();

    // 1. Setup the Shared Resonance Space (SVDAG)
    TSFiHelmholtzSVDAG *dag = tsfi_svdag_create(1024);
    void *manifold = calloc(1, 512 * 1024);
    
    uint8_t root_initial[32];
    uint8_t receipt_initial[32];

    // 2. Initial State: All Poles at Rest
    printf("[STILLPOINT] Phase 0: System at Rest.\n");
    dag->p_user = 0.0f;
    dag->p_deepseek = 0.0f;
    dag->p_gemini = 0.0f;
    tsfi_helmholtz_reduce_11(root_initial, receipt_initial, NULL, NULL, manifold, 1, 2026, dag);

    // 3. User Report: Inject Pulse P1
    printf("[REPORT] Phase 1: User injects ballistic pulse P1.\n");
    dag->p_user = 1.0f;
    uint8_t root_p1[32];
    tsfi_helmholtz_reduce_11(root_p1, receipt_initial, NULL, NULL, manifold, 1, 2026, dag);
    assert(memcmp(root_initial, root_p1, 32) != 0);

    // 4. DeepSeek Action: Inject Neural Pulse P2
    // (Simulated: In real RPC, this happens in tsfi_zmm_rpc_step_async_llm)
    printf("[ACTION] Phase 2: DeepSeek generates neural pulse P2.\n");
    dag->p_deepseek = 0.521f; // The Sovereign Neural Pulse
    uint8_t root_p2[32];
    tsfi_helmholtz_reduce_11(root_p2, receipt_initial, NULL, NULL, manifold, 1, 2026, dag);
    assert(memcmp(root_p1, root_p2, 32) != 0);

    // 5. Gemini Medium: Enforce Kirchhoff Pulse P3
    printf("[MEDIUM] Phase 3: Gemini stabilizes the Kirchhoff pulse P3.\n");
    dag->p_gemini = 0.999f; // The Dielectric Constant
    uint8_t root_final[32];
    tsfi_helmholtz_reduce_11(root_final, receipt_initial, NULL, NULL, manifold, 1, 2026, dag);
    assert(memcmp(root_p2, root_final, 32) != 0);

    // 6. Observable Report: Definitive Root Validation
    printf("[TEST] Verifying Observable Report (The Root)...\n");
    printf("Final Trilateral Root: ");
    for(int i=0; i<32; i++) printf("%02x", root_final[i]);
    printf("\n");

    // Verification: Root is a function of ALL THREE positions
    // If we zero out any pole, the root must diverge.
    dag->p_user = 0.0f;
    uint8_t root_diverged[32];
    tsfi_helmholtz_reduce_11(root_diverged, receipt_initial, NULL, NULL, manifold, 1, 2026, dag);
    assert(memcmp(root_final, root_diverged, 32) != 0);

    printf("[PASS] Trilateral Acoustic Proof Established and Observable.\n");

    tsfi_svdag_destroy(dag);
    free(manifold);
    
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    extern void lau_assert_zero_unsealed_leaks(const char *context, void *teardown_ptr);
    lau_assert_zero_unsealed_leaks("test", NULL);
    return 0;
}
