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

// Evaluator matching motzkin_natural_stream_filtration.algol61
static int verify_motzkin_natural_step(
    uint64_t x_prev,
    uint64_t x_curr,
    uint64_t x_next,
    uint32_t k_exp
) {
    if (x_prev >= MOTZKIN_PRIME || x_curr >= MOTZKIN_PRIME || x_next >= MOTZKIN_PRIME) {
        return 2; // MODULAR_FIELD_COLLAPSE
    }

    double two_to_k = (double)(1ULL << k_exp);
    double alpha = log(2.0) / log(2.71828182845904523536);
    double beta = 1.0 - (1.0 / two_to_k);

    double pred_double = alpha * (double)x_curr + beta * (double)x_prev;
    uint64_t pred = (uint64_t)fmod(pred_double, (double)MOTZKIN_PRIME);

    if (pred == x_next) {
        return 0; // NATURAL_MOTZKIN_STREAM
    } else {
        return 1; // UNNATURAL_ORDER_DEVIATION
    }
}

int main(void) {
    printf("====================================================================\n");
    printf("TEST SUITE: NATURAL VS UNNATURAL MOTZKIN RECURRENCE FILTRATION\n");
    printf("====================================================================\n\n");

    uint32_t k = 8;
    uint64_t x_minus1 = 0ULL;
    uint64_t x0 = 1199659537ULL;

    double two_to_k = (double)(1ULL << k);
    double alpha = log(2.0) / log(2.71828182845904523536);
    double beta = 1.0 - (1.0 / two_to_k);

    // Compute legitimate x1 and x2
    uint64_t x1 = (uint64_t)fmod(alpha * (double)x0 + beta * (double)x_minus1, (double)MOTZKIN_PRIME);
    uint64_t x2 = (uint64_t)fmod(alpha * (double)x1 + beta * (double)x0, (double)MOTZKIN_PRIME);

    // -------------------------------------------------------------------------
    // TEST 1: Legitimate In-Order Triad (x_minus1, x0, x1) -> NATURAL (0)
    // -------------------------------------------------------------------------
    printf("1. Testing Authentic In-Order Natural Sequence Triad (x_-1, x0, x1)...\n");
    printf("   x_-1: %lu | x0: %lu | x1: %lu\n", (unsigned long)x_minus1, (unsigned long)x0, (unsigned long)x1);
    int ruling_1 = verify_motzkin_natural_step(x_minus1, x0, x1, k);
    assert(ruling_1 == 0);
    printf("   ✓ Result: NATURAL_MOTZKIN_STREAM (ruling = %d)\n", ruling_1);

    // -------------------------------------------------------------------------
    // TEST 2: Second Authentic In-Order Natural Triad (x0, x1, x2) -> NATURAL (0)
    // -------------------------------------------------------------------------
    printf("\n2. Testing Second In-Order Natural Sequence Triad (x0, x1, x2)...\n");
    printf("   x0: %lu | x1: %lu | x2: %lu\n", (unsigned long)x0, (unsigned long)x1, (unsigned long)x2);
    int ruling_2 = verify_motzkin_natural_step(x0, x1, x2, k);
    assert(ruling_2 == 0);
    printf("   ✓ Result: NATURAL_MOTZKIN_STREAM (ruling = %d)\n", ruling_2);

    // -------------------------------------------------------------------------
    // TEST 3: Out-of-Order / Unnatural Transposition (x1, x0, x2) -> UNNATURAL (1)
    // -------------------------------------------------------------------------
    printf("\n3. Testing Out-of-Order Swapped Triad (x1, x0, x2)...\n");
    int ruling_unnatural_order = verify_motzkin_natural_step(x1, x0, x2, k);
    assert(ruling_unnatural_order == 1);
    printf("   ✓ Result: UNNATURAL_ORDER_DEVIATION (ruling = %d)\n", ruling_unnatural_order);

    // -------------------------------------------------------------------------
    // TEST 4: Corrupted Value / Synthetic Injection -> UNNATURAL (1)
    // -------------------------------------------------------------------------
    printf("\n4. Testing Value Tampering / Synthetic Injection (x0, x1, 999999)...\n");
    int ruling_corrupt = verify_motzkin_natural_step(x0, x1, 999999ULL, k);
    assert(ruling_corrupt == 1);
    printf("   ✓ Result: UNNATURAL_ORDER_DEVIATION (ruling = %d)\n", ruling_corrupt);

    // -------------------------------------------------------------------------
    // TEST 5: Modular Field Overflow Guard -> MODULAR_FIELD_COLLAPSE (2)
    // -------------------------------------------------------------------------
    printf("\n5. Testing Motzkin Field Overflow Violation (x_next >= MotzkinPrime)...\n");
    int ruling_overflow = verify_motzkin_natural_step(x0, x1, MOTZKIN_PRIME + 10ULL, k);
    assert(ruling_overflow == 2);
    printf("   ✓ Result: MODULAR_FIELD_COLLAPSE (ruling = %d)\n", ruling_overflow);

    // -------------------------------------------------------------------------
    // TEST 6: Chancery Docket Settlement & Merkle Proof
    // -------------------------------------------------------------------------
    printf("\n6. Filing Natural vs Unnatural Stream Assertions onto Chancery Docket:\n");
    ChanceryDocketState docket;
    tsfi_chancery_docket_init(&docket);

    uint32_t doc_natural = tsfi_chancery_docket_file(
        &docket,
        "Motzkin Recurrence Natural Stream Triad (x_n-1, x_n, x_n+1) Verified",
        "solidity/dysnomia/domain/std/motzkin_natural_stream_filtration.algol61",
        2026
    );
    assert(doc_natural == 7000);
    assert(tsfi_chancery_docket_resolve_zmm_r15(&docket, doc_natural, ruling_1, DOCKET_RULING_AUTHENTIC_STREAM));

    uint32_t doc_unnatural = tsfi_chancery_docket_file(
        &docket,
        "Motzkin Out-of-Order / Corrupted Stream Triad Blocked",
        "solidity/dysnomia/domain/std/motzkin_natural_stream_filtration.algol61",
        2026
    );
    assert(doc_unnatural == 7001);
    assert(tsfi_chancery_docket_resolve_zmm_r15(&docket, doc_unnatural, ruling_unnatural_order, DOCKET_RULING_UNAUTHORIZED_BLOCK));

    char audit_report[2048];
    uint64_t docket_proof = tsfi_chancery_docket_audit(&docket, audit_report, sizeof(audit_report));
    assert(docket_proof != 0);

    printf("\n%s\n", audit_report);
    printf("====================================================================\n");
    printf("NATURAL VS UNNATURAL MOTZKIN RECURRENCE TESTS PASSED & SEALED\n");
    printf("====================================================================\n");

    return 0;
}
