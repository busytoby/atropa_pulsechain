#include <stdio.h>
#include <stdlib.h>
#include "../thunks/tsfi_rau_thunk.h"
#include <assert.h>

int main(void) {
    printf("=============================================================\n");
    printf("YUL RAU BULK SERIALIZATION THUNK TEST\n");
    printf("=============================================================\n");

    uint64_t source_bank[32];
    uint64_t dest_bank[32];

    // Initialize test bank with distinct values
    for (int i = 0; i < 32; i++) {
        source_bank[i] = i * 1111;
        dest_bank[i] = 0;
    }

    // 1. Load the bank via the thunk
    printf("[TEST] Loading 32-word register bank via RAU thunk...\n");
    int load_res = tsfi_rau_thunk_load_registers(1, source_bank);
    assert(load_res == 0);

    // 2. Dump the bank via the thunk
    printf("[TEST] Dumping 32-word register bank via RAU thunk...\n");
    int dump_res = tsfi_rau_thunk_dump_registers(1, dest_bank);
    assert(dump_res == 0);

    // 3. Verify all registers were serialized correctly
    printf("\n[TEST] Verifying serialization invariants...\n");
    for (int i = 0; i < 32; i++) {
        assert(dest_bank[i] == source_bank[i]);
    }
    printf("   ✓ All 32 registers matched exactly between load and dump!\n");

    printf("=============================================================\n");
    printf("YUL RAU THUNK TESTS COMPLETED SUCCESSFULLY\n");
    printf("=============================================================\n");
    return 0;
}
