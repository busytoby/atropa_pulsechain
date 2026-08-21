#ifndef TSFI_AURORA_GLAMOUR_SOLVER_H
#define TSFI_AURORA_GLAMOUR_SOLVER_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint32_t kp_index;
    uint32_t auroral_intensity_level;
    uint32_t active_glamour_opacity;
    int64_t accumulator_energy;
    uint32_t cics_writer_id;
    int simulate_fault;
    int k_param;
} TsfiAuroraProofParams;

typedef enum {
    TSFI_AURORA_QUALIFIED = 0,
    TSFI_AURORA_INVALID_K = 1,
    TSFI_AURORA_INVALID_KP = 2,
    TSFI_AURORA_INVALID_INTENSITY = 3,
    TSFI_AURORA_INVALID_OPACITY = 4,
    TSFI_AURORA_NEGATIVE_ACCUMULATOR = 5,
    TSFI_AURORA_UNVERIFIED_CITIZEN = 6,
    TSFI_AURORA_DISSIPATION_DIVERG = 7,
    TSFI_AURORA_SHADOW_MUTATION = 8
} TsfiAuroraRuling;

TsfiAuroraRuling tsfi_verify_aurora_glamour_dissipation(const TsfiAuroraProofParams *params);

#endif // TSFI_AURORA_GLAMOUR_SOLVER_H
