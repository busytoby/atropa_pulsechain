#ifndef TSFI_MANDERFIELD_ENGINE_H
#define TSFI_MANDERFIELD_ENGINE_H

#include <stdint.h>
#include <stddef.h>

typedef struct {
    float reynolds_num;                // Particle Reynolds Number (Re_p)
    float prandtl_num;                 // Prandtl Number (Pr)
    float schmidt_num;                 // Schmidt Number (Sc)
    float heat_transfer_hp;            // Heat Transfer Coefficient (h_p)
    float mass_transfer_kg;            // Mass Transfer Coefficient (k_g)
    float fluidized_temp_kelvin;       // Fluidized Bed Temperature (K)
} tsfi_manderfield_fluidized_physics_t;

typedef struct {
    uint32_t engine_id;
    tsfi_manderfield_fluidized_physics_t physics;
    uint32_t smalgol62_statements_executed;
    uint32_t evm_gas_units;             // 280 Gas / Auncient Ether Units
    double fet_power_watts;             // 0.0109 W under 78.2% Power Cut
    char tape_dat_bin[128];             // Single-Header Array (.DAT.BIN - Rule 13)
} tsfi_manderfield_engine_t;

/* Initialize E. L. Manderfield Engine Suite */
int tsfi_manderfield_engine_init(
    uint32_t engine_id,
    tsfi_manderfield_engine_t *engine
);

/* Execute SMALGOL-62 Standard Statement (Manderfield 1962 ACM Specification) */
int tsfi_manderfield_smalgol62_exec(
    tsfi_manderfield_engine_t *engine,
    const char *statement_text
);

/* Compute Kettenring-Manderfield-Smith Fluidized System Heat & Mass Transfer Physics */
int tsfi_manderfield_fluidized_calculate(
    tsfi_manderfield_engine_t *engine,
    float gas_velocity,
    float particle_diameter
);

#endif // TSFI_MANDERFIELD_ENGINE_H
