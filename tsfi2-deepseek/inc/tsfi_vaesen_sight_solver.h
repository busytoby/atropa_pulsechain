#ifndef TSFI_VAESEN_SIGHT_SOLVER_H
#define TSFI_VAESEN_SIGHT_SOLVER_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    int sight_active_flag;
    uint32_t trauma_origin_code;
    uint32_t creature_invisibility_level;
    uint32_t microtone_bin_k;
    int64_t accumulator_energy;
    uint32_t cics_writer_id;
    int simulate_fault;
    int k_param;
} TsfiVaesenSightProofParams;

typedef enum {
    TSFI_VAESEN_SIGHT_QUALIFIED = 0,
    TSFI_VAESEN_SIGHT_INVALID_K = 1,
    TSFI_VAESEN_SIGHT_INVALID_FLAG = 2,
    TSFI_VAESEN_SIGHT_INVALID_ORIGIN = 3,
    TSFI_VAESEN_SIGHT_INVALID_INVISIBILITY = 4,
    TSFI_VAESEN_SIGHT_INVALID_MICROTONE = 5,
    TSFI_VAESEN_SIGHT_NEGATIVE_ACCUMULATOR = 6,
    TSFI_VAESEN_SIGHT_UNVERIFIED_CITIZEN = 7,
    TSFI_VAESEN_SIGHT_PERCEPTION_DIVERG = 8,
    TSFI_VAESEN_SIGHT_SHADOW_MUTATION = 9
} TsfiVaesenSightRuling;

TsfiVaesenSightRuling tsfi_verify_vaesen_sight_perception(const TsfiVaesenSightProofParams *params);

#endif // TSFI_VAESEN_SIGHT_SOLVER_H
