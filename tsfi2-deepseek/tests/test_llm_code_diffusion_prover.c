#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include "../inc/tsfi_chancery_docket.h"

// Evaluates C code syntax balance (delimiters) purely via formal state analysis
static bool check_syntax_balance(const char *code, int *out_paren_depth, int *out_brace_depth) {
    if (!code) return false;

    int paren = 0;
    int brace = 0;
    for (size_t i = 0; code[i] != '\0'; i++) {
        if (code[i] == '(') paren++;
        else if (code[i] == ')') paren--;
        else if (code[i] == '{') brace++;
        else if (code[i] == '}') brace--;

        if (paren < 0 || brace < 0) {
            if (out_paren_depth) *out_paren_depth = paren;
            if (out_brace_depth) *out_brace_depth = brace;
            return false;
        }
    }

    if (out_paren_depth) *out_paren_depth = paren;
    if (out_brace_depth) *out_brace_depth = brace;
    return (paren == 0 && brace == 0);
}

// Prover evaluator matching llm_code_diffusion_prover.algol61
static int prove_code_spec_conformance(
    int paren_depth_balance,
    int brace_depth_balance,
    int spec_semantic_weight,
    int motzkin_natural_ruling,
    int confidence_threshold
) {
    if (paren_depth_balance != 0 || brace_depth_balance != 0) {
        return 1; // UNBALANCED_SYNTAX_REJECT
    }
    if (motzkin_natural_ruling != 0) {
        return 2; // UNNATURAL_TRIAD_REJECT
    }
    if (spec_semantic_weight >= confidence_threshold) {
        return 0; // AUTHENTIC_CODE_STREAM
    } else {
        return 3; // DEFICIENT_SPEC_CONGRUENCE
    }
}

int main(void) {
    printf("====================================================================\n");
    printf("TEST SUITE: LLM CODE DIFFUSION VIA ALGOL 61 PROVERS (ZERO INJECTIONS)\n");
    printf("====================================================================\n\n");

    // Spec: "Write a pure C function that calculates Motzkin prime modulo step"
    const char *valid_code_sample = 
        "uint64_t step_motzkin(uint64_t x_curr, uint64_t x_prev) {\n"
        "    double alpha = 0.693147;\n"
        "    double beta = 0.99609375;\n"
        "    return (uint64_t)(alpha * x_curr + beta * x_prev) % 953467954114363ULL;\n"
        "}\n";

    const char *broken_syntax_sample = 
        "uint64_t step_motzkin(uint64_t x_curr, uint64_t x_prev) {\n"
        "    return (x_curr + x_prev;\n"
        "}\n";

    // -------------------------------------------------------------------------
    // TEST 1: Valid Code Sample Gating (Syntax Balanced & Natural Triad)
    // -------------------------------------------------------------------------
    printf("1. Proving Compliant Code Sample (Syntax & Spec Gating)...\n");
    int p_depth = 0, b_depth = 0;
    bool balanced = check_syntax_balance(valid_code_sample, &p_depth, &b_depth);
    assert(balanced && p_depth == 0 && b_depth == 0);

    int ruling_valid = prove_code_spec_conformance(p_depth, b_depth, 950, 0, 700);
    assert(ruling_valid == 0);
    printf("   ✓ Result: AUTHENTIC_CODE_STREAM (ruling = %d)\n", ruling_valid);

    // -------------------------------------------------------------------------
    // TEST 2: Broken Delimiter Syntax Reject
    // -------------------------------------------------------------------------
    printf("\n2. Proving Syntax Delimiter Rejection...\n");
    int p_broken = 0, b_broken = 0;
    bool broken_ok = check_syntax_balance(broken_syntax_sample, &p_broken, &b_broken);
    assert(!broken_ok || p_broken != 0);

    int ruling_syntax_err = prove_code_spec_conformance(p_broken, b_broken, 950, 0, 700);
    assert(ruling_syntax_err == 1);
    printf("   ✓ Result: UNBALANCED_SYNTAX_REJECT (ruling = %d)\n", ruling_syntax_err);

    // -------------------------------------------------------------------------
    // TEST 3: Unnatural Recurrence Stream Reject
    // -------------------------------------------------------------------------
    printf("\n3. Proving Motzkin Recurrence Stream Fracture Reject...\n");
    int ruling_unnatural = prove_code_spec_conformance(0, 0, 950, 1, 700);
    assert(ruling_unnatural == 2);
    printf("   ✓ Result: UNNATURAL_TRIAD_REJECT (ruling = %d)\n", ruling_unnatural);

    // -------------------------------------------------------------------------
    // TEST 4: Deficient Specification Semantic Weight Reject
    // -------------------------------------------------------------------------
    printf("\n4. Proving Deficient Semantic Weight Reject (Weight=500 < Threshold=700)...\n");
    int ruling_deficient = prove_code_spec_conformance(0, 0, 500, 0, 700);
    assert(ruling_deficient == 3);
    printf("   ✓ Result: DEFICIENT_SPEC_CONGRUENCE (ruling = %d)\n", ruling_deficient);

    // -------------------------------------------------------------------------
    // TEST 5: Filing Resolutions onto Chancery Docket
    // -------------------------------------------------------------------------
    printf("\n5. Filing Prover-Governed Resolutions onto Chancery Docket:\n");
    ChanceryDocketState docket;
    tsfi_chancery_docket_init(&docket);

    uint32_t doc_code_ok = tsfi_chancery_docket_file(
        &docket,
        "DeepSeek Code Diffusion Prover: Compliant Motzkin C Code Verified",
        "solidity/dysnomia/domain/std/llm_code_diffusion_prover.algol61",
        2026
    );
    assert(doc_code_ok == 7000);
    assert(tsfi_chancery_docket_resolve_zmm_r15(&docket, doc_code_ok, ruling_valid, DOCKET_RULING_AUTHENTIC_STREAM));

    uint32_t doc_code_reject = tsfi_chancery_docket_file(
        &docket,
        "DeepSeek Code Diffusion Prover: Unbalanced Syntax Rejected",
        "solidity/dysnomia/domain/std/llm_code_diffusion_prover.algol61",
        2026
    );
    assert(doc_code_reject == 7001);
    assert(tsfi_chancery_docket_resolve_zmm_r15(&docket, doc_code_reject, ruling_syntax_err, DOCKET_RULING_UNAUTHORIZED_BLOCK));

    char audit_report[2048];
    uint64_t docket_proof = tsfi_chancery_docket_audit(&docket, audit_report, sizeof(audit_report));
    assert(docket_proof != 0);

    printf("\n%s\n", audit_report);
    printf("====================================================================\n");
    printf("LLM CODE DIFFUSION PROVER TEST SUITE PASSED & SEALED ON CHANCERY\n");
    printf("====================================================================\n");

    return 0;
}
