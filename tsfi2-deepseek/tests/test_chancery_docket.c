#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include "../inc/tsfi_chancery_docket.h"
#include "../inc/tsfi_waugh_chancery.h"

// Simulated COBOL ZMM execution kernel setting mainframe register R15
static int64_t cobol_zmm_evaluate_assertion(uint32_t http_status, uint32_t turnstile_flag) {
    // COBOL Decision Matrix:
    // EVALUATE TRUE ALSO http_status ALSO turnstile_flag
    //   WHEN ANY ALSO 403 ALSO 1 MOVE 1 TO R15
    //   WHEN ANY ALSO 200 ALSO 0 MOVE 0 TO R15
    //   WHEN OTHER               MOVE 2 TO R15
    // END-EVALUATE.
    if (http_status == 403 && turnstile_flag == 1) {
        return 1; // UNAUTHORIZED_BLOCK -> R15 = 1
    } else if (http_status == 200 && turnstile_flag == 0) {
        return 0; // AUTHENTIC_STREAM    -> R15 = 0
    } else {
        return 2; // CHANCERY_CONTINUANCE -> R15 = 2
    }
}

int main(void) {
    printf("=== TESTING CHANCERY DOCKET & ZORSE COBOL ZMM (R15) DISPATCH SYSTEM ===\n\n");

    ChanceryDocketState docket;
    tsfi_chancery_docket_init(&docket);

    printf("1. Filing Unverified Assertion onto Chancery Docket:\n");
    uint32_t doc_id = tsfi_chancery_docket_file(
        &docket,
        "HathiTrust Direct CLI Plaintext Retrieval Blocked by Turnstile",
        "solidity/dysnomia/domain/std/chancery_unauthorized_block.algol61",
        2026
    );
    assert(doc_id == 7000);
    printf("   Docket Entry Created -> [Docket #%u]: %s\n", doc_id, docket.entries[0].assertion_summary);
    printf("   Target Prover:          %s\n\n", docket.entries[0].target_prover_path);

    printf("2. Simulating COBOL ZMM Vector Evaluation (ZMM0=403, ZMM1=1)...\n");
    int64_t r15_val = cobol_zmm_evaluate_assertion(403, 1);
    printf("   Mainframe Register R15 Return Code: %ld\n", (long)r15_val);
    assert(r15_val == 1);

    printf("\n3. Resolving Docket Entry via R15 Value:\n");
    bool ok = tsfi_chancery_docket_resolve_zmm_r15(&docket, doc_id, r15_val, DOCKET_RULING_UNAUTHORIZED_BLOCK);
    assert(ok);
    printf("   Docket #%u Status: RESOLVED [UNAUTHORIZED_BLOCK]\n", doc_id);

    printf("\n4. Executing Chancery Docket Cryptographic Merkle Audit...\n");
    char audit_report[2048];
    uint64_t root = tsfi_chancery_docket_audit(&docket, audit_report, sizeof(audit_report));
    assert(root != 0);
    printf("\n%s\n", audit_report);

    printf("=== ALL CHANCERY DOCKET & ZMM R15 DISPATCH PROOFS PASSED CLEANLY ===\n");
    return 0;
}
