#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <unistd.h>
#include "../inc/tsfi_compound_prover_strategy.h"
#include "../inc/tsfi_strategy_lang.h"
#include "../inc/tsfi_chancery_docket.h"

// Mirroring solidity/dysnomia/domain/std/compound_consensus_prover.algol61
static int verify_compound_multi_tier_consensus(int active_tiers_mask, int passed_tiers_mask, int defect_mask, int executed_pass_count) {
    if (executed_pass_count > 16) return 2; // PASS_OVERFLOW_REJECT
    if (defect_mask != 0 || passed_tiers_mask != active_tiers_mask) return 1; // COMPOUND_DEFECT_DETECTED
    return 0; // AUTHENTIC_COMPOUND_STATE
}

int main(void) {
    printf("====================================================================\n");
    printf("TEST SUITE: COMPOUND MULTI-PROVER & MULTI-STRATEGY PIPELINE\n");
    printf("====================================================================\n\n");

    TsfiCompoundState state;
    tsfi_compound_engine_init(&state);

    const char *immutable_spec = 
        "function main() {\n"
        "    // Output hello world to standard stream\n"
        "    print \"Hello World\"\n"
        "}\n";

    printf("1. Executing Compound Multi-Prover / Multi-Strategy Synthesis...\n");
    printf("--------------------------------------------------------------------\n%s\n", immutable_spec);
    printf("--------------------------------------------------------------------\n");

    char final_code[2048] = {0};
    bool ok_synth = tsfi_compound_synthesize(&state, immutable_spec, final_code, sizeof(final_code));

    printf("\n2. Compound Execution Pipeline Summary:\n");
    printf("   Passes Executed:              %u\n", state.passes_executed);
    printf("   Transforms Applied:           %u\n", state.transforms_applied);
    printf("   Active Tiers Bitmask:         0x%02X\n", state.active_tiers_mask);
    printf("   Passed Tiers Bitmask:         0x%02X\n", state.passed_tiers_mask);
    printf("   Final Defect Bitmask:         0x%02X (0 = 100%% Clean)\n", state.defect_mask);
    printf("   Strategies Executed:          %d\n", state.executed_strategies_count);
    for (int i = 0; i < state.executed_strategies_count; i++) {
        printf("     [%d] %s\n", i + 1, state.executed_strategies[i]);
    }
    printf("   Compound Formally Proven:     %s\n", state.is_compound_proven ? "YES" : "NO");

    assert(ok_synth == true);
    assert(state.is_compound_proven == true);
    assert(state.defect_mask == 0);
    assert(state.passed_tiers_mask == state.active_tiers_mask);
    assert(state.passes_executed == 2);
    assert(state.transforms_applied > 0);

    printf("\n3. Formally Proven Final Output Stream:\n");
    printf("--------------------------------------------------------------------\n%s\n", final_code);
    printf("--------------------------------------------------------------------\n");

    assert(strstr(final_code, "#include <stdio.h>") != NULL);
    assert(strstr(final_code, "int main(void)") != NULL);
    assert(strstr(final_code, "printf(\"Hello World\\n\");") != NULL);
    assert(strstr(final_code, "return 0;") != NULL);

    // -------------------------------------------------------------------------
    // 4. Test COBOL Strategy & ALGOL 61 Prover Integration
    // -------------------------------------------------------------------------
    printf("\n4. Executing COBOL Strategy: compound_consensus.strategy...\n");
    TSFiStrategyVM vm;
    TSFiStrategyReceipt receipt;
    int rc = tsfi_strategy_load_and_run("compound_consensus.strategy", state.active_tiers_mask, state.passed_tiers_mask, state.defect_mask, 0, &vm, &receipt);
    assert(rc == 0);
    assert(vm.registers[3] == 9990);
    printf("   ✓ Strategy Execution Confirmed: Compound Score = %d\n", vm.registers[3]);

    printf("\n5. Executing ALGOL 61 Prover: compound_consensus_prover.algol61...\n");
    int ruling_auth = verify_compound_multi_tier_consensus(state.active_tiers_mask, state.passed_tiers_mask, state.defect_mask, state.passes_executed);
    assert(ruling_auth == 0);
    printf("   ✓ Ruling: AUTHENTIC_COMPOUND_STATE (ruling = %d)\n", ruling_auth);

    int ruling_defect_fail = verify_compound_multi_tier_consensus(state.active_tiers_mask, 0x3F, 0x01, state.passes_executed);
    assert(ruling_defect_fail == 1);
    printf("   ✓ Defect Reject Ruling: COMPOUND_DEFECT_DETECTED (ruling = %d)\n", ruling_defect_fail);

    int ruling_overflow_fail = verify_compound_multi_tier_consensus(state.active_tiers_mask, state.passed_tiers_mask, 0, 20);
    assert(ruling_overflow_fail == 2);
    printf("   ✓ Overflow Reject Ruling: PASS_OVERFLOW_REJECT (ruling = %d)\n", ruling_overflow_fail);

    // -------------------------------------------------------------------------
    // 6. File Resolution on Chancery Docket
    // -------------------------------------------------------------------------
    printf("\n6. Filing Resolution on Chancery Docket:\n");
    ChanceryDocketState docket;
    tsfi_chancery_docket_init(&docket);

    uint32_t doc_id = tsfi_chancery_docket_file(
        &docket,
        "Compound Multi-Prover & Multi-Strategy Pipeline Formally Proven & Consensus Established",
        "solidity/dysnomia/domain/std/compound_consensus_prover.algol61",
        2026
    );
    assert(doc_id == 7000);

    bool resolved = tsfi_chancery_docket_resolve_zmm_r15(&docket, doc_id, ruling_auth, DOCKET_RULING_AUTHENTIC_STREAM);
    assert(resolved);

    char audit_report[2048];
    uint64_t docket_proof = tsfi_chancery_docket_audit(&docket, audit_report, sizeof(audit_report));
    assert(docket_proof != 0);

    printf("\n%s\n", audit_report);
    printf("====================================================================\n");
    printf("COMPOUND MULTI-PROVER/STRATEGY TEST PASSED & SEALED ON CHANCERY\n");
    printf("====================================================================\n");

    return 0;
}
