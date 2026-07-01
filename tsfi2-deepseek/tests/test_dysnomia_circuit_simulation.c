#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "tsfi_dysnomia_circuit_simulation.h"

int main() {
    printf("=== Dysnomia Lock Accumulator Circuit Simulation Test ===\n\n");

    DysnomiaLockCircuit circuit;
    tsfi_circuit_init(&circuit);

    printf("[INIT] Verifying default FUSE parameters:\n");
    printf("  -> Prime:       %lu\n", circuit.prime);
    printf("  -> Base:        %lu\n", circuit.base);
    printf("  -> Secret Rod:  %lu\n", circuit.secret_rod);
    printf("  -> Secret Cone: %lu\n", circuit.secret_cone);

    assert(circuit.prime == 953467954114363 && "Prime Mismatch!");
    assert(circuit.base == 381387181645745 && "Base Mismatch!");

    // 1. Execute unlocked cascade
    printf("\n[UNLOCKED] Executing full cascade (AVAIL -> MAGNETIZE)...\n");
    bool success = tsfi_circuit_execute_cascade(&circuit);
    assert(success && "Unlocked Cascade Failed!");

    printf("  -> Unlocked Monopole:  0x%lx\n", circuit.monopole);
    printf("  -> Unlocked Manifold:  0x%lx\n", circuit.manifold);
    printf("  -> Unlocked Ring:      0x%lx\n", circuit.ring);
    printf("  -> Unlocked Barn:      0x%lx\n", circuit.barn);

    // 2. Lock in accumulator coordinates
    printf("\n[LOCK] Freezing secondary accumulator values Anpn=42, Apnp=13...\n");
    tsfi_circuit_lock(&circuit, 42, 13);
    assert(circuit.is_locked && "Lock Latch Failed!");

    // 3. Execute locked modulated cascade
    printf("[LOCKED] Executing modulated cascade...\n");
    success = tsfi_circuit_execute_cascade(&circuit);
    assert(success && "Locked Cascade Failed!");

    printf("  -> Locked Monopole:  0x%lx\n", circuit.monopole);
    printf("  -> Locked Manifold:  0x%lx\n", circuit.manifold);
    printf("  -> Locked Ring:      0x%lx\n", circuit.ring);
    printf("  -> Locked Barn:      0x%lx\n", circuit.barn);

    // 4. Unlock and reset
    printf("\n[UNLOCK] Releasing lock register...\n");
    tsfi_circuit_unlock(&circuit);
    assert(!circuit.is_locked && "Unlock Failed!");

    printf("\n[SUCCESS] Dysnomia Lock Accumulator Circuit Verified.\n");
    return 0;
}
