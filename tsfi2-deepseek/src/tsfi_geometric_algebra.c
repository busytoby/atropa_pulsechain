#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "tsfi_geometric_algebra.h"

bool tsfi_ga_project_activation(const float *x, int dim, tsfi_ga_multivector_t *mv_out) {
    if (!x || dim < 8 || !mv_out) return false;

    // Project 1D activation array into 8D Clifford Cl(3,0,0) multivector grade components
    mv_out->scalar = x[0];
    mv_out->e1     = x[1];
    mv_out->e2     = x[2];
    mv_out->e3     = x[3];
    mv_out->e12    = x[4];
    mv_out->e23    = x[5];
    mv_out->e31    = x[6];
    mv_out->e123   = x[7];

    return true;
}

bool tsfi_ga_geometric_product(
    const tsfi_ga_multivector_t *a,
    const tsfi_ga_multivector_t *b,
    tsfi_ga_multivector_t *c_out
) {
    if (!a || !b || !c_out) return false;

    // 8D Clifford Geometric Product Cl(3,0,0): AB = A . B + A ^ B
    c_out->scalar = a->scalar * b->scalar + a->e1 * b->e1 + a->e2 * b->e2 + a->e3 * b->e3
                  - a->e12 * b->e12 - a->e23 * b->e23 - a->e31 * b->e31 - a->e123 * b->e123;

    c_out->e1     = a->scalar * b->e1 + a->e1 * b->scalar - a->e2 * b->e12 + a->e3 * b->e31
                  - a->e12 * b->e2 + a->e31 * b->e3 - a->e23 * b->e123 - a->e123 * b->e23;

    c_out->e2     = a->scalar * b->e2 + a->e2 * b->scalar + a->e1 * b->e12 - a->e3 * b->e23
                  + a->e12 * b->e1 - a->e23 * b->e3 - a->e31 * b->e123 - a->e123 * b->e31;

    c_out->e3     = a->scalar * b->e3 + a->e3 * b->scalar - a->e1 * b->e31 + a->e2 * b->e23
                  - a->e23 * b->e2 + a->e31 * b->e1 - a->e12 * b->e123 - a->e123 * b->e12;

    c_out->e12    = a->scalar * b->e12 + a->e12 * b->scalar + a->e1 * b->e2 - a->e2 * b->e1
                  + a->e3 * b->e123 + a->e123 * b->e3 + a->e23 * b->e31 - a->e31 * b->e23;

    c_out->e23    = a->scalar * b->e23 + a->e23 * b->scalar + a->e2 * b->e3 - a->e3 * b->e2
                  + a->e1 * b->e123 + a->e123 * b->e1 + a->e31 * b->e12 - a->e12 * b->e31;

    c_out->e31    = a->scalar * b->e31 + a->e31 * b->scalar + a->e3 * b->e1 - a->e1 * b->e3
                  + a->e2 * b->e123 + a->e123 * b->e2 + a->e12 * b->e23 - a->e23 * b->e12;

    c_out->e123   = a->scalar * b->e123 + a->e123 * b->scalar + a->e1 * b->e23 + a->e2 * b->e31 + a->e3 * b->e12
                  + a->e12 * b->e3 + a->e23 * b->e1 + a->e31 * b->e2;

    return true;
}

float tsfi_ga_evaluate_token_rotor(
    const tsfi_ga_multivector_t *mv_state,
    const float *token_emb,
    int emb_dim
) {
    if (!mv_state || !token_emb || emb_dim < 3) return 0.0f;

    // Convert candidate token embedding slice into a Grade-1 Vector v_cand
    tsfi_ga_multivector_t v_cand;
    memset(&v_cand, 0, sizeof(tsfi_ga_multivector_t));
    v_cand.e1 = token_emb[0];
    v_cand.e2 = token_emb[1];
    v_cand.e3 = token_emb[2];

    // Compute Clifford Geometric Product: R = M * v_cand
    tsfi_ga_multivector_t r_prod;
    tsfi_ga_geometric_product(mv_state, &v_cand, &r_prod);

    // Score combines Grade-0 scalar contraction and Grade-2 bivector wedge magnitude
    float scalar_contraction = fabsf(r_prod.scalar);
    float bivector_wedge = sqrtf(r_prod.e12 * r_prod.e12 + r_prod.e23 * r_prod.e23 + r_prod.e31 * r_prod.e31);

    return scalar_contraction + bivector_wedge * 1000.0f;
}
