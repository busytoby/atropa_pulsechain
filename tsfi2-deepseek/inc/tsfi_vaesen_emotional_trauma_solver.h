#ifndef TSFI_VAESEN_EMOTIONAL_TRAUMA_SOLVER_H
#define TSFI_VAESEN_EMOTIONAL_TRAUMA_SOLVER_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint32_t initial_mental_conditions_count;
    uint32_t fear_shock_intensity;
    uint32_t active_trauma_severity;
    int64_t accumulator_energy;
    uint32_t cics_writer_id;
    int simulate_fault;
    int k_param;
} TsfiVaesenProofParams;

typedef enum {
    TSFI_VAESEN_QUALIFIED = 0,
    TSFI_VAESEN_INVALID_K = 1,
    TSFI_VAESEN_INVALID_CONDITIONS = 2,
    TSFI_VAESEN_INVALID_SHOCK = 3,
    TSFI_VAESEN_INVALID_TRAUMA = 4,
    TSFI_VAESEN_NEGATIVE_ACCUMULATOR = 5,
    TSFI_VAESEN_UNVERIFIED_CITIZEN = 6,
    TSFI_VAESEN_DISSIPATION_DIVERG = 7,
    TSFI_VAESEN_SHADOW_MUTATION = 8
} TsfiVaesenRuling;

TsfiVaesenRuling tsfi_verify_vaesen_emotional_trauma_dissipation(const TsfiVaesenProofParams *params);

#endif // TSFI_VAESEN_EMOTIONAL_TRAUMA_SOLVER_H
