#define _POSIX_C_SOURCE 200809L
#include "tsfi_vaesen_sight_solver.h"

#define MOTZKIN_PRIME_SYS 953467954114363ULL

TsfiVaesenSightRuling tsfi_verify_vaesen_sight_perception(const TsfiVaesenSightProofParams *params) {
    if (!params) return TSFI_VAESEN_SIGHT_INVALID_K;

    if (params->k_param != 3) {
        return TSFI_VAESEN_SIGHT_INVALID_K;
    }

    if (params->sight_active_flag < 0 || params->sight_active_flag > 1) {
        return TSFI_VAESEN_SIGHT_INVALID_FLAG;
    }

    if (params->sight_active_flag == 1) {
        if (params->trauma_origin_code < 1 || params->trauma_origin_code > 5) {
            return TSFI_VAESEN_SIGHT_INVALID_ORIGIN;
        }
    }

    if (params->creature_invisibility_level > 10) {
        return TSFI_VAESEN_SIGHT_INVALID_INVISIBILITY;
    }

    if (params->microtone_bin_k < 1 || params->microtone_bin_k > 22) {
        return TSFI_VAESEN_SIGHT_INVALID_MICROTONE;
    }

    if (params->accumulator_energy < 0 || (uint64_t)params->accumulator_energy > MOTZKIN_PRIME_SYS) {
        return TSFI_VAESEN_SIGHT_NEGATIVE_ACCUMULATOR;
    }

    if (params->cics_writer_id != 1000000) {
        return TSFI_VAESEN_SIGHT_UNVERIFIED_CITIZEN;
    }

    uint32_t current_perception = params->creature_invisibility_level * 100;
    for (uint32_t step = 1; step <= 16; step++) {
        current_perception = (current_perception * 875) / 1000;
    }

    if (current_perception > (params->creature_invisibility_level * 100)) {
        return TSFI_VAESEN_SIGHT_PERCEPTION_DIVERG;
    }

    if (params->simulate_fault != 0) {
        uint64_t shadow_sight_baseline = 0;
        uint64_t committed_output = shadow_sight_baseline;
        if (committed_output != 0) {
            return TSFI_VAESEN_SIGHT_SHADOW_MUTATION;
        }
    }

    return TSFI_VAESEN_SIGHT_QUALIFIED;
}
