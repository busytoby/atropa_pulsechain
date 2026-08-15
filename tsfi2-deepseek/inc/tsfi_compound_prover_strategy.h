#ifndef TSFI_COMPOUND_PROVER_STRATEGY_H
#define TSFI_COMPOUND_PROVER_STRATEGY_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define MAX_COMPOUND_PROVERS 8
#define MAX_COMPOUND_STRATEGIES 8
#define MAX_COMPOUND_PASSES 16

typedef enum {
    PROVER_TIER_SYNTAX_DELIMITERS = 0,
    PROVER_TIER_MOTZKIN_RECURRENCE = 1,
    PROVER_TIER_BAYESIAN_SPEC = 2,
    PROVER_TIER_TANG_LUT_CENTROID = 3,
    PROVER_TIER_HTTP_STREAM_INTEGRITY = 4,
    PROVER_TIER_WEB_SECURITY_CORS = 5,
    PROVER_TIER_ADVANCED_H2_CANVAS = 6
} TsfiProverTier;

// Compound Evaluation Results
typedef struct {
    uint32_t active_tiers_mask;
    uint32_t passed_tiers_mask;
    uint32_t defect_mask;          // Composite bits on R15
    uint32_t passes_executed;
    uint32_t transforms_applied;
    char executed_strategies[MAX_COMPOUND_STRATEGIES][64];
    int executed_strategies_count;
    bool is_compound_proven;
    uint64_t cumulative_merkle_root;
} TsfiCompoundState;

// Initialize Compound Engine
void tsfi_compound_engine_init(TsfiCompoundState *state);

// Multi-Prover Compound Evaluation on Output Stream
uint32_t tsfi_compound_evaluate_provers(
    const char *output_stream,
    int paren_depth,
    int brace_depth,
    int delim_count,
    int motzkin_ruling,
    int spec_weight,
    int snr_db,
    int http_status,
    int cors_match,
    int csp_violations,
    int h2_stream_id,
    TsfiCompoundState *state
);

// Multi-Strategy Compound Transform Pipeline
bool tsfi_compound_apply_strategies(
    TsfiCompoundState *state,
    char *output_stream,
    size_t max_len
);

// End-to-End Multi-Prover / Multi-Strategy Synthesis Loop
bool tsfi_compound_synthesize(
    TsfiCompoundState *state,
    const char *immutable_spec,
    char *final_output_stream,
    size_t max_out_len
);

#endif // TSFI_COMPOUND_PROVER_STRATEGY_H
