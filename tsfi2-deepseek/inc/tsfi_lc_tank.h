#ifndef TSFI_LC_TANK_H
#define TSFI_LC_TANK_H

#include <stddef.h>

// 2D Cylindrical coordinate Yee grid sizes (R x Z)
#define GRID_R 32
#define GRID_Z 64

// 2D Axisymmetric FDTD Maxwell Electromagnetic Solver Model
typedef struct {
    // Electromagnetic Field tensors in cylindrical coordinates
    double Er[GRID_R][GRID_Z];  // Radial Electric Field (V/m)
    double Ez[GRID_R][GRID_Z];  // Axial Electric Field (V/m)
    double Hphi[GRID_R][GRID_Z];// Azimuthal Magnetic Field (A/m)

    // Spatial Material properties
    double sigma[GRID_R][GRID_Z]; // Electrical Conductivity (S/m)
    double eps[GRID_R][GRID_Z];   // Dielectric Permittivity (F/m)

    double dr;                  // Radial grid step size in meters
    double dz;                  // Axial grid step size in meters

    double C_tune;              // Attached lumped tuning capacitor (F)
    double v_tuning_node;       // Voltage state of lumped capacitor loop
    double i_tuning_loop;       // Current state of lumped capacitor loop
} TsfiDistributedCoil;

// Initialize the 2D Maxwell EM model setting copper conductor boundaries and lossy forms
void tsfi_distributed_coil_init(
    TsfiDistributedCoil *coil,
    double diameter,
    double height,
    double turns,
    double wire_gauge_mm,
    double C_tune
);

// Process a block of samples through the 2D FDTD solver updating field tensors
void tsfi_distributed_coil_process(
    TsfiDistributedCoil *coil,
    const float *input_rf,
    float *output_grid,
    size_t count,
    double sample_rate,
    double current_freq_hz
);

#endif // TSFI_LC_TANK_H