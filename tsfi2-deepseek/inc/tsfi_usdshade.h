#ifndef TSFI_USDSHADE_H
#define TSFI_USDSHADE_H

#include <stdint.h>
#include <stdbool.h>

// Simulated FET grid size for soft-body Verlet solver discharge calculations
#define TSFI_FET_GRID_SIZE 4

typedef struct {
    float position[3];
    float prev_position[3];
    float charge_level; // Modulates albedo/roughness
    float mass;
} TSFiFetDischargeNode;

typedef struct {
    char material_id[64];
    float albedo[3];
    float roughness;
    float metallic;
    
    // Integrated cloth-simulating FET discharge grid
    TSFiFetDischargeNode nodes[TSFI_FET_GRID_SIZE][TSFI_FET_GRID_SIZE];
} TSFiUsdShade;

// Initialize the UsdShade Material Engine context
void tsfi_usdshade_init(TSFiUsdShade *shade, const char *material_id);

// Performs a single step of the Verlet soft-body physics discharge cycle simulation
void tsfi_usdshade_step_discharge(TSFiUsdShade *shade, float delta_time);

// Binds virtual register values directly to shader parameters (dynamic shader interop)
void tsfi_usdshade_bind_registers(TSFiUsdShade *shade, float base_charge, float decay_rate);

#endif // TSFI_USDSHADE_H
