#ifndef TSFI_RAMAC_FOURIER_DIRICHLET_SOLVER_H
#define TSFI_RAMAC_FOURIER_DIRICHLET_SOLVER_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint32_t head_index;
    uint32_t cylinder_index;
    int64_t totient_state_val;
    uint32_t harmonic_step_k;
    int64_t initial_phase_jitter_fs;
    uint32_t cics_writer_id;
    int simulate_fault;
    int k_param;
} TsfiRamacFourierProofParams;

typedef enum {
    TSFI_RAMAC_FOURIER_QUALIFIED = 0,
    TSFI_RAMAC_FOURIER_INVALID_K = 1,
    TSFI_RAMAC_FOURIER_INVALID_HEAD = 2,
    TSFI_RAMAC_FOURIER_INVALID_CYLINDER = 3,
    TSFI_RAMAC_FOURIER_NON_ZERO_TOTIENT = 4,
    TSFI_RAMAC_FOURIER_INVALID_HARMONIC = 5,
    TSFI_RAMAC_FOURIER_INVALID_JITTER = 6,
    TSFI_RAMAC_FOURIER_UNVERIFIED_CITIZEN = 7,
    TSFI_RAMAC_FOURIER_JITTER_DIVERG = 8,
    TSFI_RAMAC_FOURIER_SHADOW_MUTATION = 9
} TsfiRamacFourierRuling;

TsfiRamacFourierRuling tsfi_verify_ramac_fourier_dirichlet_boundary(const TsfiRamacFourierProofParams *params);

#endif // TSFI_RAMAC_FOURIER_DIRICHLET_SOLVER_H
