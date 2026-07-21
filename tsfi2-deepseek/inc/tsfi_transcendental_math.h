#ifndef TSFI_TRANSCENDENTAL_MATH_H
#define TSFI_TRANSCENDENTAL_MATH_H

#include <stdint.h>
#include <stddef.h>

#define TSFI_PI 3.14159265358979323846
#define TSFI_E  2.71828182845904523536
#define TSFI_LIOUVILLE 0.110001000000000000000001

typedef struct {
    double exp_decay_result;     // Euler e-based RC Discharge Voltage V(t)
    double phase_sin_result;     // Pi-based Lissajous Sine Phase Wave
    uint64_t liouville_prng_seed; // Liouville-based Non-Repeating PRNG Seed
} tsfi_transcendental_eval_t;

/* Calculate Euler e-based exponential decay voltage for FET discharge physics */
double tsfi_transcendental_exp_decay(double v0, double time_sec, double rc_tau);

/* Evaluate Pi-based phase shift angle for Logan Synthesizer & Lissajous projection */
double tsfi_transcendental_phase_sin(double freq_hz, double time_sec, double phase_offset_rad);

/* Evaluate full transcendental profile for Auncient VM hardware state transitions */
int tsfi_transcendental_profile(
    double v0,
    double freq_hz,
    double time_sec,
    tsfi_transcendental_eval_t *out_eval
);

#endif // TSFI_TRANSCENDENTAL_MATH_H
