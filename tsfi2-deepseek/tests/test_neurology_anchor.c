#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_neurology.h"
#include "tsfi_helmholtz.h"
#include "tsfi_genetic.h"
#include "tsfi_vision.h"
#include "tsfi_io.h"

int main() {
    printf("=== TSFi Neurology Anchor Test ===\n");
    
    TSFiSGP sgp = {0};
    TSFiVGP vgp = {0};
    
    // 1. Initial State: Zero Root (Should Fail)
    printf("[TEST] Testing kernel with Zero Root (Expect Failure)...\n");
    tsfi_yang_process(&sgp, &vgp);
    
    if (vgp.op_ctrl == 0xDEADBEEF) {
        printf("[PASS] Kernel blocked unanchored manifold.\n");
    } else {
        printf("[FAIL] Kernel executed without Acoustic Proof! (Ctrl: 0x%08X)\n", vgp.op_ctrl);
        exit(1);
    }
    
    // 2. Establishing Proof of State
    printf("[TEST] Establishing Proof of State for Epoch 1...\n");
    tsfi_helmholtz_init();
    void *manifold = calloc(1, 512 * 1024);
    uint8_t state_root[32];
    uint8_t receipt_root[32];
    tsfi_helmholtz_reduce_11(state_root, receipt_root, NULL, NULL, manifold, 1, 2026, NULL);
    
    // 3. Anchored State: Valid Root (Should Pass)
    printf("[TEST] Testing kernel with Valid Root (Expect Success)...\n");
    memcpy(sgp.state_root, state_root, 32);
    // Initialize dummy weights for Mu -> YANG
    for(int i=0; i<8; i++) sgp.weights[i] = 0.5;
    sgp.banach_norm = 1.0;

    tsfi_yang_process(&sgp, &vgp);
    
    // 4. Kirchhoff Phase Gating (Weight Resonance)
    printf("[TEST] Testing K0Rn Weight Resonance Gating...\n");
    K0RnStream *stream = tsfi_k0rn_compile_teddy();
    TSFiResonanceAnalysis vis = {0};
    
    // Case A: Zero Root (Should be zero intensity/no mutation)
    float old_x = stream->ops[0].x;
    tsfi_k0rn_evolve_vision(stream, &vis, 1.0f);
    if (stream->ops[0].x == old_x) {
        printf("[PASS] Weight Resonance blocked for unanchored state.\n");
    } else {
        printf("[FAIL] Weights mutated without Acoustic Proof!\n");
        exit(1);
    }
    
    // Case B: Valid Root (Should allow mutation)
    memcpy(vis.state_root, state_root, 32);
    tsfi_k0rn_evolve_vision(stream, &vis, 1.0f);
    if (stream->ops[0].x != old_x) {
        printf("[PASS] Weight Resonance active for anchored state.\n");
    } else {
        printf("[FAIL] Weights remained static despite valid proof.\n");
        exit(1);
    }

    free(manifold);
    
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    extern void lau_assert_zero_unsealed_leaks(const char *context, void *teardown_ptr);
    lau_assert_zero_unsealed_leaks("test", NULL);
    return 0;
}
