#include "../src/auncient_federal_worker_sim.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT FEDERAL WORKER PHASE SIMULATION TEST SUITE\n");
    printf("=============================================================\n");

    // 1. Initialize candidate with a raw Fourier peak value (the initial imposition)
    FederalWorkerSim sim;
    auncient_worker_sim_init(&sim, 42);
    assert(sim.phase == PHASE_UNAUTHORIZED_IMPOSITION);
    assert(sim.fourier_peak_val == 42);
    printf("   ✓ Candidate starts as unauthorized Fourier imposition.\n");

    // 2. Transition to Phase 1: Audit
    bool audit_ok = auncient_worker_sim_audit(&sim);
    assert(audit_ok);
    assert(sim.phase == PHASE_AUDITED);
    printf("   ✓ Phase 1 (Audit) completed successfully.\n");

    // 3. Transition to Phase 2: Confirmation (SSN check)
    bool confirm_ok = auncient_worker_sim_confirm(&sim);
    assert(confirm_ok);
    assert(sim.phase == PHASE_CONFIRMED);
    assert(strcmp(sim.ssn, "007-12-3456") == 0); // Seed 42 matches area 7 -> Maine
    assert(strcmp(sim.site, "Maine") == 0);
    printf("   ✓ Phase 2 (Confirmation) verified (SSN: %s, Site: %s).\n", sim.ssn, sim.site);

    // 4. Attempt selection with insufficient PKI keys (expected failure)
    uint32_t invalid_keys[3] = { 1, 2, 3 };
    bool bad_select = auncient_worker_sim_select(&sim, 999, invalid_keys, 3);
    assert(!bad_select);
    assert(sim.phase == PHASE_CONFIRMED);
    printf("   ✓ Selection block on insufficient PKI quorum verified.\n");

    // 5. Complete selection successfully with a valid PKI quorum (4 keys)
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
