#ifndef TSFI_PROVER_STRATEGY_PIVOT_H
#define TSFI_PROVER_STRATEGY_PIVOT_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define MAX_PIVOT_PASSES 8

// Feedback State Tracking Multi-Pass Output Stream Transformation
typedef struct {
    uint32_t current_pass;
    uint32_t max_passes;
    int last_prover_ruling;        // Output code from ALGOL 61 prover (R15)
    char triggered_strategy[64];   // COBOL strategy executed on the output stream
    uint32_t stream_transform_count;
    bool is_formally_proven;
    uint64_t session_merkle_root;
} ProverStrategyPivotState;

// Initialize Multi-Pass Output-Stream Prover-Strategy Pivot Engine
void tsfi_pivot_engine_init(ProverStrategyPivotState *state, uint32_t max_passes);

// Refines the output stream iteratively using ALGOL 61 Provers and COBOL Strategy Transforms (Input Spec Remains Immutable)
bool tsfi_pivot_engine_synthesize_code(
    ProverStrategyPivotState *state,
    const char *immutable_pseudocode_spec,
    const char *model_asset_path,
    char *final_code_out,
    size_t max_out_len
);

#endif // TSFI_PROVER_STRATEGY_PIVOT_H
