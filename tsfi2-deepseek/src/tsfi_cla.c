#include "tsfi_cla.h"
#include <string.h>

static float max_f2(float a, float b) {
    return (a > b) ? a : b;
}

static float max_f3(float a, float b, float c) {
    float m = (a > b) ? a : b;
    return (c > m) ? c : m;
}

static float max_f4(float a, float b, float c, float d) {
    float m1 = (a > b) ? a : b;
    float m2 = (c > d) ? c : d;
    return (m1 > m2) ? m1 : m2;
}

static float max_f5(float a, float b, float c, float d, float e) {
    float m1 = max_f4(a, b, c, d);
    return (e > m1) ? e : m1;
}

float tsfi_ripple_carry_adder(uint8_t a, uint8_t b, uint8_t c_in, uint8_t *sum, uint8_t *c_out, const TsfiAdderParams *params) {
    float t_a[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    float t_b[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    float t_c[5] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
    float t_s[4] = {0.0f, 0.0f, 0.0f, 0.0f};

    t_c[0] = 0.0f; // Carry-in is stable at t=0

    uint8_t s_val = 0;
    uint8_t carry = c_in & 1;

    for (int i = 0; i < 4; i++) {
        uint8_t ai = (a >> i) & 1;
        uint8_t bi = (b >> i) & 1;

        // Gate logic values
        uint8_t pi = ai ^ bi;
        uint8_t gi = ai & bi;
        uint8_t si = pi ^ carry;
        uint8_t ti = pi & carry;
        uint8_t next_carry = gi | ti;

        s_val |= (si << i);

        // Compute signal stabilization times
        float t_pi = max_f2(t_a[i], t_b[i]) + params->t_pd_xor;
        float t_gi = max_f2(t_a[i], t_b[i]) + params->t_pd_and;
        
        t_s[i] = max_f2(t_pi, t_c[i]) + params->t_pd_xor;
        
        float t_ti = max_f2(t_pi, t_c[i]) + params->t_pd_and;
        t_c[i+1] = max_f2(t_gi, t_ti) + params->t_pd_or;

        carry = next_carry;
    }

    *sum = s_val;
    *c_out = carry;

    // The settling time of the entire adder is the max stabilization time of all outputs
    float worst_t = t_c[4];
    for (int i = 0; i < 4; i++) {
        if (t_s[i] > worst_t) worst_t = t_s[i];
    }
    return worst_t;
}

float tsfi_carry_lookahead_adder(uint8_t a, uint8_t b, uint8_t c_in, uint8_t *sum, uint8_t *c_out, const TsfiAdderParams *params) {
    float t_a[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    float t_b[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    float t_c[5] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
    float t_s[4] = {0.0f, 0.0f, 0.0f, 0.0f};

    t_c[0] = 0.0f; // Carry-in is stable at t=0

    // 1. Calculate Generate and Propagate terms
    uint8_t g[4], p[4];
    float t_g[4], t_p[4];
    for (int i = 0; i < 4; i++) {
        uint8_t ai = (a >> i) & 1;
        uint8_t bi = (b >> i) & 1;
        g[i] = ai & bi;
        p[i] = ai ^ bi;

        t_g[i] = max_f2(t_a[i], t_b[i]) + params->t_pd_and;
        t_p[i] = max_f2(t_a[i], t_b[i]) + params->t_pd_xor;
    }

    // 2. Parallel carry generation logic (CLA equations)
    // C1 = G0 + P0*C0
    uint8_t c1 = g[0] | (p[0] & c_in);
    float t_p0_c0 = max_f2(t_p[0], t_c[0]) + params->t_pd_and;
    t_c[1] = max_f2(t_g[0], t_p0_c0) + params->t_pd_or;

    // C2 = G1 + P1*G0 + P1*P0*C0
    uint8_t c2 = g[1] | (p[1] & g[0]) | (p[1] & p[0] & c_in);
    float t_p1_g0 = max_f2(t_p[1], t_g[0]) + params->t_pd_and;
    float t_p1_p0_c0 = max_f3(t_p[1], t_p[0], t_c[0]) + params->t_pd_and;
    t_c[2] = max_f3(t_g[1], t_p1_g0, t_p1_p0_c0) + params->t_pd_or;

    // C3 = G2 + P2*G1 + P2*P1*G0 + P2*P1*P0*C0
    uint8_t c3 = g[2] | (p[2] & g[1]) | (p[2] & p[1] & g[0]) | (p[2] & p[1] & p[0] & c_in);
    float t_p2_g1 = max_f2(t_p[2], t_g[1]) + params->t_pd_and;
    float t_p2_p1_g0 = max_f3(t_p[2], t_p[1], t_g[0]) + params->t_pd_and;
    float t_p2_p1_p0_c0 = max_f4(t_p[2], t_p[1], t_p[0], t_c[0]) + params->t_pd_and;
    t_c[3] = max_f4(t_g[2], t_p2_g1, t_p2_p1_g0, t_p2_p1_p0_c0) + params->t_pd_or;

    // C4 = G3 + P3*G2 + P3*P2*G1 + P3*P2*P1*G0 + P3*P2*P1*P0*C0
    uint8_t c4 = g[3] | (p[3] & g[2]) | (p[3] & p[2] & g[1]) | (p[3] & p[2] & p[1] & g[0]) | (p[3] & p[2] & p[1] & p[0] & c_in);
    float t_p3_g2 = max_f2(t_p[3], t_g[2]) + params->t_pd_and;
    float t_p3_p2_g1 = max_f3(t_p[3], t_p[2], t_g[1]) + params->t_pd_and;
    float t_p3_p2_p1_g0 = max_f4(t_p[3], t_p[2], t_p[1], t_g[0]) + params->t_pd_and;
    float t_p3_p2_p1_p0_c0 = max_f5(t_p[3], t_p[2], t_p[1], t_p[0], t_c[0]) + params->t_pd_and;
    t_c[4] = max_f5(t_g[3], t_p3_g2, t_p3_p2_g1, t_p3_p2_p1_g0, t_p3_p2_p1_p0_c0) + params->t_pd_or;

    // 3. Compute sums
    uint8_t s_val = 0;
    for (int i = 0; i < 4; i++) {
        uint8_t ci = (i == 0) ? c_in : ((i == 1) ? c1 : ((i == 2) ? c2 : c3));
        uint8_t si = p[i] ^ ci;
        s_val |= (si << i);
        t_s[i] = max_f2(t_p[i], t_c[i]) + params->t_pd_xor;
    }

    *sum = s_val;
    *c_out = c4;

    float worst_t = t_c[4];
    for (int i = 0; i < 4; i++) {
        if (t_s[i] > worst_t) worst_t = t_s[i];
    }
    return worst_t;
}
