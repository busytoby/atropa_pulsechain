#ifndef TSFI_MEMA_H
#define TSFI_MEMA_H

#include <stdint.h>

typedef struct {
    // Electrical parameters
    float trace_distance_um; // Distance between aggressor and victim trace (microns)
    float coupling_length_mm; // Parallel coupling length (mm)
    float propagation_delay_ns; // One-way propagation delay of the trace (ns)
    float noise_margin_v;     // Maximum allowable crosstalk noise before logic failure (V)

    // Thermal parameters
    float temp_ic[2];         // Temperature of the two IC dies on MEMA substrate (C)
    float temp_substrate;     // Substrate temperature (C)
    float temp_ambient;       // Ambient temperature (C)
    float power_ic[2];        // Power dissipation of the two IC dies (W)
    float thermal_res_jc;     // Junction-to-substrate thermal resistance (C/W)
    float thermal_res_sa;     // Substrate-to-ambient thermal resistance (C/W)
    float heat_capacity_ic;   // Heat capacity of the IC dies (J/C)
    float heat_capacity_sub;  // Heat capacity of the substrate (J/C)

    // Simulation variables
    float t;                  // Time accumulator (seconds)
    float last_v_agg;         // Aggressor voltage at last step (V)
    float v_agg_history[100]; // Ring buffer of past aggressor voltages for NEXT calculation
    int history_index;        // Ring buffer current index
} TsfiMemaModule;

// Initializes a MEMA packaging module
void tsfi_mema_init(TsfiMemaModule *module);

// Simulates one step of the MEMA module.
// v_agg: Current voltage on the aggressor trace (V)
// dt_ns: Simulation step duration (nanoseconds)
// v_victim_noise: Output pointer to receive the induced crosstalk noise voltage (V) on the victim trace
// Returns 1 if any logic glitch or thermal overload occurred, 0 otherwise
int tsfi_mema_step(TsfiMemaModule *module, float v_agg, float dt_ns, float *v_victim_noise);

#endif /* TSFI_MEMA_H */
