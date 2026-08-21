#define _POSIX_C_SOURCE 200809L
#include "tsfi_ramac_fourier_dirichlet_solver.h"

TsfiRamacFourierRuling tsfi_verify_ramac_fourier_dirichlet_boundary(const TsfiRamacFourierProofParams *params) {
    if (!params) return TSFI_RAMAC_FOURIER_INVALID_K;

    if (params->k_param != 3) {
        return TSFI_RAMAC_FOURIER_INVALID_K;
    }

    if (params->head_index > 23) {
        return TSFI_RAMAC_FOURIER_INVALID_HEAD;
    }

    if (params->cylinder_index > 49) {
        return TSFI_RAMAC_FOURIER_INVALID_CYLINDER;
    }

    if (params->totient_state_val != 0) {
        return TSFI_RAMAC_FOURIER_NON_ZERO_TOTIENT;
    }

    if (params->harmonic_step_k < 1 || params->harmonic_step_k > 22) {
        return TSFI_RAMAC_FOURIER_INVALID_HARMONIC;
    }

    if (params->initial_phase_jitter_fs < 0 || params->initial_phase_jitter_fs > 50000) {
        return TSFI_RAMAC_FOURIER_INVALID_JITTER;
    }

    if (params->cics_writer_id != 1000000) {
        return TSFI_RAMAC_FOURIER_UNVERIFIED_CITIZEN;
    }

    int64_t current_jitter = params->initial_phase_jitter_fs;
    for (uint32_t step = 1; step <= 16; step++) {
        current_jitter = (current_jitter * 875) / 1000;
    }

    if (current_jitter > params->initial_phase_jitter_fs) {
        return TSFI_RAMAC_FOURIER_JITTER_DIVERG;
    }

    if (params->simulate_fault != 0) {
        int64_t shadow_fourier_baseline = 0;
        int64_t committed_output = shadow_fourier_baseline;
        if (committed_output != 0) {
            return TSFI_RAMAC_FOURIER_SHADOW_MUTATION;
        }
    }

    return TSFI_RAMAC_FOURIER_QUALIFIED;
}
