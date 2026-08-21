#ifndef TSFI_RAMAC_ZMM_RAILS_SOLVER_H
#define TSFI_RAMAC_ZMM_RAILS_SOLVER_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    int zmm_rail_active_flag;
    uint32_t sector_index;
    uint32_t cylinder_index;
    int64_t totient_state_val;
    uint32_t radical_power_m;
    int64_t accumulator_energy;
    uint32_t cics_writer_id;
    int simulate_fault;
    int k_param;
} TsfiRamacZmmProofParams;

typedef enum {
    TSFI_RAMAC_ZMM_QUALIFIED = 0,
    TSFI_RAMAC_ZMM_INVALID_K = 1,
    TSFI_RAMAC_ZMM_INVALID_RAIL_FLAG = 2,
    TSFI_RAMAC_ZMM_INVALID_SECTOR = 3,
    TSFI_RAMAC_ZMM_INVALID_CYLINDER = 4,
    TSFI_RAMAC_ZMM_NON_ZERO_TOTIENT = 5,
    TSFI_RAMAC_ZMM_INVALID_POWER = 6,
    TSFI_RAMAC_ZMM_ACCUMULATOR_VIOLATION = 7,
    TSFI_RAMAC_ZMM_UNVERIFIED_CITIZEN = 8,
    TSFI_RAMAC_ZMM_LANE_FAIL = 9,
    TSFI_RAMAC_ZMM_SHADOW_MUTATION = 10
} TsfiRamacZmmRuling;

TsfiRamacZmmRuling tsfi_verify_ramac_zmm_rails_quiescence(const TsfiRamacZmmProofParams *params);

#endif // TSFI_RAMAC_ZMM_RAILS_SOLVER_H
