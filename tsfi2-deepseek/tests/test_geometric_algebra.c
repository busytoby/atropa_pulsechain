#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include "tsfi_geometric_algebra.h"

int main(void) {
    printf("================ TEST JIAN WANG ET AL. (2023) GEOMETRIC ALGEBRA ================\n");

    float x_sample[16];
    for (int i = 0; i < 16; i++) x_sample[i] = (float)(i + 1) * 0.5f;

    tsfi_ga_multivector_t mv;
    bool ok_proj = tsfi_ga_project_activation(x_sample, 16, &mv);
    assert(ok_proj);
    assert(mv.scalar == 0.5f);
    assert(mv.e1 == 1.0f);
    printf("  [PASS] Activation Vector projected into 8D Clifford Cl(3,0,0) Multivector.\n");

    tsfi_ga_multivector_t mv_prod;
    bool ok_prod = tsfi_ga_geometric_product(&mv, &mv, &mv_prod);
    assert(ok_prod);
    printf("  [PASS] Clifford Geometric Product (AB = A . B + A ^ B) calculated (Scalar = %.4f).\n", mv_prod.scalar);

    float token_emb[3] = {0.5f, 1.2f, 0.8f};
    float rotor_score = tsfi_ga_evaluate_token_rotor(&mv, token_emb, 3);
    assert(rotor_score > 0.0f);
    printf("  [PASS] Clifford Rotor Token Candidate Score evaluated (Score = %.4f).\n", rotor_score);

    printf("=================================================================================\n");
    return 0;
}
