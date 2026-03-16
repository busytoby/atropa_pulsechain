#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../src/tsfi_nand_trap_firmware.c"

int main() {
    printf("=== TSFi NAND Trap Persistence Audit ===\n");
    
    NandTrapState s1;
    memset(&s1, 0, sizeof(NandTrapState));
    s1.epoch = 123.456f;
    s1.genome = 2; // Bipedal
    for(int i=0; i<GRANS; i++) {
        s1.fibers[i].x = (float)i * 0.1f;
        s1.fibers[i].weight = (float)i * 1.5f;
    }

    const char *test_path = "tmp/nand_trap_state.bin";
    system("mkdir -p tmp");

    printf("[STAGE 1] Persisting State to %s\n", test_path);
    int rc = tsfi_nand_trap_save(test_path, &s1);
    if (rc != 0) {
        printf("[FAIL] Save failed with rc %d\n", rc);
        return 1;
    }

    NandTrapState s2;
    memset(&s2, 0, sizeof(NandTrapState));
    
    printf("[STAGE 2] Restoring State from %s\n", test_path);
    rc = tsfi_nand_trap_restore(test_path, &s2);
    if (rc != 0) {
        printf("[FAIL] Restore failed with rc %d\n", rc);
        return 1;
    }

    // Validation
    if (s1.epoch != s2.epoch) { printf("Epoch mismatch\n"); return 1; }
    if (s1.genome != s2.genome) { printf("Genome mismatch\n"); return 1; }
    for(int i=0; i<GRANS; i++) {
        if (s1.fibers[i].x != s2.fibers[i].x) { printf("Fiber %d x mismatch\n", i); return 1; }
        if (s1.fibers[i].weight != s2.fibers[i].weight) { printf("Fiber %d weight mismatch\n", i); return 1; }
    }

    printf("[SUCCESS] Bit-perfect NAND trap persistence confirmed.\n");
    return 0;
}
