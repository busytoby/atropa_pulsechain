#define _POSIX_C_SOURCE 200809L
#include "tsfi_aurora_glamour_solver.h"

#define MOTZKIN_PRIME_SYS 953467954114363ULL

TsfiAuroraRuling tsfi_verify_aurora_glamour_dissipation(const TsfiAuroraProofParams *params) {
    if (!params) return TSFI_AURORA_INVALID_K;

    if (params->k_param != 3) {
        return TSFI_AURORA_INVALID_K;
    }

    if (params->kp_index > 9) {
        return TSFI_AURORA_INVALID_KP;
    }

    if (params->auroral_intensity_level > 10) {
        return TSFI_AURORA_INVALID_INTENSITY;
    }

    if (params->active_glamour_opacity > 100) {
        return TSFI_AURORA_INVALID_OPACITY;
    }

    if (params->accumulator_energy < 0 || (uint64_t)params->accumulator_energy > MOTZKIN_PRIME_SYS) {
        return TSFI_AURORA_NEGATIVE_ACCUMULATOR;
    }

    if (params->cics_writer_id != 1000000) {
        return TSFI_AURORA_UNVERIFIED_CITIZEN;
    }

    if (params->kp_index >= 5) {
        uint32_t current_glamour = params->active_glamour_opacity;
        for (uint32_t step = 1; step <= 16; step++) {
            current_glamour = (current_glamour * 875) / 1000;
        }

        if (current_glamour > params->active_glamour_opacity) {
            return TSFI_AURORA_DISSIPATION_DIVERG;
        }
    }

    if (params->simulate_fault != 0) {
        uint64_t shadow_aurora_baseline = 0;
        uint64_t committed_output = shadow_aurora_baseline;
        if (committed_output != 0) {
            return TSFI_AURORA_SHADOW_MUTATION;
        }
    }

    return TSFI_AURORA_QUALIFIED;
}
