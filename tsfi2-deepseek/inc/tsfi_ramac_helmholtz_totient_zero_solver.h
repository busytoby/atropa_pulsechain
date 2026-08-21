#ifndef TSFI_RAMAC_HELMHOLTZ_TOTIENT_ZERO_SOLVER_H
#define TSFI_RAMAC_HELMHOLTZ_TOTIENT_ZERO_SOLVER_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    int base_allocated_flag;
    int64_t totient_state_val;
    uint32_t radical_power_m;
    int64_t accumulator_energy;
    int64_t poynting_boundary_flux;
    uint32_t cics_writer_id;
    int simulate_fault;
    int k_param;
} TsfiRamacHelmholtzProofParams;

typedef enum {
    TSFI_RAMAC_HELM_QUALIFIED = 0,
    TSFI_RAMAC_HELM_INVALID_K = 1,
    TSFI_RAMAC_HELM_BASE_ALLOCATED = 2,
    TSFI_RAMAC_HELM_NON_ZERO_TOTIENT = 3,
    TSFI_RAMAC_HELM_INVALID_POWER = 4,
    TSFI_RAMAC_HELM_ACCUMULATOR_VIOLATION = 5,
    TSFI_RAMAC_HELM_POYNTING_VIOLATION = 6,
    TSFI_RAMAC_HELM_UNVERIFIED_CITIZEN = 7,
    TSFI_RAMAC_HELM_RADICAL_FAIL = 8,
    TSFI_RAMAC_HELM_SHADOW_MUTATION = 9
} TsfiRamacHelmholtzRuling;

TsfiRamacHelmholtzRuling tsfi_verify_ramac_helmholtz_totient_zero(const TsfiRamacHelmholtzProofParams *params);

#endif // TSFI_RAMAC_HELMHOLTZ_TOTIENT_ZERO_SOLVER_H
