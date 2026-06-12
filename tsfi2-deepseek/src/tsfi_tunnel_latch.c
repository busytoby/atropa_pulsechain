#include "tsfi_tunnel_latch.h"
#include <string.h>

void tsfi_tunnel_latch_init(TsfiTunnelLatch *latch) {
    memset(latch, 0, sizeof(TsfiTunnelLatch));
    latch->i_bias = 0.6f;    // 0.6 mA bias
    latch->i_peak = 1.0f;    // 1.0 mA peak current
    latch->i_valley = 0.2f;  // 0.2 mA valley current
    latch->state = 0;
    latch->v_out = 0.1f;     // 0.1V representing Logic '0'
}

float tsfi_tunnel_latch_step(TsfiTunnelLatch *latch, float i_trigger) {
    float i_total = latch->i_bias + i_trigger;

    if (i_total > latch->i_peak) {
        // Exceeds peak current: switches to high-voltage state
        latch->state = 1;
        latch->v_out = 0.5f; // 0.5V representing Logic '1'
    } else if (i_total < latch->i_valley) {
        // Drops below valley current: drops to low-voltage state
        latch->state = 0;
        latch->v_out = 0.1f;
    }
    // If valley <= i_total <= peak, the previous state is held (bistable)

    return latch->v_out;
}
