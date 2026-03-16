#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_zmm_vm.h"
#include "tsfi_manifest.h"
#include "lau_memory.h"

// Test: Crossover Discovery (Manifold Expansion)
// Verifies that recombinating tool signatures creates a new inevitable path.

int main() {
    printf("[TEST] MCP Crossover Discovery (Evolutionary Path)...\n");

    TsfiZmmVmState state;
    tsfi_zmm_vm_init(&state);

    // Get Parents
    const TSFiPhiAtom* p1 = &state.manifold->lattice[0]; // wave512.run
    const TSFiPhiAtom* p2 = &state.manifold->lattice[1]; // shell.read_file

    // Perform Crossover
    TSFiPhiAtom child;
    tsfi_phi_crossover(p1, p2, &child);

    printf("  [PROCESS] Evolved 'wave512.run' + 'shell.read_file' -> 'wave512.read_evolved'\n");

    // Add child to manifold
    state.manifold->lattice[state.manifold->count] = child;
    state.manifold->count++;

    // Resolve Mu that matches child
    uint32_t mu_child[16];
    memcpy(mu_child, child.state_vector, 64);
    
    uint32_t tension = 0;
    int32_t res = tsfi_phi_resolve(mu_child, state.manifold, &tension);

    if (res == 2) {
        printf("    PASS: New evolved tool manifested at index 2.\n");
    } else {
        printf("    FAIL: Evolved path not inevitable (res=%d, tension=%u)\n", res, tension);
        return 1;
    }

    // Verify Secret Inheritance (Handshake)
    if (child.secret_provenance & 0x80000000) {
        printf("    PASS: Child carries Evolutionary Flag.\n");
    } else {
        printf("    FAIL: Flag lost.\n");
        return 1;
    }

    tsfi_zmm_vm_destroy(&state);
    printf("[TEST] Crossover Discovery Complete.\n");
        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}