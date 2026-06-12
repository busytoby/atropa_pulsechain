#ifndef TSFI_CLA_H
#define TSFI_CLA_H

#include <stdint.h>

typedef struct {
    // Gate propagation delays in nanoseconds
    float t_pd_and;
    float t_pd_or;
    float t_pd_xor;
} TsfiAdderParams;

// Simulates a 4-bit Ripple Carry Adder.
// Returns the worst-case settling time (nanoseconds) for the output bits to stabilize.
float tsfi_ripple_carry_adder(uint8_t a, uint8_t b, uint8_t c_in, uint8_t *sum, uint8_t *c_out, const TsfiAdderParams *params);

// Simulates a 4-bit Carry Look-Ahead Adder.
// Returns the worst-case settling time (nanoseconds) for the output bits to stabilize.
float tsfi_carry_lookahead_adder(uint8_t a, uint8_t b, uint8_t c_in, uint8_t *sum, uint8_t *c_out, const TsfiAdderParams *params);

#endif /* TSFI_CLA_H */
