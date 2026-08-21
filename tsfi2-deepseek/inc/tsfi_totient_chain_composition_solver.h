#ifndef TSFI_TOTIENT_CHAIN_COMPOSITION_SOLVER_H
#define TSFI_TOTIENT_CHAIN_COMPOSITION_SOLVER_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    int64_t initial_totient_state;
    uint32_t radical_depth_m;
    int64_t accumulator_energy;
    uint32_t bus_nodes_m;
    uint32_t cics_writer_id;
    int k_param;
} TsfiTotientChainInput;

typedef struct {
    bool nullstellensatz_zero_passed;
    bool closed_boundary_passed;
    bool accumulator_clamped;
    uint8_t composed_merkle_digest[32];
    double total_eval_latency_ns;
} TsfiTotientChainResult;

bool tsfi_solve_totient_nullstellensatz_chain(
    const TsfiTotientChainInput *input,
    TsfiTotientChainResult *out_result
);

#endif // TSFI_TOTIENT_CHAIN_COMPOSITION_SOLVER_H
