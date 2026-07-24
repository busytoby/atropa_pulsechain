#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

#define LANES 4
#define RECONCILED_FILE "reconciled_ledger_v2.dat.bin"
#define REQUIRED_QUORUM_MASK 0x07

typedef struct {
    int64_t reserve_limit;
    uint32_t required_validator_mask;
    uint32_t baseline_color;
} sgpr_bank_t;

typedef struct {
    uint32_t account_id[LANES];
    int64_t balance[LANES];
    uint32_t collected_validator_mask[LANES];
    uint32_t status[LANES]; // Output status color
} vgpr_bank_t;

typedef struct {
    uint32_t account_id;
    int64_t balance;
    uint32_t status;
} ledger_record_t;

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT HOGAN OVERNIGHT RECONCILIATION LEVEL 2 SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    // 1. Setup SGPR
    sgpr_bank_t sgprs = {
        .reserve_limit = 1000,
        .required_validator_mask = REQUIRED_QUORUM_MASK,
        .baseline_color = 0x000000
    };

    // 2. Setup VGPR
    vgpr_bank_t vgprs = {
        .account_id = { 101, 102, 103, 104 },
        .balance = { 5000, 800, 3000, 4000 },
        .collected_validator_mask = {
            0x07, // Lane 0: Pass (meets balance and signatures)
            0x07, // Lane 1: Fail (violates reserve limit)
            0x03, // Lane 2: Fail (lacks signatures 0x03 != 0x07)
            0x07  // Lane 3: Pass (meets balance and signatures)
        },
        .status = { 0 }
    };

    printf("[TEST] Executing overnight reconciliation with signature quorums...\n");
    fflush(stdout);

    // Reconcile and assign status
    for (int lane = 0; lane < LANES; lane++) {
        bool reserve_ok = (vgprs.balance[lane] >= sgprs.reserve_limit);
        bool quorum_ok = ((vgprs.collected_validator_mask[lane] & sgprs.required_validator_mask) == sgprs.required_validator_mask);

        if (reserve_ok && quorum_ok) {
            vgprs.status[lane] = 0xFFFF; // CYAN (PASS)
            printf("   [RECONCILED] Account %d balance %ld reconciled with quorum signatures.\n", 
                   vgprs.account_id[lane], vgprs.balance[lane]);
        } else {
            vgprs.status[lane] = 0x00FF; // RED (FAIL)
            printf("   [VIOLATION] Account %d failed validation (Reserve: %s, Quorum: %s).\n", 
                   vgprs.account_id[lane], reserve_ok ? "OK" : "FAIL", quorum_ok ? "OK" : "FAIL");
        }
    }
    fflush(stdout);

    // 3. Serialize to .dat.bin ledger file
    printf("[TEST] Serializing records to %s...\n", RECONCILED_FILE);
    fflush(stdout);

    FILE *file = fopen(RECONCILED_FILE, "wb");
    assert(file != NULL);

    for (int lane = 0; lane < LANES; lane++) {
        ledger_record_t record = {
            .account_id = vgprs.account_id[lane],
            .balance = vgprs.balance[lane],
            .status = vgprs.status[lane]
        };
        size_t written = fwrite(&record, sizeof(ledger_record_t), 1, file);
        assert(written == 1);
    }
    fclose(file);

    // 4. Verify serialized data
    printf("[TEST] Verifying written database records...\n");
    fflush(stdout);

    file = fopen(RECONCILED_FILE, "rb");
    assert(file != NULL);

    for (int lane = 0; lane < LANES; lane++) {
        ledger_record_t record;
        size_t read_bytes = fread(&record, sizeof(ledger_record_t), 1, file);
        assert(read_bytes == 1);

        assert(record.account_id == vgprs.account_id[lane]);
        assert(record.balance == vgprs.balance[lane]);
        assert(record.status == vgprs.status[lane]);
    }
    fclose(file);
    printf("   ✓ Overnight reconciliation database serialization validated successfully.\n");
    fflush(stdout);

    // Clean up temporary file
    remove(RECONCILED_FILE);

    printf("=============================================================\n");
    printf("HOGAN OVERNIGHT RECONCILIATION LEVEL 2 COMPLETE\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
