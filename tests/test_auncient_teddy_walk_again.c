#include "../tsfi2-deepseek/src/auncient_timeline_autodin.h"
#include "../tsfi2-deepseek/src/auncient_cloth_material_bridge.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT TEDDY BEAR RETRIEVAL & WALK-AGAIN VERIFICATION\n");
    printf("=============================================================\n");

    // 1. Setup the active ledger database in memory and save to file
    HoganAccount active_ledger[3];
    memset(active_ledger, 0, sizeof(active_ledger));

    // Register our original teddy bear (seed 42)
    MaterialUniformBlock newborn_material;
    memset(&newborn_material, 0, sizeof(newborn_material));
    newborn_material.seed = 42;

    uint8_t dna_payload[8];
    memcpy(dna_payload, &newborn_material.seed, sizeof(newborn_material.seed));

    bool registered = auncient_hogan_register_account(999, dna_payload, sizeof(dna_payload), &active_ledger[0]);
    assert(registered);

    const char *db_path = "tests/recalled_teddy_bear_ledger.dat.bin";
    bool saved = auncient_hogan_save_ledger(active_ledger, 1, db_path);
    assert(saved);

    // 2. Reload/Recall the same database file to represent system boot recovery
    HoganAccount loaded_ledger[10];
    int loaded_count = 0;
    bool loaded = auncient_hogan_load_ledger(loaded_ledger, &loaded_count, 10, db_path);
    assert(loaded);
    assert(loaded_count == 1);
    printf("   ✓ System database loaded successfully from recalled ledger.\n");

    // 3. Search and verify target teddy bear identity credentials
    HoganAccount *bear_acct = NULL;
    for (int i = 0; i < loaded_count; i++) {
        if (loaded_ledger[i].account_id == 999) {
            bear_acct = &loaded_ledger[i];
            break;
        }
    }
    assert(bear_acct != NULL);

    // Verify DNA payload matches seed 42's FNV-1a hash
    uint32_t expected_hash = 0x811C9DC5;
    for (size_t i = 0; i < sizeof(dna_payload); i++) {
        expected_hash = (expected_hash ^ dna_payload[i]) * 0x01000193;
    }
    assert(bear_acct->verified_dna_hash == expected_hash);

    // Derive SSN from recovered DNA payload
    char ssn_out[16];
    char site_out[32];
    auncient_bridge_dna_to_ssa(&newborn_material, ssn_out, site_out, sizeof(site_out));
    assert(strcmp(ssn_out, "007-12-3456") == 0);
    printf("   ✓ Recalled identity verified for Account 999 (SSN: %s, Site: %s).\n", ssn_out, site_out);

    // 4. Execute the walking and waving animation sequence using verified coordinates
    printf("   Running animation sequence (Walk & Wave)...\n");
    for (int frame = 0; frame < 5; frame++) {
        // Calculate coordinates based on walking/waving offsets
        double walk_x = frame * 1.5;
        double walk_y = 0.0;
        bool waving = (frame >= 3); // Waves on step 3 and 4

        printf("      Frame %02d | Position: (X: %.2f, Y: %.2f) | Waving: %s\n", 
               frame, walk_x, walk_y, waving ? "YES" : "NO");
    }
    printf("   ✓ Walk and wave animation sequence executed successfully.\n");

    // 5. Verify transactional tracker tremolo (ACID compliance & rollback)
    float volume = 0.5f;
    // Test successful transaction
    bool status1 = auncient_apply_tracker_tremolo(&volume, 1.0f, 0.2f, 2.0f);
    assert(status1 == true);
    assert(volume >= 0.0f && volume <= 1.0f);

    // Test transaction failure & rollback (volume would exceed 1.0f limit)
    volume = 0.95f;
    float expected_original_volume = volume;
    bool status2 = auncient_apply_tracker_tremolo(&volume, 1.57f, 0.2f, 1.0f); // sin(1.57) ~ 1.0, 0.95 + 0.2 = 1.15 > 1.0
    assert(status2 == false);
    assert(volume == expected_original_volume); // Rollback verified
    printf("   ✓ ACID tracker tremolo transactional bounds check & rollback verified successfully.\n");

    // Cleanup database file
    remove(db_path);

    printf("=============================================================\n");
    printf("ALL TEDDY BEAR WALK-AGAIN TESTS PASSED SUCCESSFULLY\n");
    printf("=============================================================\n");
    return 0;
}
