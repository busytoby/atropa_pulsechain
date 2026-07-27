#include "../src/auncient_federal_worker_sim.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT FEDERAL WORKER PHASE SIMULATION TEST SUITE\n");
    printf("=============================================================\n");

    // 1. Initialize candidate with a raw Fourier peak value (the initial imposition)
    // Peak value: 42. Since 42 has 'A' in its high byte (0x0000002A -> op is 0 which is not A-Z), it is default allowed.
    FederalWorkerSim sim;
    auncient_worker_sim_init(&sim, 42);
    assert(sim.phase == PHASE_UNAUTHORIZED_IMPOSITION);
    assert(sim.fourier_peak_val == 42);
    printf("   ✓ Candidate starts as unauthorized Fourier imposition.\n");

    // 2. Try to audit directly (expected block because qualify has not run)
    printf("[TEST] Attempting audit directly prior to EDSAC qualification (expected reject)...\n");
    bool early_audit = auncient_worker_sim_audit(&sim);
    assert(!early_audit);
    assert(sim.phase == PHASE_UNAUTHORIZED_IMPOSITION);
    printf("   ✓ Direct audit block verified.\n");

    // 3. Test qualification rejection using an analyzer that blocks the peak instruction
    // Opcode of 0x58000000 is 'X' (prohibited)
    FederalWorkerSim bad_sim;
    auncient_worker_sim_init(&bad_sim, 0x58000000);
    AuncientAnalyzer prohibited_analyzer;
    auncient_analyzer_init(&prohibited_analyzer, 1 << ('X' - 'A'));

    printf("[TEST] Qualifying candidate with prohibited opcode peak (expected reject)...\n");
    bool bad_qualify = auncient_worker_sim_qualify(&bad_sim, &prohibited_analyzer);
    assert(!bad_qualify);
    assert(bad_sim.phase == PHASE_UNAUTHORIZED_IMPOSITION);
    printf("   ✓ Qualification reject verified.\n");

    // 4. Qualify valid candidate successfully
    printf("[TEST] Qualifying valid candidate (expected pass)...\n");
    bool good_qualify = auncient_worker_sim_qualify(&sim, &prohibited_analyzer);
    assert(good_qualify);
    assert(sim.phase == PHASE_QUALIFIED);
    printf("   ✓ EDSAC qualification completed successfully.\n");

    // 5. Transition to Phase 1: Audit
    printf("[TEST] Transitioning qualified candidate to Phase 1 (Audit)...\n");
    bool audit_ok = auncient_worker_sim_audit(&sim);
    assert(audit_ok);
    assert(sim.phase == PHASE_AUDITED);
    printf("   ✓ Phase 1 (Audit) completed successfully.\n");

    // 6. Transition to Phase 2: Confirmation (SSN check)
    bool confirm_ok = auncient_worker_sim_confirm(&sim);
    assert(confirm_ok);
    assert(sim.phase == PHASE_CONFIRMED);
    assert(strcmp(sim.ssn, "007-12-3456") == 0); // Seed 42 matches area 7 -> Maine
    assert(strcmp(sim.site, "Maine") == 0);
    printf("   ✓ Phase 2 (Confirmation) verified (SSN: %s, Site: %s).\n", sim.ssn, sim.site);

    // 7. Attempt selection with insufficient PKI keys (expected failure)
    uint32_t invalid_keys[3] = { 1, 2, 3 };
    bool bad_select = auncient_worker_sim_select(&sim, 999, invalid_keys, 3);
    assert(!bad_select);
    assert(sim.phase == PHASE_CONFIRMED);
    printf("   ✓ Selection block on insufficient PKI quorum verified.\n");

    // 8. Complete selection successfully with a valid PKI quorum (4 keys)
    uint32_t valid_keys[4] = { 1, 2, 3, 4 };
    bool good_select = auncient_worker_sim_select(&sim, 999, valid_keys, 4);
    assert(good_select);
    assert(sim.phase == PHASE_SELECTED_WORKER);
    assert(sim.account.account_id == 999);
    assert(sim.account.balance_saat == 1000000);
    printf("   ✓ Phase 3 (Selection) successful. Worker established with 1,000,000 Saat.\n");

    printf("=============================================================\n");
    printf("ALL FEDERAL WORKER PHASE SIMULATION TESTS PASSED SUCCESSFULLY\n");
    printf("=============================================================\n");
    return 0;
}
