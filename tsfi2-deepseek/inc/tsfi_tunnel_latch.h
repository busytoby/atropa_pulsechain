#ifndef TSFI_TUNNEL_LATCH_H
#define TSFI_TUNNEL_LATCH_H

#include <stdint.h>

typedef struct {
    float i_bias;    // Bias current (mA)
    float i_peak;    // Peak current (mA)
    float i_valley;  // Valley current (mA)
    int state;       // 0 or 1 state
    float v_out;     // Output voltage (V)
    float v_phys;    // Continuous physical voltage (V)
} TsfiTunnelLatch;

void tsfi_tunnel_latch_init(TsfiTunnelLatch *latch);
float tsfi_tunnel_latch_step(TsfiTunnelLatch *latch, float i_trigger);

// --- W3UZN Tunnel Dipper RF Simulation ---
// Computes continuous current (mA) for a Germanium tunnel diode at voltage V
float tsfi_tunnel_diode_current(float v);

// Simulates a single time-step of W3UZN's Tunnel Dipper RF oscillator circuit.
// Updates capacitor voltage v_cap and inductor current i_ind.
// Returns the current output RF voltage.
float tsfi_w3uzn_dipper_step(float *v_cap, float *i_ind, float i_bias, float l, float c, float dt);

#endif /* TSFI_TUNNEL_LATCH_H */
