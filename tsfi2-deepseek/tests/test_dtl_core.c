#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "tsfi_dtl_core.h"

int main() {
    printf("=== TSFi2 & Computer Design Issue #1 DTL & Core Simulator Test ===\n");

    // 1. DTL NAND Gate Verification
    printf("[DTL] Verifying Diode-Transistor Logic NAND Truth Table...\n");
    float out_00 = tsfi_dtl_nand(0.2f, 0.2f);
    float out_01 = tsfi_dtl_nand(0.2f, 5.0f);
    float out_10 = tsfi_dtl_nand(5.0f, 0.2f);
    float out_11 = tsfi_dtl_nand(5.0f, 5.0f);

    printf("  NAND(0.2V, 0.2V) = %fV (expected 5.0V)\n", out_00);
    printf("  NAND(0.2V, 5.0V) = %fV (expected 5.0V)\n", out_01);
    printf("  NAND(5.0V, 0.2V) = %fV (expected 5.0V)\n", out_10);
    printf("  NAND(5.0V, 5.0V) = %fV (expected 0.2V)\n", out_11);

    assert(out_00 > 4.5f);
    assert(out_01 > 4.5f);
    assert(out_10 > 4.5f);
    assert(out_11 < 0.3f);

    // 2. Magnetic Core Verification
    printf("[CORE] Verifying Coincident-Current Hysteresis Logic...\n");
    TsfiMagneticCore core;
    tsfi_core_init(&core);
    printf("  Initial Flux Density B: %f (expected -1.0)\n", core.B);
    assert(core.B == -1.0f);

    // Coincident half-current check (should NOT flip)
    printf("  Applying half-write current H = 0.75 (below H_c = 1.0)...\n");
    tsfi_core_update(&core, 0.75f);
    assert(core.B == -1.0f);

    // Full coincident current (should flip to state 1)
    printf("  Applying full-write current H = 1.5 (Ix + Iy, above H_c = 1.0)...\n");
    tsfi_core_update(&core, 1.5f);
    assert(core.B == 1.0f);

    // Remove current (should retain state 1)
    printf("  Removing drive current H = 0.0 (remanence)...\n");
    tsfi_core_update(&core, 0.0f);
    assert(core.B == 1.0f);

    // Coincident half-current negative select (should NOT flip)
    printf("  Applying half-read current H = -0.75...\n");
    tsfi_core_update(&core, -0.75f);
    assert(core.B == 1.0f);

    // Full coincident negative current (should flip to state 0)
    printf("  Applying full-read current H = -1.5 (above H_c = 1.0)...\n");
    tsfi_core_update(&core, -1.5f);
    assert(core.B == -1.0f);

    // Check for memory leaks
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    extern void lau_assert_zero_unsealed_leaks(const char *context, void *teardown_ptr);
    lau_assert_zero_unsealed_leaks("test_dtl_core", NULL);

    printf("[PASS] DTL and Magnetic Core emulation completed successfully.\n");
    return 0;
}
