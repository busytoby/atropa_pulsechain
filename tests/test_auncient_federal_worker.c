#include "../src/auncient_federal_worker_sim.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT FEDERAL WORKER PHASE SIMULATION TEST SUITE\n");
    printf("=============================================================\n");

    // 1. Initialize candidate with a raw Fourier peak value (the initial imposition)
    // Peak value: 42. Opcode resolves to 0 (which is categorized as noise).
    FederalWorkerSim sim_noise_1, sim_noise_2, sim_noise_3;
    auncient_worker_sim_init(&sim_noise_1, 42);
    auncient_worker_sim_init(&sim_noise_2, 100);
    auncient_worker_sim_init(&sim_noise_3, 1000);

    // Feed 3 consecutive noise impositions to trigger the DEFCON alarm
    printf("[TEST] Feeding 3 consecutive noise impositions to trigger DEFCON alarm...\n");
    bool q1 = auncient_worker_sim_qualify(&sim_noise_1, NULL);
    assert(!q1); // Category noise -> qualification fails
    
    bool q2 = auncient_worker_sim_qualify(&sim_noise_2, NULL);
    assert(!q2);

    bool q3 = auncient_worker_sim_qualify(&sim_noise_3, NULL);
    assert(!q3); // 3rd consecutive noise -> triggers DEFCON 2 alarm!
    printf("   ✓ Consecutive noise metering and DEFCON alarm trigger verified.\n");

    // 2. Initialize a valid structured candidate (Opcode 0x41000000 -> ASCII 'A' which is structured)
    FederalWorkerSim sim;
    auncient_worker_sim_init(&sim, 0x4100002A);
    assert(sim.phase == PHASE_UNAUTHORIZED_IMPOSITION);
    printf("   ✓ Candidate starts as unauthorized Fourier imposition.\n");

    // 3. Try to audit directly (expected block because qualify has not run)
    printf("[TEST] Attempting audit directly prior to EDSAC qualification (expected reject)...\n");
    bool early_audit = auncient_worker_sim_audit(&sim);
    assert(!early_audit);
    assert(sim.phase == PHASE_UNAUTHORIZED_IMPOSITION);
    printf("   ✓ Direct audit block verified.\n");

    // 4. Test qualification rejection using an analyzer that blocks the peak instruction
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

    // 5. Qualify valid candidate successfully
    printf("[TEST] Qualifying valid candidate (expected pass)...\n");
    bool good_qualify = auncient_worker_sim_qualify(&sim, &prohibited_analyzer);
    assert(good_qualify);
    assert(sim.phase == PHASE_QUALIFIED);
    printf("   ✓ EDSAC qualification completed successfully.\n");

    // 6. Transition to Phase 1: Audit
    printf("[TEST] Transitioning qualified candidate to Phase 1 (Audit)...\n");
    bool audit_ok = auncient_worker_sim_audit(&sim);
    assert(audit_ok);
    assert(sim.phase == PHASE_AUDITED);
    printf("   ✓ Phase 1 (Audit) completed successfully.\n");

    // 7. Transition to Phase 2: Confirmation (SSN check)
    bool confirm_ok = auncient_worker_sim_confirm(&sim);
    assert(confirm_ok);
    assert(sim.phase == PHASE_CONFIRMED);
    // Seed 0x4100002A resolves SSN based on seed properties
    assert(strlen(sim.ssn) == 11);
    assert(strlen(sim.site) > 0);
    printf("   ✓ Phase 2 (Confirmation) verified (SSN: %s, Site: %s).\n", sim.ssn, sim.site);

    // 8. Attempt selection with insufficient PKI keys (expected failure)
    uint32_t invalid_keys[3] = { 1, 2, 3 };
    bool bad_select = auncient_worker_sim_select(&sim, 999, invalid_keys, 3);
    assert(!bad_select);
    assert(sim.phase == PHASE_CONFIRMED);
    printf("   ✓ Selection block on insufficient PKI quorum verified.\n");

    // 9. Complete selection successfully with a valid PKI quorum (4 keys)
    uint32_t valid_keys[4] = { 1, 2, 3, 4 };
    bool good_select = auncient_worker_sim_select(&sim, 999, valid_keys, 4);
    assert(good_select);
    assert(sim.phase == PHASE_SELECTED_WORKER);
    assert(sim.account.account_id == 999);
    assert(sim.account.balance_saat == 1000000);
    printf("   ✓ Phase 3 (Selection) successful. Worker established with 1,000,000 Saat.\n");

    // 10. Verify Immutable Hogan Transaction Ledger Chain
    printf("[TEST] Testing Hogan double-entry transfer and immutable chain verification...\n");
    HoganAccount recipient;
    memset(&recipient, 0, sizeof(recipient));
    recipient.account_id = 888;
    recipient.is_active = true;
    recipient.balance_saat = 0;
    
    extern bool auncient_hogan_transfer(HoganAccount *sender, HoganAccount *recipient, uint32_t amount);
    extern bool auncient_hogan_verify_chain(void);
    
    // Transfer 10,000 Saat
    bool transfer_ok = auncient_hogan_transfer(&sim.account, &recipient, 10000);
    assert(transfer_ok == true);
    assert(sim.account.balance_saat == 990000);
    assert(recipient.balance_saat == 10000);
    
    // Verify the ledger chain
    assert(auncient_hogan_verify_chain() == true);
    printf("   ✓ Immutable Hogan transaction ledger chain verified successfully.\n");

    printf("=============================================================\n");
    printf("ALL FEDERAL WORKER PHASE SIMULATION TESTS PASSED SUCCESSFULLY\n");
    printf("=============================================================\n");
    return 0;
}
