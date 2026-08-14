#ifndef TSFI_CHAMBERLAND_DUALITY_H
#define TSFI_CHAMBERLAND_DUALITY_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// Selection Pattern Identifiers for Chamberland Dual Tomography
#define CHAMBERLAND_PATTERN_HYPERTOMOGRAPHY 0x48595052  // "HYPR" Epibar 2-3 Tree Linear Functional Domain
#define CHAMBERLAND_PATTERN_HYPOTOMOGRAPHY  0x4859504F  // "HYPO" Hypobar Red-Black Tree Classification Domain
#define CHAMBERLAND_PATTERN_BIJECTION_DUAL  0x4455414C  // "DUAL" Bijective Equilibrium State

// Epibar & Hypobar Additive Accumulator State Struct
typedef struct {
    float epibar_accumulator;     // \bar{\epsilon}: Epibar 2-3 Tree Linear Functional Domain Accumulator
    float hypobar_accumulator;    // \underline{\epsilon}: Hypobar Red-Black Tree Classification Domain Accumulator
    float dual_eigenvalue;        // Chamberland \lambda_{dual} eigenvalue metric
    uint32_t selection_pattern;   // Active selection pattern (2-3 Tree Functional vs Red-Black Classification)
    uint64_t transaction_count;   // Total dual tomographic state updates
} tsfi_chamberland_accumulator_t;

// Initializes Epibar / Hypobar Additive Accumulator State
tsfi_chamberland_accumulator_t* tsfi_chamberland_accumulator_create(void);

// Updates Epibar (\bar{\epsilon}) and Hypobar (\underline{\epsilon}) accumulators non-preferentially
bool tsfi_chamberland_accumulator_update(
    tsfi_chamberland_accumulator_t *acc,
    const float *x,
    int dim,
    float epibar_delta,
    float hypobar_delta
);

// Evaluates Chamberland Duality Bijection: Hypertomography \iff Hypotomography
bool tsfi_chamberland_eval_duality_bijection(
    const tsfi_chamberland_accumulator_t *acc,
    float *duality_metric_out,
    uint32_t *pattern_out
);

// Evaluates Epibar (\bar{\epsilon}) 2-3 Tree Linear Functional Domain metric
float tsfi_chamberland_eval_epibar_23tree_functional(const float *x, int dim);

// Evaluates Hypobar (\underline{\epsilon}) Red-Black Tree Classification Domain metric
float tsfi_chamberland_eval_hypobar_rb_classification(const float *cand_logits, int count, float top_score);

// Destroys Epibar / Hypobar Additive Accumulator State
void tsfi_chamberland_accumulator_destroy(tsfi_chamberland_accumulator_t *acc);

#endif // TSFI_CHAMBERLAND_DUALITY_H
