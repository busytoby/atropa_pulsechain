#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <math.h>
#include "../inc/tsfi_chancery_docket.h"

#define MOTZKIN_PRIME 953467954114363ULL

int main(void) {
    printf("=== TESTING NON-PREFERENTIAL 3-TERM RECURRENCE HANDSHAKE (k=8) ===\n\n");

    int k = 8;
    double two_to_k = (double)(1ULL << k);
    double alpha = log(2.0) / log(M_E);
    double beta = 1.0 - (1.0 / two_to_k);

    uint64_t x_prev = 0ULL;
    uint64_t x_curr = 1199659537ULL; // x_0

    printf("1. Verifying 50-Step Sequence Computation over MotzkinPrime:\n");
    int modulo_wrap_count = 0;

    for (int n = 0; n < 49; n++) {
        double next_val = alpha * (double)x_curr + beta * (double)x_prev;
        if (next_val >= (double)MOTZKIN_PRIME) {
            modulo_wrap_count++;
        }
        uint64_t x_next = (uint64_t)fmod(next_val, (double)MOTZKIN_PRIME);

        x_prev = x_curr;
        x_curr = x_next;
    }

    printf("   Total Transitions Executed: 49 (Steps n=0..49)\n");
    printf("   Modulo Wraps Observed:      %d (Expected >= 2)\n", modulo_wrap_count);
    assert(modulo_wrap_count >= 2);
    assert(x_curr == 258583333750286ULL);
    printf("   Step 49 Value Verified:     %llu\n", (unsigned long long)x_curr);


    // 2. Docket Filing & Resolution on Chancery Docket
    printf("\n2. Filing and Resolving Handshake Assertion on Chancery Docket:\n");
    ChanceryDocketState docket;
    tsfi_chancery_docket_init(&docket);

    uint32_t doc_7004 = tsfi_chancery_docket_file(
        &docket,
        "Non-Preferential 3-Term Recurrence Handshake (k=8, 50 Steps)",
        "solidity/dysnomia/domain/std/motzkin_3term_recurrence_handshake.algol61",
        2026
    );
    assert(doc_7004 == 7000);

    bool resolved = tsfi_chancery_docket_resolve_zmm_r15(&docket, doc_7004, 0, DOCKET_RULING_AUTHENTIC_STREAM);
    assert(resolved);

    char audit_report[2048];
    uint64_t proof = tsfi_chancery_docket_audit(&docket, audit_report, sizeof(audit_report));
    assert(proof != 0);

    printf("\n%s\n", audit_report);
    printf("=== DOCKET #7004 RESOLVED & VERIFIED ON RECORD ===\n");

    return 0;
}
