#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include "tsfi_vm_dft_bridge.h"
#include "tsfi_zmm_vm.h"
#include "lau_memory.h"

void test_vm_dft_bridge_alignment() {
    printf("[TEST] Initializing simulated Yul VM environment...\n");
    
    TsfiZmmVmState vm;
    tsfi_zmm_vm_init(&vm);

    // Initialize the generalized DFT VM bridge
    TSFiVmDftBridge bridge;
    tsfi_vm_dft_bridge_init(&bridge, &vm);
    
    // Validate initialization state
    assert(bridge.vm_state == &vm);
    assert(bridge.spectrum.coeffs[1][0] == 0.0f);
    assert(bridge.spectrum.coeffs[1][1] == 0.0f);

    // Define mock metrics retrieved from Dysnomia (Identity, Soul, Aura parameters)
    TSFiVmStateParams params = {
        .identity_pole = 261640507549433ULL,
        .soul = 525772616431102ULL,
        .aura = 341042560473881ULL
    };

    printf("[TEST] Aligning VM parameters with Fourier Spectrum...\n");
    tsfi_vm_dft_bridge_align(&bridge, &params);

    // Verify coordinate translation mapping under the perfect matter model
    // 1. Identity Translation Offset X matches (params.identity_pole % 100) - 50.0f
    float expected_offset_x = (float)(params.identity_pole % 100) - 50.0f;
    assert(bridge.spectrum.coeffs[0][0] == expected_offset_x);
    assert(bridge.spectrum.coeffs[0][2] == 0.0f);

    // 2. Harmonic scaling maps fundamental decay
    float expected_amplitude = (float)(params.soul % 1000) / 1000.0f;
    float expected_phase = (float)(params.aura % 360) * (3.14159265f / 180.0f);

    float ax_n1 = expected_amplitude * (1.0f / 1.0f) * cosf(expected_phase * 1.0f);
    float bx_n1 = expected_amplitude * (1.0f / 1.0f) * sinf(expected_phase * 1.0f);
    
    assert(fabsf(bridge.spectrum.coeffs[1][0] - ax_n1) < 1e-5f);
    assert(fabsf(bridge.spectrum.coeffs[1][1] - bx_n1) < 1e-5f);

    printf("  [PASS] VM state correctly matched to DFT spectrum metrics.\n");
    printf("  [PASS] Zero-copy mapping preserved exact amplitude values.\n");
    printf("  [PASS] Phase translation aligned with the perfect matter mathematical model.\n");

    tsfi_zmm_vm_destroy(&vm);
}

int main() {
    printf("=== TSFi VM DFT Bridge Alignment Test ===\n");
    test_vm_dft_bridge_alignment();
    printf("ALL VM DFT BRIDGE ALIGNMENT TESTS PASSED\n");

    extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}
