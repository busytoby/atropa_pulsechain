#ifndef TSFI_LC_TANK_H
#define TSFI_LC_TANK_H

#include <stddef.h>

// 3D Cartesian coordinate Yee grid sizes (X x Y x Z)
#define GRID_X 16
#define GRID_Y 16
#define GRID_Z 32

// 3D Cartesian FDTD Maxwell Electromagnetic Solver Model
typedef struct {
    // Electric field tensors in 3D
    double Ex[GRID_X][GRID_Y][GRID_Z];
    double Ey[GRID_X][GRID_Y][GRID_Z];
    double Ez[GRID_X][GRID_Y][GRID_Z];

    // Magnetic field tensors in 3D
    double Hx[GRID_X][GRID_Y][GRID_Z];
    double Hy[GRID_X][GRID_Y][GRID_Z];
    double Hz[GRID_X][GRID_Y][GRID_Z];

    // Spatial Material properties in 3D
    double sigma[GRID_X][GRID_Y][GRID_Z]; // Electrical Conductivity (S/m)
    double eps[GRID_X][GRID_Y][GRID_Z];   // Dielectric Permittivity (F/m)

    double dx;                  // Grid step size along X (meters)
    double dy;                  // Grid step size along Y (meters)
    double dz;                  // Grid step size along Z (meters)

    double C_tune;              // Attached lumped tuning capacitor (F)
    double v_tuning_node;       // Voltage state of lumped capacitor loop
    double i_tuning_loop;       // Current state of lumped capacitor loop
} TsfiDistributedCoil;

// Initialize the 3D Maxwell EM model setting copper conductor boundaries and lossy forms
void tsfi_distributed_coil_init(
    TsfiDistributedCoil *coil,
    double diameter,
    double height,
    double turns,
    double wire_gauge_mm,
    double C_tune
);

// Process a block of samples through the 3D FDTD solver updating field tensors
void tsfi_distributed_coil_process(
    TsfiDistributedCoil *coil,
    const float *input_rf,
    float *output_grid,
    size_t count,
    double sample_rate,
    double current_freq_hz
);

#endif // TSFI_LC_TANK_H