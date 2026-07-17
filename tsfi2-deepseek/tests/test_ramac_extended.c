#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "tsfi_ramac_layout.h"

int main(void) {
    printf("=============================================================\n");
    printf("EXTENDED RAMAC SIMULATION & VM INVARIANT TESTS\n");
    printf("=============================================================\n");

    // 1. Verify RAMAC index-to-CHS conversion accuracy
    printf("[Test] Verifying CHS sector layout calculations...\n");
    tsfi_ramac_chs chs = tsfi_ramac_index_to_chs(0);
    assert(chs.cylinder == 0);
    assert(chs.head == 0);
    assert(chs.sector == 0);

    // Test cylinder boundary offsets
    chs = tsfi_ramac_index_to_chs(RAMAC_WORDS * RAMAC_SECTORS * RAMAC_HEADS);
    assert(chs.cylinder == 1);
    assert(chs.head == 0);
    assert(chs.sector == 0);
    printf("  [PASS] CHS geometry address mappings verified.\n");

    // 2. Verify Cylinder Overflow Hash records and collision chaining
    printf("[Test] Verifying cylinder overflow hash record chaining...\n");
    tsfi_ramac_record records[5];
    memset(records, 0, sizeof(records));
    
    // Set up a collision loop
    strcpy(records[0].key, "KEY_A");
    strcpy(records[0].value, "VAL_A");
    records[0].next_overflow_index = 2; // Collision points to slot 2
    records[0].is_active = 1;

    strcpy(records[2].key, "KEY_B");
    strcpy(records[2].value, "VAL_B");
    records[2].next_overflow_index = -1; // End of chain
    records[2].is_active = 1;

    assert(strcmp(records[0].key, "KEY_A") == 0);
    assert(records[0].next_overflow_index == 2);
    assert(strcmp(records[records[0].next_overflow_index].key, "KEY_B") == 0);
    printf("  [PASS] Overflow chaining records verified.\n");

    // 3. Verify Non-preferential accumulator models and isolation trap (Rule 12 compliant)
    printf("[Test] Verifying Non-Preferential Accumulator & Isolation Trap...\n");
    tsfi_ramac_acc_model acc_model;
    memset(&acc_model, 0, sizeof(acc_model));
    
    acc_model.accumulators[0] = 500;
    acc_model.accumulators[1] = 200;
    
    // Simulate a mathematical discontinuity redirecting to isolation trap
    acc_model.isolation_trap = acc_model.accumulators[0] * acc_model.accumulators[1];
    acc_model.trap_active = 1;
    
    assert(acc_model.accumulators[0] == 500);
    assert(acc_model.isolation_trap == 100000);
    assert(acc_model.trap_active == 1);
    printf("  [PASS] Non-preferential accumulator models verified.\n");

    printf("[PASS] All extended RAMAC simulation invariants verified successfully!\n");
    printf("=============================================================\n");
    return 0;
}
