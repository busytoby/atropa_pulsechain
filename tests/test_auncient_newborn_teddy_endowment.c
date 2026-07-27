#include "../src/auncient_timeline_autodin.h"
#include "../src/auncient_cloth_material_bridge.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

// Mock implementation of ssa site resolver if not linking the full IMF module
// Or we can link tsfi_cade_imf_nato.c
extern int tsfi_mf_ssa_resolve_issuance_site(const char *ssn, char *site_name_out, int max_len);

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT NEWBORN TEDDY BEAR REGISTRATION & ENDOWMENT TEST\n");
    printf("=============================================================\n");

    // 1. Setup newborn teddy bear material block with a specific seed
    MaterialUniformBlock newborn_material;
    memset(&newborn_material, 0, sizeof(newborn_material));
    newborn_material.seed = 42; // Seed will dictate the area number (42 % 9) + 1 = 7

    // 2. Perform SSA registration using the material to cloth bridge
    char ssn_out[16];
    char site_out[32];
    memset(ssn_out, 0, sizeof(ssn_out));
    memset(site_out, 0, sizeof(site_out));

    auncient_bridge_dna_to_ssa(&newborn_material, ssn_out, site_out, sizeof(site_out));

    printf("   SSN Generated: %s\n", ssn_out);
    printf("   SSA Site Resolved: %s\n", site_out);

    // Assert correct SSN and resolved site based on seed 42 (area 7 -> Maine)
    assert(strcmp(ssn_out, "007-12-3456") == 0);
    assert(strcmp(site_out, "Maine") == 0);
    printf("   ✓ deterministic Social Security number mapping verified.\n");
    printf("   ✓ SSA regional issuance site resolution verified.\n");

    // 3. Perform Hogan Bank Account Endowment registration
    // We treat newborn_material seed/properties as the .dna footprint
    uint8_t dna_payload[8];
    memcpy(dna_payload, &newborn_material.seed, sizeof(newborn_material.seed));

    HoganAccount newborn_account;
    memset(&newborn_account, 0, sizeof(newborn_account));

    bool registered = auncient_hogan_register_account(999, dna_payload, sizeof(dna_payload), &newborn_account);
    assert(registered);
    assert(newborn_account.account_id == 999);
    assert(newborn_account.is_active == true);
    assert(newborn_account.clearance_level == 1);
    
    // Verify default endowment is exactly 1,000,000 Saat
    assert(newborn_account.balance_saat == 1000000);
    printf("   ✓ Newborn teddy bear Hogan account registered successfully.\n");
    printf("   ✓ Initial account endowment of 1,000,000 Saat verified.\n");

    // 4. Test transactional qualification in Saat currency
    // Perform deposit of 500,000 Saat
    bool dep_ok = auncient_hogan_deposit(&newborn_account, 500000);
    assert(dep_ok);
    assert(newborn_account.balance_saat == 1500000);
    printf("   ✓ Saat deposit transaction verified.\n");

    // Perform withdrawal of 200,000 Saat
    bool withdraw_ok = auncient_hogan_withdraw(&newborn_account, 200000);
    assert(withdraw_ok);
    assert(newborn_account.balance_saat == 1300000);
    printf("   ✓ Saat withdrawal transaction verified.\n");

    // Attempt high-value transfer (clearance check)
    HoganAccount merchant_account;
    memset(&merchant_account, 0, sizeof(merchant_account));
    merchant_account.account_id = 888;
    merchant_account.is_active = true;
    merchant_account.balance_saat = 0;

    // A transfer of 150,000 Saat (> 100,000 threshold) should fail at clearance level 1
    bool high_val_tx_fail = auncient_hogan_transfer(&newborn_account, &merchant_account, 150000);
    assert(!high_val_tx_fail);
    assert(newborn_account.balance_saat == 1300000);
    assert(merchant_account.balance_saat == 0);
    printf("   ✓ High-value transfer clearance block verified.\n");

    // Escalate clearance and complete transfer
    newborn_account.clearance_level = 2;
    bool high_val_tx_pass = auncient_hogan_transfer(&newborn_account, &merchant_account, 150000);
    assert(high_val_tx_pass);
    assert(newborn_account.balance_saat == 1150000);
    assert(merchant_account.balance_saat == 150000);
    printf("   ✓ High-value transfer clearance approval verified.\n");

        // 5. Test Pixar USD-based Teddy Bear registration and qualification
    printf("   Testing Pixar USD-based Teddy Bear registration...\n");
    TSFiUsdShade usd_bear;
    tsfi_usdshade_init(&usd_bear, "/auncient/material/special_gold_plush");
    
    char usd_ssn[16];
    char usd_site[32];
    memset(usd_ssn, 0, sizeof(usd_ssn));
    memset(usd_site, 0, sizeof(usd_site));
    
    auncient_bridge_usd_to_ssa(&usd_bear, usd_ssn, usd_site, sizeof(usd_site));
    printf("   USD Bear SSN: %s, Site: %s\n", usd_ssn, usd_site);
    
    // Assert generated values
    assert(strlen(usd_ssn) == 11);
    assert(strlen(usd_site) > 0);
    
    // Register USD bear to Hogan
    HoganAccount usd_account;
    memset(&usd_account, 0, sizeof(usd_account));
    bool usd_registered = auncient_hogan_register_account(777, (const uint8_t *)usd_bear.material_id, strlen(usd_bear.material_id), &usd_account);
    assert(usd_registered);
    assert(usd_account.account_id == 777);
    assert(usd_account.balance_saat == 1000000);
    printf("   ✓ Pixar USD Teddy Bear registered to Hogan Bank with 1M Saat endowment.\n");

    // 6. Test singular entity registration (TeddyBearSkelCharacter)
    printf("   Testing singular entity registration...\n");
    char entity_ssn[16];
    char entity_site[32];
    memset(entity_ssn, 0, sizeof(entity_ssn));
    memset(entity_site, 0, sizeof(entity_site));
    
    auncient_bridge_entity_to_ssa("TeddyBearSkelCharacter", entity_ssn, entity_site, sizeof(entity_site));
    printf("   Singular Teddy Bear Entity SSN: %s, Site: %s\n", entity_ssn, entity_site);
    
    assert(strlen(entity_ssn) == 11);
    assert(strlen(entity_site) > 0);
    
    HoganAccount entity_account;
    memset(&entity_account, 0, sizeof(entity_account));
    bool entity_registered = auncient_hogan_register_account(555, (const uint8_t *)"TeddyBearSkelCharacter", strlen("TeddyBearSkelCharacter"), &entity_account);
    assert(entity_registered);
    assert(entity_account.account_id == 555);
    assert(entity_account.balance_saat == 1000000);
    printf("   ✓ Singular Teddy Bear Entity registered to Hogan Bank with 1M Saat endowment.\n");

    // 7. Test AUTODIN Startup verification of participants
    printf("   Testing AUTODIN system startup verification...\n");
    HoganAccount active_ledger[3] = { newborn_account, usd_account, entity_account };
    bool start_ok = auncient_autodin_verify_system_start(active_ledger, 3);
    assert(start_ok);
    printf("   ✓ AUTODIN system startup verification successful.\n");

    // 8. Test Active .dat.bin Ledger Persistence (Rule 13 compliance)
    printf("   Testing active ledger file persistence (.dat.bin)...\n");
    const char *db_path = "tests/test_hogan_ledger.dat.bin";
    bool saved = auncient_hogan_save_ledger(active_ledger, 3, db_path);
    assert(saved);

    HoganAccount loaded_ledger[3];
    int loaded_count = 0;
    bool loaded = auncient_hogan_load_ledger(loaded_ledger, &loaded_count, 3, db_path);
    assert(loaded);
    assert(loaded_count == 3);
    assert(loaded_ledger[0].account_id == 999);
    assert(loaded_ledger[1].account_id == 777);
    assert(loaded_ledger[2].account_id == 555);
    
    // Verify file format checks (must reject non-.dat.bin files)
    bool bad_save = auncient_hogan_save_ledger(active_ledger, 3, "tests/test_hogan_ledger.txt");
    assert(!bad_save);
    bool bad_load = auncient_hogan_load_ledger(loaded_ledger, &loaded_count, 3, "tests/test_hogan_ledger.json");
    assert(!bad_load);

    remove(db_path);
    printf("   ✓ Active database persistence and file extension guard verified.\n");

    printf("=============================================================\n");
    printf("ALL NEWBORN TEDDY BEAR ENDOWMENT TESTS PASSED SUCCESSFULLY\n");
    printf("=============================================================\n");
    return 0;
}
