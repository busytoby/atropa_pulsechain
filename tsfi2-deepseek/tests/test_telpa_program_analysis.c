#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include "tsfi_telpa_program_analysis.h"

int main(void) {
    printf("================ TEST BIN LIN ET AL. (TELPA) PROGRAM ANALYSIS ================\n");

    float x_sample[16];
    for (int i = 0; i < 16; i++) x_sample[i] = (float)(i + 1) * 0.10f;

    tsfi_telpa_state_t telpa;
    bool ok_eval = tsfi_telpa_eval_branch_distance(x_sample, 16, 0.50f, &telpa);
    assert(ok_eval);
    assert(telpa.branch_predicate_distance >= 0.0f);
    printf("  [PASS] Branch Predicate Distance d(p, x) evaluated (d = %.4f, dep_score = %.4f).\n", telpa.branch_predicate_distance, telpa.dependency_score);

    bool ok_reg = tsfi_telpa_register_counter_example(99, 1500.0f);
    assert(ok_reg);
    printf("  [PASS] Failed Candidate Token 99 registered as Counter-Example in TELPA buffer.\n");

    float bonus_clean = tsfi_telpa_evaluate_candidate_bonus(10, &telpa);
    float bonus_bad = tsfi_telpa_evaluate_candidate_bonus(99, &telpa);
    assert(bonus_clean > bonus_bad);
    printf("  [PASS] TELPA Candidate Bonus evaluated (Clean = %.4f, Counter-Example = %.4f).\n", bonus_clean, bonus_bad);

    bool ok_qing = tsfi_telpa_eval_totient_qing(x_sample, 16, 10, &telpa); // \phi(10) = 4 -> r_{qing} = 0.50
    assert(ok_qing);
    assert(telpa.totient_qing_radius == 0.50f);
    bool ok_ast = tsfi_telpa_eval_ast_predicate_distance(x_sample, 16, 3, &telpa);
    assert(ok_ast);
    printf("  [PASS] AST-Aware Branch Predicate Distance d_AST evaluated (d_AST = %.4f).\n", telpa.branch_predicate_distance);

    bool ok_decay = tsfi_telpa_decay_counter_examples(0.85f);
    assert(ok_decay);
    printf("  [PASS] Auto-Regressive Counter-Example Decay verified.\n");

    float interop_bonus = tsfi_telpa_eval_header_interop_bonus("tsfi_zorse_eval_gguf_pure_c");
    assert(interop_bonus == 5000.0f);
    printf("  [PASS] Inter-Procedural Header Token Boost verified (Bonus = %.4f).\n", interop_bonus);

    printf("=================================================================================\n");
    return 0;
}
