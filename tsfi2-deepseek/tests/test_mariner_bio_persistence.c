#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include "../inc/tsfi_mariner_biological_receptor.h"
#include "../inc/tsfi_chancery_docket.h"

int main(void) {
    printf("====================================================================\n");
    printf("MARINER PHASE 2: BIOLOGICAL STATE .DAT.BIN PERSISTENCE & ACID TEST\n");
    printf("====================================================================\n\n");

    // 1. Ingest and Transduce Sequence State
    MarinerBiologicalState original_state;
    tsfi_mariner_bio_init(&original_state);

    const char *seq = "MSKGEELFTGVVPILVELDGDVNGHKFSVSGEGEGDATYGKLTLKFICTTGKLPVPWPTLVTTFSYGVQC";
    assert(tsfi_mariner_bio_ingest_sequence(&original_state, 8005, seq));
    assert(tsfi_mariner_bio_transduce_wave512_simd(&original_state, 8));

    uint64_t original_proof = 0;
    assert(tsfi_mariner_bio_verify_binding(&original_state, &original_proof));

    printf("1. Original Biological State Prepared:\n");
    printf("   Sequence ID:      %u\n", original_state.sequence_id);
    printf("   Node Count:       %u\n", original_state.node_count);
    printf("   Binding Proof:    0x%016lx\n", (unsigned long)original_proof);

    // 2. Enforce Rule 13: Reject Invalid File Extensions (e.g. .json)
    printf("\n2. Enforcing Rule 13 (.dat.bin Only Formatting Guard)...\n");
    assert(tsfi_mariner_bio_save_dat_bin(&original_state, "test_bio.json") == false);
    assert(tsfi_mariner_bio_save_dat_bin(&original_state, "test_bio.txt") == false);
    printf("   ✓ Non-.dat.bin extensions successfully rejected.\n");

    // 3. Save to Pure Binary .dat.bin Format
    const char *dat_bin_file = "mariner_bio_state_test.dat.bin";
    assert(tsfi_mariner_bio_save_dat_bin(&original_state, dat_bin_file) == true);
    printf("   ✓ Biological State Saved to: %s\n", dat_bin_file);

    // 4. Restore into Fresh State & Verify 100% Bitwise Identity
    printf("\n3. Restoring Biological State from .dat.bin Slice...\n");
    MarinerBiologicalState restored_state;
    tsfi_mariner_bio_init(&restored_state);

    assert(tsfi_mariner_bio_load_dat_bin(&restored_state, dat_bin_file) == true);

    uint64_t restored_proof = 0;
    assert(tsfi_mariner_bio_verify_binding(&restored_state, &restored_proof));

    assert(restored_state.sequence_id == original_state.sequence_id);
    assert(restored_state.sequence_length == original_state.sequence_length);
    assert(restored_state.fnv1a_dna_hash == original_state.fnv1a_dna_hash);
    assert(restored_state.node_count == original_state.node_count);
    assert(restored_proof == original_proof);

    printf("   ✓ Bitwise Integrity Confirmed across all %u nodes.\n", restored_state.node_count);
    printf("   ✓ Restored Binding Proof: 0x%016lx (Exact Match)\n", (unsigned long)restored_proof);

    // Clean up test file
    remove(dat_bin_file);

    // 5. File and Settle on Chancery Docket
    printf("\n4. Filing Resolution on Chancery Docket:\n");
    ChanceryDocketState docket;
    tsfi_chancery_docket_init(&docket);

    uint32_t doc_7009 = tsfi_chancery_docket_file(
        &docket,
        "Mariner Phase 2 Biological Receptor .dat.bin Storage & Conservation Proof",
        "solidity/dysnomia/domain/std/mariner_biological_receptor_transduction.algol61",
        2026
    );
    assert(doc_7009 == 7000);

    bool resolved = tsfi_chancery_docket_resolve_zmm_r15(&docket, doc_7009, 0, DOCKET_RULING_AUTHENTIC_STREAM);
    assert(resolved);

    char audit_report[2048];
    uint64_t proof = tsfi_chancery_docket_audit(&docket, audit_report, sizeof(audit_report));
    assert(proof != 0);

    printf("\n%s\n", audit_report);
    printf("====================================================================\n");
    printf("MARINER PHASE 2 .DAT.BIN PERSISTENCE TEST PASSED & SEALED ON RECORD\n");
    printf("====================================================================\n");

    return 0;
}
