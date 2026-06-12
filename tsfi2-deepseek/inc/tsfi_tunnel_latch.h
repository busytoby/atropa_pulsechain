#ifndef TSFI_TUNNEL_LATCH_H
#define TSFI_TUNNEL_LATCH_H

#include <stdint.h>

typedef struct {
    float i_bias;    // Bias current (mA)
    float i_peak;    // Peak current (mA)
    float i_valley;  // Valley current (mA)
    int state;       // 0 or 1 state
    float v_out;     // Output voltage (V)
} TsfiTunnelLatch;

void tsfi_tunnel_latch_init(TsfiTunnelLatch *latch);
float tsfi_tunnel_latch_step(TsfiTunnelLatch *latch, float i_trigger);

#endif /* TSFI_TUNNEL_LATCH_H */
