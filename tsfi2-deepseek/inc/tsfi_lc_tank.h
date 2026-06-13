#ifndef TSFI_LC_TANK_H
#define TSFI_LC_TANK_H

#include <stddef.h>

#define COIL_SEGMENTS 16

// Distributed Parameter Transmission Line Coil Model
typedef struct {
    double L_total;             // Total nominal inductance in Henries
    double R_dc;                // DC resistance in Ohms
    double C_total_parasitic;   // Total Medhurst parasitic capacitance in Farads
    double C_tune;              // Tuning capacitor capacitance in Farads

    // Discretized spatial states (16 segments)
    double v[COIL_SEGMENTS];    // Voltage at each node along the coil axis
    double i_l[COIL_SEGMENTS];  // Current through each inductor segment

    // Matrix parameter fields
    double L_seg[COIL_SEGMENTS]; // Self-inductance of each segment
    double C_seg[COIL_SEGMENTS]; // Distributed shunt capacitance of each segment
    double R_seg[COIL_SEGMENTS]; // Resistance of each segment (updated dynamically for skin effect)

    double wire_radius;          // Radius of the copper wire in meters (for skin depth)
    double coil_diameter;        // Outer diameter of the coil in meters
    double coil_height;          // Height of the coil in meters
} TsfiDistributedCoil;

// Initialize the distributed parameter coil model using Medhurst empirical relations
void tsfi_distributed_coil_init(
    TsfiDistributedCoil *coil,
    double diameter,
    double height,
    double turns,
    double wire_gauge_mm,
    double C_tune
);

// Process a block of samples through the distributed transmission line simulation
void tsfi_distributed_coil_process(
    TsfiDistributedCoil *coil,
    const float *input_rf,
    float *output_grid,
    size_t count,
    double sample_rate,
    double current_freq_hz
);

#endif // TSFI_LC_TANK_H