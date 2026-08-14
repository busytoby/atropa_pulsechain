#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include "tsfi_code_review.h"

int main(void) {
    printf("================ TEST MIKU WATANABE ET AL. (2024) CODE REVIEW ================\n");

    const char *sample_code = "void test_func() { int x = 10; }";
    tsfi_code_review_state_t review;
    bool ok_audit = tsfi_code_review_audit_stub(sample_code, &review);
    assert(ok_audit);
    assert(review.verdict == REVIEW_VERDICT_ACCEPT);
    assert(review.acceptance_score > 0.70f);
    printf("  [PASS] Clean C code stub audited: Verdict = 0x%08X (ACCEPT), Acceptance = %.4f.\n", review.verdict, review.acceptance_score);

    const char *bad_code = "void test_func() { int x = 10;"; // Unbalanced bracket
    bool ok_bad = tsfi_code_review_audit_stub(bad_code, &review);
    assert(ok_bad);
    assert(review.verdict == REVIEW_VERDICT_REJECT_BUG_FOUND);
    printf("  [PASS] Unbalanced C code stub rejected (BUG_FOUND): Verdict = 0x%08X.\n", review.verdict);

    bool is_outsourcing = false;
    bool ok_intent = tsfi_code_review_classify_intent("function test_wmq_eval()", &is_outsourcing);
    assert(ok_intent);
    assert(is_outsourcing);
    printf("  [PASS] Developer Prompt Intent classified (Outsourcing = TRUE).\n");

    char mutable_buf[128] = "void test_func() { int x = 10;";
    bool ok_rewrite = tsfi_code_review_rewrite_correction(mutable_buf, sizeof(mutable_buf), &review);
    assert(ok_rewrite);
    assert(strstr(mutable_buf, "}") != NULL);
    printf("  [PASS] Auto-Correction Code Rewriter verified.\n");

    printf("==============================================================================\n");
    return 0;
}
