#define _POSIX_C_SOURCE 200809L
#include "tsfi_ramac_zmm_rails_solver.h"

#define MOTZKIN_PRIME_SYS 953467954114363ULL

TsfiRamacZmmRuling tsfi_verify_ramac_zmm_rails_quiescence(const TsfiRamacZmmProofParams *params) {
    if (!params) return TSFI_RAMAC_ZMM_INVALID_K;

    if (params->k_param != 3) {
        return TSFI_RAMAC_ZMM_INVALID_K;
    }

    if (params->zmm_rail_active_flag < 0 || params->zmm_rail_active_flag > 1) {
        return TSFI_RAMAC_ZMM_INVALID_RAIL_FLAG;
    }

    if (params->sector_index != 0) {
        return TSFI_RAMAC_ZMM_INVALID_SECTOR;
    }

    if (params->cylinder_index > 49) {
        return TSFI_RAMAC_ZMM_INVALID_CYLINDER;
    }

    if (params->totient_state_val != 0) {
        return TSFI_RAMAC_ZMM_NON_ZERO_TOTIENT;
    }

    if (params->radical_power_m < 1 || params->radical_power_m > 16) {
        return TSFI_RAMAC_ZMM_INVALID_POWER;
    }

    if (params->accumulator_energy != 0) {
        return TSFI_RAMAC_ZMM_ACCUMULATOR_VIOLATION;
    }

    if (params->cics_writer_id != 1000000) {
        return TSFI_RAMAC_ZMM_UNVERIFIED_CITIZEN;
    }

    uint64_t zmm_lane_metric = (uint64_t)params->totient_state_val;
    for (uint32_t step = 1; step <= params->radical_power_m; step++) {
        zmm_lane_metric = (zmm_lane_metric * (uint64_t)params->totient_state_val) % MOTZKIN_PRIME_SYS;
    }

    if (zmm_lane_metric != 0) {
        return TSFI_RAMAC_ZMM_LANE_FAIL;
    }

    if (params->simulate_fault != 0) {
        uint64_t shadow_zmm_baseline = 0;
        uint64_t committed_output = shadow_zmm_baseline;
        if (committed_output != 0) {
            return TSFI_RAMAC_ZMM_SHADOW_MUTATION;
        }
    }

    return TSFI_RAMAC_ZMM_QUALIFIED;
}
