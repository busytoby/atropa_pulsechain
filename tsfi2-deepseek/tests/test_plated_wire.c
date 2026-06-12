#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include "tsfi_plated_wire.h"

int main() {
    printf("=== TSFi2 & Computer Design Issue #15 Plated-Wire Memory Test ===\n");

    TsfiPlatedWireCell cell;
    tsfi_plated_wire_init(&cell);

    // Initial state check
    printf("[PLATED-WIRE] Initial State: %d, Theta: %f\n", cell.state, cell.theta);
    assert(cell.state == 1);
    assert(cell.theta == 0.0f);

    float dt = 0.5f; // 0.5 ns steps
    float v_sense = 0.0f;

    // Test 1: Read State 1 (NDRO verification)
    printf("[PLATED-WIRE] Reading State 1 (NDRO)...\n");
    // Apply word current (transverse field)
    float peak_v_sense = 0.0f;
    for (int i = 0; i < 5; i++) {
        tsfi_plated_wire_step(&cell, 1.5f, 0.0f, dt, &v_sense);
        if (fabsf(v_sense) > fabsf(peak_v_sense)) {
            peak_v_sense = v_sense;
        }
    }
    printf("[PLATED-WIRE]   Sense voltage peak: %f mV (Expected positive)\n", peak_v_sense);
    assert(peak_v_sense > 0.0f);

    // Remove word current and let it settle
    tsfi_plated_wire_step(&cell, 0.0f, 0.0f, dt, &v_sense);
    printf("[PLATED-WIRE]   State after read: %d, Theta: %f\n", cell.state, cell.theta);
    assert(cell.state == 1); // State must be retained (Non-Destructive!)

    // Test 2: Write State 0
    printf("[PLATED-WIRE] Writing State 0...\n");
    // Apply word field to rotate to hard axis, and negative bit field to steer
    for (int i = 0; i < 40; i++) {
        tsfi_plated_wire_step(&cell, 1.5f, -1.0f, dt, &v_sense);
    }
    // Remove word field to write state, keeping bit steering field on
    tsfi_plated_wire_step(&cell, 0.0f, -1.0f, dt, &v_sense);
    // Finally turn off bit current
    tsfi_plated_wire_step(&cell, 0.0f, 0.0f, dt, &v_sense);
    printf("[PLATED-WIRE]   State after write: %d, Theta: %f\n", cell.state, cell.theta);
    assert(cell.state == 0);

    // Test 3: Read State 0 (NDRO verification)
    printf("[PLATED-WIRE] Reading State 0 (NDRO)...\n");
    // Apply word current (transverse field)
    peak_v_sense = 0.0f;
    for (int i = 0; i < 5; i++) {
        tsfi_plated_wire_step(&cell, 1.5f, 0.0f, dt, &v_sense);
        if (fabsf(v_sense) > fabsf(peak_v_sense)) {
            peak_v_sense = v_sense;
        }
    }
    printf("[PLATED-WIRE]   Sense voltage peak: %f mV (Expected negative)\n", peak_v_sense);
    assert(peak_v_sense < 0.0f);

    // Remove word current and let it settle
    tsfi_plated_wire_step(&cell, 0.0f, 0.0f, dt, &v_sense);
    printf("[PLATED-WIRE]   State after read: %d, Theta: %f\n", cell.state, cell.theta);
    assert(cell.state == 0); // State must be retained (Non-Destructive!)

    // Test 4: Write State 1
    printf("[PLATED-WIRE] Writing State 1...\n");
    for (int i = 0; i < 40; i++) {
        tsfi_plated_wire_step(&cell, 1.5f, 1.0f, dt, &v_sense);
    }
    tsfi_plated_wire_step(&cell, 0.0f, 1.0f, dt, &v_sense);
    tsfi_plated_wire_step(&cell, 0.0f, 0.0f, dt, &v_sense);
    printf("[PLATED-WIRE]   State after write: %d, Theta: %f\n", cell.state, cell.theta);
    assert(cell.state == 1);

    // Memory sanitation checks
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    extern void lau_assert_zero_unsealed_leaks(const char *context, void *teardown_ptr);
    lau_assert_zero_unsealed_leaks("test_plated_wire", NULL);

    printf("[PASS] Plated-Wire Memory simulation verified.\n");
    return 0;
}
