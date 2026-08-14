#ifndef TSFI_GEOMETRIC_ALGEBRA_H
#define TSFI_GEOMETRIC_ALGEBRA_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// 8D Clifford Geometric Algebra Cl(3,0,0) Multivector Struct (Wang et al., 2023)
typedef struct {
    float scalar;         // Grade 0: a
    float e1, e2, e3;     // Grade 1: Vector v_1 e_1 + v_2 e_2 + v_3 e_3
    float e12, e23, e31;  // Grade 2: Bivector B_12 e_12 + B_23 e_23 + B_31 e_31
    float e123;           // Grade 3: Pseudoscalar I e_123
} tsfi_ga_multivector_t;

// Projects activation vector x into a Cl(3,0,0) Multivector
bool tsfi_ga_project_activation(const float *x, int dim, tsfi_ga_multivector_t *mv_out);

// Clifford Geometric Product: C = A * B
bool tsfi_ga_geometric_product(
    const tsfi_ga_multivector_t *a,
    const tsfi_ga_multivector_t *b,
    tsfi_ga_multivector_t *c_out
);

// Clifford Rotor Candidate Token Selection Score
float tsfi_ga_evaluate_token_rotor(
    const tsfi_ga_multivector_t *mv_state,
    const float *token_emb,
    int emb_dim
);

#endif // TSFI_GEOMETRIC_ALGEBRA_H
