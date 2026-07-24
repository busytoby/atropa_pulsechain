#include "auncient_sdk.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>

#define SIMULATED_XPL_SIZE 3

// Simulated instruction format parsed from a compiled .xpl binary
typedef struct {
    uint8_t opcode;
    uint32_t value;
    bool approvals[SDK_NUM_NODES];
} xpl_instruction_t;

// -------------------------------------------------------------
// .xpl Binary Loader and Interpreter Loop
// -------------------------------------------------------------
bool execute_xpl_payload(sdk_cics_context_t *ctx, const xpl_instruction_t *instructions, int count) {
    // Acquire AUTODIN spin-lock to serialize execution of the .xpl binary
    printf("   [LOADER] Acquiring AUTODIN spin-lock for .xpl loader execution...\n");
    if (!auncient_sdk_autodin_spin_lock(ctx, 0x999, 'F')) {
        return false;
    }

    // Execute instruction stream
    for (int i = 0; i < count; i++) {
        uint32_t result = 0;
        printf("   [LOADER] Executing .xpl instruction %d: Opcode 0x%02X, Value %u\n",
               i, instructions[i].opcode, instructions[i].value);
        
        bool ok = auncient_sdk_alu_execute(ctx, instructions[i].opcode, instructions[i].value, instructions[i].approvals, &result);
        if (!ok) {
            printf("   [LOADER] Instruction execution failed. Aborting and unlocking.\n");
            auncient_sdk_autodin_spin_unlock(ctx, 0x999);
            return false;
        }
    }

    // Release lock
    auncient_sdk_autodin_spin_unlock(ctx, 0x999);
    printf("   [LOADER] Execution complete. AUTODIN spin-lock released.\n");
    return true;
}

// -------------------------------------------------------------
// Unit Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT .XPL BINARY LOADER INTEGRATION SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    sdk_coaxial_env_t env;
    bool ok = auncient_sdk_init_coaxial(&env);
    assert(ok == true);

    sdk_kermit_cache_t cache = { .cached_value = 0, .cached_ts = { 0, 0 }, .is_warm = false };
    
    // TopSecret Clearance (Level 3) CICS execution context
    sdk_cics_context_t ctx = {
        .env = &env,
        .cache = &cache,
        .quorum_type = SDK_QUORUM_MAJORITY,
        .writer_id = 42,
        .security_clearance = 3
    };

    // Simulated .xpl compiled instruction payload:
    // 1. Write ABD (Value 800) with Majority approvals
    // 2. Read Kermit cache
    // 3. Eval Ackerman consensus with Majority approvals
    xpl_instruction_t payload[SIMULATED_XPL_SIZE] = {
        { .opcode = ALU_OP_WRITE_ABD, .value = 800, .approvals = { true, true, true, false } },
        { .opcode = ALU_OP_READ_KERMIT, .value = 0, .approvals = { 0 } },
        { .opcode = ALU_OP_EVAL_ACKERMAN, .value = 0, .approvals = { true, true, true, false } }
    };

    printf("[TEST] Running .xpl payload loader simulation...\n");
    fflush(stdout);
    ok = execute_xpl_payload(&ctx, payload, SIMULATED_XPL_SIZE);
    assert(ok == true);
    printf("   ✓ .xpl instruction stream executed successfully.\n");
    fflush(stdout);

    // Verify cache warmed state after .xpl loader run
    assert(cache.is_warm == true);
    assert(cache.cached_value == 800);
    printf("   ✓ Local cache correctly warmed by .xpl ALU write instruction.\n");
    fflush(stdout);

    auncient_sdk_close_coaxial(&env);
    printf("=============================================================\n");
    printf("XPL LOADER INTEGRATION TESTS PASSED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
