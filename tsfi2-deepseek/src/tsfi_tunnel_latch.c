#include "tsfi_tunnel_latch.h"
#include <string.h>
#include <math.h>

void tsfi_tunnel_latch_init(TsfiTunnelLatch *latch) {
    memset(latch, 0, sizeof(TsfiTunnelLatch));
    latch->i_bias = 0.6f;    // 0.6 mA bias
    latch->i_peak = 1.0f;    // 1.0 mA peak current
    latch->i_valley = 0.2f;  // 0.2 mA valley current
    latch->state = 0;
    latch->v_out = 0.1f;     // 0.1V representing Logic '0'
    latch->v_phys = 0.04f;   // Physical stable low voltage
}

float tsfi_tunnel_latch_step(TsfiTunnelLatch *latch, float i_trigger) {
    float i_total = latch->i_bias + i_trigger;

    // Physical solver: solve continuous I_d(v) = i_total using Newton-Raphson
    float v = latch->v_phys;
    
    // Relaxation state-transition: tunnel diode switches bands when peak/valley is crossed
    if (i_total > latch->i_peak) {
        if (v < 0.35f) v = 0.45f;
    } else if (i_total < latch->i_valley) {
        if (v > 0.1f) v = 0.04f;
    }

    for (int iter = 0; iter < 100; iter++) {
        float ratio = v / 0.065f;
        float exp1 = expf(1.0f - ratio);
        float exp2 = expf(10.0f * (v - 0.35f));
        float exp3 = expf(v / 0.026f);

        // I_d(v) = I_peak * (v/v_p) * e^(1-v/v_p) + I_valley * e^(10*(v-v_v)) + I_s * (e^(v/v_t) - 1)
        float id = latch->i_peak * ratio * exp1 + latch->i_valley * exp2 + 1.0e-6f * (exp3 - 1.0f);
        float did_dv = (latch->i_peak / 0.065f) * (1.0f - ratio) * exp1 + 10.0f * latch->i_valley * exp2 + (1.0e-6f / 0.026f) * exp3;

        float F = id - i_total;
        float F_prime = did_dv;

        float diff = F / F_prime;
        if (diff > 0.02f) diff = 0.02f;
        if (diff < -0.02f) diff = -0.02f;

        v -= diff;
        if (v < 0.0f) v = 0.0f;

        if (fabsf(diff) < 1.0e-6f) {
            break;
        }
    }
    latch->v_phys = v;

    // Update digital state based on physical voltage threshold (e.g. 0.2V)
    latch->state = (v > 0.2f) ? 1 : 0;
    
    // Map return value to exact discrete voltages for unit test backwards compatibility
    latch->v_out = latch->state ? 0.5f : 0.1f;
    return latch->v_out;
}

float tsfi_tunnel_diode_current(float v) {
    if (v < 0.0f) {
        // Reverse conduction is very high (tunneling)
        return v * 50.0f; 
    }
    float ratio = v / 0.065f;
    float exp1 = expf(1.0f - ratio);
    float exp2 = expf(10.0f * (v - 0.35f));
    float exp3 = expf(v / 0.026f);
    
    // Standard Germanium tunnel diode values: Ip = 1.0 mA, Iv = 0.2 mA
    return 1.0f * ratio * exp1 + 0.2f * exp2 + 1.0e-6f * (exp3 - 1.0f);
}

float tsfi_w3uzn_dipper_step(float *v_cap, float *i_ind, float v_bias, float l, float c, float dt) {
    // Series-biased Tunnel Diode RF Oscillator:
    // d(i_ind)/dt = (v_bias - v_cap) / L (in A/sec, scaled by 1e3 to mA/sec)
    // d(v_cap)/dt = (i_ind - I_d(v_cap)) / C (using A, so scaled by 1e-3)
    float id = tsfi_tunnel_diode_current(*v_cap);
    float di = (v_bias - *v_cap) / l * dt * 1e3f;
    float dv = (*i_ind - id) * 1e-3f / c * dt;
    
    // Clamp time-step change to ensure numerical stability at larger dt
    if (dv > 0.05f) dv = 0.05f;
    if (dv < -0.05f) dv = -0.05f;
    
    *v_cap += dv;
    *i_ind += di;
    
    // Physical voltage limits of Germanium tunnel diode junction
    if (*v_cap > 0.8f) *v_cap = 0.8f;
    if (*v_cap < -0.1f) *v_cap = -0.1f;
    
    return *v_cap;
}
