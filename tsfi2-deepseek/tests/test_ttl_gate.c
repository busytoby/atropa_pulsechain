#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include "tsfi_ttl_gate.h"

int main() {
    printf("=== TSFi2 & Computer Design Issue #11 TTL NAND Gate Test ===\n");

    TsfiTtlNandGate gate;
    tsfi_ttl_nand_init(&gate);

    // Test 1: Initial state (inputs low)
    printf("[TTL-NAND] Initial state: V_out = %fV, State = %d\n", gate.v_out, gate.state);
    assert(gate.state == 1);
    assert(gate.v_out == 3.5f);

    float dt = 0.05f; // 0.05 ns step
    float v_out = 3.5f;

    // Test 2: Apply Inputs A=1, B=1 (output should switch to Logic 0)
    printf("[TTL-NAND] Applying inputs A=1, B=1...\n");
    float time_to_fall = 0.0f;
    while (v_out > 1.4f && time_to_fall < 100.0f) {
        tsfi_ttl_nand_step(&gate, 1, 1, dt, &v_out);
        time_to_fall += dt;
    }
    printf("[TTL-NAND] Switched to Logic 0 in %f ns (V_out = %fV, base_charge = %f pC)\n",
           time_to_fall, v_out, gate.base_charge_pf);
    assert(gate.state == 0);
    assert(time_to_fall < 20.0f); // Fast active pull-down

    // Test 3: Apply Inputs A=0, B=1 (output recovery to Logic 1)
    printf("[TTL-NAND] Restoring input A=0 (switches output back to Logic 1 via totem-pole)...\n");
    float time_to_rise = 0.0f;
    while (v_out < 1.4f && time_to_rise < 100.0f) {
        tsfi_ttl_nand_step(&gate, 0, 1, dt, &v_out);
        time_to_rise += dt;
    }
    printf("[TTL-NAND] Restored to Logic 1 in %f ns (V_out = %fV, base_charge = %f pC)\n",
           time_to_rise, v_out, gate.base_charge_pf);
    assert(gate.state == 1);
    assert(time_to_rise < 25.0f); // Fast active pull-up recovery

    // Check for memory leaks
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    extern void lau_assert_zero_unsealed_leaks(const char *context, void *teardown_ptr);
    lau_assert_zero_unsealed_leaks("test_ttl_gate", NULL);

    printf("[PASS] Transistor-Transistor Logic NAND gate simulation verified.\n");
    return 0;
}
