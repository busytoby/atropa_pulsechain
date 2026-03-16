#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include "tsfi_nand_trap_firmware.h"

int main() {
    const char *storage = "tmp/test_autonomous_state.bin";
    unlink(storage); // Start fresh

    printf("[TEST] Running Autonomous Step 1 (Initializes State)...\n");
    int status1 = tsfi_nand_trap_autonomous_step(storage, 0.1f);
    if (status1 != 0) {
        printf("[FAILURE] Step 1 failed with status %d\n", status1);
        return 1;
    }

    NandTrapState s1;
    int r1 = tsfi_nand_trap_restore(storage, &s1);
    if (r1 != 0) {
        printf("[FAILURE] Restore 1 failed with status %d\n", r1);
        return 1;
    }
    printf("  Epoch after step 1: %.2f\n", s1.epoch);
    assert(s1.epoch == 1.0f);
    assert(s1.magic == NAND_TRAP_MAGIC);

    printf("[TEST] Running Autonomous Step 2 (Evolves from Persistent State)...\n");
    int status2 = tsfi_nand_trap_autonomous_step(storage, 0.1f);
    if (status2 != 0) {
        printf("[FAILURE] Step 2 failed with status %d\n", status2);
        return 1;
    }

    NandTrapState s2;
    int r2 = tsfi_nand_trap_restore(storage, &s2);
    if (r2 != 0) {
        printf("[FAILURE] Restore 2 failed with status %d\n", r2);
        return 1;
    }
    printf("  Epoch after step 2: %.2f\n", s2.epoch);
    assert(s2.epoch == 2.0f);
    assert(s2.magic == NAND_TRAP_MAGIC);

    // Verify fiber change (Ricci discovery)
    printf("  Fiber 0 evolved from %.4f to %.4f\n", s1.fibers[0].x, s2.fibers[0].x);
    if (s1.fibers[0].x == s2.fibers[0].x) {
        printf("[FAILURE] Fiber state did not evolve!\n");
        return 1;
    }

    printf("[SUCCESS] NAND Trap Autonomous Persistence Verified.\n");
    return 0;
}
