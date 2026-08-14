#ifndef TSFI_TELPA_PROGRAM_ANALYSIS_H
#define TSFI_TELPA_PROGRAM_ANALYSIS_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// Bin Lin et al. (TELPA) Program Analysis State Struct
typedef struct {
    float branch_predicate_distance; // d(p, x): Path condition predicate distance (0.0 = satisfied)
    float dependency_score;          // \mathcal{D}_{dep}: Inter-procedural dependency score
    float totient_qing_radius;       // r_{qing} = \phi(n): Totient Qing reference domain radius
    size_t counter_example_count;    // Number of registered counter-examples in ring buffer
    bool  hard_branch_covered;       // True if hard-to-reach branch condition is met
} tsfi_telpa_state_t;

// Evaluates TELPA Branch Predicate Distance d(p, x) relative to Totient Qing Domain r_{qing} = \phi(n)
bool tsfi_telpa_eval_totient_qing(
    const float *x,
    int dim,
    uint32_t n_totient,
    tsfi_telpa_state_t *telpa_out
);

// Evaluates TELPA Branch Predicate Distance d(p, x) over activation vector x
bool tsfi_telpa_eval_branch_distance(
    const float *x,
    int dim,
    float target_value,
    tsfi_telpa_state_t *telpa_out
);

// Registers a failed candidate token as a Counter-Example in TELPA feedback buffer
bool tsfi_telpa_register_counter_example(
    uint32_t token_id,
    float penalty_weight
);

// Evaluates candidate token bonus incorporating Bin Lin's TELPA Program Analysis
float tsfi_telpa_evaluate_candidate_bonus(
    uint32_t token_id,
    const tsfi_telpa_state_t *telpa_state
);

// AST-Aware Branch Predicate Distance for DeepSeek-Coder
bool tsfi_telpa_eval_ast_predicate_distance(
    const float *x,
    int dim,
    int ast_depth,
    tsfi_telpa_state_t *telpa_out
);

// Decays counter-example penalties over auto-regressive generation steps
bool tsfi_telpa_decay_counter_examples(float decay_factor);

// Inter-Procedural Header Token Boost for DeepSeek-Coder
float tsfi_telpa_eval_header_interop_bonus(const char *token_str);

#endif // TSFI_TELPA_PROGRAM_ANALYSIS_H
