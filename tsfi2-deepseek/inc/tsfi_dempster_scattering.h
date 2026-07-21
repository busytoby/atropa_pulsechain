#ifndef TSFI_DEMPSTER_SCATTERING_H
#define TSFI_DEMPSTER_SCATTERING_H

#include <stdint.h>
#include <stddef.h>

typedef struct {
    double real;
    double imag;
} tsfi_complex_t;

typedef struct {
    uint32_t solve_id;                 // Solver ID
    tsfi_complex_t s_matrix[4][4];     // 4x4 Feynman Scattering Matrix
    double born_amplitude;             // Born Approximation T_fi
    uint32_t evm_gas_units;             // 230 Gas / Auncient Ether Units
    double fet_power_watts;             // 0.0109 W under 78.2% Power Cut
    char tape_dat_bin[128];             // Single-Header Array (.DAT.BIN - Rule 13)
} tsfi_dempster_scattering_solver_t;

/* Initialize Dempster Feynman-Born Scattering Matrix Solver */
int tsfi_dempster_scattering_init(
    uint32_t solve_id,
    tsfi_dempster_scattering_solver_t *solver
);

/* Compute Feynman-Born Scattering Matrix T_fi and S_fi for given incident energy */
int tsfi_dempster_scattering_compute(
    tsfi_dempster_scattering_solver_t *solver,
    double incident_energy_ev,
    double potential_v0
);

#endif // TSFI_DEMPSTER_SCATTERING_H
