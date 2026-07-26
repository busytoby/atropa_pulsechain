#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include "../tsfi2-deepseek/inc/tsfi_npnpnpvalve.h"

int main(void) {
    printf("=== RUNNING AUNCIENT NPNPNPVALVE CHARGE TESTS ===\n");

    TSFiNpnPnpValve valve;
    // Coupling Capacitance = 10uF
    tsfi_npnpnpvalve_init(&valve, 10e-6);

    assert(fabs(valve.coupling_capacitance - 10e-6) < 1e-9);
    assert(fabs(valve.npn_charge - 0.0) < 1e-5);

    // Initial state: conductance should be low (potential below 0.7V threshold)
    double init_cond = tsfi_npnpnpvalve_get_conductance(&valve);
    assert(init_cond < 0.1);

    // Apply 5V input to trigger charge flow
    for (int i = 0; i < 100; i++) {
        tsfi_npnpnpvalve_update(&valve, 0.001, 5.0);
    }

    // After charging, NPN/PNP charge potential expands, opening the valve conductance
    double active_cond = tsfi_npnpnpvalve_get_conductance(&valve);
    assert(active_cond > init_cond);

    printf("   ✓ NPN-PNP charge potential switches verified.\n");
    printf("   ✓ Sigmoid valve conductance thresholds verified successfully.\n");
    printf("=== AUNCIENT NPNPNPVALVE CHARGE TESTS COMPLETE (PASS) ===\n");
    return 0;
}
